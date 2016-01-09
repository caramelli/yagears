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

#include <errno.h>
#include <math.h>
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
#include <X11/Xutil.h>
#endif
#if defined(EGL_DIRECTFB)
#include <directfb.h>
#endif
#if defined(EGL_FBDEV)
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#endif
#if defined(EGL_WAYLAND)
#include <wayland-client.h>
#include <sys/mman.h>
#include <xkbcommon/xkbcommon.h>
#endif
#if defined(EGL_DRM)
#include <fcntl.h>
#include <gbm.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libevdev/libevdev.h>
#endif
#if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_DRM)
#include <EGL/egl.h>
#endif

#include "gears_engine.h"

/******************************************************************************/

static char *backend = NULL;
static gears_engine_t *gears_engine = NULL;

static int loop = 1, animate = 1, redisplay = 1, win_width = 0, win_height = 0, win_posx = 0, win_posy = 0;
static float fps = 0, view_tz = -40.0, view_rx = 20.0, view_ry = 30.0, model_rz = 210.0;

/******************************************************************************/

static void sighandler(int signum)
{
  /* benchmark */

  if (fps) {
    fps /= (loop - 1);
    printf("Gears demo: %.2f fps\n", fps);
  }

  loop = 0;
}

/******************************************************************************/

#if defined(GL_X11) || defined(EGL_X11)
static void x11_keyboard_handle_key(XEvent *event)
{
  switch (XLookupKeysym(&event->xkey, 0)) {
    case XK_Escape:
      sighandler(SIGTERM);
      return;
    case XK_space:
      animate = !animate;
      if (animate) {
        redisplay = 1;
      }
      else {
        redisplay = 0;
      }
      return;
    case XK_Page_Down:
      view_tz -= -5.0;
      break;
    case XK_Page_Up:
      view_tz += -5.0;
      break;
    case XK_Down:
      view_rx -= 5.0;
      break;
    case XK_Up:
      view_rx += 5.0;
      break;
    case XK_Right:
      view_ry -= 5.0;
      break;
    case XK_Left:
      view_ry += 5.0;
      break;
    default:
      return;
  }

  if (!animate) {
    redisplay = 1;
  }
}
#endif

#if defined(GL_DIRECTFB) || defined(EGL_DIRECTFB)
static void dfb_keyboard_handle_key(DFBWindowEvent *event)
{
  switch (event->key_symbol) {
    case DIKS_ESCAPE:
      sighandler(SIGTERM);
      return;
    case DIKS_SPACE:
      animate = !animate;
      if (animate) {
        redisplay = 1;
      }
      else {
        redisplay = 0;
      }
      return;
    case DIKS_PAGE_DOWN:
      view_tz -= -5.0;
      break;
    case DIKS_PAGE_UP:
      view_tz += -5.0;
      break;
    case DIKS_CURSOR_DOWN:
      view_rx -= 5.0;
      break;
    case DIKS_CURSOR_UP:
      view_rx += 5.0;
      break;
    case DIKS_CURSOR_RIGHT:
      view_ry -= 5.0;
      break;
    case DIKS_CURSOR_LEFT:
      view_ry += 5.0;
      break;
    default:
      return;
  }

  if (!animate) {
    redisplay = 1;
  }
}
#endif

#if defined(GL_FBDEV) || defined(EGL_FBDEV)
struct fb_window {
  int width;
  int height;
  int posx;
  int posy;
};

static void fb_keyboard_handle_key(struct input_event *event)
{
  switch (event->code) {
    case KEY_ESC:
      sighandler(SIGTERM);
      return;
    case KEY_SPACE:
      animate = !animate;
      if (animate) {
        redisplay = 1;
      }
      else {
        redisplay = 0;
      }
      return;
    case KEY_PAGEDOWN:
      view_tz -= -5.0;
      break;
    case KEY_PAGEUP:
      view_tz += -5.0;
      break;
    case KEY_DOWN:
      view_rx -= 5.0;
      break;
    case KEY_UP:
      view_rx += 5.0;
      break;
    case KEY_RIGHT:
      view_ry -= 5.0;
      break;
    case KEY_LEFT:
      view_ry += 5.0;
      break;
    default:
      return;
  }

  if (!animate) {
    redisplay = 1;
  }
}
#endif

#if defined(EGL_WAYLAND)
struct wl_window {
  struct wl_surface *surface;
  int width;
  int height;
  int dx;
  int dy;
  int attached_width;
  int attached_height;
};

struct wl_data {
  int width;
  int height;
  struct wl_registry *wl_registry;
  struct wl_output *wl_output;
  struct wl_compositor *wl_compositor;
  struct wl_shell *wl_shell;
  struct wl_seat *wl_seat;
  struct wl_keyboard *wl_keyboard;
  struct xkb_context *xkb_context;
  struct xkb_keymap *xkb_keymap;
  struct xkb_state *xkb_state;
};

static void wl_output_handle_geometry(void *data, struct wl_output *output, int x, int y, int physical_width, int physical_height, int subpixel, const char *make, const char *model, int transform)
{
}

static void wl_output_handle_mode(void *data, struct wl_output *output, unsigned int flags, int width, int height, int refresh)
{
  struct wl_data *wl_data = data;

  wl_data->width = width;
  wl_data->height = height;
}

static void wl_output_handle_done(void *data, struct wl_output *output)
{
}

static void wl_output_handle_scale(void *data, struct wl_output *output, int factor)
{
}

static struct wl_output_listener wl_output_listener = { wl_output_handle_geometry, wl_output_handle_mode, wl_output_handle_done, wl_output_handle_scale };

static void wl_keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard, unsigned int format, int fd, unsigned int size)
{
  struct wl_data *wl_data = data;
  char *string;

  string = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
  wl_data->xkb_keymap = xkb_map_new_from_string(wl_data->xkb_context, string, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_MAP_COMPILE_NO_FLAGS);
  munmap(string, size);
  close(fd);

  wl_data->xkb_state = xkb_state_new(wl_data->xkb_keymap);
}

static void wl_keyboard_handle_enter(void *data, struct wl_keyboard *keyboard, unsigned int serial, struct wl_surface *surface, struct wl_array *keys)
{
}

static void wl_keyboard_handle_leave(void *data, struct wl_keyboard *keyboard, unsigned int serial, struct wl_surface *surface)
{
}

