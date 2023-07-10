/*
  yagears                  Yet Another Gears OpenGL / Vulkan demo
  Copyright (C) 2013-2023  Nicolas Caramelli

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
#include <dlfcn.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "engine.h"

#include "image_loader.h"

extern struct list engine_list;

static void identity(float *a)
{
  float m[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
  };

  memcpy(a, m, sizeof(m));
}

static void multiply(float *a, const float *b)
{
  float m[16];
  int i, j;
  div_t d;

  for (i = 0; i < 16; i++) {
    m[i] = 0;
    d = div(i, 4);
    for (j = 0; j < 4; j++)
      m[i] += (a + d.rem)[j * 4] * (b + d.quot * 4)[j];
  }

  memcpy(a, m, sizeof(m));
}

static void translate(float *a, float tx, float ty, float tz)
{
  float m[16] = {
     1,  0,  0, 0,
     0,  1,  0, 0,
     0,  0,  1, 0,
    tx, ty, tz, 1
  };

  multiply(a, m);
}

static void rotate(float *a, float r, float ux, float uy, float uz)
{
  float s, c;

  sincosf(r * M_PI / 180, &s, &c);

  float m[16] = {
         ux * ux * (1 - c) + c, uy * ux * (1 - c) + uz * s, ux * uz * (1 - c) - uy * s, 0,
    ux * uy * (1 - c) - uz * s,      uy * uy * (1 - c) + c, uy * uz * (1 - c) + ux * s, 0,
    ux * uz * (1 - c) + uy * s, uy * uz * (1 - c) - ux * s,      uz * uz * (1 - c) + c, 0,
                             0,                          0,                          0, 1
  };

  multiply(a, m);
}

static void transpose(float *a)
{
  float m[16] = {
    a[0], a[4], a[8],  a[12],
    a[1], a[5], a[9],  a[13],
    a[2], a[6], a[10], a[14],
    a[3], a[7], a[11], a[15]
  };

  memcpy(a, m, sizeof(m));
}

static void invert(float *a)
{
  float m[16] = {
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

#define GEAR0 0
#define GEAR1 1
#define GEAR2 2

typedef float Vertex[8];

typedef struct {
  int begin;
  int count;
} Strip;

struct gear {
  int nvertices;
  Vertex *vertices;
  int nstrips;
  Strip *strips;
  GLuint vbo;
};

struct gears {
  void *lib_handle;
  void           (*glAttachShader)(GLuint, GLuint);
  void           (*glBindAttribLocation)(GLuint, GLuint, const GLchar *);
  void           (*glBindBuffer)(GLenum, GLuint);
  void           (*glBufferData)(GLenum, GLsizeiptr, const GLvoid *, GLenum);
  void           (*glClear)(GLbitfield);
  void           (*glClearColor)(GLfloat, GLfloat, GLfloat, GLfloat);
  void           (*glCompileShader)(GLuint);
  GLuint         (*glCreateProgram)();
  GLuint         (*glCreateShader)(GLenum);
  void           (*glDeleteBuffers)(GLsizei, const GLuint *);
  void           (*glDeleteProgram)(GLuint);
  void           (*glDeleteShader)(GLuint);
  void           (*glDisableVertexAttribArray)(GLuint);
  void           (*glDrawArrays)(GLenum, GLint, GLsizei);
  void           (*glEnable)(GLenum);
  void           (*glEnableVertexAttribArray)(GLuint);
  void           (*glGenBuffers)(GLsizei, GLuint *);
  GLenum         (*glGetError)();
  void           (*glGetProgramInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *);
  void           (*glGetProgramiv)(GLuint, GLenum, GLint *);
  void           (*glGetShaderInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *);
  void           (*glGetShaderiv)(GLuint, GLenum, GLint *);
  const GLubyte *(*glGetString)(GLenum);
  int            (*glGetUniformLocation)(GLuint, const GLchar *);
  void           (*glLinkProgram)(GLuint);
  void           (*glShaderSource)(GLuint, GLsizei, const GLchar **, const GLint *);
  void           (*glUniform1i)(GLint, GLint);
  void           (*glUniform4fv)(GLint, GLsizei, const GLfloat *);
  void           (*glUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat *);
  void           (*glUseProgram)(GLuint);
  void           (*glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
  void           (*glTexParameteri)(GLenum, GLenum, GLint);
  void           (*glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);
  void           (*glViewport)(GLint, GLint, GLsizei, GLsizei);
  GLuint program;
  struct gear *gear[3];
  float Projection[16];
  float View[16];
};

static void delete_gear(gears_t *gears, int id)
{
  struct gear *gear = gears->gear[id];

  if (!gear) {
    return;
  }

  if (gear->vbo) {
    gears->glDeleteBuffers(1, &gear->vbo);
  }
  if (gear->strips) {
    free(gear->strips);
  }
  if (gear->vertices) {
    free(gear->vertices);
  }

  free(gear);

  gears->gear[id] = NULL;
}

static int create_gear(gears_t *gears, int id, float inner, float outer, float width, int teeth, float tooth_depth)
{
  struct gear *gear;
  float r0, r1, r2, da, a1, ai, s[5], c[5];
  int i, j;
  float n[3], t[2];
  int k = 0;
  GLenum err = GL_NO_ERROR;

  gear = calloc(1, sizeof(struct gear));
  if (!gear) {
    printf("calloc gear failed\n");
    return -1;
  }

  gears->gear[id] = gear;

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

  /* vertex buffer object */

  gears->glGenBuffers(1, &gear->vbo);
  if (!gear->vbo) {
    printf("glGenBuffers failed\n");
    goto out;
  }

  gears->glBindBuffer(GL_ARRAY_BUFFER, gear->vbo);
  err = gears->glGetError();
  if (err) {
    printf("glBindBuffer failed: 0x%x\n", (unsigned int)err);
    goto out;
  }

  gears->glBufferData(GL_ARRAY_BUFFER, gear->nvertices * sizeof(Vertex), gear->vertices, GL_STATIC_DRAW);
  err = gears->glGetError();
  if (err) {
    printf("glBufferData failed: 0x%x\n", (unsigned int)err);
    goto out;
  }

  return 0;

