/*
  yagears                  Yet Another Gears OpenGL / Vulkan demo
  Copyright (C) 2013-2020  Nicolas Caramelli

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
  void *lib_handle;
  void (*glEnable)(GLenum);
  void (*glDisable)(GLenum);
  void (*glLightfv)(GLenum, GLenum, const GLfloat *);
  void (*glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
  void (*glTexParameteri)(GLenum, GLenum, GLint);
  void (*glTexEnvi)(GLenum, GLenum, GLint);
  void (*glClearColor)(GLfloat, GLfloat, GLfloat, GLfloat);
  void (*glGenBuffers)(GLsizei, GLuint *);
  void (*glBindBuffer)(GLenum, GLuint);
  void (*glBufferData)(GLenum, GLsizeiptr, const GLvoid *, GLenum);
  void (*glViewport)(GLint, GLint, GLsizei, GLsizei);
  void (*glMatrixMode)(GLenum);
  void (*glFrustumf)(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
  void (*glClear)(GLbitfield);
  void (*glLoadIdentity)();
  void (*glTranslatef)(GLfloat, GLfloat, GLfloat);
  void (*glRotatef)(GLfloat, GLfloat, GLfloat, GLfloat);
  void (*glPushMatrix)();
  void (*glMaterialfv)(GLenum, GLenum, const GLfloat *);
  void (*glVertexPointer)(GLint, GLenum, GLsizei, const GLvoid *);
  void (*glNormalPointer)(GLenum, GLsizei, const GLvoid *);
  void (*glTexCoordPointer)(GLint, GLenum, GLsizei, const GLvoid *);
  void (*glEnableClientState)(GLenum);
  void (*glDrawArrays)(GLenum, GLint, GLsizei);
  void (*glDisableClientState)(GLenum);
  void (*glPopMatrix)();
  void (*glDeleteBuffers)(GLsizei, const GLuint *);
  const GLubyte *(*glGetString)(GLenum);
  struct gear *gear1;
  struct gear *gear2;
  struct gear *gear3;
};

static struct gear *create_gear(gears_t *gears, GLfloat inner, GLfloat outer, GLfloat width, GLint teeth, GLfloat tooth_depth)
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

  /* vertex buffer object */

  gears->glGenBuffers(1, &gear->vbo);
  if (!gear->vbo) {
    printf("glGenBuffers failed\n");
    goto out;
  }

  gears->glBindBuffer(GL_ARRAY_BUFFER, gear->vbo);

  gears->glBufferData(GL_ARRAY_BUFFER, gear->nvertices * sizeof(Vertex), gear->vertices, GL_STATIC_DRAW);

  return gear;

out:
  if (gear->strips) {
    free(gear->strips);
  }
  if (gear->vertices) {
    free(gear->vertices);
  }
  free(gear);
  return NULL;
}

static void draw_gear(gears_t *gears, struct gear *gear, GLfloat model_tx, GLfloat model_ty, GLfloat model_rz, const GLfloat *color)
{
  const GLfloat pos[4] = { 5.0, 5.0, 10.0, 0.0 };
  GLint k;

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
  gears->glNormalPointer(GL_FLOAT, sizeof(Vertex), (const GLfloat *)NULL + 3);
  gears->glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (const GLfloat *)NULL + 6);

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

static void delete_gear(gears_t *gears, struct gear *gear)
{
  gears->glDeleteBuffers(1, &gear->vbo);
  free(gear->strips);
  free(gear->vertices);

  free(gear);
}

/******************************************************************************/