static void wl_keyboard_handle_key(void *data, struct wl_keyboard *keyboard, unsigned int serial, unsigned int time, unsigned int key, unsigned int state)
{
  struct wl_data *wl_data = data;
  const xkb_keysym_t *syms;

  if (state == WL_KEYBOARD_KEY_STATE_RELEASED)
    return;

  xkb_key_get_syms(wl_data->xkb_state, key + 8, &syms);

  switch (syms[0]) {
    case XKB_KEY_Escape:
      sighandler(SIGTERM);
      return;
    case XKB_KEY_space:
      animate = !animate;
      if (animate) {
        redisplay = 1;
      }
      else {
        redisplay = 0;
      }
      return;
    case XKB_KEY_Page_Down:
      view_tz -= -5.0;
      break;
    case XKB_KEY_Page_Up:
      view_tz += -5.0;
      break;
    case XKB_KEY_Down:
      view_rx -= 5.0;
      break;
    case XKB_KEY_Up:
      view_rx += 5.0;
      break;
    case XKB_KEY_Right:
      view_ry -= 5.0;
      break;
    case XKB_KEY_Left:
      view_ry += 5.0;
      break;
    default:
      return;
  }

  if (!animate) {
    redisplay = 1;
  }
}

static void wl_keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard, unsigned int serial, unsigned int mods_depressed, unsigned int mods_latched, unsigned int mods_locked, unsigned int group)
{
}

static struct wl_keyboard_listener wl_keyboard_listener = { wl_keyboard_handle_keymap, wl_keyboard_handle_enter, wl_keyboard_handle_leave, wl_keyboard_handle_key, wl_keyboard_handle_modifiers };

static void wl_seat_handle_capabilities(void *data, struct wl_seat *seat, enum wl_seat_capability caps)
{
  struct wl_data *wl_data = data;

  wl_data->wl_keyboard = wl_seat_get_keyboard(wl_data->wl_seat);
  wl_keyboard_add_listener(wl_data->wl_keyboard, &wl_keyboard_listener, wl_data);
}

static void wl_seat_handle_name(void *data, struct wl_seat *seat, const char *name)
{
}

static struct wl_seat_listener wl_seat_listener = { wl_seat_handle_capabilities, wl_seat_handle_name };

static void wl_registry_handle_global(void *data, struct wl_registry *registry, unsigned int name, const char *interface, unsigned int version)
{
  struct wl_data *wl_data = data;

  if (!strcmp(interface, "wl_output")) {
    wl_data->wl_output = wl_registry_bind(registry, name, &wl_output_interface, 1);
    wl_output_add_listener(wl_data->wl_output, &wl_output_listener, wl_data);
  }
  else if (!strcmp(interface, "wl_compositor")) {
    wl_data->wl_compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 1);
  }
  else if (!strcmp(interface, "wl_shell")) {
    wl_data->wl_shell = wl_registry_bind(registry, name, &wl_shell_interface, 1);
  }
  else if (!strcmp(interface, "wl_seat")) {
    wl_data->wl_seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
    wl_seat_add_listener(wl_data->wl_seat, &wl_seat_listener, wl_data);
  }
}

static void wl_registry_handle_global_remove(void *data, struct wl_registry *registry, unsigned int name)
{
}

static struct wl_registry_listener wl_registry_listener = { wl_registry_handle_global, wl_registry_handle_global_remove };
#endif

#if defined(EGL_DRM)
static void drm_destroy_user_data(struct gbm_bo *bo, void *data)
{
  drmModeRmFB(gbm_device_get_fd(gbm_bo_get_device(bo)), (unsigned int)data);
}

static void drm_keyboard_handle_key(struct input_event *event)
{
  switch (event->code) {
    case KEY_ESC:
      sighandler(SIGTERM);
      return;
    case KEY_SPACE:
      animate = !animate;
      if (animate) {
        redisplay = 1;
      }
      else {
        redisplay = 0;
      }
      return;
    case KEY_PAGEDOWN:
      view_tz -= -5.0;
      break;
    case KEY_PAGEUP:
      view_tz += -5.0;
      break;
    case KEY_DOWN:
      view_rx -= 5.0;
      break;
    case KEY_UP:
      view_rx += 5.0;
      break;
    case KEY_RIGHT:
      view_ry -= 5.0;
      break;
    case KEY_LEFT:
      view_ry += 5.0;
      break;
    default:
      return;
  }

  if (!animate) {
    redisplay = 1;
  }
}
#endif

/******************************************************************************/

