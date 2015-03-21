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

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "config.h"

#if defined(GL_X11)
#include <GL/glx.h>
#endif
#if defined(GL_DIRECTFB)
#include <directfbgl.h>
#endif
#if defined(GL_FBDEV)
#include <GL/glfbdev.h>
#endif

#if defined(EGL_X11)
#include <X11/Xlib.h>
#endif
#if defined(EGL_DIRECTFB)
#include <directfb.h>
#endif
#if defined(EGL_FBDEV)
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#endif
#if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV)
#include <EGL/egl.h>
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

static int loop = 1;
static float fps = 0;

static void sighandler(int signum)
{
  if (fps) {
    fps /= (loop - 1);
    printf("%.2f fps\n", fps);
  }

  loop = 0;
}

int main(int argc, char *argv[])
{
  char backends[64], *backend = NULL, *backend_arg = NULL, engines[64], *engine = NULL, *engine_arg = NULL, *c;
  float seconds, tZ = -40.0, rX = 20.0, rY = 30.0, rz = 0.0;
  int opt, win_width = 0, win_height = 0, win_depth = 0, frames = 0;
  struct timeval last, now;

  #if defined(GL_X11) || defined(EGL_X11)
  Display *x11_dpy = NULL;
  Window x11_win = 0;
  XEvent x11_event;
  #endif
  #if defined(GL_X11)
  XVisualInfo *x11_visual = NULL;
  int x11_attr[5];
  GLXContext x11_ctx = NULL;
  #endif
  #if defined(GL_DIRECTFB) || defined(EGL_DIRECTFB)
  IDirectFB *dfb_dpy = NULL;
  IDirectFBDisplayLayer *dfb_layer = NULL;
  DFBDisplayLayerConfig dfb_layer_config;
  IDirectFBSurface *dfb_win = NULL;
  DFBSurfaceDescription dfb_attr;
  IDirectFBEventBuffer *dfb_event_buffer = NULL;
  DFBInputEvent dfb_event;
  #endif
  #if defined(GL_DIRECTFB)
  IDirectFBGL *dfb_ctx = NULL;
  #endif
  #if defined(GL_FBDEV) || defined(EGL_FBDEV)
  int fb_dpy = 0;
  struct fb_fix_screeninfo fb_finfo;
  struct fb_var_screeninfo fb_vinfo;
  void *fb_win = NULL;
  int fb_input = 0;
  struct input_event fb_event;
  #endif
  #if defined(GL_FBDEV)
  void *fb_buffer = NULL;
  GLFBDevVisualPtr fb_visual = NULL;
  int fb_attr[4];
  GLFBDevContextPtr fb_ctx = NULL;
  #endif

  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV)
  EGLDisplay egl_dpy = NULL;
  EGLSurface egl_win = NULL;
  EGLint egl_config_attr[3];
  EGLint egl_nconfigs = 0;
  EGLConfig egl_config = NULL;
  EGLint egl_ctx_attr[1];
  EGLContext egl_ctx = NULL;
  #endif

  /* process command line */

  memset(backends, 0, sizeof(backends));
  #if defined(GL_X11)
  strcat(backends, "gl-x11 ");
  #endif
  #if defined(GL_DIRECTFB)
  strcat(backends, "gl-directfb ");
  #endif
  #if defined(GL_FBDEV)
  strcat(backends, "gl-fbdev ");
  #endif
  #if defined(EGL_X11)
  strcat(backends, "egl-x11 ");
  #endif
  #if defined(EGL_DIRECTFB)
  strcat(backends, "egl-directfb ");
  #endif
  #if defined(EGL_FBDEV)
  strcat(backends, "egl-fbdev ");
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

  while ((opt = getopt(argc, argv, "b:e:")) != -1) {
    switch (opt) {
      case 'b':
        backend_arg = optarg;
        break;
      case 'e':
        engine_arg = optarg;
        break;
      default:
        break;
    }
  }

  if (argc != 5) {
    printf("\n\tUsage: %s -b backend -e engine\n\n", argv[0]);
    printf("\t\tbackends: %s\n\n", backends);
    printf("\t\tengines:  %s\n\n", engines);
    return EXIT_FAILURE;
  }

  backend = backends;
  while ((c = strchr(backend, ' '))) {
    *c = '\0';
    if (!strcmp(backend, backend_arg))
      break;
    else
      backend = c + 1;
  }

  if (!c) {
    printf("%s: backend unknown\n", backend_arg);
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

  /* open display */

  #if defined(GL_X11) || defined(EGL_X11)
  if (!strcmp(backend, "gl-x11") || !strcmp(backend, "egl-x11")) {
    x11_dpy = XOpenDisplay(NULL);
    win_width = DisplayWidth(x11_dpy, 0);
    win_height = DisplayHeight(x11_dpy, 0);
    win_depth = DisplayPlanes(x11_dpy, 0);
  }
  #endif
  #if defined(GL_DIRECTFB) || defined(EGL_DIRECTFB)
  if (!strcmp(backend, "gl-directfb") || !strcmp(backend, "egl-directfb")) {
    DirectFBInit(NULL, NULL);
    DirectFBCreate(&dfb_dpy);
    dfb_dpy->SetCooperativeLevel(dfb_dpy, DFSCL_FULLSCREEN);
    dfb_dpy->GetDisplayLayer(dfb_dpy, DLID_PRIMARY, &dfb_layer);
    memset(&dfb_layer_config, 0, sizeof(DFBDisplayLayerConfig));
    dfb_layer->GetConfiguration(dfb_layer, &dfb_layer_config);
    win_width = dfb_layer_config.width;
    win_height = dfb_layer_config.height;
    win_depth = DFB_BITS_PER_PIXEL(dfb_layer_config.pixelformat);
  }
  #endif
  #if defined(GL_FBDEV) || defined(EGL_FBDEV)
  if (!strcmp(backend, "gl-fbdev") || !strcmp(backend, "egl-fbdev")) {
    if (getenv("FRAMEBUFFER")) {
      fb_dpy = open(getenv("FRAMEBUFFER"), O_RDWR);
    }
    else {
      fb_dpy = open("/dev/fb0", O_RDWR);
    }
    memset(&fb_finfo, 0, sizeof(struct fb_fix_screeninfo));
    ioctl(fb_dpy, FBIOGET_FSCREENINFO, &fb_finfo);
    memset(&fb_vinfo, 0, sizeof(struct fb_var_screeninfo));
    ioctl(fb_dpy, FBIOGET_VSCREENINFO, &fb_vinfo);
    win_width = fb_vinfo.xres;
    win_height = fb_vinfo.yres;
    win_depth = fb_vinfo.bits_per_pixel;
  }
  #endif

  #if defined(EGL_X11)
  if (!strcmp(backend, "egl-x11")) {
    setenv("EGL_PLATFORM", "x11", 1);
    egl_dpy = eglGetDisplay((EGLNativeDisplayType)x11_dpy);
  }
  #endif
  #if defined(EGL_DIRECTFB)
  if (!strcmp(backend, "egl-directfb")) {
    setenv("EGL_PLATFORM", "directfb", 1);
    egl_dpy = eglGetDisplay((EGLNativeDisplayType)dfb_dpy);
  }
  #endif
  #if defined(EGL_FBDEV)
  if (!strcmp(backend, "egl-fbdev")) {
    setenv("EGL_PLATFORM", "fbdev", 1);
    egl_dpy = eglGetDisplay((EGLNativeDisplayType)fb_dpy);
  }
  #endif

  /* set attributes */

  #if defined(GL_X11)
  if (!strcmp(backend, "gl-x11")) {
    x11_attr[0] = GLX_RGBA;
    x11_attr[1] = GLX_DOUBLEBUFFER;
    x11_attr[2] = GLX_DEPTH_SIZE;
    x11_attr[3] = win_depth;
    x11_attr[4] = None;
    x11_visual = glXChooseVisual(x11_dpy, 0, x11_attr);
  }
  #endif
  #if defined(GL_DIRECTFB) || defined(EGL_DIRECTFB)
  if (!strcmp(backend, "gl-directfb") || !strcmp(backend, "egl-directfb")) {
    memset(&dfb_attr, 0, sizeof(DFBSurfaceDescription));
    dfb_attr.flags = DSDESC_CAPS;
    dfb_attr.caps = DSCAPS_PRIMARY | DSCAPS_DOUBLE;
  }
  #endif
  #if defined(GL_FBDEV)
  if (!strcmp(backend, "gl-fbdev")) {
    fb_attr[0] = GLFBDEV_DOUBLE_BUFFER;
    fb_attr[1] = GLFBDEV_DEPTH_SIZE;
    fb_attr[2] = win_depth;
    fb_attr[3] = GLFBDEV_NONE;
    fb_visual = glFBDevCreateVisual(&fb_finfo, &fb_vinfo, fb_attr);
  }
  #endif

  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV)
  if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev")) {
    eglInitialize(egl_dpy, NULL, NULL);
    eglBindAPI(EGL_OPENGL_API);
    egl_config_attr[0] = EGL_DEPTH_SIZE;
    egl_config_attr[1] = win_depth;
    egl_config_attr[2] = EGL_NONE;
    eglChooseConfig(egl_dpy, egl_config_attr, &egl_config, 1, &egl_nconfigs);
  }
  #endif

  /* create window associated to the display */

  #if defined(GL_X11) || defined(EGL_X11)
  if (!strcmp(backend, "gl-x11") || !strcmp(backend, "egl-x11")) {
    x11_win = XCreateSimpleWindow(x11_dpy, DefaultRootWindow(x11_dpy), 0, 0, win_width, win_height, 0, 0, 0);
    XMapWindow(x11_dpy, x11_win);
  }
  #endif
  #if defined(GL_DIRECTFB) || defined(EGL_DIRECTFB)
  if (!strcmp(backend, "gl-directfb") || !strcmp(backend, "egl-directfb")) {
    dfb_dpy->CreateSurface(dfb_dpy, &dfb_attr, &dfb_win);
  }
  #endif
  #if defined(GL_FBDEV)
  if (!strcmp(backend, "gl-fbdev")) {
    fb_buffer = mmap(NULL, win_width * win_height * win_depth >> 3, PROT_WRITE, MAP_SHARED, fb_dpy, 0);
    fb_win = glFBDevCreateBuffer(&fb_finfo, &fb_vinfo, fb_visual, fb_buffer, NULL, win_width * win_height * win_depth >> 3);
  }
  #endif

  #if defined(EGL_X11)
  if (!strcmp(backend, "egl-x11")) {
    egl_win = eglCreateWindowSurface(egl_dpy, egl_config, (EGLNativeWindowType)x11_win, NULL);
  }
  #endif
  #if defined(EGL_DIRECTFB)
  if (!strcmp(backend, "egl-directfb")) {
    egl_win = eglCreateWindowSurface(egl_dpy, egl_config, (EGLNativeWindowType)dfb_win, NULL);
  }
  #endif
  #if defined(EGL_FBDEV)
  if (!strcmp(backend, "egl-fbdev")) {
    egl_win = eglCreateWindowSurface(egl_dpy, egl_config, (EGLNativeWindowType)fb_win, NULL);
  }
  #endif

  /* create context and attach it to the window */

  #if defined(GL_X11)
  if (!strcmp(backend, "gl-x11")) {
    x11_ctx = glXCreateContext(x11_dpy, x11_visual, NULL, True);
    glXMakeCurrent(x11_dpy, x11_win, x11_ctx);
  }
  #endif
  #if defined(GL_DIRECTFB)
  if (!strcmp(backend, "gl-directfb")) {
    dfb_win->GetGL(dfb_win, &dfb_ctx);
    dfb_ctx->Lock(dfb_ctx);
  }
  #endif
  #if defined(GL_FBDEV)
  if (!strcmp(backend, "gl-fbdev")) {
    fb_ctx = glFBDevCreateContext(fb_visual, NULL);
    glFBDevMakeCurrent(fb_ctx, fb_win, fb_win);
  }
  #endif

  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV)
  if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev")) {
    egl_ctx_attr[0] = EGL_NONE;
    egl_ctx = eglCreateContext(egl_dpy, egl_config, EGL_NO_CONTEXT, egl_ctx_attr);
    eglMakeCurrent(egl_dpy, egl_win, egl_win, egl_ctx);
  }
  #endif

  /* initialize input event */

  #if defined(GL_X11) || defined(EGL_X11)
  if (!strcmp(backend, "gl-x11") || !strcmp(backend, "egl-x11")) {
    XSelectInput(x11_dpy, x11_win, KeyPressMask);
  }
  #endif
  #if defined(GL_DIRECTFB) || defined(EGL_DIRECTFB)
  if (!strcmp(backend, "gl-directfb") || !strcmp(backend, "egl-directfb")) {
    dfb_dpy->CreateInputEventBuffer(dfb_dpy, DICAPS_KEYS, DFB_FALSE, &dfb_event_buffer);
  }
  #endif
  #if defined(GL_FBDEV) || defined(EGL_FBDEV)
  if (!strcmp(backend, "gl-fbdev") || !strcmp(backend, "egl-fbdev")) {
    if (getenv("DEVINPUT")) {
      fb_input = open(getenv("DEVINPUT"), O_RDONLY | O_NONBLOCK);
    }
    else {
      fb_input = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
    }
  }
  #endif

  /* drawing */

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

  signal(SIGINT, sighandler);

  gettimeofday(&last, NULL);

  while (loop) {
    #if defined(GL)
    if (!strcmp(engine, "gl")) {
      gl_gears_draw(tZ, rX, rY, rz);
    }
    #endif
    #if defined(GLESV1_CM)
    if (!strcmp(engine, "glesv1_cm")) {
      glesv1_cm_gears_draw(tZ, rX, rY, rz);
    }
    #endif
    #if defined(GLESV2)
    if (!strcmp(engine, "glesv2")) {
      glesv2_gears_draw(tZ, rX, rY, rz);
    }
    #endif

    #if defined(GL_X11)
    if (!strcmp(backend, "gl-x11")) {
      glXSwapBuffers(x11_dpy, x11_win);
    }
    #endif
    #if defined(GL_DIRECTFB)
    if (!strcmp(backend, "gl-directfb")) {
      dfb_win->Flip(dfb_win, NULL, DSFLIP_WAITFORSYNC);
    }
    #endif
    #if defined(GL_FBDEV)
    if (!strcmp(backend, "gl-fbdev")) {
      glFBDevSwapBuffers(fb_win);
    }
    #endif

    #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV)
    if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev")) {
      eglSwapBuffers(egl_dpy, egl_win);
    }
    #endif

    frames++;

    gettimeofday(&now, NULL);
    seconds = (now.tv_sec - last.tv_sec) + (now.tv_usec - last.tv_usec) * 1e-6;
    if (seconds >= 2) {
      loop++;
      fps += frames / seconds;
      last = now;
      frames = 0;
    }

    rz += 1.0;

    #if defined(GL_X11) || defined(EGL_X11)
    if (!strcmp(backend, "gl-x11") || !strcmp(backend, "egl-x11")) {
      if (XPending(x11_dpy)) {
        XNextEvent(x11_dpy, &x11_event);
        switch (XLookupKeysym(&x11_event.xkey, 0)) {
          case XK_Escape:
            sighandler(SIGTERM);
            break;
          case XK_Page_Down:
            tZ -= -5.0;
            break;
          case XK_Page_Up:
            tZ += -5.0;
            break;
          case XK_Down:
            rX -= 5.0;
            break;
          case XK_Up:
            rX += 5.0;
            break;
          case XK_Right:
            rY -= 5.0;
            break;
          case XK_Left:
            rY += 5.0;
            break;
          default:
            break;
        }
      }
    }
    #endif
    #if defined(GL_DIRECTFB) || defined(EGL_DIRECTFB)
    if (!strcmp(backend, "gl-directfb") || !strcmp(backend, "egl-directfb")) {
      if (!dfb_event_buffer->GetEvent(dfb_event_buffer, (DFBEvent *)&dfb_event) && dfb_event.type == DIET_KEYPRESS) {
        switch (dfb_event.key_symbol) {
          case DIKS_ESCAPE:
            sighandler(SIGTERM);
            break;
          case DIKS_PAGE_DOWN:
            tZ -= -5.0;
            break;
          case DIKS_PAGE_UP:
            tZ += -5.0;
            break;
          case DIKS_CURSOR_DOWN:
            rX -= 5.0;
            break;
          case DIKS_CURSOR_UP:
            rX += 5.0;
            break;
          case DIKS_CURSOR_RIGHT:
            rY -= 5.0;
            break;
          case DIKS_CURSOR_LEFT:
            rY += 5.0;
            break;
          default:
            break;
        }
      }
    }
    #endif
    #if defined(GL_FBDEV) || defined(EGL_FBDEV)
    if (!strcmp(backend, "gl-fbdev") || !strcmp(backend, "egl-fbdev")) {
      read(fb_input, &fb_event, sizeof(fb_event));
      if (fb_event.type == EV_KEY && fb_event.value) {
        switch (fb_event.code) {
          case KEY_ESC:
            sighandler(SIGTERM);
            break;
          case KEY_PAGEDOWN:
            tZ -= -5.0;
            break;
          case KEY_PAGEUP:
            tZ += -5.0;
            break;
          case KEY_DOWN:
            rX -= 5.0;
            break;
          case KEY_UP:
            rX += 5.0;
            break;
          case KEY_RIGHT:
            rY -= 5.0;
            break;
          case KEY_LEFT:
            rY += 5.0;
            break;
          default:
            break;
        }
      }
    }
    #endif
  }

  #if defined(GL)
  if (!strcmp(engine, "gl")) {
    gl_gears_free();
  }
  #endif
  #if defined(GLESV1_CM)
  if (!strcmp(engine, "glesv1_cm")) {
    glesv1_cm_gears_free();
  }
  #endif
  #if defined(GLESV2)
  if (!strcmp(engine, "glesv2")) {
    glesv2_gears_free();
  }
  #endif

  /* release input event */

  #if defined(GL_X11) || defined(EGL_X11)
  if (!strcmp(backend, "gl-x11") || !strcmp(backend, "egl-x11")) {
    XSelectInput(x11_dpy, x11_win, NoEventMask);
  }
  #endif
  #if defined(GL_DIRECTFB) || defined(EGL_DIRECTFB)
  if (!strcmp(backend, "gl-directfb") || !strcmp(backend, "egl-directfb")) {
    dfb_event_buffer->Release(dfb_event_buffer);
  }
  #endif
  #if defined(GL_FBDEV) || defined(EGL_FBDEV)
  if (!strcmp(backend, "gl-fbdev") || !strcmp(backend, "egl-fbdev")) {
    close(fb_input);
  }
  #endif

  /* destroy context */

  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV)
  if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev")) {
    eglMakeCurrent(egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(egl_dpy, egl_ctx);
  }
  #endif

  #if defined(GL_X11)
  if (!strcmp(backend, "gl-x11")) {
    glXMakeCurrent(x11_dpy, None, NULL);
    glXDestroyContext(x11_dpy, x11_ctx);
  }
  #endif
  #if defined(GL_DIRECTFB)
  if (!strcmp(backend, "gl-directfb")) {
    dfb_ctx->Unlock(dfb_ctx);
    dfb_ctx->Release(dfb_ctx);
  }
  #endif
  #if defined(GL_FBDEV)
  if (!strcmp(backend, "gl-fbdev")) {
    glFBDevMakeCurrent(fb_ctx, NULL, NULL);
    glFBDevDestroyContext(fb_ctx);
  }
  #endif

  /* destroy window and close display */

  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV)
  if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev")) {
    eglDestroySurface(egl_dpy, egl_win);
    eglTerminate(egl_dpy);
  }
  #endif

  #if defined(GL_X11)
  if (!strcmp(backend, "gl-x11")) {
    XFree(x11_visual);
  }
  #endif
  #if defined(GL_X11) || defined(EGL_X11)
  if (!strcmp(backend, "gl-x11") || !strcmp(backend, "egl-x11")) {
    XDestroyWindow(x11_dpy, x11_win);
    XCloseDisplay(x11_dpy);
  }
  #endif
  #if defined(GL_DIRECTFB) || defined(EGL_DIRECTFB)
  if (!strcmp(backend, "gl-directfb") || !strcmp(backend, "egl-directfb")) {
    dfb_win->Release(dfb_win);
    dfb_dpy->Release(dfb_dpy);
  }
  #endif
  #if defined(GL_FBDEV)
  if (!strcmp(backend, "gl-fbdev")) {
    glFBDevDestroyBuffer(fb_win);
    munmap(fb_buffer, win_width * win_height * win_depth >> 3);
    glFBDevDestroyVisual(fb_visual);
  }
  #endif
  #if defined(GL_FBDEV) || defined(EGL_FBDEV)
  if (!strcmp(backend, "gl-fbdev") || !strcmp(backend, "egl-fbdev")) {
    close(fb_dpy);
  }
  #endif

  return EXIT_SUCCESS;
}
