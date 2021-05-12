/*
  yagears                  Yet Another Gears OpenGL / Vulkan demo
  Copyright (C) 2013-2021  Nicolas Caramelli

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

#include GLESV1_CM_H
#include <dlfcn.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "engine.h"

#include "image_loader.h"

extern struct list engine_list;

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
  void           (*glBindBuffer)(GLenum, GLuint);
  void           (*glBufferData)(GLenum, GLsizeiptr, const GLvoid *, GLenum);
  void           (*glClear)(GLbitfield);
  void           (*glClearColor)(GLfloat, GLfloat, GLfloat, GLfloat);
  void           (*glDeleteBuffers)(GLsizei, const GLuint *);
  void           (*glDisable)(GLenum);
  void           (*glDisableClientState)(GLenum);
  void           (*glDrawArrays)(GLenum, GLint, GLsizei);
  void           (*glEnable)(GLenum);
  void           (*glEnableClientState)(GLenum);
  void           (*glFrustumf)(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
  void           (*glGenBuffers)(GLsizei, GLuint *);
  GLenum         (*glGetError)();
  const GLubyte *(*glGetString)(GLenum);
  void           (*glLightfv)(GLenum, GLenum, const GLfloat *);
  void           (*glLoadIdentity)();
  void           (*glMaterialfv)(GLenum, GLenum, const GLfloat *);
  void           (*glMatrixMode)(GLenum);
  void           (*glNormalPointer)(GLenum, GLsizei, const GLvoid *);
  void           (*glPopMatrix)();
  void           (*glPushMatrix)();
  void           (*glRotatef)(GLfloat, GLfloat, GLfloat, GLfloat);
  void           (*glTexCoordPointer)(GLint, GLenum, GLsizei, const GLvoid *);
  void           (*glTexEnvi)(GLenum, GLenum, GLint);
  void           (*glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
  void           (*glTexParameteri)(GLenum, GLenum, GLint);
  void           (*glTranslatef)(GLfloat, GLfloat, GLfloat);
  void           (*glVertexPointer)(GLint, GLenum, GLsizei, const GLvoid *);
  void           (*glViewport)(GLint, GLint, GLsizei, GLsizei);
  struct gear *gear[3];
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
    printf("glBindBuffer failed: 0x%x\n", err);
    goto out;
  }

  gears->glBufferData(GL_ARRAY_BUFFER, gear->nvertices * sizeof(Vertex), gear->vertices, GL_STATIC_DRAW);
  err = gears->glGetError();
  if (err) {
    printf("glBufferData failed: 0x%x\n", err);
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
  int k;

  if (!gear) {
    return;
  }

  gears->glPushMatrix();
  gears->glLoadIdentity();
  gears->glLightfv(GL_LIGHT0, GL_POSITION, pos);
  gears->glPopMatrix();

  gears->glPushMatrix();

  gears->glTranslatef(model_tx, model_ty, 0);
  gears->glRotatef(model_rz, 0, 0, 1);

  gears->glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);

  if (getenv("NO_TEXTURE"))
    gears->glDisable(GL_TEXTURE_2D);
  else
    gears->glEnable(GL_TEXTURE_2D);

  gears->glBindBuffer(GL_ARRAY_BUFFER, gear->vbo);

  gears->glVertexPointer(3, GL_FLOAT, sizeof(Vertex), NULL);
  gears->glNormalPointer(GL_FLOAT, sizeof(Vertex), (const float *)NULL + 3);
  gears->glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (const float *)NULL + 6);

  gears->glEnableClientState(GL_VERTEX_ARRAY);
  gears->glEnableClientState(GL_NORMAL_ARRAY);
  gears->glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  for (k = 0; k < gear->nstrips; k++) {
    gears->glDrawArrays(GL_TRIANGLE_STRIP, gear->strips[k].begin, gear->strips[k].count);
  }

  gears->glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  gears->glDisableClientState(GL_NORMAL_ARRAY);
  gears->glDisableClientState(GL_VERTEX_ARRAY);

  gears->glPopMatrix();
}

/******************************************************************************/

