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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "config.h"

#if defined(EFL)
#include <Elementary.h>
#endif
#if defined(GLUT)
#include <GL/glut.h>
#endif
#if defined(GTK)
#include <gtk/gtkgl.h>
#include <gdk/gdkkeysyms.h>
#endif
#if defined(QT)
#include <QGLWidget>
#include <QtGui>
#endif
#if defined(SDL)
#include <SDL.h>
static void SDL_Display();
#endif

#if defined(GL)
#include "gl_gears.h"
#endif
#if defined(GLESV1_CM)
#include "glesv1_cm_gears.h"
#endif
#if defined(GLESV2)
#include "glesv2_gears.h"
#endif

/******************************************************************************/

static char *toolkit = NULL;
static char *engine = NULL;

static int loop = 0, animate = 1, t_rate = 0, t_rot = 0, frames = 0, win_width = 0, win_height = 0;
static float fps = 0, view_tz = -40.0, view_rx = 20.0, view_ry = 30.0, model_rz = 0.0;

/******************************************************************************/

static void init()
{
  #if defined(GL)
  if (!strcmp(engine, "gl")) {
    gl_gears_init(win_width, win_height);
  }
  #endif
  #if defined(GLESV1_CM)
  if (!strcmp(engine, "glesv1_cm")) {
    glesv1_cm_gears_init(win_width, win_height);
  }
  #endif
  #if defined(GLESV2)
  if (!strcmp(engine, "glesv2")) {
    glesv2_gears_init(win_width, win_height);
  }
  #endif
}

static void draw()
{
  struct timeval tv;

  if (animate && !frames) {
    gettimeofday(&tv, NULL);
    t_rate = t_rot = tv.tv_sec * 1000 + tv.tv_usec / 1000;
  }

  #if defined(GL)
  if (!strcmp(engine, "gl")) {
    gl_gears_draw(view_tz, view_rx, view_ry, model_rz);
  }
  #endif
  #if defined(GLESV1_CM)
  if (!strcmp(engine, "glesv1_cm")) {
    glesv1_cm_gears_draw(view_tz, view_rx, view_ry, model_rz);
  }
  #endif
  #if defined(GLESV2)
  if (!strcmp(engine, "glesv2")) {
    glesv2_gears_draw(view_tz, view_rx, view_ry, model_rz);
  }
  #endif

  if (animate) {
    frames++;
  }
}

static void term()
{
  #if defined(GL)
  if (!strcmp(engine, "gl")) {
    gl_gears_term();
  }
  #endif
  #if defined(GLESV1_CM)
  if (!strcmp(engine, "glesv1_cm")) {
    glesv1_cm_gears_term();
  }
  #endif
  #if defined(GLESV2)
  if (!strcmp(engine, "glesv2")) {
    glesv2_gears_term();
  }
  #endif
}

/******************************************************************************/

static void idle(void *widget)
{
  int t;
  struct timeval tv;

  if (animate) {
    if (!frames) {
      return;
    }

    gettimeofday(&tv, NULL);
    t = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    if (t - t_rate >= 2000) {
      loop++;
      fps += frames * 1000.0 / (t - t_rate);
      t_rate = t;
      frames = 0;
    }

    model_rz += 15 * (t - t_rot) / 1000.0;
    model_rz = fmod(model_rz, 360);
    t_rot = t;

    #if defined(EFL)
    if (!strcmp(toolkit, "efl")) {
      elm_glview_changed_set((Evas_Object *)widget);
    }
    #endif
    #if defined(GLUT)
    if (!strcmp(toolkit, "glut")) {
      glutPostRedisplay();
    }
    #endif
    #if defined(GTK)
    if (!strcmp(toolkit, "gtk")) {
      gdk_window_invalidate_rect(((GtkWidget *)widget)->window, &((GtkWidget *)widget)->allocation, FALSE);
    }
    #endif
    #if defined(QT)
    if (!strcmp(toolkit, "qt")) {
      ((QGLWidget *)widget)->updateGL();
    }
    #endif
    #if defined(SDL)
    if (!strcmp(toolkit, "sdl")) {
      SDL_Display();
    }
    #endif
  }
  else {
    if (frames) {
      frames = 0;
    }
  }
}

/******************************************************************************/

#if defined(EFL)
static Ecore_Animator *ecore_animator_id = NULL;

static void elm_glview_init(Evas_Object *object)
{
  init();
}

static void elm_glview_render(Evas_Object *object)
{
  draw();
}

