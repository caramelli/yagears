/*
  yagears                  Yet Another Gears OpenGL demo
  Copyright (C) 2013-2017  Nicolas Caramelli

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include GLESV2_H
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "engine.h"

#include "image_loader.h"

extern struct list engine_list;

static void identity(GLfloat *a)
{
  GLfloat m[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
  };

  memcpy(a, m, sizeof(m));
}

static void multiply(GLfloat *a, const GLfloat *b)
{
  GLfloat m[16];
  GLint i, j;
  div_t d;

  for (i = 0; i < 16; i++) {
    m[i] = 0;
    d = div(i, 4);
    for (j = 0; j < 4; j++)
      m[i] += (a + d.rem)[j * 4] * (b + d.quot * 4)[j];
  }

  memcpy(a, m, sizeof(m));
}

static void translate(GLfloat *a, GLfloat tx, GLfloat ty, GLfloat tz)
{
  GLfloat m[16] = {
     1,  0,  0, 0,
     0,  1,  0, 0,
     0,  0,  1, 0,
    tx, ty, tz, 1
  };

  multiply(a, m);
}

static void rotate(GLfloat *a, GLfloat r, GLfloat ux, GLfloat uy, GLfloat uz)
{
  GLfloat s, c;

  sincosf(r * M_PI / 180, &s, &c);

  GLfloat m[16] = {
         ux * ux * (1 - c) + c, uy * ux * (1 - c) + uz * s, ux * uz * (1 - c) - uy * s, 0,
    ux * uy * (1 - c) - uz * s,      uy * uy * (1 - c) + c, uy * uz * (1 - c) + ux * s, 0,
    ux * uz * (1 - c) + uy * s, uy * uz * (1 - c) - ux * s,      uz * uz * (1 - c) + c, 0,
                             0,                          0,                          0, 1
  };

  multiply(a, m);
}

static void transpose(GLfloat *a)
{
  GLfloat m[16] = {
    a[0], a[4], a[8],  a[12],
    a[1], a[5], a[9],  a[13],
    a[2], a[6], a[10], a[14],
    a[3], a[7], a[11], a[15]
  };

  memcpy(a, m, sizeof(m));
}

static void invert(GLfloat *a)
{
  GLfloat m[16] = {
         1,      0,      0, 0,
         0,      1,      0, 0,
         0,      0,      1, 0,
    -a[12], -a[13], -a[14], 1,
  };

  a[12] = a[13] = a[14] = 0;
  transpose(a);

  multiply(a, m);
}

/******************************************************************************/

typedef GLfloat Vertex[8];

typedef struct {
  GLint begin;
  GLsizei count;
} Strip;

struct gear {
  GLint nvertices;
  Vertex *vertices;
  GLint nstrips;
  Strip *strips;
  GLuint vbo;
};

struct gears {
  GLuint program;
  GLuint vertShader;
  GLuint fragShader;
  image_t image;
  struct gear *gear1;
  struct gear *gear2;
  struct gear *gear3;
  GLfloat Projection[16];
};

static struct gear *create_gear(GLfloat inner, GLfloat outer, GLfloat width, GLint teeth, GLfloat tooth_depth)
{
  struct gear *gear;
  GLfloat r0, r1, r2, da, a1, ai, s[5], c[5];
  GLint i, j;
  GLfloat n[3], t[2];
  GLint k = 0;

  gear = calloc(1, sizeof(struct gear));
  if (!gear) {
    printf("calloc gear failed\n");
    return NULL;
  }

  gear->nvertices = 0;
  gear->vertices = calloc(34 * teeth, sizeof(Vertex));
  if (!gear->vertices) {
    printf("calloc vertices failed\n");
    goto out;
  }

  gear->nstrips = 7 * teeth;
  gear->strips = calloc(gear->nstrips, sizeof(Strip));
  if (!gear->strips) {
    printf("calloc strips failed\n");
    goto out;
  }

  r0 = inner;
  r1 = outer - tooth_depth / 2;
  r2 = outer + tooth_depth / 2;
  a1 = 2 * M_PI / teeth;
  da = a1 / 4;

  #define normal(nx, ny, nz) \
    n[0] = nx; \
    n[1] = ny; \
    n[2] = nz;

  #define texcoord(tx, ty) \
    t[0] = tx; \
    t[1] = ty;

