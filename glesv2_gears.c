/*
  yagears                  Yet Another Gears OpenGL demo
  Copyright (C) 2013-2014  Nicolas Caramelli

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

#include <GLES2/gl2.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

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
      m[i] += (a + d.quot * 4)[j] * (b + d.rem)[j * 4];
  }

  memcpy(a, m, sizeof(m));
}

static void translate_x(GLfloat *a, GLfloat xtrans)
{
  GLfloat m[16] = {
    1, 0, 0, xtrans,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  };

  multiply(a, m);
}

static void translate_y(GLfloat *a, GLfloat ytrans)
{
  GLfloat m[16] = {
    1, 0, 0, 0,
    0, 1, 0, ytrans,
    0, 0, 1, 0,
    0, 0, 0, 1
  };

  multiply(a, m);
}

static void translate_z(GLfloat *a, GLfloat ztrans)
{
  GLfloat m[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, ztrans,
    0, 0, 0, 1
  };

  multiply(a, m);
}

static void rotate_x(GLfloat *a, GLfloat xrot)
{
  GLfloat rot;

  rot = xrot * M_PI / 180;

  GLfloat m[16] = {
    1,        0,         0, 0,
    0, cos(rot), -sin(rot), 0,
    0, sin(rot),  cos(rot), 0,
    0,        0,         0, 1
  };

  multiply(a, m);
}

static void rotate_y(GLfloat *a, GLfloat yrot)
{
  GLfloat rot;

  rot = yrot * M_PI / 180;

  GLfloat m[16] = {
     cos(rot), 0, sin(rot), 0,
            0, 1,        0, 0,
    -sin(rot), 0, cos(rot), 0,
            0, 0,        0, 1
  };

  multiply(a, m);
}

static void rotate_z(GLfloat *a, GLfloat zrot)
{
  GLfloat rot;

  rot = zrot * M_PI / 180;

  GLfloat m[16] = {
    cos(rot), -sin(rot), 0, 0,
    sin(rot),  cos(rot), 0, 0,
           0,         0, 1, 0,
           0,         0, 0, 1
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
    1, 0, 0,  -a[3],
    0, 1, 0,  -a[7],
    0, 0, 1, -a[11],
    0, 0, 0,      1,
  };

  a[3] = a[7] = a[11] = 0;
  transpose(a);

  multiply(a, m);
}

/******************************************************************************/

static GLuint program, vertex_shader, fragment_shader;
static GLint LightPos_location, ModelViewProjection_location, Normal_location, Color_location;

struct strip {
  GLint first;
  GLsizei count;
};

struct gear {
  GLint nvertices;
  GLfloat *vertices;
  GLint nstrips;
  struct strip *strips;
};

static struct gear *gear1, *gear2, *gear3;

static GLfloat Projection[16];

static struct gear *create_gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width, GLint teeth, GLfloat tooth_depth)
{
  struct gear *gear;
  GLfloat r0, r1, r2, da, angle;
  GLint i;
  GLfloat n[3];
  GLint k = 0;

  gear = calloc(1, sizeof(struct gear));
  gear->nvertices = 0;
  gear->vertices = calloc(34 * teeth, 6 * sizeof(GLfloat));
  gear->nstrips = 7 * teeth;
  gear->strips = calloc(gear->nstrips, sizeof(struct strip));

  r0 = inner_radius;
  r1 = outer_radius - tooth_depth / 2;
  r2 = outer_radius + tooth_depth / 2;
  da = 2 * M_PI / teeth / 4;

  #define normal(nx, ny, nz) \
    n[0] = nx; \
    n[1] = ny; \
    n[2] = nz;

  #define vertex(x, y, z) \
    gear->vertices[6 * gear->nvertices]     = x; \
    gear->vertices[6 * gear->nvertices + 1] = y; \
    gear->vertices[6 * gear->nvertices + 2] = z; \
    gear->vertices[6 * gear->nvertices + 3] = n[0]; \
    gear->vertices[6 * gear->nvertices + 4] = n[1]; \
    gear->vertices[6 * gear->nvertices + 5] = n[2]; \
    gear->nvertices++;

