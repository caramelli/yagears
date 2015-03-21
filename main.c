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
#include <sys/time.h>

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
#include <unistd.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#endif
#if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV)
#include <EGL/egl.h>
#endif

extern void init(int winWidth, int winHeight);
extern void draw(float tZ, float rX, float rY, float rz);
extern void fini();

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
  char backends[64], *backend = NULL, *c;
  float seconds, tZ = -40.0, rX = 20.0, rY = 30.0, rz = 0.0;
  int winWidth = 0, winHeight = 0, winDepth = 0, frames = 0;
  struct timeval last, now;

  #if defined(GL_X11) || defined(EGL_X11)
  Display *x11_dpy = NULL;
  Window x11_win = 0;
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
  #endif
  #if defined(GL_DIRECTFB)
  IDirectFBGL *dfb_ctx = NULL;
  #endif
  #if defined(GL_FBDEV) || defined(EGL_FBDEV)
  int fb_dpy = 0;
  struct fb_fix_screeninfo fb_finfo;
  struct fb_var_screeninfo fb_vinfo;
  void *fb_win = NULL;
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
  EGLint egl_attr[3];
  EGLint egl_nconfigs = 0;
  EGLConfig egl_config = NULL;
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

  if (argc != 2) {
    printf("\n\tUsage: %s backend\n\n", argv[0]);
    printf("\t\t%s\n\n", backends);
    return EXIT_FAILURE;
  }

  backend = backends;
  while ((c = strchr(backend, ' '))) {
    *c = '\0';
    if (!strcmp(backend, argv[1]))
      break;
    else
      backend = c + 1;
  }

  if (!c) {
    printf("%s backend unknown\n", argv[1]);
    return EXIT_FAILURE;
  }

  /* open display */

  #if defined(GL_X11) || defined(EGL_X11)
  if (!strcmp(backend, "gl-x11") || !strcmp(backend, "egl-x11")) {
    x11_dpy = XOpenDisplay(NULL);
    winWidth = DisplayWidth(x11_dpy, 0);
    winHeight = DisplayHeight(x11_dpy, 0);
    winDepth = DisplayPlanes(x11_dpy, 0);
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
    winWidth = dfb_layer_config.width;
    winHeight = dfb_layer_config.height;
    winDepth = DFB_BITS_PER_PIXEL(dfb_layer_config.pixelformat);
  }
  #endif
  #if defined(GL_FBDEV) || defined(EGL_FBDEV)
  if (!strcmp(backend, "gl-fbdev") || !strcmp(backend, "egl-fbdev")) {
    fb_dpy = open("/dev/fb0", O_RDWR);
    memset(&fb_finfo, 0, sizeof(struct fb_fix_screeninfo));
    ioctl(fb_dpy, FBIOGET_FSCREENINFO, &fb_finfo);
    memset(&fb_vinfo, 0, sizeof(struct fb_var_screeninfo));
    ioctl(fb_dpy, FBIOGET_VSCREENINFO, &fb_vinfo);
    winWidth = fb_vinfo.xres;
    winHeight = fb_vinfo.yres;
    winDepth = fb_vinfo.bits_per_pixel;
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
    x11_attr[3] = winDepth;
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
    fb_attr[2] = winDepth;
    fb_attr[3] = GLFBDEV_NONE;
    fb_visual = glFBDevCreateVisual(&fb_finfo, &fb_vinfo, fb_attr);
  }
  #endif

  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV)
  if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev")) {
    eglInitialize(egl_dpy, NULL, NULL);
    eglBindAPI(EGL_OPENGL_API);
    egl_attr[0] = EGL_DEPTH_SIZE;
    egl_attr[1] = winDepth;
    egl_attr[2] = EGL_NONE;
    eglChooseConfig(egl_dpy, egl_attr, &egl_config, 1, &egl_nconfigs);
  }
  #endif

  /* create window associated to the display */

  #if defined(GL_X11) || defined(EGL_X11)
  if (!strcmp(backend, "gl-x11") || !strcmp(backend, "egl-x11")) {
    x11_win = XCreateSimpleWindow(x11_dpy, DefaultRootWindow(x11_dpy), 0, 0, winWidth, winHeight, 0, 0, 0);
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
    fb_buffer = mmap(NULL, winWidth * winHeight * winDepth >> 3, PROT_WRITE, MAP_SHARED, fb_dpy, 0);
    fb_win = glFBDevCreateBuffer(&fb_finfo, &fb_vinfo, fb_visual, fb_buffer, NULL, winWidth * winHeight * winDepth >> 3);
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
    egl_ctx = eglCreateContext(egl_dpy, egl_config, EGL_NO_CONTEXT, NULL);
    eglMakeCurrent(egl_dpy, egl_win, egl_win, egl_ctx);
  }
  #endif

  /* drawing */

  init(winWidth, winHeight);

  signal(SIGINT, sighandler);

  gettimeofday(&last, NULL);

  while (loop) {
    draw(tZ, rX, rY, rz);

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
    seconds = (now.tv_sec - last.tv_sec) + (now.tv_usec - last.tv_usec) / 1000000.0;
    if (seconds >= 2) {
      loop++;
      fps += frames / seconds;
      last = now;
      frames = 0;
    }

    rz += 1.0;
  }

  fini();

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
    munmap(fb_buffer, winWidth * winHeight * winDepth >> 3);
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
