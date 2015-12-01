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
#endif

#include "engine.h"

struct list engine_list = LIST_INIT(engine_list);

/******************************************************************************/

static char *toolkit = NULL;
static engine_t *engine = NULL;
static gears_t *gears = NULL;

static int loop = 0, animate = 1, t_rate = 0, t_rot = 0, frames = 0, win_width = 0, win_height = 0, win_posx = 0, win_posy = 0;
static float fps = 0, view_tz = -40.0, view_rx = 20.0, view_ry = 30.0, model_rz = 210.0;

/******************************************************************************/

static int current_time()
{
  struct timeval tv;

  gettimeofday(&tv, NULL);

  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

/******************************************************************************/

static void rotate()
{
  int t;

  t = current_time();

  if (t - t_rate >= 2000) {
    loop++;
    fps += frames * 1000.0 / (t - t_rate);
    t_rate = t;
    frames = 0;
  }

  model_rz += 15 * (t - t_rot) / 1000.0;
  model_rz = fmod(model_rz, 360);

  t_rot = t;
}

/******************************************************************************/

#if defined(EFL)
static void elm_glview_render(Evas_Object *object)
{
  if (animate) { if (frames) rotate(); else t_rate = t_rot = current_time(); }
  engine->draw(gears, view_tz, view_rx, view_ry, model_rz);
  if (animate) frames++;
}

static Eina_Bool ecore_animator(void *data)
{
  if (animate) {
    if (!frames) return EINA_TRUE;
    elm_glview_changed_set((Evas_Object *)data);
  }
  else {
    if (frames) frames = 0;
  }

  return EINA_TRUE;
}

static Eina_Bool ecore_event_key_down(void *widget, int type, void *event)
{
  if (!strcmp(((Ecore_Event_Key *)event)->keyname, "Escape")) {
    ecore_animator_del((Ecore_Animator *)evas_object_data_get((Evas_Object *)widget, "animator"));
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
static void glutDisplay()
{
  if (animate) { if (frames) rotate(); else t_rate = t_rot = current_time(); }
  engine->draw(gears, view_tz, view_rx, view_ry, model_rz);
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
static gboolean gtk_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  if (animate) { if (frames) rotate(); else t_rate = t_rot = current_time(); }
  engine->draw(gears, view_tz, view_rx, view_ry, model_rz);
  if (animate) frames++;
  #ifndef GTKGLEXT_CHECK_VERSION
  gtk_gl_area_swapbuffers(GTK_GL_AREA(widget));
  #else
  gdk_gl_drawable_swap_buffers(gtk_widget_get_gl_drawable(widget));
  #endif

  return TRUE;
}

static gboolean gtk_idle(gpointer data)
{
  if (animate) {
    if (!frames) return TRUE;
    gtk_widget_draw((GtkWidget *)data, NULL);
  }
  else {
    if (frames) frames = 0;
  }

  return TRUE;
}

static gboolean gtk_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  switch (event->keyval) {
    case GDK_Escape:
      g_source_remove(*(guint *)data);
      gtk_main_quit();
      return TRUE;
    case SDLK_SPACE:
      animate = !animate;
      if (animate) {
        gtk_widget_draw(widget, NULL);
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
    gtk_widget_draw(widget, NULL);
  }

  return TRUE;
}
#endif

#if defined(QT)
class QtGLWidget : public QGLWidget {
int qt_timer_id;

void initializeGL()
{
  qt_timer_id = startTimer(0);
}

void paintGL()
{
  if (animate) { if (frames) rotate(); else t_rate = t_rot = current_time(); }
  engine->draw(gears, view_tz, view_rx, view_ry, model_rz);
  if (animate) frames++;
}

void timerEvent(QTimerEvent *event)
{
  if (animate) {
    if (!frames) return;
    updateGL();
  }
  else {
    if (frames) frames = 0;
  }
}

void keyPressEvent(QKeyEvent *event)
{
  switch (event->key()) {
    case Qt::Key_Escape:
      killTimer(qt_timer_id);
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
#if SDL_VERSION_ATLEAST(2, 0, 0)
static void SDL_Display(SDL_Window *window)
#else
static void SDL_Display()
#endif
{
  if (animate) { if (frames) rotate(); else t_rate = t_rot = current_time(); }
  engine->draw(gears, view_tz, view_rx, view_ry, model_rz);
  if (animate) frames++;
  #if SDL_VERSION_ATLEAST(2, 0, 0)
  SDL_GL_SwapWindow(window);
  #else
  SDL_GL_SwapBuffers();
  #endif
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
static void SDL_Idle(SDL_Window *window)
#else
static void SDL_Idle()
#endif
{
  if (animate) {
    if (!frames) return;
    #if SDL_VERSION_ATLEAST(2, 0, 0)
    SDL_Display(window);
    #else
    SDL_Display();
    #endif
  }
  else {
    if (frames) frames = 0;
  }
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
static void SDL_KeyDownEvent(SDL_Window *window, SDL_Event *event, void *data)
#else
static void SDL_KeyDownEvent(SDL_Event *event, void *data)
#endif
{
  switch (event->key.keysym.sym) {
    case SDLK_ESCAPE:
      *(int *)data = 0;
      SDL_Event quit;
      quit.type = SDL_USEREVENT;
      SDL_PushEvent(&quit);
      return;
    case SDLK_SPACE:
      animate = !animate;
      if (animate) {
        #if SDL_VERSION_ATLEAST(2, 0, 0)
        SDL_Display(window);
        #else
        SDL_Display();
        #endif
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
    #if SDL_VERSION_ATLEAST(2, 0, 0)
    SDL_Display(window);
    #else
    SDL_Display();
    #endif
  }
}
#endif

/******************************************************************************/

int main(int argc, char *argv[])
{
  int ret = EXIT_SUCCESS;
  char toolkits[64], *toolkit_arg = NULL, *engine_arg = NULL, *c;
  struct list *engine_entry = NULL;
  int opt;
  #if defined(EFL)
  Evas_Object *elm_win;
  Ecore_Animator *ecore_animator_id;
  #endif
  #if defined(GLUT)
  int glut_win;
  #endif
  #if defined(GTK)
  GtkWidget *gtk_win;
  guint gtk_idle_id;
  #endif
  #if defined(QT)
  QApplication *qt_app;
  QtGLWidget *qt_win;
  #endif
  #if defined(SDL)
  #if SDL_VERSION_ATLEAST(2, 0, 0)
  SDL_Window *sdl_win;
  #else
  SDL_Surface *sdl_win;
  #endif
  int sdl_idle_id;
  #endif

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
    printf("\t\tengines:  ");
    LIST_FOR_EACH(engine_entry, &engine_list) {
      engine = LIST_ENTRY(engine_entry, engine_t, entry);
      printf("%s ", engine->name);
    }
    printf("\n\n");
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

  LIST_FOR_EACH(engine_entry, &engine_list) {
    engine = LIST_ENTRY(engine_entry, engine_t, entry);
    if (!strcmp(engine->name, engine_arg))
      break;
    engine = NULL;
  }

  if (!engine) {
    printf("%s: engine unknown\n", engine_arg);
    return EXIT_FAILURE;
  }

  argc = 1;

  /* Toolkit init */

  #if defined(EFL)
  if (!strcmp(toolkit, "efl")) {
    elm_init(argc, argv);

    Ecore_Evas *ee = ecore_evas_new(NULL, 0, 0, 0, 0, NULL);
    ecore_evas_screen_geometry_get(ee, 0, 0, &win_width, &win_height);
    ecore_evas_free(ee);
  }
  #endif

  #if defined(GLUT)
  if (!strcmp(toolkit, "glut")) {
    glutInit(&argc, argv);

    win_width = glutGet(GLUT_SCREEN_WIDTH);
    win_height = glutGet(GLUT_SCREEN_HEIGHT);
  }
  #endif

  #if defined(GTK)
  if (!strcmp(toolkit, "gtk")) {
    gtk_init(&argc, &argv);

    win_width = gdk_screen_width();
    win_height = gdk_screen_height();
  }
  #endif

  #if defined(QT)
  if (!strcmp(toolkit, "qt")) {
    qt_app = new QApplication(argc, argv);

    win_width = qt_app->desktop()->width();
    win_height = qt_app->desktop()->height();
  }
  #endif

  #if defined(SDL)
  if (!strcmp(toolkit, "sdl")) {
    SDL_Init(SDL_INIT_VIDEO);

    #if SDL_VERSION_ATLEAST(2, 0, 0)
    SDL_DisplayMode sdl_mode;
    SDL_GetDisplayMode(0, 0, &sdl_mode);
    win_width = sdl_mode.w;
    win_height = sdl_mode.h;
    #else
    SDL_Rect **sdl_mode = SDL_ListModes(NULL, SDL_FULLSCREEN);
    win_width = sdl_mode[0]->w;
    win_height = sdl_mode[0]->h;
    #endif
  }
  #endif

  if (getenv("WIDTH")) {
    win_width = atoi(getenv("WIDTH"));
  }

  if (getenv("HEIGHT")) {
    win_height = atoi(getenv("HEIGHT"));
  }

  if (getenv("POSX")) {
    win_posx = atoi(getenv("POSX"));
  }

  if (getenv("POSY")) {
    win_posy = atoi(getenv("POSY"));
  }

  /* Toolkit window */

  #if defined(EFL)
  if (!strcmp(toolkit, "efl")) {
    elm_win = elm_win_add(NULL, NULL, ELM_WIN_BASIC);
    Evas_Object *elm_glview = elm_glview_add(elm_win);
    elm_glview_mode_set(elm_glview, ELM_GLVIEW_DEPTH);
    elm_glview_render_func_set(elm_glview, elm_glview_render);
    elm_win_resize_object_add(elm_win, elm_glview);
    ecore_animator_id = ecore_animator_add(ecore_animator, elm_glview);
    evas_object_data_set(elm_glview, "animator", ecore_animator_id);
    ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, ecore_event_key_down, elm_glview);
    evas_object_resize(elm_win, win_width, win_height);
    evas_object_move(elm_win, win_posx, win_posy);
    evas_object_show(elm_glview);
    evas_object_show(elm_win);
  }
  #endif

  #if defined(GLUT)
  if (!strcmp(toolkit, "glut")) {
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(win_width, win_height);
    glutInitWindowPosition(win_posx, win_posy);
    glut_win = glutCreateWindow(NULL);
    glutDisplayFunc(glutDisplay);
    glutIdleFunc(glutIdle);
    glutKeyboardFunc(glutKeyboard);
    glutSpecialFunc(glutSpecial);
  }
  #endif

  #if defined(GTK)
  if (!strcmp(toolkit, "gtk")) {
    gtk_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *gtk_drawing_area;
    #ifndef GTKGLEXT_CHECK_VERSION
    int gdk_glconfig[] = { GDK_GL_RGBA, GDK_GL_DOUBLEBUFFER, GDK_GL_DEPTH_SIZE,1, GDK_GL_NONE };
    gtk_drawing_area = gtk_gl_area_new(gdk_glconfig);
    #else
    GdkGLConfig *gdk_glconfig = gdk_gl_config_new_by_mode((GdkGLConfigMode)(GDK_GL_MODE_DOUBLE | GDK_GL_MODE_DEPTH));
    gtk_drawing_area = gtk_drawing_area_new();
    gtk_widget_set_gl_capability(gtk_drawing_area, gdk_glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE);
    #endif
    g_signal_connect(gtk_drawing_area, "expose_event", G_CALLBACK(gtk_expose_event), NULL);
    gtk_container_add(GTK_CONTAINER(gtk_win), gtk_drawing_area);
    gtk_idle_id = g_idle_add(gtk_idle, gtk_drawing_area);
    g_signal_connect(gtk_win, "key_press_event", G_CALLBACK(gtk_key_press_event), &gtk_idle_id);
    gtk_window_set_default_size(GTK_WINDOW(gtk_win), win_width, win_height);
    gtk_window_move(GTK_WINDOW(gtk_win), win_posx, win_posy);
    gtk_widget_show(gtk_drawing_area);
    gtk_widget_show(gtk_win);
    #ifndef GTKGLEXT_CHECK_VERSION
    gtk_gl_area_make_current(GTK_GL_AREA(gtk_drawing_area));
    #else
    gdk_gl_drawable_gl_begin(gtk_widget_get_gl_drawable(gtk_drawing_area), gtk_widget_get_gl_context(gtk_drawing_area));
    #endif
  }
  #endif

  #if defined(QT)
  if (!strcmp(toolkit, "qt")) {
    qt_win = new QtGLWidget;
    qt_win->resize(win_width, win_height);
    qt_win->move(win_posx, win_posy);
    qt_win->show();
  }
  #endif

  #if defined(SDL)
  if (!strcmp(toolkit, "sdl")) {
    #if SDL_VERSION_ATLEAST(2, 0, 0)
    sdl_win = SDL_CreateWindow(NULL, win_posx, win_posy, win_width, win_height, SDL_WINDOW_OPENGL);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, engine->version);
    SDL_GL_CreateContext(sdl_win);
    #else
    char sdl_variable[64];
    sprintf(sdl_variable, "SDL_VIDEO_WINDOW_POS=%d,%d", win_posx, win_posy);
    SDL_putenv(sdl_variable);
    sdl_win = SDL_SetVideoMode(win_width, win_height, 0, SDL_OPENGL);
    #endif
    sdl_idle_id = 1;
  }
  #endif

  /* drawing (main event loop) */

  gears = engine->init(win_width, win_height);
  if (!gears) {
    ret = EXIT_FAILURE;
    goto out;
  }

  if (getenv("NO_ANIM")) {
    animate = 0;
  }

  #if defined(EFL)
  if (!strcmp(toolkit, "efl")) {
    elm_run();
  }
  #endif

  #if defined(GLUT)
  if (!strcmp(toolkit, "glut")) {
    glutMainLoop();
  }
  #endif

  #if defined(GTK)
  if (!strcmp(toolkit, "gtk")) {
    gtk_main();
  }
  #endif

  #if defined(QT)
  if (!strcmp(toolkit, "qt")) {
    qt_app->exec();
  }
  #endif

  #if defined(SDL)
  if (!strcmp(toolkit, "sdl")) {
    #if SDL_VERSION_ATLEAST(2, 0, 0)
    SDL_Display(sdl_win);
    #else
    SDL_Display();
    #endif
    while (1) {
      if (sdl_idle_id)
        #if SDL_VERSION_ATLEAST(2, 0, 0)
        SDL_Idle(sdl_win);
        #else
        SDL_Idle();
        #endif
      SDL_Event event;
      memset(&event, 0, sizeof(SDL_Event));
      while (SDL_PollEvent(&event))
        if (event.type == SDL_KEYDOWN)
          #if SDL_VERSION_ATLEAST(2, 0, 0)
          SDL_KeyDownEvent(sdl_win, &event, &sdl_idle_id);
          #else
          SDL_KeyDownEvent(&event, &sdl_idle_id);
          #endif
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

  engine->term(gears);

out:

  /* Toolkit term */

  #if defined(EFL)
  if (!strcmp(toolkit, "efl")) {
    evas_object_del(elm_win);
    elm_shutdown();
  }
  #endif

  #if defined(GLUT)
  if (!strcmp(toolkit, "glut")) {
    glutDestroyWindow(glut_win);
    glutExit();
  }
  #endif

  #if defined(GTK)
  if (!strcmp(toolkit, "gtk")) {
    gtk_widget_destroy(gtk_win);
    gdk_display_close(gdk_display_get_default());
  }
  #endif

  #if defined(QT)
  if (!strcmp(toolkit, "qt")) {
    delete qt_win;
    delete qt_app;
  }
  #endif

  #if defined(SDL)
  if (!strcmp(toolkit, "sdl")) {
    #if SDL_VERSION_ATLEAST(2, 0, 0)
    SDL_DestroyWindow(sdl_win);
    #else
    SDL_FreeSurface(sdl_win);
    #endif
    SDL_Quit();
  }
  #endif

  return ret;
}