  #define vertex(x, y, z) \
    gear->vertices[gear->nvertices][0] = x; \
    gear->vertices[gear->nvertices][1] = y; \
    gear->vertices[gear->nvertices][2] = z; \
    gear->vertices[gear->nvertices][3] = n[0]; \
    gear->vertices[gear->nvertices][4] = n[1]; \
    gear->vertices[gear->nvertices][5] = n[2]; \
    gear->vertices[gear->nvertices][6] = t[0]; \
    gear->vertices[gear->nvertices][7] = t[1]; \
    gear->nvertices++;

  for (i = 0; i < teeth; i++) {
    ai = i * a1;
    for (j = 0; j < 5; j++) {
      sincosf(ai + j * da, &s[j], &c[j]);
    }

    /* front face begin */
    gear->strips[k].begin = gear->nvertices;
    /* front face normal */
    normal(0, 0, 1);
    /* front face vertices */
    texcoord(0.36 * r2 * s[1] / r1 + 0.5, 0.36 * r2 * c[1] / r1 + 0.5);
    vertex(r2 * c[1], r2 * s[1], width / 2);
    texcoord(0.36 * r2 * s[2] / r1 + 0.5, 0.36 * r2 * c[2] / r1 + 0.5);
    vertex(r2 * c[2], r2 * s[2], width / 2);
    texcoord(0.36 * r1 * s[0] / r1 + 0.5, 0.36 * r1 * c[0] / r1 + 0.5);
    vertex(r1 * c[0], r1 * s[0], width / 2);
    texcoord(0.36 * r1 * s[3] / r1 + 0.5, 0.36 * r1 * c[3] / r1 + 0.5);
    vertex(r1 * c[3], r1 * s[3], width / 2);
    texcoord(0.36 * r0 * s[0] / r1 + 0.5, 0.36 * r0 * c[0] / r1 + 0.5);
    vertex(r0 * c[0], r0 * s[0], width / 2);
    texcoord(0.36 * r1 * s[4] / r1 + 0.5, 0.36 * r1 * c[4] / r1 + 0.5);
    vertex(r1 * c[4], r1 * s[4], width / 2);
    texcoord(0.36 * r0 * s[4] / r1 + 0.5, 0.36 * r0 * c[4] / r1 + 0.5);
    vertex(r0 * c[4], r0 * s[4], width / 2);
    texcoord(0, 0);
    /* front face end */
    gear->strips[k].count = 7;
    k++;

    /* back face begin */
    gear->strips[k].begin = gear->nvertices;
    /* back face normal */
    normal(0, 0, -1);
    /* back face vertices */
    vertex(r2 * c[1], r2 * s[1], -width / 2);
    vertex(r2 * c[2], r2 * s[2], -width / 2);
    vertex(r1 * c[0], r1 * s[0], -width / 2);
    vertex(r1 * c[3], r1 * s[3], -width / 2);
    vertex(r0 * c[0], r0 * s[0], -width / 2);
    vertex(r1 * c[4], r1 * s[4], -width / 2);
    vertex(r0 * c[4], r0 * s[4], -width / 2);
    /* back face end */
    gear->strips[k].count = 7;
    k++;

    /* first outward face begin */
    gear->strips[k].begin = gear->nvertices;
    /* first outward face normal */
    normal(r2 * s[1] - r1 * s[0], r1 * c[0] - r2 * c[1], 0);
    /* first outward face vertices */
    vertex(r1 * c[0], r1 * s[0],  width / 2);
    vertex(r1 * c[0], r1 * s[0], -width / 2);
    vertex(r2 * c[1], r2 * s[1],  width / 2);
    vertex(r2 * c[1], r2 * s[1], -width / 2);
    /* first outward face end */
    gear->strips[k].count = 4;
    k++;

    /* second outward face begin */
    gear->strips[k].begin = gear->nvertices;
    /* second outward face normal */
    normal(s[2] - s[1], c[1] - c[2], 0);
    /* second outward face vertices */
    vertex(r2 * c[1], r2 * s[1],  width / 2);
    vertex(r2 * c[1], r2 * s[1], -width / 2);
    vertex(r2 * c[2], r2 * s[2],  width / 2);
    vertex(r2 * c[2], r2 * s[2], -width / 2);
    /* second outward face end */
    gear->strips[k].count = 4;
    k++;

    /* third outward face begin */
    gear->strips[k].begin = gear->nvertices;
    /* third outward face normal */
    normal(r1 * s[3] - r2 * s[2], r2 * c[2] - r1 * c[3], 0);
    /* third outward face vertices */
    vertex(r2 * c[2], r2 * s[2],  width / 2);
    vertex(r2 * c[2], r2 * s[2], -width / 2);
    vertex(r1 * c[3], r1 * s[3],  width / 2);
    vertex(r1 * c[3], r1 * s[3], -width / 2);
    /* third outward face end */
    gear->strips[k].count = 4;
    k++;

    /* fourth outward face begin */
    gear->strips[k].begin = gear->nvertices;
    /* fourth outward face normal */
    normal(s[4] - s[3], c[3] - c[4], 0);
    /* fourth outward face vertices */
    vertex(r1 * c[3], r1 * s[3],  width / 2);
    vertex(r1 * c[3], r1 * s[3], -width / 2);
    vertex(r1 * c[4], r1 * s[4],  width / 2);
    vertex(r1 * c[4], r1 * s[4], -width / 2);
    /* fourth outward face end */
    gear->strips[k].count = 4;
    k++;

    /* inside face begin */
    gear->strips[k].begin = gear->nvertices;
    /* inside face normal */
    normal(s[0] - s[4], c[4] - c[0], 0);
    /* inside face vertices */
    vertex(r0 * c[0], r0 * s[0],  width / 2);
    vertex(r0 * c[0], r0 * s[0], -width / 2);
    vertex(r0 * c[4], r0 * s[4],  width / 2);
    vertex(r0 * c[4], r0 * s[4], -width / 2);
    /* inside face end */
    gear->strips[k].count = 4;
    k++;
  }

