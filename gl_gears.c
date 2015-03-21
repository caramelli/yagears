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

#include <GL/gl.h>
#include <math.h>
#include <stdlib.h>

/******************************************************************************/

struct gear {
  GLuint list;
};

static struct gear *gear1, *gear2, *gear3;

static struct gear *create_gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width, GLint teeth, GLfloat tooth_depth)
{
  struct gear *gear;
  GLfloat r0, r1, r2, da, angle;
  GLint i;

  gear = calloc(1, sizeof(struct gear));
  gear->list = glGenLists(1);
  glNewList(gear->list, GL_COMPILE);

  r0 = inner_radius;
  r1 = outer_radius - tooth_depth / 2;
  r2 = outer_radius + tooth_depth / 2;
  da = 2 * M_PI / teeth / 4;

  for (i = 0; i < teeth; i++) {
    angle = i * 2 * M_PI / teeth;

    /* front face begin */
    glBegin(GL_TRIANGLE_STRIP);
    /* front face normal */
    glNormal3f(0, 0, 1);
    /* front face vertices */
    glVertex3f(r2 * cos(angle + da),     r2 * sin(angle + da),     width / 2);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width / 2);
    glVertex3f(r1 * cos(angle),          r1 * sin(angle),          width / 2);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width / 2);
    glVertex3f(r0 * cos(angle),          r0 * sin(angle),          width / 2);
    glVertex3f(r1 * cos(angle + 4 * da), r1 * sin(angle + 4 * da), width / 2);
    glVertex3f(r0 * cos(angle + 4 * da), r0 * sin(angle + 4 * da), width / 2);
    /* front face end */
    glEnd();

    /* back face begin */
    glBegin(GL_TRIANGLE_STRIP);
    /* back face normal */
    glNormal3f(0, 0, -1);
    /* back face vertices */
    glVertex3f(r2 * cos(angle + da),     r2 * sin(angle + da),     -width / 2);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width / 2);
    glVertex3f(r1 * cos(angle),          r1 * sin(angle),          -width / 2);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width / 2);
    glVertex3f(r0 * cos(angle),          r0 * sin(angle),          -width / 2);
    glVertex3f(r1 * cos(angle + 4 * da), r1 * sin(angle + 4 * da), -width / 2);
    glVertex3f(r0 * cos(angle + 4 * da), r0 * sin(angle + 4 * da), -width / 2);
    /* back face end */
    glEnd();

    /* first outward face begin */
    glBegin(GL_TRIANGLE_STRIP);
    /* first outward face normal */
    glNormal3f(r2 * sin(angle + da) - r1 * sin(angle), r1 * cos(angle) - r2 * cos(angle + da), 0);
    /* first outward face vertices */
    glVertex3f(r1 * cos(angle),      r1 * sin(angle),       width / 2);
    glVertex3f(r1 * cos(angle),      r1 * sin(angle),      -width / 2);
    glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da),  width / 2);
    glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width / 2);
    /* first outward face end */
    glEnd();

    /* second outward face begin */
    glBegin(GL_TRIANGLE_STRIP);
    /* second outward face normal */
    glNormal3f(r2 * sin(angle + 2 * da) - r2 * sin(angle + da), r2 * cos(angle + da) - r2 * cos(angle + 2 * da), 0);
    /* second outward face vertices */
    glVertex3f(r2 * cos(angle + da),     r2 * sin(angle + da),      width / 2);
    glVertex3f(r2 * cos(angle + da),     r2 * sin(angle + da),     -width / 2);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da),  width / 2);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width / 2);
    /* second outward face end */
    glEnd();

    /* third outward face begin */
    glBegin(GL_TRIANGLE_STRIP);
    /* third outward face normal */
    glNormal3f(r1 * sin(angle + 3 * da) - r2 * sin(angle + 2 * da), r2 * cos(angle + 2 * da) - r1 * cos(angle + 3 * da), 0);
    /* third outward face vertices */
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da),  width / 2);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width / 2);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da),  width / 2);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width / 2);
    /* third outward face end */
    glEnd();

    /* fourth outward face begin */
    glBegin(GL_TRIANGLE_STRIP);
    /* fourth outward face normal */
    glNormal3f(r1 * sin(angle + 4 * da) - r1 * sin(angle + 3 * da), r1 * cos(angle + 3 * da) - r1 * cos(angle + 4 * da), 0);
    /* fourth outward face vertices */
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da),  width / 2);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width / 2);
    glVertex3f(r1 * cos(angle + 4 * da), r1 * sin(angle + 4 * da),  width / 2);
    glVertex3f(r1 * cos(angle + 4 * da), r1 * sin(angle + 4 * da), -width / 2);
    /* fourth outward face end */
    glEnd();

    /* inside face begin */
    glBegin(GL_TRIANGLE_STRIP);
    /* inside face normal */
    glNormal3f(r0 * sin(angle) - r0 * sin(angle + 4 * da), r0 * cos(angle + 4 * da) - r0 * cos(angle), 0);
    /* inside face vertices */
    glVertex3f(r0 * cos(angle),          r0 * sin(angle),           width / 2);
    glVertex3f(r0 * cos(angle),          r0 * sin(angle),          -width / 2);
    glVertex3f(r0 * cos(angle + 4 * da), r0 * sin(angle + 4 * da),  width / 2);
    glVertex3f(r0 * cos(angle + 4 * da), r0 * sin(angle + 4 * da), -width / 2);
    /* inside face end */
    glEnd();
  }

  glEndList();

  return gear;
}