out:
  delete_gear(gears, id);
  return -1;
}

static void draw_gear(gears_t *gears, int id, float model_tx, float model_ty, float model_rz, const float *color)
{
  struct gear *gear = gears->gear[id];
  const float pos[4] = { 5.0, 5.0, 10.0, 0.0 };
  float ModelView[16], ModelViewProjection[16];
  GLint LightPos_loc, ModelViewProjection_loc, Normal_loc, Color_loc, TextureEnable_loc;
  int k;

  if (!gear) {
    return;
  }

  LightPos_loc = gears->glGetUniformLocation(gears->program, "u_LightPos");
  gears->glUniform4fv(LightPos_loc, 1, pos);

  memcpy(ModelView, gears->View, sizeof(ModelView));

  translate(ModelView, model_tx, model_ty, 0);
  rotate(ModelView, model_rz, 0, 0, 1);

  memcpy(ModelViewProjection, gears->Projection, sizeof(ModelViewProjection));
  multiply(ModelViewProjection, ModelView);
  ModelViewProjection_loc = gears->glGetUniformLocation(gears->program, "u_ModelViewProjectionMatrix");
  gears->glUniformMatrix4fv(ModelViewProjection_loc, 1, GL_FALSE, ModelViewProjection);

  invert(ModelView);
  transpose(ModelView);
  Normal_loc = gears->glGetUniformLocation(gears->program, "u_NormalMatrix");
  gears->glUniformMatrix4fv(Normal_loc, 1, GL_FALSE, ModelView);

  Color_loc = gears->glGetUniformLocation(gears->program, "u_Color");
  gears->glUniform4fv(Color_loc, 1, color);

  TextureEnable_loc = gears->glGetUniformLocation(gears->program, "u_TextureEnable");
  if (getenv("NO_TEXTURE"))
    gears->glUniform1i(TextureEnable_loc, 0);
  else
    gears->glUniform1i(TextureEnable_loc, 1);

  gears->glBindBuffer(GL_ARRAY_BUFFER, gear->vbo);

  gears->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
  gears->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const float *)NULL + 3);
  gears->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const float *)NULL + 6);

  gears->glEnableVertexAttribArray(0);
  gears->glEnableVertexAttribArray(1);
  gears->glEnableVertexAttribArray(2);

  for (k = 0; k < gear->nstrips; k++) {
    gears->glDrawArrays(GL_TRIANGLE_STRIP, gear->strips[k].begin, gear->strips[k].count);
  }

  gears->glDisableVertexAttribArray(2);
  gears->glDisableVertexAttribArray(1);
  gears->glDisableVertexAttribArray(0);
}

/******************************************************************************/

