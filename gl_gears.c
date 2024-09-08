/*
  yagears                  Yet Another Gears OpenGL demo
  Copyright (C) 2013-2024  Nicolas Caramelli

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

#include <GL/gl.h>
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

struct gear {
  GLuint list;
};

struct gears {
  struct gear *gear[3];
};

static void delete_gear(gears_t *gears, int id)
{
  struct gear *gear = gears->gear[id];

  if (!gear) {
    return;
  }

  if (gear->list) {
    glDeleteLists(gear->list, 1);
  }

  free(gear);

  gears->gear[id] = NULL;
}

static int create_gear(gears_t *gears, int id, float inner, float outer, float width, int teeth, float tooth_depth)
{
  struct gear *gear;
  float r0, r1, r2, da, a1, ai, s[5], c[5];
  int i, j;
  GLenum err = GL_NO_ERROR;

  gear = calloc(1, sizeof(struct gear));
  if (!gear) {
    printf("calloc gear failed\n");
    return -1;
  }

  gears->gear[id] = gear;

  gear->list = glGenLists(1);
  if (!gear->list) {
    printf("glGenLists failed\n");
    goto out;
  }

  glNewList(gear->list, GL_COMPILE);
  err = glGetError();
  if (err) {
    printf("glNewList failed: 0x%x\n", (unsigned int)err);
    goto out;
  }

  r0 = inner;
  r1 = outer - tooth_depth / 2;
  r2 = outer + tooth_depth / 2;
  a1 = 2 * M_PI / teeth;
  da = a1 / 4;

  for (i = 0; i < teeth; i++) {
    ai = i * a1;
    for (j = 0; j < 5; j++) {
      sincosf(ai + j * da, &s[j], &c[j]);
    }

    /* front face begin */
    glBegin(GL_TRIANGLE_STRIP);
    /* front face normal */
    glNormal3f(0, 0, 1);
    /* front face vertices */
    glTexCoord2f(0.36 * r2 * s[1] / r1 + 0.5, 0.36 * r2 * c[1] / r1 + 0.5);
    glVertex3f(r2 * c[1], r2 * s[1], width / 2);
    glTexCoord2f(0.36 * r2 * s[2] / r1 + 0.5, 0.36 * r2 * c[2] / r1 + 0.5);
    glVertex3f(r2 * c[2], r2 * s[2], width / 2);
    glTexCoord2f(0.36 * r1 * s[0] / r1 + 0.5, 0.36 * r1 * c[0] / r1 + 0.5);
    glVertex3f(r1 * c[0], r1 * s[0], width / 2);
    glTexCoord2f(0.36 * r1 * s[3] / r1 + 0.5, 0.36 * r1 * c[3] / r1 + 0.5);
    glVertex3f(r1 * c[3], r1 * s[3], width / 2);
    glTexCoord2f(0.36 * r0 * s[0] / r1 + 0.5, 0.36 * r0 * c[0] / r1 + 0.5);
    glVertex3f(r0 * c[0], r0 * s[0], width / 2);
    glTexCoord2f(0.36 * r1 * s[4] / r1 + 0.5, 0.36 * r1 * c[4] / r1 + 0.5);
    glVertex3f(r1 * c[4], r1 * s[4], width / 2);
    glTexCoord2f(0.36 * r0 * s[4] / r1 + 0.5, 0.36 * r0 * c[4] / r1 + 0.5);
    glVertex3f(r0 * c[4], r0 * s[4], width / 2);
    glTexCoord2f(0, 0);
    /* front face end */
    glEnd();

    /* back face begin */
    glBegin(GL_TRIANGLE_STRIP);
    /* back face normal */
    glNormal3f(0, 0, -1);
    /* back face vertices */
    glVertex3f(r2 * c[1], r2 * s[1], -width / 2);
    glVertex3f(r2 * c[2], r2 * s[2], -width / 2);
    glVertex3f(r1 * c[0], r1 * s[0], -width / 2);
    glVertex3f(r1 * c[3], r1 * s[3], -width / 2);
    glVertex3f(r0 * c[0], r0 * s[0], -width / 2);
    glVertex3f(r1 * c[4], r1 * s[4], -width / 2);
    glVertex3f(r0 * c[4], r0 * s[4], -width / 2);
    /* back face end */
    glEnd();

    /* first outward face begin */
    glBegin(GL_TRIANGLE_STRIP);
    /* first outward face normal */
    glNormal3f(r2 * s[1] - r1 * s[0], r1 * c[0] - r2 * c[1], 0);
    /* first outward face vertices */
    glVertex3f(r1 * c[0], r1 * s[0],  width / 2);
    glVertex3f(r1 * c[0], r1 * s[0], -width / 2);
    glVertex3f(r2 * c[1], r2 * s[1],  width / 2);
    glVertex3f(r2 * c[1], r2 * s[1], -width / 2);
    /* first outward face end */
    glEnd();

    /* second outward face begin */
    glBegin(GL_TRIANGLE_STRIP);
    /* second outward face normal */
    glNormal3f(s[2] - s[1], c[1] - c[2], 0);
    /* second outward face vertices */
    glVertex3f(r2 * c[1], r2 * s[1],  width / 2);
    glVertex3f(r2 * c[1], r2 * s[1], -width / 2);
    glVertex3f(r2 * c[2], r2 * s[2],  width / 2);
    glVertex3f(r2 * c[2], r2 * s[2], -width / 2);
    /* second outward face end */
    glEnd();

    /* third outward face begin */
    glBegin(GL_TRIANGLE_STRIP);
    /* third outward face normal */
    glNormal3f(r1 * s[3] - r2 * s[2], r2 * c[2] - r1 * c[3], 0);
    /* third outward face vertices */
    glVertex3f(r2 * c[2], r2 * s[2],  width / 2);
    glVertex3f(r2 * c[2], r2 * s[2], -width / 2);
    glVertex3f(r1 * c[3], r1 * s[3],  width / 2);
    glVertex3f(r1 * c[3], r1 * s[3], -width / 2);
    /* third outward face end */
    glEnd();

    /* fourth outward face begin */
    glBegin(GL_TRIANGLE_STRIP);
    /* fourth outward face normal */
    glNormal3f(s[4] - s[3], c[3] - c[4], 0);
    /* fourth outward face vertices */
    glVertex3f(r1 * c[3], r1 * s[3],  width / 2);
    glVertex3f(r1 * c[3], r1 * s[3], -width / 2);
    glVertex3f(r1 * c[4], r1 * s[4],  width / 2);
    glVertex3f(r1 * c[4], r1 * s[4], -width / 2);
    /* fourth outward face end */
    glEnd();

    /* inside face begin */
    glBegin(GL_TRIANGLE_STRIP);
    /* inside face normal */
    glNormal3f(s[0] - s[4], c[4] - c[0], 0);
    /* inside face vertices */
    glVertex3f(r0 * c[0], r0 * s[0],  width / 2);
    glVertex3f(r0 * c[0], r0 * s[0], -width / 2);
    glVertex3f(r0 * c[4], r0 * s[4],  width / 2);
    glVertex3f(r0 * c[4], r0 * s[4], -width / 2);
    /* inside face end */
    glEnd();
  }

  glEndList();
  err = glGetError();
  if (err) {
    printf("glEndList failed: 0x%x\n", (unsigned int)err);
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

  if (!gear) {
    return;
  }

  glPushMatrix();
  glLoadIdentity();
  glLightfv(GL_LIGHT0, GL_POSITION, pos);
  glPopMatrix();

  glPushMatrix();

  glTranslatef(model_tx, model_ty, 0);
  glRotatef(model_rz, 0, 0, 1);

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);

  if (getenv("NO_TEXTURE"))
    glDisable(GL_TEXTURE_2D);
  else
    glEnable(GL_TEXTURE_2D);

  glCallList(gear->list);

  glPopMatrix();
}

