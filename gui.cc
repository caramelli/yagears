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

#include "config.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#if defined(EFL)
#include <Elementary.h>
#endif
#if defined(GLFW)
#include <GLFW/glfw3.h>
#endif
#if defined(GLUT)
#include <GL/glut.h>
extern "C" {
void glutLeaveMainLoop();
void glutExit();
}
#endif
#if defined(GTK)
#include <gtk/gtk.h>
#if !GTK_CHECK_VERSION(3,16,0)
#include <gtkgl/gtkglarea.h>
#endif
#include <gdk/gdkkeysyms.h>
#ifndef GDK_Escape
#include <gdk/gdkkeysyms-compat.h>
#endif
#endif
#if defined(QT)
#include <QGLWidget>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#else
#include <QtGui>
#endif
#endif
#if defined(FLTK)
#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
void fl_open_display();
void fl_close_display();
#endif
#if defined(SDL)
#include <SDL.h>
#endif
#if defined(SFML)
#include <SFML/Graphics.hpp>
#endif
#if defined(WX)
#include <wx/wx.h>
#include <wx/glcanvas.h>
#endif

#include "gears_engine.h"

/******************************************************************************/

static char *toolkit = NULL;
static gears_engine_t *gears_engine = NULL;

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
  gears_engine_draw(gears_engine, view_tz, view_rx, view_ry, model_rz);
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
  if (event) {
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
  }

  if (!animate) {
    elm_glview_changed_set((Evas_Object *)widget);
  }

  return EINA_TRUE;
}
#endif

#if defined(FLTK)
class FlGlWindow : public Fl_Gl_Window {
public:
int idle_id, quit;

FlGlWindow() : Fl_Gl_Window(0, 0, 0, 0)
{
  idle_id = 1;
  quit = 0;
}

private:
void draw()
{
  if (animate) { if (frames) rotate(); else t_rate = t_rot = current_time(); }
  gears_engine_draw(gears_engine, view_tz, view_rx, view_ry, model_rz);
  if (animate) frames++;
}

public:
void idle()
{
  if (animate) {
    if (!frames) return;
    redraw();
  }
  else {
    if (frames) frames = 0;
  }
}

private:
int handle(int event)
{
  if (event != FL_KEYDOWN)
    return 0;

  switch (Fl::event_key()) {
    case FL_Escape:
      idle_id = 0;
      quit = 1;
      return 1;
    case ' ':
      animate = !animate;
      if (animate) {
        redraw();
      }
      return 1;
    case FL_Page_Down:
      view_tz -= -5.0;
      break;
    case FL_Page_Up:
      view_tz += -5.0;
      break;
    case FL_Down:
      view_rx -= 5.0;
      break;
    case FL_Up:
      view_rx += 5.0;
      break;
    case FL_Right:
      view_ry -= 5.0;
      break;
    case FL_Left:
      view_ry += 5.0;
      break;
    default:
      return 0;
  }

  if (!animate) {
    redraw();
  }

  return 1;
}
};
#endif

#if defined(GLFW)
static void glfwDisplay(GLFWwindow *window)
{
  if (animate) { if (frames) rotate(); else t_rate = t_rot = current_time(); }
  gears_engine_draw(gears_engine, view_tz, view_rx, view_ry, model_rz);
  if (animate) frames++;
  glfwSwapBuffers(window);
}

static void glfwIdle(GLFWwindow *window)
{
  if (animate) {
    if (!frames) return;
    glfwDisplay(window);
  }
  else {
    if (frames) frames = 0;
  }
}

static void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if (action != GLFW_PRESS)
    return;

  switch (key) {
    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(window, GL_TRUE);
      return;
    case GLFW_KEY_SPACE:
      animate = !animate;
      if (animate) {
        glfwDisplay(window);
      }
      return;
    case GLFW_KEY_PAGE_DOWN:
      view_tz -= -5.0;
      break;
    case GLFW_KEY_PAGE_UP:
      view_tz += -5.0;
      break;
    case GLFW_KEY_DOWN:
      view_rx -= 5.0;
      break;
    case GLFW_KEY_UP:
      view_rx += 5.0;
      break;
    case GLFW_KEY_RIGHT:
      view_ry -= 5.0;
      break;
    case GLFW_KEY_LEFT:
      view_ry += 5.0;
      break;
    default:
      return;
  }

  if (!animate) {
    glfwDisplay(window);
  }

}
#endif

