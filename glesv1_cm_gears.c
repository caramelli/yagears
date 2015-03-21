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

#include <GLES/gl.h>
#include <math.h>
#include <stdlib.h>

/******************************************************************************/

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
  const GLfloat material_ambient[4] = { 0.0, 0.0, 0.0, 1.0 };
  GLint k;

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

  glVertexPointer(3, GL_FLOAT, 6 * sizeof(GLfloat), gear->vertices);
  glNormalPointer(GL_FLOAT, 6 * sizeof(GLfloat), gear->vertices + 3);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);

  for (k = 0; k < gear->nstrips; k++) {
    glDrawArrays(GL_TRIANGLE_STRIP, gear->strips[k].first, gear->strips[k].count);
  }

  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
}

static void delete_gear(struct gear *gear)
{
  free(gear->strips);
  free(gear->vertices);

  free(gear);
}

/******************************************************************************/

void glesv1_cm_gears_init(int win_width, int win_height)
{
  const GLfloat pos[4] = { 5.0, 5.0, 10.0, 0.0 };
  GLfloat zNear = 5, zFar = 60;

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
  glFrustumf(-1, 1, -(GLfloat)win_height/win_width, (GLfloat)win_height/win_width, zNear, zFar);

  glMatrixMode(GL_MODELVIEW);
}

void glesv1_cm_gears_draw(float tZ, float rX, float rY, float rz)
{
  const GLfloat red[4] = { 0.8, 0.1, 0.0, 1.0 };
  const GLfloat green[4] = { 0.0, 0.8, 0.2, 1.0 };
  const GLfloat blue[4] = { 0.2, 0.2, 1.0, 1.0 };

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  draw_gear(gear1, (GLfloat)tZ, (GLfloat)rX, (GLfloat)rY, -3.0, -2.0,      (GLfloat)rz     , red);
  draw_gear(gear2, (GLfloat)tZ, (GLfloat)rX, (GLfloat)rY,  3.1, -2.0, -2 * (GLfloat)rz - 9 , green);
  draw_gear(gear3, (GLfloat)tZ, (GLfloat)rX, (GLfloat)rY, -3.1,  4.2, -2 * (GLfloat)rz - 25, blue);
}

void glesv1_cm_gears_free()
{
  delete_gear(gear1);
  delete_gear(gear2);
  delete_gear(gear3);
}