  glGenBuffers(1, &gear->vbo);
  if (!gear->vbo) {
    printf("glGenBuffers failed\n");
    goto out;
  }

  glBindBuffer(GL_ARRAY_BUFFER, gear->vbo);

  glBufferData(GL_ARRAY_BUFFER, gear->nvertices * sizeof(Vertex), gear->vertices, GL_STATIC_DRAW);

  return gear;

out:
  if (gear->vbo) {
    glDeleteBuffers(1, &gear->vbo);
  }
  if (gear->strips) {
    free(gear->strips);
  }
  if (gear->vertices) {
    free(gear->vertices);
  }
  free(gear);
  return NULL;
}

static void draw_gear(struct gear *gear, GLfloat model_tx, GLfloat model_ty, GLfloat model_rz, const GLfloat *color, GLfloat *View, GLfloat *Projection, GLuint program)
{
  GLfloat ModelView[16], ModelViewProjection[16];
  GLint ModelViewProjection_loc;
  GLint Normal_loc;
  GLint Color_loc;
  GLint k;

  memcpy(ModelView, View, sizeof(ModelView));

  translate(ModelView, model_tx, model_ty, 0);
  rotate(ModelView, model_rz, 0, 0, 1);

  /* Set u_ModelViewProjectionMatrix */
  memcpy(ModelViewProjection, Projection, sizeof(ModelViewProjection));
  multiply(ModelViewProjection, ModelView);
  ModelViewProjection_loc = glGetUniformLocation(program, "u_ModelViewProjectionMatrix");
  glUniformMatrix4fv(ModelViewProjection_loc, 1, GL_FALSE, ModelViewProjection);

  /* Set u_NormalMatrix */
  invert(ModelView);
  transpose(ModelView);
  Normal_loc = glGetUniformLocation(program, "u_NormalMatrix");
  glUniformMatrix4fv(Normal_loc, 1, GL_FALSE, ModelView);

  /* Set u_Color */
  Color_loc = glGetUniformLocation(program, "u_Color");
  glUniform4fv(Color_loc, 1, color);

  glBindBuffer(GL_ARRAY_BUFFER, gear->vbo);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLfloat *)NULL + 3);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLfloat *)NULL + 6);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  for (k = 0; k < gear->nstrips; k++) {
    glDrawArrays(GL_TRIANGLE_STRIP, gear->strips[k].begin, gear->strips[k].count);
  }

  glDisableVertexAttribArray(2);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(0);
}

static void delete_gear(struct gear *gear)
{
  glDeleteBuffers(1, &gear->vbo);
  free(gear->strips);
  free(gear->vertices);

  free(gear);
}

/******************************************************************************/