#if defined(GLUT)
static void glutDisplay()
{
  if (animate) { if (frames) rotate(); else t_rate = t_rot = current_time(); }
  gears_engine_draw(gears_engine, view_tz, view_rx, view_ry, model_rz);
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
static gboolean gtk_render(GtkWidget *widget)
{
  if (animate) { if (frames) rotate(); else t_rate = t_rot = current_time(); }
  gears_engine_draw(gears_engine, view_tz, view_rx, view_ry, model_rz);
  if (animate) frames++;
  #if !GTK_CHECK_VERSION(3,16,0)
  gtk_gl_area_swap_buffers(GTK_GL_AREA(widget));
  #endif

  return TRUE;
}

static gboolean gtk_idle(gpointer data)
{
  if (animate) {
    if (!frames) return TRUE;
    gtk_widget_queue_draw(GTK_WIDGET(data));
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
    case GDK_space:
      animate = !animate;
      if (animate) {
        gtk_widget_queue_draw(widget);
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
    gtk_widget_queue_draw(widget);
  }

  return TRUE;
}
#endif

#if defined(QT)
class QtGLWidget : public QGLWidget {
int timer_id;

void initializeGL()
{
  timer_id = startTimer(0);
}

void paintGL()
{
  if (animate) { if (frames) rotate(); else t_rate = t_rot = current_time(); }
  gears_engine_draw(gears_engine, view_tz, view_rx, view_ry, model_rz);
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
      killTimer(timer_id);
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
  gears_engine_draw(gears_engine, view_tz, view_rx, view_ry, model_rz);
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
  if (event->type != SDL_KEYDOWN)
    return;

  switch (event->key.keysym.sym) {
    case SDLK_ESCAPE:
      *(int *)data = 0;
      SDL_Event sdl_quit;
      sdl_quit.type = SDL_USEREVENT;
      SDL_PushEvent(&sdl_quit);
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

#if defined(SFML)
class SfRenderWindow : public sf::RenderWindow {
public:
int idle_id;

SfRenderWindow(const sf::String& title) : RenderWindow(sf::VideoMode(win_width, win_height), title, sf::Style::Default, sf::ContextSettings(1))
{
  idle_id = 1;
}

void draw()
{
  if (animate) { if (frames) rotate(); else t_rate = t_rot = current_time(); }
  gears_engine_draw(gears_engine, view_tz, view_rx, view_ry, model_rz);
  if (animate) frames++;
  display();
}

void idle()
{
  if (animate) {
    if (!frames) return;
    draw();
  }
  else {
    if (frames) frames = 0;
  }
}

void keyPressedEvent(sf::Event &event)
{
  if (event.type != sf::Event::KeyPressed)
    return;

  switch (event.key.code) {
    case sf::Keyboard::Escape:
      idle_id = 0;
      event.type = sf::Event::Closed;
      return;
    case sf::Keyboard::Space:
      animate = !animate;
      if (animate) {
        draw();
      }
      return;
    case sf::Keyboard::PageDown:
      view_tz -= -5.0;
      break;
    case sf::Keyboard::PageUp:
      view_tz += -5.0;
      break;
    case sf::Keyboard::Down:
      view_rx -= 5.0;
      break;
    case sf::Keyboard::Up:
      view_rx += 5.0;
      break;
    case sf::Keyboard::Right:
      view_ry -= 5.0;
      break;
    case sf::Keyboard::Left:
      view_ry += 5.0;
      break;
    default:
      return;
  }

  if (!animate) {
    draw();
  }
}
};
#endif

#if defined(WX)
class WxGLContext : public wxGLContext {
public:
WxGLContext(wxGLCanvas *canvas) : wxGLContext(canvas)
{
  SetCurrent(*canvas);
}
};

class WxGLCanvas : public wxGLCanvas {
public:
WxGLCanvas(wxWindow *parent, int *glconfig) : wxGLCanvas(parent, (wxGLCanvas*)NULL, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, wxT("GLCanvas"), glconfig), glcontext(NULL), timer_id(this)
{
  timer_id.Start(1);
}

private:
WxGLContext *glcontext;
wxTimer timer_id;

void WxPaintEventHandler(wxPaintEvent &event)
{
  if (animate) { if (frames) rotate(); else t_rate = t_rot = current_time(); }
  gears_engine_draw(gears_engine, view_tz, view_rx, view_ry, model_rz);
  if (animate) frames++;
  if (!glcontext)
    glcontext = new WxGLContext(this);
  SwapBuffers();
}

void WxTimerEventHandler(wxTimerEvent &event)
{
  if (animate) {
    if (!frames) return;
    Refresh();
  }
  else {
    if (frames) frames = 0;
  }
}

void WxKeyEventHandler(wxKeyEvent &event)
{
  switch (event.GetKeyCode()) {
    case WXK_ESCAPE:
      timer_id.Stop();
      wxExit();
      return;
    case WXK_SPACE:
      animate = !animate;
      if (animate) {
        Refresh();
      }
      return;
    case WXK_PAGEDOWN:
      view_tz -= -5.0;
      break;
    case WXK_PAGEUP:
      view_tz += -5.0;
      break;
    case WXK_DOWN:
      view_rx -= 5.0;
      break;
    case WXK_UP:
      view_rx += 5.0;
      break;
    case WXK_RIGHT:
      view_ry -= 5.0;
      break;
    case WXK_LEFT:
      view_ry += 5.0;
      break;
    default:
      return;
  }

  if (!animate) {
    Refresh();
  }
}

DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(WxGLCanvas, wxGLCanvas)
  EVT_PAINT(WxGLCanvas::WxPaintEventHandler)
  EVT_KEY_DOWN(WxGLCanvas::WxKeyEventHandler)
  EVT_TIMER(wxID_ANY, WxGLCanvas::WxTimerEventHandler)
END_EVENT_TABLE()

class WxFrame : public wxFrame {
public:
WxFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title, wxPoint(win_posx, win_posy))
{
  int wx_glconfig[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 1, 0 };
  WxGLCanvas *wx_glcanvas = new WxGLCanvas(this, wx_glconfig);
  SetClientSize(win_width, win_height);
  Show();
}
};

wxAppConsole *WxAppInitializer()
{
  return new wxApp;
}
#endif

/******************************************************************************/

int main(int argc, char *argv[])
{
  int err = 0, ret = EXIT_FAILURE;
  char toolkits[64], *toolkit_arg = NULL, *engine_arg = NULL, *c;
  int opt;
  #if defined(EFL)
  Evas_Object *elm_win;
  Ecore_Animator *ecore_animator_id;
  #endif
  #if defined(FLTK)
  FlGlWindow *fltk_win;
  #endif
  #if defined(GLFW)
  GLFWwindow *glfw_win;
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
  #if defined(SFML)
  sf::VideoMode *sfml_app;
  SfRenderWindow *sfml_win;
  #endif
  #if defined(WX)
  wxAppInitializer *wx_app;
  WxFrame *wx_win;
  #endif

  /* process command line */

  memset(toolkits, 0, sizeof(toolkits));
  #if defined(EFL)
  strcat(toolkits, "efl ");
  #endif
  #if defined(FLTK)
  strcat(toolkits, "fltk ");
  #endif
  #if defined(GLFW)
  strcat(toolkits, "glfw ");
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
  #if defined(SFML)
  strcat(toolkits, "sfml ");
  #endif
  #if defined(WX)
  strcat(toolkits, "wx ");
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
    printf("\n\tUsage: %s -t Toolkit -e Engine\n\n", argv[0]);
    printf("\t\tToolkits: %s\n\n", toolkits);
    printf("\t\tEngines:  ");
    for (opt = 0; opt < gears_engine_nb(); opt++) {
      printf("%s ", gears_engine_name(opt));
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
    printf("%s: Toolkit unknown\n", toolkit_arg);
    return EXIT_FAILURE;
  }

  for (opt = 0; opt < gears_engine_nb(); opt++) {
    if (!strcmp(gears_engine_name(opt), engine_arg))
      break;
  }

  if (opt == gears_engine_nb()) {
    printf("%s: Engine unknown\n", engine_arg);
    return EXIT_FAILURE;
  }

  gears_engine = gears_engine_new(gears_engine_name(opt));
  if (!gears_engine) {
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

  #if defined(FLTK)
  if (!strcmp(toolkit, "fltk")) {
    fl_open_display();

    win_width = Fl::w();
    win_height = Fl::h();
  }
  #endif

  #if defined(GLFW)
  if (!strcmp(toolkit, "glfw")) {
    glfwInit();

    const GLFWvidmode *glfw_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    win_width = glfw_mode->width;
    win_height = glfw_mode->height;
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

  #if defined(SFML)
  if (!strcmp(toolkit, "sfml")) {
    sfml_app = new sf::VideoMode;

    win_width = sfml_app->getDesktopMode().width;
    win_height = sfml_app->getDesktopMode().height;
  }
  #endif

  #if defined(WX)
  if (!strcmp(toolkit, "wx")) {
    wx_app = new wxAppInitializer(WxAppInitializer);
    wxInitialize(argc, argv);

    wxDisplaySize(&win_width, &win_height);
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
    elm_win_title_set(elm_win, "yagears");
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

  #if defined(FLTK)
  if (!strcmp(toolkit, "fltk")) {
    fltk_win = new FlGlWindow;
    fltk_win->label("yagears");
    fltk_win->resize(win_posx, win_posy, win_width, win_height);
    fltk_win->show();
    while (!fltk_win->valid())
      Fl::check();
  }
  #endif

  #if defined(GLFW)
  if (!strcmp(toolkit, "glfw")) {
    glfw_win = glfwCreateWindow(win_width, win_height, "yagears", NULL, NULL);
    glfwSetWindowPos(glfw_win, win_posx, win_posy);
    glfwSetKeyCallback(glfw_win, glfwKeyCallback);
    glfwMakeContextCurrent(glfw_win);
  }
  #endif

  #if defined(GLUT)
  if (!strcmp(toolkit, "glut")) {
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(win_width, win_height);
    glutInitWindowPosition(win_posx, win_posy);
    glut_win = glutCreateWindow("yagears");
    glutDisplayFunc(glutDisplay);
    glutIdleFunc(glutIdle);
    glutKeyboardFunc(glutKeyboard);
    glutSpecialFunc(glutSpecial);
  }
  #endif

  #if defined(GTK)
  if (!strcmp(toolkit, "gtk")) {
    gtk_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(gtk_win), "yagears");
    GtkWidget *gtk_glarea;
    #if GTK_CHECK_VERSION(3,16,0)
    gtk_glarea = gtk_gl_area_new();
    gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(gtk_glarea), TRUE);
    g_signal_connect(gtk_glarea, "render", G_CALLBACK(gtk_render), NULL);
    #else
    int gdk_glconfig[] = { GDK_GL_RGBA, GDK_GL_DOUBLEBUFFER, GDK_GL_DEPTH_SIZE, 1, GDK_GL_NONE };
    gtk_glarea = gtk_gl_area_new(gdk_glconfig);
    #if GTK_CHECK_VERSION(3,0,0)
    g_signal_connect(gtk_glarea, "draw", G_CALLBACK(gtk_render), NULL);
    #else
    g_signal_connect(gtk_glarea, "expose-event", G_CALLBACK(gtk_render), NULL);
    #endif
    #endif
    gtk_widget_set_size_request(gtk_glarea, win_width, win_height);
    gtk_container_add(GTK_CONTAINER(gtk_win), gtk_glarea);
    gtk_idle_id = g_idle_add(gtk_idle, gtk_glarea);
    g_signal_connect(gtk_win, "key-press-event", G_CALLBACK(gtk_key_press_event), &gtk_idle_id);
    gtk_window_set_default_size(GTK_WINDOW(gtk_win), win_width, win_height);
    gtk_window_move(GTK_WINDOW(gtk_win), win_posx, win_posy);
    gtk_widget_show(gtk_glarea);
    gtk_widget_show(gtk_win);
    gtk_gl_area_make_current(GTK_GL_AREA(gtk_glarea));
  }
  #endif

  #if defined(QT)
  if (!strcmp(toolkit, "qt")) {
    qt_win = new QtGLWidget();
    qt_win->setWindowTitle("yagears");
    qt_win->resize(win_width, win_height);
    qt_win->move(win_posx, win_posy);
    qt_win->show();
  }
  #endif

  #if defined(SDL)
  if (!strcmp(toolkit, "sdl")) {
    #if SDL_VERSION_ATLEAST(2, 0, 0)
    sdl_win = SDL_CreateWindow("yagears", win_posx, win_posy, win_width, win_height, SDL_WINDOW_OPENGL);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gears_engine_version(gears_engine));
    SDL_GL_CreateContext(sdl_win);
    #else
    char sdl_variable[64];
    sprintf(sdl_variable, "SDL_VIDEO_WINDOW_POS=%d,%d", win_posx, win_posy);
    SDL_putenv(sdl_variable);
    sdl_win = SDL_SetVideoMode(win_width, win_height, 0, SDL_OPENGL);
    SDL_WM_SetCaption("yagears", NULL);
    #endif
    sdl_idle_id = 1;
  }
  #endif

  #if defined(SFML)
  if (!strcmp(toolkit, "sfml")) {
    sfml_win = new SfRenderWindow("yagears");
    sfml_win->setPosition(sf::Vector2i(win_posx, win_posy));
  }
  #endif

  #if defined(WX)
  if (!strcmp(toolkit, "wx")) {
    wx_win = new WxFrame(wxT("yagears"));
  }
  #endif

  /* drawing (main event loop) */

  err = gears_engine_init(gears_engine, win_width, win_height);
  if (err == -1) {
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

  #if defined(FLTK)
  if (!strcmp(toolkit, "fltk")) {
    fltk_win->redraw();
    Fl::check();
    while (1) {
      if (fltk_win->idle_id)
        fltk_win->idle();
      Fl::check();
      if (fltk_win->quit)
        break;
    }
  }
  #endif

  #if defined(GLFW)
  if (!strcmp(toolkit, "glfw")) {
    glfwDisplay(glfw_win);
    while (!glfwWindowShouldClose(glfw_win))
    {
      glfwIdle(glfw_win);
      glfwPollEvents();
    }
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
      while (SDL_PollEvent(&event))
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

  #if defined(SFML)
  if (!strcmp(toolkit, "sfml")) {
    sfml_win->draw();
    while (1) {
      if (sfml_win->idle_id)
        sfml_win->idle();
      sf::Event event;
      while (sfml_win->pollEvent(event)) {
        sfml_win->keyPressedEvent(event);
        if (event.type == sf::Event::Closed)
          break;
      }
      if (event.type == sf::Event::Closed)
        break;
    }
  }
  #endif

  #if defined(WX)
  if (!strcmp(toolkit, "wx")) {
    wxEntry(argc, argv);
  }
  #endif

  /* benchmark */

  if (fps) {
    fps /= loop;
    printf("Gears demo: %.2f fps\n", fps);
  }

  gears_engine_term(gears_engine);

  ret = EXIT_SUCCESS;

out:

  /* Toolkit term */

  #if defined(EFL)
  if (!strcmp(toolkit, "efl")) {
    evas_object_del(elm_win);
    elm_shutdown();
  }
  #endif

  #if defined(FLTK)
  if (!strcmp(toolkit, "fltk")) {
    delete fltk_win;
    fl_close_display();
  }
  #endif

  #if defined(GLFW)
  if (!strcmp(toolkit, "glfw")) {
    glfwDestroyWindow(glfw_win);
    glfwTerminate();
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

  #if defined(SFML)
  if (!strcmp(toolkit, "sfml")) {
    delete sfml_win;
    delete sfml_app;
  }
  #endif

  #if defined(WX)
  if (!strcmp(toolkit, "wx")) {
    delete wx_win;
    delete wx_app;
  }
  #endif

  gears_engine_free(gears_engine);

  return ret;
}