static void draw_gear(struct gear *gear, GLfloat tZ, GLfloat rX, GLfloat rY, GLfloat tx, GLfloat ty, GLfloat rz, const GLfloat *color)
{
  const GLfloat material_ambient[4] = { 0.0, 0.0, 0.0, 1.0 };

  glLoadIdentity();
  /* View */
  glTranslatef(0, 0, tZ);
  glRotatef(rX, 1, 0, 0);
  glRotatef(rY, 0, 1, 0);
  /* Model */
  glTranslatef(tx, ty, 0);
  glRotatef(rz, 0, 0, 1);

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_ambient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);

  glCallList(gear->list);
}

static void delete_gear(struct gear *gear)
{
  glDeleteLists(gear->list, 1);

  free(gear);
}

/******************************************************************************/

void gl_gears_init(int win_width, int win_height)
{
  const GLfloat pos[4] = { 5.0, 5.0, 10.0, 0.0 };
  GLdouble zNear = 5, zFar = 60;

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glLightfv(GL_LIGHT0, GL_POSITION, pos);

  gear1 = create_gear(1.0, 4.0, 1.0, 20, 0.7);
  gear2 = create_gear(0.5, 2.0, 2.0, 10, 0.7);
  gear3 = create_gear(1.3, 2.0, 0.5, 10, 0.7);

  glMatrixMode(GL_PROJECTION);

  /* Projection */
  glFrustum(-1, 1, -(GLdouble)win_height/win_width, (GLdouble)win_height/win_width, zNear, zFar);

  glMatrixMode(GL_MODELVIEW);
}

void gl_gears_draw(float tZ, float rX, float rY, float rz)
{
  const GLfloat red[4] = { 0.8, 0.1, 0.0, 1.0 };
  const GLfloat green[4] = { 0.0, 0.8, 0.2, 1.0 };
  const GLfloat blue[4] = { 0.2, 0.2, 1.0, 1.0 };

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  draw_gear(gear1, (GLfloat)tZ, (GLfloat)rX, (GLfloat)rY, -3.0, -2.0,      (GLfloat)rz     , red);
  draw_gear(gear2, (GLfloat)tZ, (GLfloat)rX, (GLfloat)rY,  3.1, -2.0, -2 * (GLfloat)rz - 9 , green);
  draw_gear(gear3, (GLfloat)tZ, (GLfloat)rX, (GLfloat)rY, -3.1,  4.2, -2 * (GLfloat)rz - 25, blue);
}

void gl_gears_free()
{
  delete_gear(gear1);
  delete_gear(gear2);
  delete_gear(gear3);
}
