/*
  yagears                  Yet Another Gears OpenGL / Vulkan demo
  Copyright (C) 2013-2022  Nicolas Caramelli

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

#include "config.h"

#include <GL/glut.h>
void glutSetWindowData(void *data);
void *glutGetWindowData();
void glutLeaveMainLoop();
void glutExit();
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "gears_engine.h"

#define COLS 4
#define ROWS 3

/******************************************************************************/

static gears_engine_t *gears_engine[COLS * ROWS];

static int loop = 0, animate = 1, t_rate = 0, t_rot = 0, frames = 0, win_width = 0, win_height = 0, win_posx = 0, win_posy = 0;
static float fps = 0, view_tz[COLS * ROWS], view_rx[COLS * ROWS], view_ry[COLS * ROWS], model_rz[COLS * ROWS];

/******************************************************************************/

static int current_time()
{
  struct timeval tv;

  gettimeofday(&tv, NULL);

  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

/******************************************************************************/

static void rotate(int n)
{
  int t;

  t = current_time();

  if (t - t_rate >= 2000) {
    loop++;
    fps += frames * 1000.0 / (t - t_rate);
    t_rate = t;
    frames = 0;
  }

  model_rz[n] += 15 * (t - t_rot) / 1000.0;
  model_rz[n] = fmod(model_rz[n], 360);

  t_rot = t;
}

/******************************************************************************/

static void glutDisplay()
{
  int n = (long)glutGetWindowData();

  if (animate) { if (frames) rotate(n); else t_rate = t_rot = current_time(); }
  gears_engine_draw(gears_engine[n], view_tz[n], view_rx[n], view_ry[n], model_rz[n]);
  if (animate) frames++;
  glutSwapBuffers();
}

static void glutIdle()
{
  if (animate) {
    if (!frames) return;
    glutPostRedisplay();
  }
  else {
    if (frames) frames = 0;
  }
}

static void glutKeyboard(unsigned char key, int x, int y)
{
  switch (key) {
    case 27:
      glutIdleFunc(NULL);
      glutLeaveMainLoop();
      break;
    case ' ':
      animate = !animate;
      if (animate) {
        glutPostRedisplay();
      }
      break;
    default:
      break;
  }
}

static void glutSpecial(int key, int x, int y)
{
  int n = (long)glutGetWindowData();

  switch (key) {
    case GLUT_KEY_PAGE_DOWN:
      view_tz[n] -= -5.0;
      break;
    case GLUT_KEY_PAGE_UP:
      view_tz[n] += -5.0;
      break;
    case GLUT_KEY_DOWN:
      view_rx[n] -= 5.0;
      break;
    case GLUT_KEY_UP:
      view_rx[n] += 5.0;
      break;
    case GLUT_KEY_RIGHT:
      view_ry[n] -= 5.0;
      break;
    case GLUT_KEY_LEFT:
      view_ry[n] += 5.0;
      break;
    default:
      return;
  }

  if (!animate) {
    glutPostRedisplay();
  }
}

static void glutPassiveMotion(int x, int y)
{
}

/******************************************************************************/

int main(int argc, char *argv[])
{
  int opt, i, j;
  int glut_win[COLS * ROWS];

  if (argc != 2) {
    printf("\n\tUsage: %s Engine : ", argv[0]);
    for (opt = 0; opt < gears_engine_nb(); opt++) {
      printf("%s ", gears_engine_name(opt));
    }
    printf("\n\n");
    return EXIT_FAILURE;
  }

  for (opt = 0; opt < gears_engine_nb(); opt++) {
    if (!strcmp(gears_engine_name(opt), argv[1]))
      break;
  }

  if (opt == gears_engine_nb()) {
    printf("%s: Engine unknown\n", argv[1]);
    return EXIT_FAILURE;
  }

  /* init */

  glutInit(&argc, argv);

  win_width = glutGet(GLUT_SCREEN_WIDTH);
  win_height = glutGet(GLUT_SCREEN_HEIGHT);

  /* windows */

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(win_width / COLS, win_height / ROWS);
  for (i = 0; i < ROWS; i++) {
    for (j = 0; j < COLS; j++) {
      gears_engine[i * COLS + j] = gears_engine_new(gears_engine_name(opt));
      glutInitWindowPosition(win_posx, win_posy);
      glut_win[i * COLS + j] = glutCreateWindow(NULL);
      glutSetWindowData((void *)(long)(i * COLS + j));
      glutDisplayFunc(glutDisplay);
      glutIdleFunc(glutIdle);
      glutKeyboardFunc(glutKeyboard);
      glutSpecialFunc(glutSpecial);
      glutPassiveMotionFunc(glutPassiveMotion);
      view_tz[i * COLS + j] = -40.0;
      view_rx[i * COLS + j] = 20.0;
      view_ry[i * COLS + j] = 30.0;
      model_rz[i * COLS + j] = 210.0;
      gears_engine_init(gears_engine[i * COLS + j], win_width / COLS, win_height / ROWS);
      win_posx += win_width / COLS;
    }
    win_posx = 0;
    win_posy += win_height / ROWS;
  }

  /* drawing (main event loop) */

  if (getenv("NO_ANIM")) {
    animate = 0;
  }

  glutMainLoop();

  /* benchmark */

  if (fps) {
    fps /= loop;
    printf("Gears demo: %.2f fps\n", fps);
  }

  /* term */

  for (i = 0; i < ROWS; i++) {
    for (j = 0; j < COLS; j++) {
      gears_engine_term(gears_engine[i * COLS + j]);
      glutDestroyWindow(glut_win[i * COLS + j]);
      gears_engine_free(gears_engine[i * COLS + j]);
    }
  }

  glutExit();

  return EXIT_SUCCESS;
}