  for (i = 0; i < teeth; i++) {
    angle = i * 2 * M_PI / teeth;

    /* front face begin */
    gear->strips[k].first = gear->nvertices;
    /* front face normal */
    normal(0, 0, 1);
    /* front face vertices */
    vertex(r2 * cos(angle + da),     r2 * sin(angle + da),     width / 2);
    vertex(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width / 2);
    vertex(r1 * cos(angle),          r1 * sin(angle),          width / 2);
    vertex(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width / 2);
    vertex(r0 * cos(angle),          r0 * sin(angle),          width / 2);
    vertex(r1 * cos(angle + 4 * da), r1 * sin(angle + 4 * da), width / 2);
    vertex(r0 * cos(angle + 4 * da), r0 * sin(angle + 4 * da), width / 2);
    /* front face end */
    gear->strips[k].count = 7;
    k++;

    /* back face begin */
    gear->strips[k].first = gear->nvertices;
    /* back face normal */
    normal(0, 0, -1);
    /* back face vertices */
    vertex(r2 * cos(angle + da),     r2 * sin(angle + da),     -width / 2);
    vertex(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width / 2);
    vertex(r1 * cos(angle),          r1 * sin(angle),          -width / 2);
    vertex(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width / 2);
    vertex(r0 * cos(angle),          r0 * sin(angle),          -width / 2);
    vertex(r1 * cos(angle + 4 * da), r1 * sin(angle + 4 * da), -width / 2);
    vertex(r0 * cos(angle + 4 * da), r0 * sin(angle + 4 * da), -width / 2);
    /* back face end */
    gear->strips[k].count = 7;
    k++;

    /* first outward face begin */
    gear->strips[k].first = gear->nvertices;
    /* first outward face normal */
    normal(r2 * sin(angle + da) - r1 * sin(angle), r1 * cos(angle) - r2 * cos(angle + da), 0);
    /* first outward face vertices */
    vertex(r1 * cos(angle),      r1 * sin(angle),       width / 2);
    vertex(r1 * cos(angle),      r1 * sin(angle),      -width / 2);
    vertex(r2 * cos(angle + da), r2 * sin(angle + da),  width / 2);
    vertex(r2 * cos(angle + da), r2 * sin(angle + da), -width / 2);
    /* first outward face end */
    gear->strips[k].count = 4;
    k++;

    /* second outward face begin */
    gear->strips[k].first = gear->nvertices;
    /* second outward face normal */
    normal(r2 * sin(angle + 2 * da) - r2 * sin(angle + da), r2 * cos(angle + da) - r2 * cos(angle + 2 * da), 0);
    /* second outward face vertices */
    vertex(r2 * cos(angle + da),     r2 * sin(angle + da),      width / 2);
    vertex(r2 * cos(angle + da),     r2 * sin(angle + da),     -width / 2);
    vertex(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da),  width / 2);
    vertex(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width / 2);
    /* second outward face end */
    gear->strips[k].count = 4;
    k++;

    /* third outward face begin */
    gear->strips[k].first = gear->nvertices;
    /* third outward face normal */
    normal(r1 * sin(angle + 3 * da) - r2 * sin(angle + 2 * da), r2 * cos(angle + 2 * da) - r1 * cos(angle + 3 * da), 0);
    /* third outward face vertices */
    vertex(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da),  width / 2);
    vertex(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width / 2);
    vertex(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da),  width / 2);
    vertex(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width / 2);
    /* third outward face end */
    gear->strips[k].count = 4;
    k++;

    /* fourth outward face begin */
    gear->strips[k].first = gear->nvertices;
    /* fourth outward face normal */
    normal(r1 * sin(angle + 4 * da) - r1 * sin(angle + 3 * da), r1 * cos(angle + 3 * da) - r1 * cos(angle + 4 * da), 0);
    /* fourth outward face vertices */
    vertex(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da),  width / 2);
    vertex(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width / 2);
    vertex(r1 * cos(angle + 4 * da), r1 * sin(angle + 4 * da),  width / 2);
    vertex(r1 * cos(angle + 4 * da), r1 * sin(angle + 4 * da), -width / 2);
    /* fourth outward face end */
    gear->strips[k].count = 4;
    k++;

    /* inside face begin */
    gear->strips[k].first = gear->nvertices;
    /* inside face normal */
    normal(r0 * sin(angle) - r0 * sin(angle + 4 * da), r0 * cos(angle + 4 * da) - r0 * cos(angle), 0);
    /* inside face vertices */
    vertex(r0 * cos(angle),          r0 * sin(angle),           width / 2);
    vertex(r0 * cos(angle),          r0 * sin(angle),          -width / 2);
    vertex(r0 * cos(angle + 4 * da), r0 * sin(angle + 4 * da),  width / 2);
    vertex(r0 * cos(angle + 4 * da), r0 * sin(angle + 4 * da), -width / 2);
    /* inside face end */
    gear->strips[k].count = 4;
    k++;
  }

  return gear;
}