static gears_t *glesv2_gears_init(int win_width, int win_height)
{
  gears_t *gears = NULL;
  const char *vertShaderSource =
    "attribute vec3 a_Vertex;\n"
    "attribute vec3 a_Normal;\n"
    "attribute vec2 a_TexCoord;\n"
    "uniform vec4 u_LightPos;\n"
    "uniform mat4 u_ModelViewProjectionMatrix;\n"
    "uniform mat4 u_NormalMatrix;\n"
    "uniform vec4 u_Color;\n"
    "varying vec4 v_Color;\n"
    "varying vec2 v_TexCoord;\n"
    "void main(void)\n"
    "{\n"
    "  gl_Position = u_ModelViewProjectionMatrix * vec4(a_Vertex, 1);\n"
    "  v_Color = u_Color * vec4(0.2, 0.2, 0.2, 1) + u_Color * dot(normalize(u_LightPos.xyz), normalize(vec3(u_NormalMatrix * vec4(a_Normal, 1))));\n"
    "  v_TexCoord = a_TexCoord;\n"
    "}";
  const char *fragShaderSource =
    "precision mediump float;\n"
    "uniform sampler2D u_Texture;\n"
    "varying vec4 v_Color;\n"
    "varying vec2 v_TexCoord;\n"
    "void main()\n"
    "{\n"
    "  vec4 t = texture2D(u_Texture, v_TexCoord);\n"
    "  if (t.a == 0.0)\n"
    "    gl_FragColor = v_Color;\n"
    "  else\n"
    "    gl_FragColor = t;\n"
    "}\n";
  GLint params;
  GLchar *log;
  GLint LightPos_loc;
  const GLfloat pos[4] = { 5.0, 5.0, 10.0, 0.0 };
  GLint Texture_loc;
  const GLfloat zNear = 5, zFar = 60;

  gears = calloc(1, sizeof(gears_t));
  if (!gears) {
    printf("calloc gears failed\n");
    return NULL;
  }

  glEnable(GL_DEPTH_TEST);

  gears->program = glCreateProgram();
  if (!gears->program) {
    printf("glCreateProgram failed\n");
    goto out;
  }

  /* vertex shader */

  gears->vertShader = glCreateShader(GL_VERTEX_SHADER);
  if (!gears->vertShader) {
    printf("glCreateShader vertex failed\n");
    goto out;
  }

  glShaderSource(gears->vertShader, 1, &vertShaderSource, NULL);

  glCompileShader(gears->vertShader);
  glGetShaderiv(gears->vertShader, GL_COMPILE_STATUS, &params);
  if (!params) {
    glGetShaderiv(gears->vertShader, GL_INFO_LOG_LENGTH, &params);
    log = calloc(1, params);
    if (!log) {
      printf("calloc log failed\n");
      goto out;
    }
    glGetShaderInfoLog(gears->vertShader, params, NULL, log);
    printf("glCompileShader vertex failed: %s", log);
    free(log);
    goto out;
  }

  glAttachShader(gears->program, gears->vertShader);

  /* fragment shader */

  gears->fragShader = glCreateShader(GL_FRAGMENT_SHADER);
  if (!gears->fragShader) {
    printf("glCreateShader fragment failed\n");
    goto out;
  }

  if (strstr((char *)glGetString(GL_SHADING_LANGUAGE_VERSION), "1.20") ||
      strstr((char *)glGetString(GL_SHADING_LANGUAGE_VERSION), "1.30")) {
    fragShaderSource += strlen("precision mediump float;\n");
  }
  glShaderSource(gears->fragShader, 1, &fragShaderSource, NULL);

  glCompileShader(gears->fragShader);
  glGetShaderiv(gears->fragShader, GL_COMPILE_STATUS, &params);
  if (!params) {
    glGetShaderiv(gears->fragShader, GL_INFO_LOG_LENGTH, &params);
    log = calloc(1, params);
    if (!log) {
      printf("calloc log failed\n");
      goto out;
    }
    glGetShaderInfoLog(gears->fragShader, params, NULL, log);
    printf("glCompileShader fragment failed: %s", log);
    free(log);
    goto out;
  }

  glAttachShader(gears->program, gears->fragShader);

  /* link and use program */

  glBindAttribLocation(gears->program, 0, "a_Vertex");
  glBindAttribLocation(gears->program, 1, "a_Normal");
  glBindAttribLocation(gears->program, 2, "a_TexCoord");

  glLinkProgram(gears->program);
  glGetProgramiv(gears->program, GL_LINK_STATUS, &params);
  if (!params) {
    glGetProgramiv(gears->program, GL_INFO_LOG_LENGTH, &params);
    log = calloc(1, params);
    if (!log) {
      printf("calloc log failed\n");
      goto out;
    }
    glGetProgramInfoLog(gears->program, params, NULL, log);
    printf("glLinkProgram failed: %s", log);
    free(log);
    goto out;
  }

  glUseProgram(gears->program);

  /* Set u_LightPos */
  LightPos_loc = glGetUniformLocation(gears->program, "u_LightPos");
  glUniform4fv(LightPos_loc, 1, pos);

  /* Set u_Texture */
  Texture_loc = glGetUniformLocation(gears->program, "u_Texture");
  glUniform1i(Texture_loc, 0);

  image_load(getenv("TEXTURE"), &gears->image);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gears->image.width, gears->image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, gears->image.pixel_data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  gears->gear1 = create_gear(1.0, 4.0, 1.0, 20, 0.7);
  if (!gears->gear1) {
    goto out;
  }

  gears->gear2 = create_gear(0.5, 2.0, 2.0, 10, 0.7);
  if (!gears->gear2) {
    goto out;
  }

  gears->gear3 = create_gear(1.3, 2.0, 0.5, 10, 0.7);
  if (!gears->gear3) {
    goto out;
  }

  glViewport(0, 0, (GLint)win_width, (GLint)win_height);

  memset(gears->Projection, 0, sizeof(gears->Projection));
  gears->Projection[0] = zNear;
  gears->Projection[5] = (GLfloat)win_width/win_height * zNear;
  gears->Projection[10] = -(zFar + zNear) / (zFar - zNear);
  gears->Projection[11] = -1;
  gears->Projection[14] = -2 * zFar * zNear / (zFar - zNear);

  return gears;

out:
  if (gears->gear3) {
    delete_gear(gears->gear3);
  }
  if (gears->gear2) {
    delete_gear(gears->gear2);
  }
  if (gears->gear1) {
    delete_gear(gears->gear1);
  }
  if (gears->image.pixel_data) {
    image_unload(&gears->image);
  }
  if (gears->fragShader) {
    glDeleteShader(gears->fragShader);
  }
  if (gears->vertShader) {
    glDeleteShader(gears->vertShader);
  }
  if (gears->program) {
    glDeleteProgram(gears->program);
  }
  free(gears);
  return NULL;
}