static Eina_Bool ecore_animator(void *data)
{
  idle(data);

  return EINA_TRUE;
}

static Eina_Bool ecore_event_key_down(void *widget, int type, void *event)
{
  if (!strcmp(((Ecore_Event_Key *)event)->keyname, "Escape")) {
    ecore_animator_del(ecore_animator_id);
    term();
    elm_exit();
    return EINA_TRUE;
  }
  else if (!strcmp(((Ecore_Event_Key *)event)->keyname, "space")) {
    animate = !animate;
    if (animate) {
      elm_glview_changed_set((Evas_Object *)widget);
    }
    return EINA_TRUE;
  }
  else if (!strcmp(((Ecore_Event_Key *)event)->keyname, "Next")) {
    view_tz -= -5.0;
  }
  else if (!strcmp(((Ecore_Event_Key *)event)->keyname, "Prior")) {
    view_tz += -5.0;
  }
  else if (!strcmp(((Ecore_Event_Key *)event)->keyname, "Down")) {
    view_rx -= 5.0;
  }
  else if (!strcmp(((Ecore_Event_Key *)event)->keyname, "Up")) {
    view_rx += 5.0;
  }
  else if (!strcmp(((Ecore_Event_Key *)event)->keyname, "Right")) {
    view_ry -= 5.0;
  }
  else if (!strcmp(((Ecore_Event_Key *)event)->keyname, "Left")) {
    view_ry += 5.0;
  }
  else {
    return EINA_FALSE;
  }

  if (!animate) {
    elm_glview_changed_set((Evas_Object *)widget);
  }

  return EINA_TRUE;
}
#endif

#if defined(GLUT)
static int glut_win = 0;

static void glutReshape(int width, int height)
{
  init();
}

static void glutDisplay()
{
  draw();
  glutSwapBuffers();
}

static void glutIdle()
{
  idle(NULL);
}

static void glutKeyboard(unsigned char key, int x, int y)
{
  switch (key) {
    case 27:
      glutIdleFunc(NULL);
      term();
      glutDestroyWindow(glut_win);
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
  switch (key) {
    case GLUT_KEY_PAGE_DOWN:
      view_tz -= -5.0;
      break;
    case GLUT_KEY_PAGE_UP:
      view_tz += -5.0;
      break;
    case GLUT_KEY_DOWN:
      view_rx -= 5.0;
      break;
    case GLUT_KEY_UP:
      view_rx += 5.0;
      break;
    case GLUT_KEY_RIGHT:
      view_ry -= 5.0;
      break;
    case GLUT_KEY_LEFT:
      view_ry += 5.0;
      break;
    default:
      return;
  }

  if (!animate) {
    glutPostRedisplay();
  }
}
#endif

#if defined(GTK)
static guint gtk_idle_id = 0;

static gboolean gtk_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
  gdk_gl_drawable_gl_begin(gtk_widget_get_gl_drawable(widget), gtk_widget_get_gl_context(widget));
  init();

  return TRUE;
}

static gboolean gtk_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  draw();
  gdk_gl_drawable_swap_buffers(gtk_widget_get_gl_drawable(widget));

  return TRUE;
}

static gboolean gtk_idle(gpointer data)
{
  idle(data);

  return TRUE;
}

static gboolean gtk_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  switch (event->keyval) {
    case GDK_Escape:
      g_source_remove(gtk_idle_id);
      term();
      gtk_main_quit();
      return TRUE;
    case SDLK_SPACE:
      animate = !animate;
      if (animate) {
        gdk_window_invalidate_rect(widget->window, &widget->allocation, FALSE);
      }
      return TRUE;
    case GDK_Page_Down:
      view_tz -= -5.0;
      break;
    case GDK_Page_Up:
      view_tz += -5.0;
      break;
    case GDK_Down:
      view_rx -= 5.0;
      break;
    case GDK_Up:
      view_rx += 5.0;
      break;
    case GDK_Right:
      view_ry -= 5.0;
      break;
    case GDK_Left:
      view_ry += 5.0;
      break;
    default:
      return FALSE;
  }

  if (!animate) {
    gdk_window_invalidate_rect(widget->window, &widget->allocation, FALSE);
  }

  return TRUE;
}
#endif

#if defined(QT)
static int qt_timer_id = 0;