static void draw_gear(struct gear *gear, GLfloat tZ, GLfloat rX, GLfloat rY, GLfloat tx, GLfloat ty, GLfloat rz, const GLfloat *color)
{
  GLfloat ModelView[16], ModelViewProjection[16];
  GLint k;

  identity(ModelView);
  /* View */
  translate_z(ModelView, tZ);
  rotate_x(ModelView, rX);
  rotate_y(ModelView, rY);
  /* Model */
  translate_x(ModelView, tx);
  translate_y(ModelView, ty);
  rotate_z(ModelView, rz);

  /* Set uModelViewProjection */
  memcpy(ModelViewProjection, Projection, sizeof(ModelViewProjection));
  multiply(ModelViewProjection, ModelView);
  transpose(ModelViewProjection);
  glUniformMatrix4fv(ModelViewProjection_location, 1, GL_FALSE, ModelViewProjection);

  /* Set uNormal */
  invert(ModelView);
  glUniformMatrix4fv(Normal_location, 1, GL_FALSE, ModelView);

  /* Set uColor */
  glUniform4fv(Color_location, 1, color);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), gear->vertices);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), gear->vertices + 3);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  for (k = 0; k < gear->nstrips; k++) {
    glDrawArrays(GL_TRIANGLE_STRIP, gear->strips[k].first, gear->strips[k].count);
  }

  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(0);
}

static void delete_gear(struct gear *gear)
{
  free(gear->strips);
  free(gear->vertices);

  free(gear);
}

/******************************************************************************/

void glesv2_gears_init(int win_width, int win_height)
{
  const char *vertex_shader_text =
    "attribute vec3 aPosition;\n"
    "attribute vec3 aNormal;\n"
    "uniform vec4 uLightPos;\n"
    "uniform mat4 uModelViewProjection;\n"
    "uniform mat4 uNormal;\n"
    "uniform vec4 uColor;\n"
    "varying vec4 vColor;\n"
    "void main(void)\n"
    "{\n"
    "  gl_Position = uModelViewProjection * vec4(aPosition, 1);\n"
    "  vColor = uColor * dot(normalize(uLightPos.xyz), normalize(vec3(uNormal * vec4(aNormal, 1))));\n"
    "}";
  const char *fragment_shader_text =
    "varying vec4 vColor;\n"
    "void main()\n"
    "{\n"
    "  gl_FragColor = vColor;\n"
    "}\n";
  const GLfloat pos[4] = { 5.0, 5.0, 10.0, 0.0 };
  GLfloat zNear = 5, zFar = 60;

  glEnable(GL_DEPTH_TEST);

  program = glCreateProgram();

  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
  glCompileShader(vertex_shader);
  glAttachShader(program, vertex_shader);

  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
  glCompileShader(fragment_shader);
  glAttachShader(program, fragment_shader);

  glBindAttribLocation(program, 0, "aPosition");
  glBindAttribLocation(program, 1, "aNormal");

  glLinkProgram(program);

  glUseProgram(program);

  LightPos_location = glGetUniformLocation(program, "uLightPos");
  ModelViewProjection_location = glGetUniformLocation(program, "uModelViewProjection");
  Normal_location = glGetUniformLocation(program, "uNormal");
  Color_location = glGetUniformLocation(program, "uColor");

  /* Set uLightPos */
  glUniform4fv(LightPos_location, 1, pos);

  gear1 = create_gear(1.0, 4.0, 1.0, 20, 0.7);
  gear2 = create_gear(0.5, 2.0, 2.0, 10, 0.7);
  gear3 = create_gear(1.3, 2.0, 0.5, 10, 0.7);

  /* Projection */
  memset(Projection, 0, sizeof(Projection));
  Projection[0] = zNear;
  Projection[5] = (GLfloat)win_width/win_height * zNear;
  Projection[10] = -(zFar + zNear) / (zFar - zNear);
  Projection[11] = -2 * zFar * zNear / (zFar - zNear);
  Projection[14] = -1;
}

void glesv2_gears_draw(float tZ, float rX, float rY, float rz)
{
  const GLfloat red[4] = { 0.8, 0.1, 0.0, 1.0 };
  const GLfloat green[4] = { 0.0, 0.8, 0.2, 1.0 };
  const GLfloat blue[4] = { 0.2, 0.2, 1.0, 1.0 };

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  draw_gear(gear1, (GLfloat)tZ, (GLfloat)rX, (GLfloat)rY, -3.0, -2.0,      (GLfloat)rz     , red);
  draw_gear(gear2, (GLfloat)tZ, (GLfloat)rX, (GLfloat)rY,  3.1, -2.0, -2 * (GLfloat)rz - 9 , green);
  draw_gear(gear3, (GLfloat)tZ, (GLfloat)rX, (GLfloat)rY, -3.1,  4.2, -2 * (GLfloat)rz - 25, blue);
}

void glesv2_gears_free()
{
  delete_gear(gear1);
  delete_gear(gear2);
  delete_gear(gear3);

  glDeleteShader(fragment_shader);
  glDeleteShader(vertex_shader);

  glDeleteProgram(program);
}
