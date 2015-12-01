/*
  yagears                  Yet Another Gears OpenGL demo
  Copyright (C) 2013-2015  Nicolas Caramelli

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

/******************************************************************************/

struct gear {
  GLuint list;
};

static struct gear *gear1 = NULL, *gear2 = NULL, *gear3 = NULL;

static struct gear *create_gear(GLfloat inner, GLfloat outer, GLfloat width, GLint teeth, GLfloat tooth_depth)
{
  struct gear *gear;
  GLfloat r0, r1, r2, da, a1, ai, s[5], c[5];
  GLint i, j;
  GLenum err;

  gear = calloc(1, sizeof(struct gear));
  if (!gear) {
    printf("calloc gear failed\n");
    return NULL;
  }

  gear->list = glGenLists(1);
  if (!gear->list) {
    printf("glGenLists failed\n");
    return NULL;
  }

  glNewList(gear->list, GL_COMPILE);
  err = glGetError();
  if (err) {
    printf("glNewList failed: 0x%x\n", err);
    return NULL;
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
    glVertex3f(r2 * c[1], r2 * s[1], width / 2);
    glVertex3f(r2 * c[2], r2 * s[2], width / 2);
    glVertex3f(r1 * c[0], r1 * s[0], width / 2);
    glVertex3f(r1 * c[3], r1 * s[3], width / 2);
    glVertex3f(r0 * c[0], r0 * s[0], width / 2);
    glVertex3f(r1 * c[4], r1 * s[4], width / 2);
    glVertex3f(r0 * c[4], r0 * s[4], width / 2);
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

  return gear;
}

static void draw_gear(struct gear *gear, GLfloat model_tx, GLfloat model_ty, GLfloat model_rz, const GLfloat *color)
{
  const GLfloat material_ambient[4] = { 0.0, 0.0, 0.0, 1.0 };

  glPushMatrix();

  glTranslatef(model_tx, model_ty, 0);
  glRotatef(model_rz, 0, 0, 1);

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_ambient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);

  glCallList(gear->list);

  glPopMatrix();
}

static void delete_gear(struct gear *gear)
{
  glDeleteLists(gear->list, 1);

  free(gear);
}

/******************************************************************************/

static void gl_gears_init(int win_width, int win_height)
{
  const GLfloat pos[4] = { 5.0, 5.0, 10.0, 0.0 };
  GLdouble zNear = 5, zFar = 60;

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glLightfv(GL_LIGHT0, GL_POSITION, pos);

  gear1 = create_gear(1.0, 4.0, 1.0, 20, 0.7);
  if (!gear1) {
    return;
  }

  gear2 = create_gear(0.5, 2.0, 2.0, 10, 0.7);
  if (!gear2) {
    return;
  }

  gear3 = create_gear(1.3, 2.0, 0.5, 10, 0.7);
  if (!gear3) {
    return;
  }

  glViewport(0, 0, (GLint)win_width, (GLint)win_height);

  glMatrixMode(GL_PROJECTION);

  glFrustum(-1, 1, -(GLdouble)win_height/win_width, (GLdouble)win_height/win_width, zNear, zFar);

  glMatrixMode(GL_MODELVIEW);
}

static void gl_gears_draw(float view_tz, float view_rx, float view_ry, float model_rz)
{
  const GLfloat red[4] = { 0.8, 0.1, 0.0, 1.0 };
  const GLfloat green[4] = { 0.0, 0.8, 0.2, 1.0 };
  const GLfloat blue[4] = { 0.2, 0.2, 1.0, 1.0 };

  if (!gear1 || !gear2 || !gear3) {
    return;
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity();
  glTranslatef(0, 0, (GLfloat)view_tz);
  glRotatef((GLfloat)view_rx, 1, 0, 0);
  glRotatef((GLfloat)view_ry, 0, 1, 0);

  draw_gear(gear1, -3.0, -2.0,      (GLfloat)model_rz     , red);
  draw_gear(gear2,  3.1, -2.0, -2 * (GLfloat)model_rz - 9 , green);
  draw_gear(gear3, -3.1,  4.2, -2 * (GLfloat)model_rz - 25, blue);
}

static void gl_gears_term()
{
  if (gear1) {
    delete_gear(gear1);
    gear1 = NULL;
  }

  if (gear2) {
    delete_gear(gear2);
    gear2 = NULL;
  }

  if (gear3) {
    delete_gear(gear3);
    gear3 = NULL;
  }

  printf("%s\n", glGetString(GL_VERSION));
}

/******************************************************************************/

Engine GL_Engine = {
  "gl",
  0,
  gl_gears_init,
  gl_gears_draw,
  gl_gears_term
};