static void glesv1_cm_gears_term(gears_t *gears)
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

  printf("%s\n", gears->glGetString(GL_VERSION));

  if (gears->lib_handle) {
    dlclose(gears->lib_handle);
  }

  free(gears);
}

static gears_t *glesv1_cm_gears_init(int win_width, int win_height)
{
  gears_t *gears = NULL;
  int texture_width, texture_height;
  void *texture_data = NULL;
  const float zNear = 5, zFar = 60;

  gears = calloc(1, sizeof(gears_t));
  if (!gears) {
    printf("calloc gears failed\n");
    return NULL;
  }

  gears->lib_handle = dlopen(GLESV1_CM_LIB, RTLD_LAZY);
  if (!gears->lib_handle) {
    printf("%s library not found\n", GLESV1_CM_LIB);
    goto out;
  }

  #define DLSYM(sym) gears->sym = dlsym(gears->lib_handle, #sym); \
  if (!gears->sym) { \
    printf("%s not found\n", #sym); \
    goto out; \
  }

  DLSYM(glBindBuffer);
  DLSYM(glBufferData);
  DLSYM(glClear);
  DLSYM(glClearColor);
  DLSYM(glDisable);
  DLSYM(glDisableClientState);
  DLSYM(glDeleteBuffers);
  DLSYM(glDrawArrays);
  DLSYM(glEnable);
  DLSYM(glEnableClientState);
  DLSYM(glFrustumf);
  DLSYM(glGenBuffers);
  DLSYM(glGetError);
  DLSYM(glGetString);
  DLSYM(glLightfv);
  DLSYM(glLoadIdentity);
  DLSYM(glMaterialfv);
  DLSYM(glMatrixMode);
  DLSYM(glNormalPointer);
  DLSYM(glPopMatrix);
  DLSYM(glPushMatrix);
  DLSYM(glRotatef);
  DLSYM(glTexCoordPointer);
  DLSYM(glTexEnvi);
  DLSYM(glTexImage2D);
  DLSYM(glTexParameteri);
  DLSYM(glTranslatef);
  DLSYM(glVertexPointer);
  DLSYM(glViewport);

  gears->glEnable(GL_DEPTH_TEST);
  gears->glEnable(GL_NORMALIZE);
  gears->glEnable(GL_LIGHTING);
  gears->glEnable(GL_LIGHT0);

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
  gears->glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

  /* set clear values, set viewport */

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

  gears->glMatrixMode(GL_PROJECTION);

  gears->glFrustumf(-1, 1, -(float)win_height/win_width, (float)win_height/win_width, zNear, zFar);

  gears->glMatrixMode(GL_MODELVIEW);

  return gears;

out:
  glesv1_cm_gears_term(gears);
  return NULL;
}

static void glesv1_cm_gears_draw(gears_t *gears, float view_tz, float view_rx, float view_ry, float model_rz)
{
  const float red[4] = { 0.8, 0.1, 0.0, 1.0 };
  const float green[4] = { 0.0, 0.8, 0.2, 1.0 };
  const float blue[4] = { 0.2, 0.2, 1.0, 1.0 };

  if (!gears) {
    return;
  }

  gears->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  gears->glLoadIdentity();
  gears->glTranslatef(0, 0, view_tz);
  gears->glRotatef(view_rx, 1, 0, 0);
  gears->glRotatef(view_ry, 0, 1, 0);

  draw_gear(gears, GEAR0, -3.0, -2.0,      model_rz     , red);
  draw_gear(gears, GEAR1,  3.1, -2.0, -2 * model_rz - 9 , green);
  draw_gear(gears, GEAR2, -3.1,  4.2, -2 * model_rz - 25, blue);
}

/******************************************************************************/

static engine_t glesv1_cm_engine = {
  "glesv1_cm",
  1,
  glesv1_cm_gears_init,
  glesv1_cm_gears_draw,
  glesv1_cm_gears_term
};

static void __attribute__((constructor)) engine_ctor()
{
  list_add(&glesv1_cm_engine.entry, &engine_list);
}