int main(int argc, char *argv[])
{
  int err = 0, ret = EXIT_FAILURE;
  char backends[64], *backend_arg = NULL, *engine_arg = NULL, *c;
  int opt, t_rate = 0, t_rot = 0, t, frames = 0;
  struct timeval tv;

  #if defined(GL_X11) || defined(EGL_X11)
  Display *x11_dpy = NULL;
  Window x11_win = 0;
  int x11_event_mask = NoEventMask;
  XEvent x11_event;
  #endif
  #if defined(GL_X11)
  XVisualInfo *x11_visual = NULL;
  int x11_attr[5];
  GLXContext x11_ctx = NULL;
  int glx_major_version = 0, glx_minor_version = 0, glx_depth_size = 0, glx_red_size = 0, glx_green_size = 0, glx_blue_size = 0, glx_alpha_size = 0;
  #endif
  #if defined(GL_DIRECTFB) || defined(EGL_DIRECTFB)
  IDirectFB *dfb_dpy = NULL;
  IDirectFBDisplayLayer *dfb_layer = NULL;
  DFBDisplayLayerConfig dfb_layer_config;
  DFBSurfaceCapabilities dfb_attr = DSCAPS_NONE;
  DFBWindowDescription dfb_desc;
  IDirectFBWindow *dfb_window = NULL;
  IDirectFBSurface *dfb_win = NULL;
  IDirectFBEventBuffer *dfb_event_buffer = NULL;
  DFBWindowEventType dfb_event_mask = DWET_ALL;
  DFBWindowEvent dfb_event;
  #endif
  #if defined(GL_DIRECTFB)
  IDirectFBGL *dfb_ctx = NULL;
  DFBGLAttributes directfbgl;
  #endif
  #if defined(GL_FBDEV) || defined(EGL_FBDEV)
  int fb_dpy = -1;
  struct fb_fix_screeninfo fb_finfo;
  struct fb_var_screeninfo fb_vinfo;
  struct fb_window *fb_win = NULL;
  int fb_keyboard = -1;
  struct input_event fb_event;
  #endif
  #if defined(GL_FBDEV)
  void *fb_addr = NULL;
  GLFBDevVisualPtr fb_visual = NULL;
  GLFBDevBufferPtr fb_buffer = NULL;
  int fb_attr[4];
  GLFBDevContextPtr fb_ctx = NULL;
  int glfbdev_depth_size = 0;
  #endif
  #if defined(EGL_WAYLAND)
  struct wl_display *wl_dpy = NULL;
  struct wl_data wl_data;
  struct wl_surface *wl_surface = NULL;
  struct wl_shell_surface *wl_shell_surface = NULL;
  struct wl_window *wl_win = NULL;
  #endif
  #if defined(EGL_DRM)
  int drm_fd = -1;
  struct gbm_device *drm_dpy = NULL;
  drmModeResPtr drm_resources = NULL;
  drmModeConnectorPtr drm_connector = NULL;
  drmModeEncoderPtr drm_encoder = NULL;
  drmModeCrtcPtr drm_crtc = NULL;
  struct gbm_surface *drm_win = NULL;
  struct gbm_bo *drm_bo = NULL;
  unsigned int drm_fb_id = 0;
  drmEventContext drm_context = { DRM_EVENT_CONTEXT_VERSION, NULL, NULL };
  int drm_keyboard = -1;
  struct libevdev *drm_evdev = NULL;
  struct input_event drm_event;
  #endif

  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_DRM)
  EGLDisplay egl_dpy = NULL;
  EGLSurface egl_win = NULL;
  EGLint egl_config_attr[5];
  EGLint egl_nconfigs = 0;
  EGLConfig *egl_configs = NULL, egl_config = NULL;
  EGLint egl_ctx_attr[3];
  EGLContext egl_ctx = NULL;
  EGLint egl_major_version = 0, egl_minor_version = 0, egl_depth_size = 0, egl_red_size = 0, egl_green_size = 0, egl_blue_size = 0, egl_alpha_size = 0;
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
  #if defined(EGL_WAYLAND)
  strcat(backends, "egl-wayland ");
  #endif
  #if defined(EGL_DRM)
  strcat(backends, "egl-drm ");
  #endif

  while ((opt = getopt(argc, argv, "b:e:h")) != -1) {
    switch (opt) {
      case 'b':
        backend_arg = optarg;
        break;
      case 'e':
        engine_arg = optarg;
        break;
      case 'h':
      default:
        break;
    }
  }

  if (argc != 5 || !backend_arg || !engine_arg) {
    printf("\n\tUsage: %s -b backend -e engine\n\n", argv[0]);
    printf("\t\tbackends: %s\n\n", backends);
    printf("\t\tengines:  ");
    for (opt = 0; opt < gears_engine_nb(); opt++) {
      printf("%s ", gears_engine_name(opt));
    }
    printf("\n\n");
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

  for (opt = 0; opt < gears_engine_nb(); opt++) {
    if (!strcmp(gears_engine_name(opt), engine_arg))
      break;
  }

  if (opt == gears_engine_nb()) {
    printf("%s: engine unknown\n", engine_arg);
    return EXIT_FAILURE;
  }

  gears_engine = gears_engine_new(gears_engine_name(opt));
  if (!gears_engine) {
    return EXIT_FAILURE;
  }

  /* open display */

  #if defined(GL_X11) || defined(EGL_X11)
  if (!strcmp(backend, "gl-x11") || !strcmp(backend, "egl-x11")) {
    x11_dpy = XOpenDisplay(NULL);
    if (!x11_dpy) {
      printf("XOpenDisplay failed\n");
      goto out;
    }

    XCloseDisplay(x11_dpy);
    x11_dpy = XOpenDisplay(NULL);

    win_width = DisplayWidth(x11_dpy, 0);
    win_height = DisplayHeight(x11_dpy, 0);
  }
  #endif
  #if defined(GL_DIRECTFB) || defined(EGL_DIRECTFB)
  if (!strcmp(backend, "gl-directfb") || !strcmp(backend, "egl-directfb")) {
    err = DirectFBInit(NULL, NULL);
    if (err) {
      printf("DirectFBInit failed: %s\n", DirectFBErrorString(err));
      goto out;
    }

    err = DirectFBCreate(&dfb_dpy);
    if (err) {
      printf("DirectFBCreate failed: %s\n", DirectFBErrorString(err));
      goto out;
    }

    err = dfb_dpy->GetDisplayLayer(dfb_dpy, DLID_PRIMARY, &dfb_layer);
    if (err) {
      printf("GetDisplayLayer failed: %s\n", DirectFBErrorString(err));
      goto out;
    }

    memset(&dfb_layer_config, 0, sizeof(DFBDisplayLayerConfig));
    err = dfb_layer->GetConfiguration(dfb_layer, &dfb_layer_config);
    if (err) {
      printf("GetConfiguration failed: %s\n", DirectFBErrorString(err));
      goto out;
    }

    win_width = dfb_layer_config.width;
    win_height = dfb_layer_config.height;
  }
  #endif
  #if defined(GL_FBDEV) || defined(EGL_FBDEV)
  if (!strcmp(backend, "gl-fbdev") || !strcmp(backend, "egl-fbdev")) {
    if (getenv("FRAMEBUFFER")) {
      fb_dpy = open(getenv("FRAMEBUFFER"), O_RDWR);
      if (fb_dpy == -1) {
        printf("open %s failed: %s\n", getenv("FRAMEBUFFER"), strerror(errno));
        goto out;
      }
    }
    else {
      fb_dpy = open("/dev/fb0", O_RDWR);
      if (fb_dpy == -1) {
        printf("open /dev/fb0 failed: %s\n", strerror(errno));
        goto out;
      }
    }

    memset(&fb_finfo, 0, sizeof(struct fb_fix_screeninfo));
    err = ioctl(fb_dpy, FBIOGET_FSCREENINFO, &fb_finfo);
    if (err == -1) {
      printf("ioctl FBIOGET_FSCREENINFO failed: %s\n", strerror(errno));
      goto out;
    }

    memset(&fb_vinfo, 0, sizeof(struct fb_var_screeninfo));
    err = ioctl(fb_dpy, FBIOGET_VSCREENINFO, &fb_vinfo);
    if (err == -1) {
      printf("ioctl FBIOGET_VSCREENINFO failed: %s\n", strerror(errno));
      goto out;
    }

    win_width = fb_vinfo.xres;
    win_height = fb_vinfo.yres;
  }
  #endif
  #if defined(EGL_WAYLAND)
  if (!strcmp(backend, "egl-wayland")) {
    wl_dpy = wl_display_connect(NULL);
    if (!wl_dpy) {
      printf("wl_display_connect failed\n");
      goto out;
    }

    memset(&wl_data, 0, sizeof(struct wl_data));

    wl_data.wl_registry = wl_display_get_registry(wl_dpy);
    if (!wl_data.wl_registry) {
      printf("wl_display_get_registry failed\n");
      goto out;
    }

    err = wl_registry_add_listener(wl_data.wl_registry, &wl_registry_listener, &wl_data);
    if (err == -1) {
      printf("wl_registry_add_listener failed\n");
      goto out;
    }

    err = wl_display_dispatch(wl_dpy);
    if (err == -1) {
      printf("wl_display_dispatch failed\n");
      goto out;
    }

    err = wl_display_roundtrip(wl_dpy);
    if (err == -1) {
      printf("wl_display_roundtrip failed\n");
      goto out;
    }

    win_width = wl_data.width;
    win_height = wl_data.height;
  }
  #endif
  #if defined(EGL_DRM)
  if (!strcmp(backend, "egl-drm")) {
    if (getenv("DRICARD")) {
      drm_fd = open(getenv("DRICARD"), O_RDWR);
      if (drm_fd == -1) {
        printf("open %s failed: %s\n", getenv("DRICARD"), strerror(errno));
        goto out;
      }
    }
    else {
      drm_fd = open("/dev/dri/card0", O_RDWR);
      if (drm_fd == -1) {
        printf("open /dev/dri/card0 failed: %s\n", strerror(errno));
        goto out;
      }
    }

    drm_dpy = gbm_create_device(drm_fd);
    if (!drm_dpy) {
      printf("gbm_create_device failed\n");
      goto out;
    }

    drm_resources = drmModeGetResources(drm_fd);
    if (!drm_resources) {
      printf("drmModeGetResources failed\n");
      goto out;
    }

    drm_connector = drmModeGetConnector(drm_fd, drm_resources->connectors[0]);
    if (!drm_connector) {
      printf("drmModeGetConnector failed\n");
      goto out;
    }

    drm_encoder = drmModeGetEncoder(drm_fd, drm_resources->encoders[0]);
    if (!drm_encoder) {
      printf("drmModeGetEncoder failed\n");
      goto out;
    }

    drm_crtc = drmModeGetCrtc(drm_fd, drm_encoder->crtc_id);
    if (!drm_crtc) {
      printf("drmModeGetCrtc failed\n");
      goto out;
    }

    win_width = drm_connector->modes[0].hdisplay;
    win_height = drm_connector->modes[0].vdisplay;
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
  #if defined(EGL_WAYLAND)
  if (!strcmp(backend, "egl-wayland")) {
    setenv("EGL_PLATFORM", "wayland", 1);
    egl_dpy = eglGetDisplay((EGLNativeDisplayType)wl_dpy);
  }
  #endif
  #if defined(EGL_DRM)
  if (!strcmp(backend, "egl-drm")) {
    setenv("EGL_PLATFORM", "drm", 1);
    egl_dpy = eglGetDisplay((EGLNativeDisplayType)drm_dpy);
  }
  #endif
  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_DRM)
  if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev") || !strcmp(backend, "egl-wayland") || !strcmp(backend, "egl-drm")) {
    if (!egl_dpy) {
      printf("eglGetDisplay failed: 0x%x\n", eglGetError());
      goto out;
    }
  }
  #endif

  /* set attributes */

  #if defined(GL_X11)
  if (!strcmp(backend, "gl-x11")) {
    x11_attr[0] = GLX_RGBA;
    x11_attr[1] = GLX_DOUBLEBUFFER;
    x11_attr[2] = GLX_DEPTH_SIZE;
    x11_attr[3] = 1;
    x11_attr[4] = None;
    x11_visual = glXChooseVisual(x11_dpy, 0, x11_attr);
    if (!x11_visual) {
      printf("glXChooseVisual failed\n");
      goto out;
    }
  }
  #endif
  #if defined(GL_DIRECTFB)
  if (!strcmp(backend, "gl-directfb")) {
    dfb_attr = DSCAPS_DOUBLE | DSCAPS_DEPTH;
  }
  #endif
  #if defined(GL_FBDEV)
  if (!strcmp(backend, "gl-fbdev")) {
    fb_attr[0] = GLFBDEV_DOUBLE_BUFFER;
    fb_attr[1] = GLFBDEV_DEPTH_SIZE;
    fb_attr[2] = 1;
    fb_attr[3] = GLFBDEV_NONE;
    fb_visual = glFBDevCreateVisual(&fb_finfo, &fb_vinfo, fb_attr);
    if (!fb_visual) {
      printf("glFBDevCreateVisual failed\n");
      goto out;
    }

    fb_addr = mmap(NULL, fb_finfo.smem_len, PROT_WRITE, MAP_SHARED, fb_dpy, 0);
    if (fb_addr == MAP_FAILED) {
      printf("mmap failed: %s\n", strerror(errno));
      goto out;
    }

    fb_buffer = glFBDevCreateBuffer(&fb_finfo, &fb_vinfo, fb_visual, fb_addr, NULL, fb_finfo.smem_len);
    if (!fb_buffer) {
      printf("glFBDevCreateBuffer failed\n");
      goto out;
    }
  }
  #endif

  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_DRM)
  if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev") || !strcmp(backend, "egl-wayland") || !strcmp(backend, "egl-drm")) {
    err = eglInitialize(egl_dpy, &egl_major_version, &egl_minor_version);
    if (!err) {
      printf("eglInitialize failed: 0x%x\n", eglGetError());
      goto out;
    }

    if (!gears_engine_version(gears_engine)) {
      err = eglBindAPI(EGL_OPENGL_API);
      if (!err) {
        printf("eglBindAPI failed: 0x%x\n", eglGetError());
        goto out;
      }
    }

    egl_config_attr[0] = EGL_DEPTH_SIZE;
    egl_config_attr[1] = 1;
    egl_config_attr[2] = EGL_RENDERABLE_TYPE;
    egl_config_attr[3] = EGL_OPENGL_BIT;
    egl_config_attr[4] = EGL_NONE;
    err = eglChooseConfig(egl_dpy, egl_config_attr, NULL, 0, &egl_nconfigs);
    if (!err || !egl_nconfigs) {
      printf("eglChooseConfig failed: 0x%x, %d\n", eglGetError(), egl_nconfigs);
      goto out;
    }

    egl_configs = malloc(egl_nconfigs * sizeof(EGLConfig));
    if (!egl_configs) {
      printf("malloc failed: %s\n", strerror(errno));
      goto out;
    }

    err = eglChooseConfig(egl_dpy, egl_config_attr, egl_configs, egl_nconfigs, &egl_nconfigs);
    if (!err) {
      printf("eglChooseConfig failed: 0x%x\n", eglGetError());
      goto out;
    }

    egl_config = egl_configs[0];
  }
  #endif

  /* create window associated to the display */

  #if defined(GL_X11) || defined(EGL_X11)
  if (!strcmp(backend, "gl-x11") || !strcmp(backend, "egl-x11")) {
    x11_win = XCreateSimpleWindow(x11_dpy, DefaultRootWindow(x11_dpy), win_posx, win_posy, win_width, win_height, 0, 0, 0);
    if (!x11_win) {
      printf("XCreateSimpleWindow failed\n");
      goto out;
    }

    XMapWindow(x11_dpy, x11_win);

    x11_event_mask = ExposureMask;
    XSelectInput(x11_dpy, x11_win, x11_event_mask);
  }
  #endif
  #if defined(GL_DIRECTFB) || defined(EGL_DIRECTFB)
  if (!strcmp(backend, "gl-directfb") || !strcmp(backend, "egl-directfb")) {
    memset(&dfb_desc, 0, sizeof(DFBWindowDescription));
    dfb_desc.flags = DWDESC_SURFACE_CAPS | DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_POSX | DWDESC_POSY;
    dfb_desc.surface_caps = dfb_attr;
    dfb_desc.width = win_width;
    dfb_desc.height = win_height;
    dfb_desc.posx = win_posx;
    dfb_desc.posy = win_posy;
    err = dfb_layer->CreateWindow(dfb_layer, &dfb_desc, &dfb_window);
    if (err) {
      printf("CreateWindow failed: %s\n", DirectFBErrorString(err));
      goto out;
    }

    err = dfb_window->GetSurface(dfb_window, &dfb_win);
    if (err) {
      printf("GetSurface failed: %s\n", DirectFBErrorString(err));
      goto out;
    }
  }
  #endif
  #if defined(GL_FBDEV) || defined(EGL_FBDEV)
  if (!strcmp(backend, "gl-fbdev")) {
    fb_win = calloc(1, sizeof(struct fb_window));
    if (!fb_win) {
      printf("fb_window calloc failed\n");
      goto out;
    }
    else {
      fb_win->width = win_width;
      fb_win->height = win_height;
      fb_win->posx = win_posx;
      fb_win->posy = win_posy;
    }
  }
  #endif
  #if defined(EGL_WAYLAND)
  if (!strcmp(backend, "egl-wayland")) {
    wl_surface = wl_compositor_create_surface(wl_data.wl_compositor);
    if (!wl_surface) {
      printf("wl_compositor_create_surface failed\n");
      goto out;
    }

    wl_win = calloc(1, sizeof(struct wl_window));
    if (!wl_win) {
      printf("wl_window calloc failed\n");
      goto out;
    }
    else {
      wl_win->surface = wl_surface;
      wl_win->width = win_width;
      wl_win->height = win_height;
    }

    wl_shell_surface = wl_shell_get_shell_surface(wl_data.wl_shell, wl_surface);
    if (!wl_shell_surface) {
      printf("wl_shell_get_shell_surface failed\n");
      goto out;
    }

    wl_shell_surface_set_position(wl_shell_surface, win_posx, win_posy);

    wl_shell_surface_set_toplevel(wl_shell_surface);
  }
  #endif
  #if defined(EGL_DRM)
  if (!strcmp(backend, "egl-drm")) {
    drm_win = gbm_surface_create(drm_dpy, win_width, win_height, GBM_BO_FORMAT_XRGB8888, GBM_BO_USE_SCANOUT);
    if (!drm_win) {
      printf("gbm_surface_create failed\n");
      goto out;
    }
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
  #if defined(EGL_WAYLAND)
  if (!strcmp(backend, "egl-wayland")) {
    egl_win = eglCreateWindowSurface(egl_dpy, egl_config, (EGLNativeWindowType)wl_win, NULL);
  }
  #endif
  #if defined(EGL_DRM)
  if (!strcmp(backend, "egl-drm")) {
    egl_win = eglCreateWindowSurface(egl_dpy, egl_config, (EGLNativeWindowType)drm_win, NULL);
  }
  #endif
  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_DRM)
  if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev") || !strcmp(backend, "egl-wayland") || !strcmp(backend, "egl-drm")) {
    if (!egl_win) {
      printf("eglCreateWindowSurface failed: 0x%x\n", eglGetError());
      goto out;
    }
  }
  #endif

  /* create context and attach it to the window */

  #if defined(GL_X11)
  if (!strcmp(backend, "gl-x11")) {
    x11_ctx = glXCreateContext(x11_dpy, x11_visual, NULL, True);
    if (!x11_ctx) {
      printf("glXCreateContext failed\n");
      goto out;
    }

    err = glXMakeCurrent(x11_dpy, x11_win, x11_ctx);
    if (!err) {
      printf("glXMakeCurrent failed\n");
      goto out;
    }
  }
  #endif
  #if defined(GL_DIRECTFB)
  if (!strcmp(backend, "gl-directfb")) {
    err = dfb_win->GetGL(dfb_win, &dfb_ctx);
    if (err) {
      printf("GetGL failed: %s\n", DirectFBErrorString(err));
      goto out;
    }

    err = dfb_ctx->Lock(dfb_ctx);
    if (err) {
      printf("Lock failed: %s\n", DirectFBErrorString(err));
      goto out;
    }
  }
  #endif
  #if defined(GL_FBDEV)
  if (!strcmp(backend, "gl-fbdev")) {
    glFBDevSetWindow(fb_buffer, fb_win);

    fb_ctx = glFBDevCreateContext(fb_visual, NULL);
    if (!fb_ctx) {
      printf("glFBDevCreateContext failed\n");
      goto out;
    }

    err = glFBDevMakeCurrent(fb_ctx, fb_buffer, fb_buffer);
    if (!err) {
      printf("glFBDevMakeCurrent failed\n");
      goto out;
    }
  }
  #endif

  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_DRM)
  if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev") || !strcmp(backend, "egl-wayland") || !strcmp(backend, "egl-drm")) {
    opt = 0;
    memset(egl_ctx_attr, 0, sizeof(egl_ctx_attr));
    if (gears_engine_version(gears_engine) == 2) {
      egl_ctx_attr[opt++] = EGL_CONTEXT_CLIENT_VERSION;
      egl_ctx_attr[opt++] = gears_engine_version(gears_engine);
    }
    egl_ctx_attr[opt] = EGL_NONE;
    egl_ctx = eglCreateContext(egl_dpy, egl_config, EGL_NO_CONTEXT, egl_ctx_attr);
    if (!egl_ctx) {
      printf("eglCreateContext failed: 0x%x\n", eglGetError());
      goto out;
    }

    err = eglMakeCurrent(egl_dpy, egl_win, egl_win, egl_ctx);
    if (!err) {
      printf("eglMakeCurrent failed: 0x%x\n", eglGetError());
      goto out;
    }

    eglSwapInterval(egl_dpy, 0);
  }
  #endif

  /* initialize input event */

  #if defined(GL_X11) || defined(EGL_X11)
  if (!strcmp(backend, "gl-x11") || !strcmp(backend, "egl-x11")) {
    x11_event_mask = x11_event_mask | KeyPressMask;
    XSelectInput(x11_dpy, x11_win, x11_event_mask);
  }
  #endif
  #if defined(GL_DIRECTFB) || defined(EGL_DIRECTFB)
  if (!strcmp(backend, "gl-directfb") || !strcmp(backend, "egl-directfb")) {
    dfb_event_mask ^= DWET_KEYDOWN;
    err = dfb_window->DisableEvents(dfb_window, dfb_event_mask);
    if (err) {
      printf("DisableEvents failed: %s\n", DirectFBErrorString(err));
      goto out;
    }

    err = dfb_window->CreateEventBuffer(dfb_window, &dfb_event_buffer);
    if (err) {
      printf("CreateEventBuffer failed: %s\n", DirectFBErrorString(err));
      goto out;
    }

    err = dfb_window->SetOpacity(dfb_window, 0xff);
    if (err) {
      printf("SetOpacity failed: %s\n", DirectFBErrorString(err));
      goto out;
    }

    err = dfb_window->RequestFocus(dfb_window);
    if (err) {
      printf("RequestFocus failed: %s\n", DirectFBErrorString(err));
      goto out;
    }
  }
  #endif
  #if defined(GL_FBDEV) || defined(EGL_FBDEV)
  if (!strcmp(backend, "gl-fbdev") || !strcmp(backend, "egl-fbdev")) {
    if (getenv("KEYBOARD")) {
      fb_keyboard = open(getenv("KEYBOARD"), O_RDONLY | O_NONBLOCK);
      if (fb_keyboard == -1) {
        printf("open %s failed: %s\n", getenv("KEYBOARD"), strerror(errno));
        goto out;
      }
    }
    else {
      fb_keyboard = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
      if (fb_keyboard == -1) {
        printf("open /dev/input/event0 failed: %s\n", strerror(errno));
        goto out;
      }
    }
  }
  #endif
  #if defined(EGL_WAYLAND)
  if (!strcmp(backend, "egl-wayland")) {
    wl_data.xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!wl_data.xkb_context) {
      printf("xkb_context_new failed\n");
      goto out;
    }
  }
  #endif
  #if defined(EGL_DRM)
  if (!strcmp(backend, "egl-drm")) {
    if (getenv("KEYBOARD")) {
      drm_keyboard = open(getenv("KEYBOARD"), O_RDONLY | O_NONBLOCK);
      if (drm_keyboard == -1) {
        printf("open %s failed: %s\n", getenv("KEYBOARD"), strerror(errno));
        goto out;
      }
    }
    else {
      drm_keyboard = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
      if (drm_keyboard == -1) {
        printf("open /dev/input/event0 failed: %s\n", strerror(errno));
        goto out;
      }
    }

    err = libevdev_new_from_fd(drm_keyboard, &drm_evdev);
    if (err < 0) {
      printf("libevdev_new_from_fd failed: %s\n", strerror(-err));
      goto out;
    }
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

  signal(SIGINT, sighandler);

  while (loop) {
    if (animate && redisplay) {
      err = gettimeofday(&tv, NULL);
      if (err == -1) {
        printf("gettimeofday failed: %s\n", strerror(errno));
      }

      t = tv.tv_sec * 1000 + tv.tv_usec / 1000;

      if (!frames) {
        t_rate = t_rot = t;
      }
      else {
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
    }
    else {
      if (frames) {
        frames = 0;
      }
    }

    if (redisplay) {
      gears_engine_draw(gears_engine, view_tz, view_rx, view_ry, model_rz);

      if (animate) {
        frames++;
      }

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
        glFBDevSwapBuffers(fb_buffer);
      }
      #endif

      #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_DRM)
      if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev") || !strcmp(backend, "egl-wayland") || !strcmp(backend, "egl-drm")) {
        eglSwapBuffers(egl_dpy, egl_win);
      }
      #endif
    }

    #if defined(GL_X11) || defined(EGL_X11)
    if (!strcmp(backend, "gl-x11") || !strcmp(backend, "egl-x11")) {
      if (!animate && redisplay) {
        redisplay = 0;
      }

      if (XPending(x11_dpy)) {
        XNextEvent(x11_dpy, &x11_event);
        if (x11_event.type == Expose && !redisplay) {
          redisplay = 1;
        }
        else if (x11_event.type == KeyPress) {
          x11_keyboard_handle_key(&x11_event);
        }
      }
    }
    #endif
    #if defined(GL_DIRECTFB) || defined(EGL_DIRECTFB)
    if (!strcmp(backend, "gl-directfb") || !strcmp(backend, "egl-directfb")) {
      if (!animate && redisplay) {
        redisplay = 0;
      }

      if (!dfb_event_buffer->GetEvent(dfb_event_buffer, (DFBEvent *)&dfb_event)) {
        if (dfb_event.type == DWET_KEYDOWN) {
          dfb_keyboard_handle_key(&dfb_event);
        }
      }
    }
    #endif
    #if defined(GL_FBDEV) || defined(EGL_FBDEV)
    if (!strcmp(backend, "gl-fbdev") || !strcmp(backend, "egl-fbdev")) {
      if (!animate && redisplay) {
        redisplay = 0;
      }

      read(fb_keyboard, &fb_event, sizeof(struct input_event));
      if (fb_event.type == EV_KEY) {
        if (fb_event.value) {
          fb_keyboard_handle_key(&fb_event);
        }
      }
    }
    #endif
    #if defined(EGL_WAYLAND)
    if (!strcmp(backend, "egl-wayland")) {
      if (!animate && redisplay) {
        redisplay = 0;
      }

      wl_display_dispatch(wl_dpy);
    }
    #endif
    #if defined(EGL_DRM)
    if (!strcmp(backend, "egl-drm")) {
      if (redisplay) {
        if (drm_bo) {
          gbm_surface_release_buffer(drm_win, drm_bo);
        }
        drm_bo = gbm_surface_lock_front_buffer(drm_win);
        if (drm_bo) {
          drm_fb_id = (unsigned int)gbm_bo_get_user_data(drm_bo);
          if(!drm_fb_id) {
            drmModeAddFB(drm_fd, win_width, win_height, 24, 32, win_width * 4, gbm_bo_get_handle(drm_bo).u32, &drm_fb_id);
            gbm_bo_set_user_data(drm_bo, (void *)drm_fb_id, drm_destroy_user_data);
          }
          drmModePageFlip(drm_fd, drm_encoder->crtc_id, drm_fb_id, DRM_MODE_PAGE_FLIP_EVENT, NULL);
          drmHandleEvent(drm_fd, &drm_context);
        }
        if (!animate) {
          redisplay = 0;
        }
      }

      libevdev_next_event(drm_evdev, LIBEVDEV_READ_FLAG_NORMAL, &drm_event);
      if (drm_event.type == EV_KEY) {
        if (drm_event.value) {
          drm_keyboard_handle_key(&drm_event);
        }
      }
    }
    #endif
  }

  gears_engine_term(gears_engine);

  /* print info */

  #if defined(GL_X11)
  if (!strcmp(backend, "gl-x11")) {
    glXQueryVersion(x11_dpy, &glx_major_version, &glx_minor_version);
    glXGetConfig(x11_dpy, x11_visual, GLX_DEPTH_SIZE, &glx_depth_size);
    glXGetConfig(x11_dpy, x11_visual, GLX_RED_SIZE, &glx_red_size);
    glXGetConfig(x11_dpy, x11_visual, GLX_GREEN_SIZE, &glx_green_size);
    glXGetConfig(x11_dpy, x11_visual, GLX_BLUE_SIZE, &glx_blue_size);
    glXGetConfig(x11_dpy, x11_visual, GLX_ALPHA_SIZE, &glx_alpha_size);
    printf("GLX %d.%d (depth %d, red %d, green %d, blue %d, alpha %d)\n", glx_major_version, glx_minor_version, glx_depth_size, glx_red_size, glx_green_size, glx_blue_size, glx_alpha_size);
  }
  #endif
  #if defined(GL_DIRECTFB)
  if (!strcmp(backend, "gl-directfb")) {
    memset(&directfbgl, 0, sizeof(DFBGLAttributes));
    dfb_ctx->GetAttributes(dfb_ctx, &directfbgl);
    printf("DirectFBGL %d (depth %d, red %d, green %d, blue %d, alpha %d)\n", DIRECTFBGL_INTERFACE_VERSION, directfbgl.depth_size, directfbgl.red_size, directfbgl.green_size, directfbgl.blue_size, directfbgl.alpha_size);
  }
  #endif
  #if defined(GL_FBDEV)
  if (!strcmp(backend, "gl-fbdev")) {
    glfbdev_depth_size = glFBDevGetVisualAttrib(fb_visual, GLFBDEV_DEPTH_SIZE);
    printf("GLFBDev %s (depth %d, red %d, green %d, blue %d, alpha %d)\n", glFBDevGetString(GLFBDEV_VERSION), glfbdev_depth_size, fb_vinfo.red.length, fb_vinfo.green.length, fb_vinfo.blue.length, fb_vinfo.transp.length);
  }
  #endif

  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_DRM)
  if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev") || !strcmp(backend, "egl-wayland") || !strcmp(backend, "egl-drm")) {
    eglGetConfigAttrib(egl_dpy, egl_config, EGL_DEPTH_SIZE, &egl_depth_size);
    eglGetConfigAttrib(egl_dpy, egl_config, EGL_RED_SIZE, &egl_red_size);
    eglGetConfigAttrib(egl_dpy, egl_config, EGL_GREEN_SIZE, &egl_green_size);
    eglGetConfigAttrib(egl_dpy, egl_config, EGL_BLUE_SIZE, &egl_blue_size);
    eglGetConfigAttrib(egl_dpy, egl_config, EGL_ALPHA_SIZE, &egl_alpha_size);
    printf("EGL %d.%d (depth %d, red %d, green %d, blue %d, alpha %d)\n", egl_major_version, egl_minor_version, egl_depth_size, egl_red_size, egl_green_size, egl_blue_size, egl_alpha_size);
  }
  #endif

  ret = EXIT_SUCCESS;