class QtGLWidget : public QGLWidget {
void initializeGL()
{
  init();
}

void paintGL()
{
  draw();
}

void timerEvent(QTimerEvent *event)
{
  idle(this);
}

void keyPressEvent(QKeyEvent *event)
{
  switch (event->key()) {
    case Qt::Key_Escape:
      killTimer(qt_timer_id);
      term();
      close();
      return;
    case Qt::Key_Space:
      animate = !animate;
      if (animate) {
        updateGL();
      }
      return;
    case Qt::Key_PageDown:
      view_tz -= -5.0;
      break;
    case Qt::Key_PageUp:
      view_tz += -5.0;
      break;
    case Qt::Key_Down:
      view_rx -= 5.0;
      break;
    case Qt::Key_Up:
      view_rx += 5.0;
      break;
    case Qt::Key_Right:
      view_ry -= 5.0;
      break;
    case Qt::Key_Left:
      view_ry += 5.0;
      break;
    default:
      return;
  }

  if (!animate) {
    updateGL();
  }
}
};
#endif

#if defined(SDL)
static int sdl_idle_id = 1;

static void SDL_Reshape()
{
  init();
}

static void SDL_Display()
{
  draw();
  SDL_GL_SwapBuffers();
}

static void SDL_Idle()
{
  idle(NULL);
}

static void SDL_KeyDownEvent(SDL_Event *event)
{
  switch (event->key.keysym.sym) {
    case SDLK_ESCAPE:
      sdl_idle_id = 0;
      term();
      SDL_Event quit;
      quit.type = SDL_USEREVENT;
      SDL_PushEvent(&quit);
      return;
    case SDLK_SPACE:
      animate = !animate;
      if (animate) {
        SDL_Display();
      }
      return;
    case SDLK_PAGEDOWN:
      view_tz -= -5.0;
      break;
    case SDLK_PAGEUP:
      view_tz += -5.0;
      break;
    case SDLK_DOWN:
      view_rx -= 5.0;
      break;
    case SDLK_UP:
      view_rx += 5.0;
      break;
    case SDLK_RIGHT:
      view_ry -= 5.0;
      break;
    case SDLK_LEFT:
      view_ry += 5.0;
      break;
    default:
      return;
  }

  if (!animate) {
    SDL_Display();
  }
}
#endif

/******************************************************************************/