static gears_t *glesv1_cm_gears_init(int win_width, int win_height)
{
  gears_t *gears = NULL;
  image_t image;
  const GLfloat zNear = 5, zFar = 60;

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

  DLSYM(glEnable);
  DLSYM(glDisable);
  DLSYM(glLightfv);
  DLSYM(glTexImage2D);
  DLSYM(glTexParameteri);
  DLSYM(glTexEnvi);
  DLSYM(glClearColor);
  DLSYM(glGenBuffers);
  DLSYM(glBindBuffer);
  DLSYM(glBufferData);
  DLSYM(glViewport);
  DLSYM(glMatrixMode);
  DLSYM(glFrustumf);
  DLSYM(glClear);
  DLSYM(glLoadIdentity);
  DLSYM(glTranslatef);
  DLSYM(glRotatef);
  DLSYM(glPushMatrix);
  DLSYM(glMaterialfv);
  DLSYM(glVertexPointer);
  DLSYM(glNormalPointer);
  DLSYM(glTexCoordPointer);
  DLSYM(glEnableClientState);
  DLSYM(glDrawArrays);
  DLSYM(glDisableClientState);
  DLSYM(glPopMatrix);
  DLSYM(glDeleteBuffers);
  DLSYM(glGetString);

  gears->glEnable(GL_DEPTH_TEST);
  gears->glEnable(GL_NORMALIZE);
  gears->glEnable(GL_LIGHTING);
  gears->glEnable(GL_LIGHT0);

  /* load texture */

  image_load(getenv("TEXTURE"), &image);
  gears->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.pixel_data);
  image_unload(&image);
  gears->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  gears->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  gears->glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

  /* set clear values, set viewport */

  gears->glClearColor(0, 0, 0, 1);

  gears->glViewport(0, 0, (GLint)win_width, (GLint)win_height);

  /* create gears */

  gears->gear1 = create_gear(gears, 1.0, 4.0, 1.0, 20, 0.7);
  if (!gears->gear1) {
    goto out;
  }

  gears->gear2 = create_gear(gears, 0.5, 2.0, 2.0, 10, 0.7);
  if (!gears->gear2) {
    goto out;
  }

  gears->gear3 = create_gear(gears, 1.3, 2.0, 0.5, 10, 0.7);
  if (!gears->gear3) {
    goto out;
  }

  gears->glMatrixMode(GL_PROJECTION);

  gears->glFrustumf(-1, 1, -(GLfloat)win_height/win_width, (GLfloat)win_height/win_width, zNear, zFar);

  gears->glMatrixMode(GL_MODELVIEW);

  return gears;

out:
  if (gears->gear3) {
    delete_gear(gears, gears->gear3);
  }
  if (gears->gear2) {
    delete_gear(gears, gears->gear2);
  }
  if (gears->gear1) {
    delete_gear(gears, gears->gear1);
  }
  if (gears->lib_handle) {
    dlclose(gears->lib_handle);
  }
  free(gears);
  return NULL;
}

static void glesv1_cm_gears_draw(gears_t *gears, float view_tz, float view_rx, float view_ry, float model_rz)
{
  const GLfloat red[4] = { 0.8, 0.1, 0.0, 1.0 };
  const GLfloat green[4] = { 0.0, 0.8, 0.2, 1.0 };
  const GLfloat blue[4] = { 0.2, 0.2, 1.0, 1.0 };

  if (!gears) {
    return;
  }

  gears->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  gears->glLoadIdentity();
  gears->glTranslatef(0, 0, (GLfloat)view_tz);
  gears->glRotatef((GLfloat)view_rx, 1, 0, 0);
  gears->glRotatef((GLfloat)view_ry, 0, 1, 0);

  draw_gear(gears, gears->gear1, -3.0, -2.0,      (GLfloat)model_rz     , red);
  draw_gear(gears, gears->gear2,  3.1, -2.0, -2 * (GLfloat)model_rz - 9 , green);
  draw_gear(gears, gears->gear3, -3.1,  4.2, -2 * (GLfloat)model_rz - 25, blue);
}

static void glesv1_cm_gears_term(gears_t *gears)
{
  if (!gears) {
    return;
  }

  delete_gear(gears, gears->gear1);
  delete_gear(gears, gears->gear2);
  delete_gear(gears, gears->gear3);

  printf("%s\n", gears->glGetString(GL_VERSION));

  dlclose(gears->lib_handle);

  free(gears);
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