out:

  /* release input event */

  #if defined(GL_X11) || defined(EGL_X11)
  if (!strcmp(backend, "gl-x11") || !strcmp(backend, "egl-x11")) {
    if (x11_event_mask) {
      XSelectInput(x11_dpy, x11_win, NoEventMask);
    }
  }
  #endif
  #if defined(GL_DIRECTFB) || defined(EGL_DIRECTFB)
  if (!strcmp(backend, "gl-directfb") || !strcmp(backend, "egl-directfb")) {
    if (dfb_event_buffer) {
      dfb_event_buffer->Release(dfb_event_buffer);
    }
  }
  #endif
  #if defined(GL_FBDEV) || defined(EGL_FBDEV)
  if (!strcmp(backend, "gl-fbdev") || !strcmp(backend, "egl-fbdev")) {
    if (fb_keyboard != -1) {
      close(fb_keyboard);
    }
  }
  #endif
  #if defined(EGL_WAYLAND)
  if (!strcmp(backend, "egl-wayland")) {
    if (wl_data.xkb_state) {
      xkb_state_unref(wl_data.xkb_state);
    }

    if (wl_data.xkb_keymap) {
      xkb_map_unref(wl_data.xkb_keymap);
    }

    if (wl_data.xkb_context) {
      xkb_context_unref(wl_data.xkb_context);
    }
  }
  #endif
  #if defined(EGL_DRM)
  if (!strcmp(backend, "egl-drm")) {
    if (drm_evdev) {
      libevdev_free(drm_evdev);
    }

    if (drm_keyboard != -1) {
      close(drm_keyboard);
    }
  }
  #endif

  /* destroy context */

  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_DRM)
  if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev") || !strcmp(backend, "egl-wayland") || !strcmp(backend, "egl-drm")) {
    if (egl_ctx) {
      eglMakeCurrent(egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
      eglDestroyContext(egl_dpy, egl_ctx);
    }
  }
  #endif

  #if defined(GL_X11)
  if (!strcmp(backend, "gl-x11")) {
    if (x11_ctx) {
      glXMakeCurrent(x11_dpy, None, NULL);
      glXDestroyContext(x11_dpy, x11_ctx);
    }
  }
  #endif
  #if defined(GL_DIRECTFB)
  if (!strcmp(backend, "gl-directfb")) {
    if (dfb_ctx) {
      dfb_ctx->Unlock(dfb_ctx);
      dfb_ctx->Release(dfb_ctx);
    }
  }
  #endif
  #if defined(GL_FBDEV)
  if (!strcmp(backend, "gl-fbdev")) {
    if (fb_ctx) {
      glFBDevMakeCurrent(NULL, NULL, NULL);
      glFBDevDestroyContext(fb_ctx);
    }
  }
  #endif

  /* destroy window and close display */

  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_DRM)
  if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev") || !strcmp(backend, "egl-wayland") || !strcmp(backend, "egl-drm")) {
    if (egl_win) {
      eglDestroySurface(egl_dpy, egl_win);
    }

    if (egl_configs) {
      free(egl_configs);
    }

    if (egl_dpy) {
      eglTerminate(egl_dpy);
    }
  }
  #endif

  #if defined(GL_X11)
  if (!strcmp(backend, "gl-x11")) {
    if (x11_visual) {
      XFree(x11_visual);
    }
  }
  #endif
  #if defined(GL_X11) || defined(EGL_X11)
  if (!strcmp(backend, "gl-x11") || !strcmp(backend, "egl-x11")) {
    if (x11_win) {
      XDestroyWindow(x11_dpy, x11_win);
    }

    if (x11_dpy) {
      XCloseDisplay(x11_dpy);
    }
  }
  #endif
  #if defined(GL_DIRECTFB)
  if (!strcmp(backend, "gl-directfb")) {
      dfb_attr = DSCAPS_NONE;
  }
  #endif
  #if defined(GL_DIRECTFB) || defined(EGL_DIRECTFB)
  if (!strcmp(backend, "gl-directfb") || !strcmp(backend, "egl-directfb")) {
    if (dfb_win) {
      dfb_win->Release(dfb_win);
    }

    if (dfb_window) {
      dfb_window->Release(dfb_window);
    }

    if (dfb_layer) {
      dfb_layer->Release(dfb_layer);
    }

    if (dfb_dpy) {
      dfb_dpy->Release(dfb_dpy);
    }
  }
  #endif
  #if defined(GL_FBDEV)
  if (!strcmp(backend, "gl-fbdev")) {
    if (fb_buffer) {
      glFBDevDestroyBuffer(fb_buffer);
    }

    if (fb_addr) {
      munmap(fb_addr, fb_finfo.smem_len);
    }

    if (fb_visual) {
      glFBDevDestroyVisual(fb_visual);
    }
  }
  #endif
  #if defined(GL_FBDEV) || defined(EGL_FBDEV)
  if (!strcmp(backend, "gl-fbdev") || !strcmp(backend, "egl-fbdev")) {
    if (fb_win) {
      free(fb_win);
    }

    if (fb_dpy != -1) {
      close(fb_dpy);
    }
  }
  #endif
  #if defined(EGL_WAYLAND)
  if (!strcmp(backend, "egl-wayland")) {
    if (wl_shell_surface) {
      wl_shell_surface_destroy(wl_shell_surface);
    }

    if (wl_win) {
      free(wl_win);
    }

    if (wl_surface) {
      wl_surface_destroy(wl_surface);
    }

    if (wl_data.wl_keyboard) {
      wl_keyboard_destroy(wl_data.wl_keyboard);
    }

    if (wl_data.wl_seat) {
      wl_seat_destroy(wl_data.wl_seat);
    }

    if (wl_data.wl_shell) {
      wl_shell_destroy(wl_data.wl_shell);
    }

    if (wl_data.wl_compositor) {
      wl_compositor_destroy(wl_data.wl_compositor);
    }

    if (wl_data.wl_output) {
      wl_output_destroy(wl_data.wl_output);
    }

    if (wl_data.wl_registry) {
      wl_registry_destroy(wl_data.wl_registry);
    }

    if (wl_dpy) {
      wl_display_disconnect(wl_dpy);
    }
  }
  #endif
  #if defined(EGL_DRM)
  if (!strcmp(backend, "egl-drm")) {
    if (drm_win) {
      gbm_surface_destroy(drm_win);
    }

    if (drm_crtc) {
      drmModeSetCrtc(drm_fd, drm_crtc->crtc_id, drm_crtc->buffer_id, drm_crtc->x, drm_crtc->y, &drm_connector->connector_id, 1, &drm_crtc->mode);
      drmModeFreeCrtc(drm_crtc);
    }

    if (drm_encoder) {
      drmModeFreeEncoder(drm_encoder);
    }

    if (drm_connector) {
      drmModeFreeConnector(drm_connector);
    }

    if (drm_resources) {
      drmModeFreeResources(drm_resources);
    }

    if (drm_dpy) {
      gbm_device_destroy(drm_dpy);
    }

    if (drm_fd != -1) {
      close(drm_fd);
    }
  }
  #endif

  gears_engine_free(gears_engine);

  return ret;
}