int main(int argc, char *argv[])
{
  char toolkits[64], *toolkit_arg = NULL, engines[64], *engine_arg = NULL, *c;
  int opt;

  /* process command line */

  memset(toolkits, 0, sizeof(toolkits));
  #if defined(EFL)
  strcat(toolkits, "efl ");
  #endif
  #if defined(GLUT)
  strcat(toolkits, "glut ");
  #endif
  #if defined(GTK)
  strcat(toolkits, "gtk ");
  #endif
  #if defined(QT)
  strcat(toolkits, "qt ");
  #endif
  #if defined(SDL)
  strcat(toolkits, "sdl ");
  #endif

  memset(engines, 0, sizeof(engines));
  #if defined(GL)
  strcat(engines, "gl ");
  #endif
  #if defined(GLESV1_CM)
  strcat(engines, "glesv1_cm ");
  #endif
  #if defined(GLESV2)
  strcat(engines, "glesv2 ");
  #endif

  while ((opt = getopt(argc, argv, "t:e:h")) != -1) {
    switch (opt) {
      case 't':
        toolkit_arg = optarg;
        break;
      case 'e':
        engine_arg = optarg;
        break;
      case 'h':
      default:
        break;
    }
  }

  if (argc != 5 || !toolkit_arg || !engine_arg) {
    printf("\n\tUsage: %s -t toolkit -e engine\n\n", argv[0]);
    printf("\t\ttoolkits: %s\n\n", toolkits);
    printf("\t\tengines:  %s\n\n", engines);
    return EXIT_FAILURE;
  }

  toolkit = toolkits;
  while ((c = strchr(toolkit, ' '))) {
    *c = '\0';
    if (!strcmp(toolkit, toolkit_arg))
      break;
    else
      toolkit = c + 1;
  }

  if (!c) {
    printf("%s: toolkit unknown\n", toolkit_arg);
    return EXIT_FAILURE;
  }

  engine = engines;
  while ((c = strchr(engine, ' '))) {
    *c = '\0';
    if (!strcmp(engine, engine_arg))
      break;
    else
      engine = c + 1;
  }

  if (!c) {
    printf("%s: engine unknown\n", engine_arg);
    return EXIT_FAILURE;
  }

  argc = 1;

  /* EFL toolkit */

  #if defined(EFL)
  if (!strcmp(toolkit, "efl")) {
    elm_init(argc, argv);

    Ecore_Evas *ee = ecore_evas_new(NULL, 0, 0, 0, 0, NULL);
    ecore_evas_screen_geometry_get(ee, 0, 0, &win_width, &win_height);
    ecore_evas_free(ee);

    Evas_Object *elm_win = elm_win_add(NULL, NULL, ELM_WIN_BASIC);
    Evas_Object *elm_glview = elm_glview_add(elm_win);
    elm_glview_mode_set(elm_glview, ELM_GLVIEW_DEPTH);
    elm_glview_init_func_set(elm_glview, elm_glview_init);
    elm_glview_render_func_set(elm_glview, elm_glview_render);
    elm_win_resize_object_add(elm_win, elm_glview);
    ecore_animator_id = ecore_animator_add(ecore_animator, elm_glview);
    ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, ecore_event_key_down, elm_glview);
    evas_object_resize(elm_win, win_width, win_height);
    evas_object_show(elm_glview);
    evas_object_show(elm_win);

    elm_run();
  }
  #endif

  /* GLUT toolkit */

  #if defined(GLUT)
  if (!strcmp(toolkit, "glut")) {
    glutInit(&argc, argv);

    win_width = glutGet(GLUT_SCREEN_WIDTH);
    win_height = glutGet(GLUT_SCREEN_HEIGHT);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(win_width, win_height);
    glut_win = glutCreateWindow(NULL);
    glutReshapeFunc(glutReshape);
    glutDisplayFunc(glutDisplay);
    glutIdleFunc(glutIdle);
    glutKeyboardFunc(glutKeyboard);
    glutSpecialFunc(glutSpecial);

    glutMainLoop();
  }
  #endif

  /* GTK+ toolkit */

  #if defined(GTK)
  if (!strcmp(toolkit, "gtk")) {
    gtk_init(&argc, &argv);

    win_width = gdk_screen_width();
    win_height = gdk_screen_height();

    GtkWidget *gtk_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GdkGLConfig *gdk_glconfig = gdk_gl_config_new_by_mode((GdkGLConfigMode)(GDK_GL_MODE_DOUBLE | GDK_GL_MODE_DEPTH));
    GtkWidget *gtk_drawing_area = gtk_drawing_area_new();
    gtk_widget_set_gl_capability(gtk_drawing_area, gdk_glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE);
    g_signal_connect(gtk_drawing_area, "configure_event", G_CALLBACK(gtk_configure_event), NULL);
    g_signal_connect(gtk_drawing_area, "expose_event", G_CALLBACK(gtk_expose_event), NULL);
    gtk_container_add(GTK_CONTAINER(gtk_win), gtk_drawing_area);
    gtk_idle_id = g_idle_add(gtk_idle, gtk_drawing_area);
    g_signal_connect_swapped(gtk_win, "key_press_event", G_CALLBACK(gtk_key_press_event), gtk_drawing_area);
    gtk_widget_set_size_request(gtk_drawing_area, win_width, win_height);
    gtk_widget_show(gtk_drawing_area);
    gtk_widget_show(gtk_win);

    gtk_main();
  }
  #endif

  /* Qt toolkit */

  #if defined(QT)
  if (!strcmp(toolkit, "qt")) {
    QApplication qt_app(argc, argv);

    win_width = qt_app.desktop()->width();
    win_height = qt_app.desktop()->height();

    QtGLWidget qt_win;
    qt_timer_id = qt_win.startTimer(0);
    qt_win.resize(win_width, win_height);
    qt_win.show();

    qt_app.exec();
  }
  #endif

  /* SDL toolkit */

  #if defined(SDL)
  if (!strcmp(toolkit, "sdl")) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Rect **sdl_mode = SDL_ListModes(NULL, SDL_FULLSCREEN);
    win_width = sdl_mode[0]->w;
    win_height = sdl_mode[0]->h;

    SDL_Surface *sdl_win = SDL_SetVideoMode(win_width, win_height, 0, SDL_OPENGL);
    SDL_Reshape();
    SDL_Display();
    while (1) {
      if (sdl_idle_id)
        SDL_Idle();
      SDL_Event event;
      memset(&event, 0, sizeof(SDL_Event));
      while (SDL_PollEvent(&event))
        if (event.type == SDL_KEYDOWN)
          SDL_KeyDownEvent(&event);
      if (event.type == SDL_USEREVENT)
        break;
    }
  }
  #endif

  /* benchmark */

  if (fps) {
    fps /= loop;
    printf("Gears demo: %.2f fps\n", fps);
  }

  return EXIT_SUCCESS;
}