static void glesv2_gears_draw(gears_t *gears, float view_tz, float view_rx, float view_ry, float model_rz)
{
  const GLfloat red[4] = { 0.8, 0.1, 0.0, 1.0 };
  const GLfloat green[4] = { 0.0, 0.8, 0.2, 1.0 };
  const GLfloat blue[4] = { 0.2, 0.2, 1.0, 1.0 };
  GLfloat View[16];

  if (!gears) {
    return;
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  identity(View);
  translate(View, 0, 0, (GLfloat)view_tz);
  rotate(View, (GLfloat)view_rx, 1, 0, 0);
  rotate(View, (GLfloat)view_ry, 0, 1, 0);

  draw_gear(gears->gear1, -3.0, -2.0,      (GLfloat)model_rz     , red  , View, gears->Projection, gears->program);
  draw_gear(gears->gear2,  3.1, -2.0, -2 * (GLfloat)model_rz - 9 , green, View, gears->Projection, gears->program);
  draw_gear(gears->gear3, -3.1,  4.2, -2 * (GLfloat)model_rz - 25, blue , View, gears->Projection, gears->program);
}

static void glesv2_gears_term(gears_t *gears)
{
  if (!gears) {
    return;
  }

  delete_gear(gears->gear1);
  delete_gear(gears->gear2);
  delete_gear(gears->gear3);
  image_unload(&gears->image);
  glDeleteShader(gears->fragShader);
  glDeleteShader(gears->vertShader);
  glDeleteProgram(gears->program);

  printf("%s\n", glGetString(GL_VERSION));
  printf("%s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
}

/******************************************************************************/

static engine_t glesv2_engine = {
  "glesv2",
  2,
  glesv2_gears_init,
  glesv2_gears_draw,
  glesv2_gears_term
};

static void __attribute__((constructor)) engine_ctor()
{
  list_add(&glesv2_engine.entry, &engine_list);
}
