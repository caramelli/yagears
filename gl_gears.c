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

/******************************************************************************/

static GLuint gear1, gear2, gear3;

static GLuint create_gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width, GLint teeth, GLfloat tooth_depth, const GLfloat *color)
{
  GLuint gear;
  GLfloat r0, r1, r2, da, angle;
  GLint i;

  gear = glGenLists(1);
  glNewList(gear, GL_COMPILE);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, color);

  r0 = inner_radius;
  r1 = outer_radius - tooth_depth / 2;
  r2 = outer_radius + tooth_depth / 2;
  da = 2 * M_PI / teeth / 4;

  for (i = 0; i < teeth; i++) {
    angle = i * 2 * M_PI / teeth;

  /*
    front face and front sides of teeth
  */

    glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(0, 0, 1);
    glVertex3f(r2 * cos(angle + da),     r2 * sin(angle + da),     width / 2);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width / 2);
    glVertex3f(r1 * cos(angle),          r1 * sin(angle),          width / 2);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width / 2);
    glVertex3f(r0 * cos(angle),          r0 * sin(angle),          width / 2);
    glVertex3f(r1 * cos(angle + 4 * da), r1 * sin(angle + 4 * da), width / 2);
    glVertex3f(r0 * cos(angle + 4 * da), r0 * sin(angle + 4 * da), width / 2);
    glEnd();

  /*
    back face and back sides of teeth
  */

    glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(0, 0, -1);
    glVertex3f(r2 * cos(angle + da),     r2 * sin(angle + da),     -width / 2);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width / 2);
    glVertex3f(r1 * cos(angle),          r1 * sin(angle),          -width / 2);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width / 2);
    glVertex3f(r0 * cos(angle),          r0 * sin(angle),          -width / 2);
    glVertex3f(r1 * cos(angle + 4 * da), r1 * sin(angle + 4 * da), -width / 2);
    glVertex3f(r0 * cos(angle + 4 * da), r0 * sin(angle + 4 * da), -width / 2);
    glEnd();

  /*
    outward faces of teeth
  */

    glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(r2 * sin(angle + da) - r1 * sin(angle), r1 * cos(angle) - r2 * cos(angle + da), 0);
    glVertex3f(r1 * cos(angle),      r1 * sin(angle),       width / 2);
    glVertex3f(r1 * cos(angle),      r1 * sin(angle),      -width / 2);
    glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da),  width / 2);
    glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width / 2);
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(r2 * sin(angle + 2 * da) - r2 * sin(angle + da), r2 * cos(angle + da) - r2 * cos(angle + 2 * da), 0);
    glVertex3f(r2 * cos(angle + da),     r2 * sin(angle + da),      width / 2);
    glVertex3f(r2 * cos(angle + da),     r2 * sin(angle + da),     -width / 2);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da),  width / 2);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width / 2);
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(r1 * sin(angle + 3 * da) - r2 * sin(angle + 2 * da), r2 * cos(angle + 2 * da) - r1 * cos(angle + 3 * da), 0);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da),  width / 2);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width / 2);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da),  width / 2);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width / 2);
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(r1 * sin(angle + 4 * da) - r1 * sin(angle + 3 * da), r1 * cos(angle + 3 * da) - r1 * cos(angle + 4 * da), 0);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da),  width / 2);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width / 2);
    glVertex3f(r1 * cos(angle + 4 * da), r1 * sin(angle + 4 * da),  width / 2);
    glVertex3f(r1 * cos(angle + 4 * da), r1 * sin(angle + 4 * da), -width / 2);
    glEnd();

  /*
    inside face
  */

    glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(r0 * sin(angle) - r0 * sin(angle + 4 * da), r0 * cos(angle + 4 * da) - r0 * cos(angle), 0);
    glVertex3f(r0 * cos(angle),          r0 * sin(angle),           width / 2);
    glVertex3f(r0 * cos(angle),          r0 * sin(angle),          -width / 2);
    glVertex3f(r0 * cos(angle + 4 * da), r0 * sin(angle + 4 * da),  width / 2);
    glVertex3f(r0 * cos(angle + 4 * da), r0 * sin(angle + 4 * da), -width / 2);
    glEnd();
  }

  glEndList();

  return gear;
}

static void draw_gear(GLuint list, GLfloat tZ, GLfloat rX, GLfloat rY, GLfloat tx, GLfloat ty, GLfloat rz)
{
  glLoadIdentity();
  /* View */
  glTranslatef(0, 0, tZ);
  glRotatef(rX, 1, 0, 0);
  glRotatef(rY, 0, 1, 0);
  /* Model */
  glTranslatef(tx, ty, 0);
  glRotatef(rz, 0, 0, 1);

  glCallList(list);
}

static void delete_gear(GLuint list)
{
  glDeleteLists(list, 1);
}

/******************************************************************************/

void init(int winWidth, int winHeight)
{
  const GLfloat light_model_ambient[4] = { 0.0, 0.0, 0.0, 1.0 };
  const GLfloat pos[4] = { 5.0, 5.0, 10.0, 0.0 };
  const GLfloat red[4] = { 0.8, 0.1, 0.0, 1.0 };
  const GLfloat green[4] = { 0.0, 0.8, 0.2, 1.0 };
  const GLfloat blue[4] = { 0.2, 0.2, 1.0, 1.0 };

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_model_ambient);

  glLightfv(GL_LIGHT0, GL_POSITION, pos);

  gear1 = create_gear(1.0, 4.0, 1.0, 20, 0.7, red);
  gear2 = create_gear(0.5, 2.0, 2.0, 10, 0.7, green);
  gear3 = create_gear(1.3, 2.0, 0.5, 10, 0.7, blue);

  glMatrixMode(GL_PROJECTION);

  /* Projection */
  glFrustum(-1, 1, -(GLdouble)winHeight/winWidth, (GLdouble)winHeight/winWidth, 5, 60);

  glMatrixMode(GL_MODELVIEW);
}

void draw(float tZ, float rX, float rY, float rz)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  draw_gear(gear1, (GLfloat)tZ, (GLfloat)rX, (GLfloat)rY, -3.0, -2.0,      (GLfloat)rz);
  draw_gear(gear2, (GLfloat)tZ, (GLfloat)rX, (GLfloat)rY,  3.1, -2.0, -2 * (GLfloat)rz - 9);
  draw_gear(gear3, (GLfloat)tZ, (GLfloat)rX, (GLfloat)rY, -3.1,  4.2, -2 * (GLfloat)rz - 25);
}

void fini()
{
  delete_gear(gear1);
  delete_gear(gear2);
  delete_gear(gear3);
}