static void glesv2_gears_term(gears_t *gears)
{
  if (!gears) {
    return;
  }

  if (gears->gear[GEAR2]) {
    delete_gear(gears, GEAR2);
  }
  if (gears->gear[GEAR1]) {
    delete_gear(gears, GEAR1);
  }
  if (gears->gear[GEAR0]) {
    delete_gear(gears, GEAR0);
  }
  if (gears->program) {
    gears->glDeleteProgram(gears->program);
  }

  printf("%s\n", gears->glGetString(GL_VERSION));
  printf("%s\n", gears->glGetString(GL_SHADING_LANGUAGE_VERSION));

  if (gears->lib_handle) {
    dlclose(gears->lib_handle);
  }

  free(gears);
}

static gears_t *glesv2_gears_init(int win_width, int win_height)
{
  gears_t *gears = NULL;
  const char vertShaderSource[] = {
    #include "vert.xxd"
  };
  const char fragShaderSource[] = {
    #include "frag.xxd"
  };
  const GLchar *code;
  GLint params;
  GLchar *log;
  GLuint vertShader = 0;
  GLuint fragShader = 0;
  int texture_width, texture_height;
  void *texture_data = NULL;
  const float zNear = 5, zFar = 60;

  gears = calloc(1, sizeof(gears_t));
  if (!gears) {
    printf("calloc gears failed\n");
    return NULL;
  }

  gears->lib_handle = dlopen(GLESV2_LIB, RTLD_LAZY);
  if (!gears->lib_handle) {
    printf("%s library not found\n", GLESV2_LIB);
    goto out;
  }

  #define DLSYM(sym) gears->sym = dlsym(gears->lib_handle, #sym); \
  if (!gears->sym) { \
    printf("%s not found\n", #sym); \
    goto out; \
  }

  DLSYM(glAttachShader);
  DLSYM(glBindAttribLocation);
  DLSYM(glBindBuffer);
  DLSYM(glBufferData);
  DLSYM(glClear);
  DLSYM(glClearColor);
  DLSYM(glCompileShader);
  DLSYM(glCreateProgram);
  DLSYM(glCreateShader);
  DLSYM(glDeleteBuffers);
  DLSYM(glDeleteShader);
  DLSYM(glDeleteProgram);
  DLSYM(glDisableVertexAttribArray);
  DLSYM(glDrawArrays);
  DLSYM(glEnable);
  DLSYM(glEnableVertexAttribArray);
  DLSYM(glGenBuffers);
  DLSYM(glGetError);
  DLSYM(glGetProgramInfoLog);
  DLSYM(glGetProgramiv);
  DLSYM(glGetShaderInfoLog);
  DLSYM(glGetShaderiv);
  DLSYM(glGetString);
  DLSYM(glGetUniformLocation);
  DLSYM(glLinkProgram);
  DLSYM(glShaderSource);
  DLSYM(glUniform1i);
  DLSYM(glUniform4fv);
  DLSYM(glUniformMatrix4fv);
  DLSYM(glUseProgram);
  DLSYM(glTexImage2D);
  DLSYM(glTexParameteri);
  DLSYM(glVertexAttribPointer);
  DLSYM(glViewport);

  gears->glEnable(GL_DEPTH_TEST);

  gears->program = gears->glCreateProgram();
  if (!gears->program) {
    printf("glCreateProgram failed\n");
    goto out;
  }

  /* vertex shader */

  vertShader = gears->glCreateShader(GL_VERTEX_SHADER);
  if (!vertShader) {
    printf("glCreateShader vertex failed\n");
    goto out;
  }

  code = vertShaderSource;
  gears->glShaderSource(vertShader, 1, &code, NULL);

  gears->glCompileShader(vertShader);
  gears->glGetShaderiv(vertShader, GL_COMPILE_STATUS, &params);
  if (!params) {
    gears->glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &params);
    log = calloc(1, params);
    if (!log) {
      printf("calloc log failed\n");
      goto out;
    }
    gears->glGetShaderInfoLog(vertShader, params, NULL, log);
    printf("glCompileShader vertex failed: %s", log);
    free(log);
    goto out;
  }

  gears->glAttachShader(gears->program, vertShader);

  /* fragment shader */

  fragShader = gears->glCreateShader(GL_FRAGMENT_SHADER);
  if (!fragShader) {
    printf("glCreateShader fragment failed\n");
    goto out;
  }

  code = fragShaderSource;
  if (strstr((char *)gears->glGetString(GL_SHADING_LANGUAGE_VERSION), "1.20") ||
      strstr((char *)gears->glGetString(GL_SHADING_LANGUAGE_VERSION), "1.30")) {
    code += strlen("precision mediump float;\n");
  }
  gears->glShaderSource(fragShader, 1, &code, NULL);

  gears->glCompileShader(fragShader);
  gears->glGetShaderiv(fragShader, GL_COMPILE_STATUS, &params);
  if (!params) {
    gears->glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &params);
    log = calloc(1, params);
    if (!log) {
      printf("calloc log failed\n");
      goto out;
    }
    gears->glGetShaderInfoLog(fragShader, params, NULL, log);
    printf("glCompileShader fragment failed: %s", log);
    free(log);
    goto out;
  }

  gears->glAttachShader(gears->program, fragShader);

  /* link and use program */

  gears->glBindAttribLocation(gears->program, 0, "a_Position");
  gears->glBindAttribLocation(gears->program, 1, "a_Normal");
  gears->glBindAttribLocation(gears->program, 2, "a_TexCoord");

  gears->glLinkProgram(gears->program);
  gears->glGetProgramiv(gears->program, GL_LINK_STATUS, &params);
  if (!params) {
    gears->glGetProgramiv(gears->program, GL_INFO_LOG_LENGTH, &params);
    log = calloc(1, params);
    if (!log) {
      printf("calloc log failed\n");
      goto out;
    }
    gears->glGetProgramInfoLog(gears->program, params, NULL, log);
    printf("glLinkProgram failed: %s", log);
    free(log);
    goto out;
  }

  /* destory shaders */

  gears->glDeleteShader(fragShader);
  gears->glDeleteShader(vertShader);
  vertShader = fragShader = 0;

  /* load texture */

  image_load(getenv("TEXTURE"), NULL, &texture_width, &texture_height);

  texture_data = malloc(texture_width * texture_height * 4);
  if (!texture_data) {
    printf("malloc texture_data failed\n");
    goto out;
  }

  image_load(getenv("TEXTURE"), texture_data, &texture_width, &texture_height);

  gears->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);

  free(texture_data);

  gears->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  gears->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  /* install program, set clear values, set viewport */

  gears->glUseProgram(gears->program);

  gears->glClearColor(0, 0, 0, 1);

  gears->glViewport(0, 0, win_width, win_height);

  /* create gears */

  if (create_gear(gears, GEAR0, 1.0, 4.0, 1.0, 20, 0.7)) {
    goto out;
  }

  if (create_gear(gears, GEAR1, 0.5, 2.0, 2.0, 10, 0.7)) {
    goto out;
  }

  if (create_gear(gears, GEAR2, 1.3, 2.0, 0.5, 10, 0.7)) {
    goto out;
  }

  memset(gears->Projection, 0, sizeof(gears->Projection));
  gears->Projection[0] = zNear;
  gears->Projection[5] = (float)win_width/win_height * zNear;
  gears->Projection[10] = -(zFar + zNear) / (zFar - zNear);
  gears->Projection[11] = -1;
  gears->Projection[14] = -2 * zFar * zNear / (zFar - zNear);

  return gears;