/******************************************************************************/

static void gl_gears_term(gears_t *gears)
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

  printf("%s\n", glGetString(GL_VERSION));

  free(gears);
}

static gears_t *gl_gears_init(int win_width, int win_height)
{
  gears_t *gears = NULL;
  int texture_width, texture_height;
  void *texture_data = NULL;
  const GLdouble zNear = 5, zFar = 60;

  gears = calloc(1, sizeof(gears_t));
  if (!gears) {
    printf("calloc gears failed\n");
    return NULL;
  }

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  /* load texture */

  image_load(getenv("TEXTURE"), NULL, &texture_width, &texture_height);

  texture_data = malloc(texture_width * texture_height * 4);
  if (!texture_data) {
    printf("malloc texture_data failed\n");
    goto out;
  }

  image_load(getenv("TEXTURE"), texture_data, &texture_width, &texture_height);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);

  free(texture_data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

  /* set clear values, set viewport */

  glClearColor(0, 0, 0, 1);

  glViewport(0, 0, win_width, win_height);

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

  glMatrixMode(GL_PROJECTION);

  glFrustum(-1, 1, -(GLdouble)win_height/win_width, (GLdouble)win_height/win_width, zNear, zFar);

  glMatrixMode(GL_MODELVIEW);

  return gears;

out:
  gl_gears_term(gears);
  return NULL;
}

static void gl_gears_draw(gears_t *gears, float view_tz, float view_rx, float view_ry, float model_rz)
{
  const float red[4] = { 0.8, 0.1, 0.0, 1.0 };
  const float green[4] = { 0.0, 0.8, 0.2, 1.0 };
  const float blue[4] = { 0.2, 0.2, 1.0, 1.0 };

  if (!gears) {
    return;
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity();
  glTranslatef(0, 0, view_tz);
  glRotatef(view_rx, 1, 0, 0);
  glRotatef(view_ry, 0, 1, 0);

  draw_gear(gears, GEAR0, -3.0, -2.0,      model_rz     , red);
  draw_gear(gears, GEAR1,  3.1, -2.0, -2 * model_rz - 9 , green);
  draw_gear(gears, GEAR2, -3.1,  4.2, -2 * model_rz - 25, blue);
}

/******************************************************************************/

static engine_t gl_engine = {
  "gl",
  0,
  gl_gears_init,
  gl_gears_draw,
  gl_gears_term
};

void __attribute__((constructor)) engine_ctor(void)
{
  list_add(&gl_engine.entry, &engine_list);
}