out:
  if (fragShader) {
    gears->glDeleteShader(fragShader);
  }
  if (vertShader) {
    gears->glDeleteShader(vertShader);
  }
  glesv2_gears_term(gears);
  return NULL;
}

static void glesv2_gears_draw(gears_t *gears, float view_tz, float view_rx, float view_ry, float model_rz)
{
  const float red[4] = { 0.8, 0.1, 0.0, 1.0 };
  const float green[4] = { 0.0, 0.8, 0.2, 1.0 };
  const float blue[4] = { 0.2, 0.2, 1.0, 1.0 };

  if (!gears) {
    return;
  }

  gears->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  identity(gears->View);
  translate(gears->View, 0, 0, view_tz);
  rotate(gears->View, view_rx, 1, 0, 0);
  rotate(gears->View, view_ry, 0, 1, 0);

  draw_gear(gears, GEAR0, -3.0, -2.0,      model_rz     , red);
  draw_gear(gears, GEAR1,  3.1, -2.0, -2 * model_rz - 9 , green);
  draw_gear(gears, GEAR2, -3.1,  4.2, -2 * model_rz - 25, blue);
}

/******************************************************************************/

static engine_t glesv2_engine = {
  "glesv2",
  2,
  glesv2_gears_init,
  glesv2_gears_draw,
  glesv2_gears_term
};

void
#ifdef ENGINE_CTOR
__attribute__((constructor))
#endif
glesv2_engine_ctor(void)
{
  list_add(&glesv2_engine.entry, &engine_list);
}
