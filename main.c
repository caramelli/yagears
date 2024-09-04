/*
  yagears                  Yet Another Gears OpenGL / Vulkan demo
  Copyright (C) 2013-2024  Nicolas Caramelli

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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#if defined(GL_X11)
#include <GL/glx.h>
#endif
#if defined(GL_DIRECTFB)
#include <directfbgl.h>
#endif
#if defined(GL_FBDEV)
#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <GL/glfbdev.h>
#endif

#if defined(EGL_X11)
#include <X11/Xutil.h>
#endif
#if defined(EGL_DIRECTFB)
#include <directfb.h>
#endif
#if defined(EGL_FBDEV)
#include <dirent.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#endif
#if defined(EGL_WAYLAND)
#include <sys/mman.h>
#include <wayland-egl.h>
#include <xkbcommon/xkbcommon.h>
#endif
#if defined(EGL_XCB)
#include <xcb/xcb.h>
#endif
#if defined(EGL_DRM)
#include <dirent.h>
#include <fcntl.h>
#include <gbm.h>
#include <libevdev/libevdev.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#ifdef HAVE_DRI
#include <dlfcn.h>
#include <limits.h>

struct __DRIextensionRec {
  char *name;
  int version;
};

struct __DRIdrawableRec;
struct __DRIimageList;
struct __DRIimageLoaderExtensionRec {
  struct __DRIextensionRec base;
  int (*getBuffers)(struct __DRIdrawableRec *, unsigned int, unsigned int *, void *, unsigned int, struct __DRIimageList *);
};

struct __DRIscreenRec;
struct __DRIconfigRec;
struct __DRIcoreExtensionRec {
  struct __DRIextensionRec base;
  struct __DRIscreenRec *(*createNewScreen)(int, int, unsigned int, struct __DRIextensionRec **, struct __DRIconfigRec ***, void *);
  void (*destroyScreen)(struct __DRIscreenRec *);
  struct __DRIextensionRec **(*getExtensions)(struct __DRIscreenRec *);
};

struct __DRIcontextRec;
struct __DRIbufferRec;
struct __DRIdri2ExtensionRec {
  struct __DRIextensionRec base;
  struct __DRIscreenRec *(*createNewScreen)(int, int, struct __DRIextensionRec **, struct __DRIconfigRec ***, void *);
  struct __DRIdrawableRec *(*createNewDrawable)(struct __DRIscreenRec *, struct __DRIconfigRec *, void *);
  struct __DRIcontextRec *(*createNewContext)(struct __DRIscreenRec *, struct __DRIconfigRec *, struct __DRIcontextRec *, void *);
  unsigned int (*getAPIMask)(struct __DRIscreenRec *);
  struct __DRIcontextRec *(*createNewContextForAPI)(struct __DRIscreenRec *, int, struct __DRIconfigRec *, struct __DRIcontextRec *, void *);
  struct __DRIbufferRec *(*allocateBuffer)(struct __DRIscreenRec *, unsigned int, unsigned int, int, int);
  void (*releaseBuffer)(struct __DRIscreenRec *, struct __DRIbufferRec *);
  struct __DRIcontextRec *(*createContextAttribs)(struct __DRIscreenRec *, int, struct __DRIconfigRec *, struct __DRIcontextRec *, unsigned int, unsigned int *, unsigned int *, void *);
  struct __DRIscreenRec *(*createNewScreen2)(int, int, struct __DRIextensionRec **, struct __DRIextensionRec **, struct __DRIconfigRec ***, void *);
};

struct __DRIimageRec;
struct __DRIimageExtensionRec {
  struct __DRIextensionRec base;
  struct __DRIimageRec *(*createImageFromName)(struct __DRIscreenRec *, int, int, int, int, int, void *);
  struct __DRIimageRec *(*createImageFromRenderbuffer)(struct __DRIcontextRec *, int, void *);
  void (*destroyImage)(struct __DRIimageRec *);
  struct __DRIimageRec *(*createImage)(struct __DRIscreenRec *, int, int, int, unsigned int, void *);
  int (*queryImage)(struct __DRIimageRec *, int, int *);
};
#endif
#endif
#if defined(EGL_RPI)
#include <bcm_host.h>
#include <fcntl.h>
#include <termios.h>
#endif
#if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_XCB) || defined(EGL_DRM) || defined(EGL_RPI)
#include <EGL/egl.h>
#endif
#if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_XCB) || defined(EGL_DRM)
#include <EGL/eglext.h>
#endif
#if defined(WAFFLE)
#include <fcntl.h>
#include <libinput.h>
#include <linux/input.h>
#include <waffle.h>
#endif

#include "gears_engine.h"

#if !defined(ENGINE_CTOR) && defined(YAGEARS_ENGINE)
#define stringify_engine_ctor(name)       name##_engine_ctor()
#define engine_ctor(name)                 stringify_engine_ctor(name)
#define stringify_engine_ctor_proto(name) void name##_engine_ctor(void)
#define engine_ctor_proto(name)           stringify_engine_ctor_proto(name)

engine_ctor_proto(YAGEARS_ENGINE);
#endif

#if defined(YAGEARS_BACKEND) && defined(YAGEARS_ENGINE)
#define XSTRINGIFY(x) #x
#define STRINGIFY(x)  XSTRINGIFY(x)
#endif

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
    case XK_t:
      if (getenv("NO_TEXTURE")) {
        unsetenv("NO_TEXTURE");
      }
      else {
        setenv("NO_TEXTURE", "1", 1);
      }
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
    case DIKS_SMALL_T:
      if (getenv("NO_TEXTURE")) {
        unsetenv("NO_TEXTURE");
      }
      else {
        setenv("NO_TEXTURE", "1", 1);
      }
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
    case KEY_T:
      if (getenv("NO_TEXTURE")) {
        unsetenv("NO_TEXTURE");
      }
      else {
        setenv("NO_TEXTURE", "1", 1);
      }
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
#define wl_window wl_egl_window

struct wl_window {
  long version;
  int width;
  int height;
  int dx;
  int dy;
  int attached_width;
  int attached_height;
  void *private;
  void (*resize_callback)(struct wl_window *, void *);
  void (*destroy_window_callback)(void *);
  struct wl_surface *surface;
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
    case XKB_KEY_t:
      if (getenv("NO_TEXTURE")) {
        unsetenv("NO_TEXTURE");
      }
      else {
        setenv("NO_TEXTURE", "1", 1);
      }
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

#if defined(EGL_XCB)
static void xcb_keyboard_handle_key(xcb_generic_event_t *event)
{
  switch (((xcb_key_press_event_t *)event)->detail) {
    case 0x09:
      sighandler(SIGTERM);
      return;
    case 0x41:
      animate = !animate;
      if (animate) {
        redisplay = 1;
      }
      else {
        redisplay = 0;
      }
      return;
    case 0x75:
      view_tz -= -5.0;
      break;
    case 0x70:
      view_tz += -5.0;
      break;
    case 0x74:
      view_rx -= 5.0;
      break;
    case 0x6f:
      view_rx += 5.0;
      break;
    case 0x72:
      view_ry -= 5.0;
      break;
    case 0x71:
      view_ry += 5.0;
      break;
    case 0x1c:
      if (getenv("NO_TEXTURE")) {
        unsetenv("NO_TEXTURE");
      }
      else {
        setenv("NO_TEXTURE", "1", 1);
      }
      break;
    default:
      return;
  }

  if (!animate) {
    redisplay = 1;
  }
}
#endif

#if defined(EGL_DRM)
#define drm_display gbm_device
#define drm_surface gbm_surface
#define drm_bo gbm_bo

#ifdef HAVE_DRI
struct drm_display {
  struct drm_display *(*dummy)(int);
  int fd;
  char *name;
  unsigned int refcount;
  struct stat stat;
  void (*destroy)(struct drm_display *);
  int (*is_format_supported)(struct drm_display *, unsigned int, unsigned int);
  struct drm_bo *(*bo_create)(struct drm_display *, unsigned int, unsigned int, unsigned int, unsigned int);
  struct drm_bo *(*bo_import)(struct drm_display *, unsigned int, void *, unsigned int);
  int (*bo_write)(struct drm_bo *, void *, size_t);
  #if DRI_MAJOR_VERSION > 10 || (DRI_MAJOR_VERSION == 10 && DRI_MINOR_VERSION >= 2)
  int (*bo_get_fd)(struct drm_bo *);
  #endif
  void (*bo_destroy)(struct drm_bo *);
  struct drm_surface *(*surface_create)(struct drm_display *, unsigned int, unsigned int, unsigned int, unsigned int);
  struct drm_bo *(*surface_lock_front_buffer)(struct drm_surface *);
  void (*surface_release_buffer)(struct drm_surface *, struct drm_bo *);
  int (*surface_has_free_buffers)(struct drm_surface *);
  void (*surface_destroy)(struct drm_surface *);
  int type;
  char *driver_name;
  void *driver;
  struct __DRIscreenRec *screen;
  struct __DRIcoreExtensionRec *core;
  struct __DRIdri2ExtensionRec *dri2;
  struct __DRIimageExtensionRec *image;
  #if DRI_MAJOR_VERSION > 10 || (DRI_MAJOR_VERSION == 10 && DRI_MINOR_VERSION >= 3)
  struct __DRIswrastExtensionRec *swrast;
  #endif
  struct __DRI2flushExtensionRec *flush;
  struct __DRIdri2LoaderExtensionRec *loader;
  struct __DRIconfigRec **driver_configs;
  #if DRI_MAJOR_VERSION > 10 || (DRI_MAJOR_VERSION == 10 && DRI_MINOR_VERSION >= 2)
  struct __DRIextensionRec **extensions;
  #else
  struct __DRIextensionRec *extensions[5];
  #endif
  struct __DRIextensionRec **driver_extensions;
  struct __DRIimageRec *(*lookup_image)(struct __DRIscreenRec *, void *, void *);
  void *lookup_user_data;
  struct __DRIbufferRec *(*get_buffers)(struct __DRIdrawableRec *, int *, int *, unsigned int *, int, int *, void *);
  void (*flush_front_buffer)(struct __DRIdrawableRec *, void *);
  struct __DRIbufferRec *(*get_buffers_with_format)(struct __DRIdrawableRec *, int *, int *, unsigned int *, int, int *, void *);
  int (*image_get_buffers)(struct __DRIdrawableRec *, unsigned int, unsigned int *, void *, unsigned int, struct __DRIimageList *);
  #if DRI_MAJOR_VERSION > 10 || (DRI_MAJOR_VERSION == 10 && DRI_MINOR_VERSION >= 3)
  void (*swrast_put_image2)(struct __DRIdrawableRec *, int, int, int, int, int, int, char *, void *);
  void (*swrast_get_image)(struct __DRIdrawableRec *, int, int, int, int, char *, void *);
  #endif
};

struct drm_surface {
  struct drm_display *display;
  unsigned int width;
  unsigned int height;
  unsigned int format;
  unsigned int flags;
  void *private;
};

struct drm_bo {
  struct drm_display *display;
  unsigned int width;
  unsigned int height;
  unsigned int stride;
  unsigned int format;
  unsigned long long handle;
  void *user_data;
  void (*destroy_user_data)(struct drm_bo *, void *);
  struct __DRIimageRec *image;
};

static int image_get_buffers(struct __DRIdrawableRec *drawable, unsigned int format, unsigned int *stamp, void *loaderPrivate, unsigned int buffer_mask, struct __DRIimageList *buffers)
{
  struct drm_surface *surface = loaderPrivate;
  struct drm_display *display = surface->display;

  return display->image_get_buffers(drawable, format, stamp, surface->private, buffer_mask, buffers);
}

static struct __DRIimageLoaderExtensionRec image_loader_extension = {
  .base = { "DRI_IMAGE_LOADER", 1 },
  .getBuffers = image_get_buffers,
};

static struct drm_bo *drm_bo_create(struct drm_display *display, unsigned int width, unsigned int height, unsigned int format, unsigned int usage)
{
  struct drm_bo *bo;

  bo = calloc(1, sizeof(struct drm_bo));
  bo->display = display;
  bo->width = width;
  bo->height = height;
  bo->format = format;
  bo->image = display->image->createImage(display->screen, width, height, 0x1002, 2, bo);
  display->image->queryImage(bo->image, 0x2000, (int *)&bo->stride);
  display->image->queryImage(bo->image, 0x2001, (int *)&bo->handle);

  return bo;
}

static void drm_bo_destroy(struct drm_bo *bo)
{
  bo->display->image->destroyImage(bo->image);

  free(bo);
}

static void drm_destroy_user_data(struct drm_bo *bo, void *data)
{
  drmModeRmFB(bo->display->fd, (uintptr_t)data);
}
#endif

static void gbm_destroy_user_data(struct gbm_bo *bo, void *data)
{
  drmModeRmFB(gbm_device_get_fd(gbm_bo_get_device(bo)), (uintptr_t)data);
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
    case KEY_T:
      if (getenv("NO_TEXTURE")) {
        unsetenv("NO_TEXTURE");
      }
      else {
        setenv("NO_TEXTURE", "1", 1);
      }
      break;
    default:
      return;
  }

  if (!animate) {
    redisplay = 1;
  }
}
#endif

#if defined(EGL_RPI)
typedef struct {
  DISPMANX_ELEMENT_HANDLE_T element;
  int width;
  int height;
} DISPMANX_WINDOW_T;

static void rpi_keyboard_handle_key(char *event)
{
  if (event) {
    if (!strcmp(event, "\x1b")) {
      sighandler(SIGTERM);
      return;
    }
    else if (!strcmp(event, "\x20")) {
      animate = !animate;
      if (animate) {
        redisplay = 1;
      }
      else {
        redisplay = 0;
      }
      return;
    }
    else if (!strcmp(event, "\x1b[6~")) {
      view_tz -= -5.0;
    }
    else if (!strcmp(event, "\x1b[5~")) {
      view_tz += -5.0;
    }
    else if (!strcmp(event,"\x1b[B")) {
      view_rx -= 5.0;
    }
    else if (!strcmp(event,"\x1b[A")) {
      view_rx += 5.0;
    }
    else if (!strcmp(event,"\x1b[C")) {
      view_ry -= 5.0;
    }
    else if (!strcmp(event,"\x1b[D")) {
      view_ry += 5.0;
    }
    else if (!strcmp(event, "\x74")) {
      if (getenv("NO_TEXTURE")) {
        unsetenv("NO_TEXTURE");
      }
      else {
        setenv("NO_TEXTURE", "1", 1);
      }
    }
    else {
      return;
    }
  }

  if (!animate) {
    redisplay = 1;
  }

}
#endif

#if defined(WAFFLE)
static void waffle_keyboard_handle_key(struct libinput_event_keyboard *event)
{
  switch (libinput_event_keyboard_get_key(event)) {
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
    case KEY_T:
      if (getenv("NO_TEXTURE")) {
        unsetenv("NO_TEXTURE");
      }
      else {
        setenv("NO_TEXTURE", "1", 1);
      }
      break;
    default:
      return;
  }

  if (!animate) {
    redisplay = 1;
  }
}

static int waffle_input_handle_open(const char *path, int flags, void *user_data)
{
  return open(path, flags);
}

static void waffle_input_handle_close(int fd, void *user_data)
{
  close(fd);
}

static struct libinput_interface waffle_input_interface = { waffle_input_handle_open, waffle_input_handle_close };
#endif

/******************************************************************************/

int main(int argc, char *argv[])
{
  int err = 0, ret = EXIT_FAILURE;
  const char *backend_arg = NULL, *engine_arg = NULL;
  char backends[64], *c;
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
  IDirectFBSurface *dfb_win = NULL;
  IDirectFBDisplayLayer *dfb_layer = NULL;
  DFBDisplayLayerConfig dfb_layer_config;
  DFBSurfaceCapabilities dfb_attr = DSCAPS_NONE;
  DFBWindowDescription dfb_desc;
  IDirectFBWindow *dfb_window = NULL;
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
  struct fb_window *fb_win = NULL;
  struct fb_fix_screeninfo fb_finfo;
  struct fb_var_screeninfo fb_vinfo;
  int fb_keyboard = -1;
  DIR *fb_input_dir = NULL;
  struct dirent *fb_input_dev = NULL;
  unsigned char fb_key_bits[(KEY_CNT - 1) / 8 + 1];
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
  struct wl_window *wl_win = NULL;
  struct wl_data wl_data;
  struct wl_surface *wl_surface = NULL;
  struct wl_shell_surface *wl_shell_surface = NULL;
  #endif
  #if defined(EGL_XCB)
  xcb_connection_t *xcb_dpy = NULL;
  xcb_window_t xcb_win = -1;
  xcb_void_cookie_t xcb_cookie;
  uint32_t xcb_value_list[2];
  xcb_event_mask_t xcb_event_mask = XCB_EVENT_MASK_NO_EVENT;
  xcb_generic_event_t *xcb_event = NULL;
  #endif
  #if defined(EGL_DRM)
  struct drm_display *drm_dpy = NULL;
  struct drm_surface *drm_win = NULL;
  int drm_fd = -1;
  #ifdef HAVE_DRI
  char drm_driver_path[PATH_MAX];
  struct __DRIcoreExtensionRec **drm_driver_extensions = NULL;
  struct __DRIextensionRec *drm_extensions[] = { &image_loader_extension.base, NULL };
  #endif
  drmModeResPtr drm_resources = NULL;
  drmModeConnectorPtr drm_connector = NULL;
  drmModeEncoderPtr drm_encoder = NULL;
  drmModeCrtcPtr drm_crtc = NULL;
  struct gbm_bo *drm_bo = NULL;
  uint32_t drm_fb_id = 0;
  drmEventContext drm_context = { DRM_EVENT_CONTEXT_VERSION, NULL, NULL };
  int drm_keyboard = -1;
  DIR *drm_input_dir = NULL;
  struct dirent *drm_input_dev = NULL;
  struct libevdev *drm_evdev = NULL;
  struct input_event drm_event;
  #endif
  #if defined(EGL_RPI)
  DISPMANX_DISPLAY_HANDLE_T rpi_dpy = DISPMANX_NO_HANDLE;
  DISPMANX_WINDOW_T *rpi_win = NULL;
  DISPMANX_MODEINFO_T rpi_info;
  DISPMANX_UPDATE_HANDLE_T rpi_update = DISPMANX_NO_HANDLE;
  DISPMANX_ELEMENT_HANDLE_T rpi_element = DISPMANX_NO_HANDLE;
  VC_RECT_T rpi_dst_rect;
  VC_RECT_T rpi_src_rect;
  struct termios *rpi_termios = NULL, rpi_termios_new;
  int rpi_fdflags = -1;
  char rpi_event[5];
  #endif
  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_XCB) || defined(EGL_DRM)
  #ifdef EGL_EXT_platform_base
  const char *egl_extension_name = NULL, *egl_extensions = NULL;
  PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = NULL;
  PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC eglCreatePlatformWindowSurfaceEXT = NULL;
  #endif
  #endif
  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_XCB) || defined(EGL_DRM) || defined(EGL_RPI)
  EGLDisplay egl_dpy = NULL;
  EGLSurface egl_win = NULL;
  EGLint egl_config_attr[16];
  EGLint egl_configs_count = 0;
  EGLConfig *egl_configs = NULL, egl_config = NULL;
  EGLint egl_ctx_attr[3];
  EGLContext egl_ctx = NULL;
  EGLint egl_major_version = 0, egl_minor_version = 0, egl_depth_size = 0, egl_red_size = 0, egl_green_size = 0, egl_blue_size = 0, egl_alpha_size = 0;
  #endif
  #if defined(WAFFLE)
  struct waffle_display *waffle_dpy = NULL;
  struct waffle_window *waffle_win = NULL;
  int waffle_init_attr[3];
  int waffle_config_attr[16];
  struct waffle_config *waffle_config = NULL;
  struct waffle_context *waffle_ctx = NULL;
  struct libinput *waffle_input = NULL;
  struct libinput_event *waffle_event = NULL;
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
  #if defined(EGL_XCB)
  strcat(backends, "egl-xcb ");
  #endif
  #if defined(EGL_DRM)
  strcat(backends, "egl-drm ");
  #endif
  #if defined(EGL_RPI)
  strcat(backends, "egl-rpi ");
  #endif
  #if defined(WAFFLE)
  strcat(backends, "waffle ");
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

  #if !defined(ENGINE_CTOR) && defined(YAGEARS_ENGINE)
  engine_ctor(YAGEARS_ENGINE);
  #endif

  #if defined(YAGEARS_BACKEND) && defined(YAGEARS_ENGINE)
  if (argc == 1) {
    backend_arg = STRINGIFY(YAGEARS_BACKEND);
    engine_arg = STRINGIFY(YAGEARS_ENGINE);
  }
  else
  #endif
  if (argc != 5 || !backend_arg || !engine_arg) {
    printf("\n\tUsage: %s -b Backend -e Engine\n\n", argv[0]);
    printf("\t\tBackends: %s\n\n", backends);
    printf("\t\tEngines:  ");
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
    printf("%s: Backend unknown\n", backend_arg);
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
    fb_dpy = open(getenv("FRAMEBUFFER") ? getenv("FRAMEBUFFER") : "/dev/fb0", O_RDWR);
    if (fb_dpy == -1) {
      printf("open %s failed: %m\n", getenv("FRAMEBUFFER") ? getenv("FRAMEBUFFER") : "/dev/fb0");
      goto out;
    }

    memset(&fb_finfo, 0, sizeof(struct fb_fix_screeninfo));
    err = ioctl(fb_dpy, FBIOGET_FSCREENINFO, &fb_finfo);
    if (err == -1) {
      printf("ioctl FBIOGET_FSCREENINFO failed: %m\n");
      goto out;
    }

    memset(&fb_vinfo, 0, sizeof(struct fb_var_screeninfo));
    err = ioctl(fb_dpy, FBIOGET_VSCREENINFO, &fb_vinfo);
    if (err == -1) {
      printf("ioctl FBIOGET_VSCREENINFO failed: %m\n");
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
  #if defined(EGL_XCB)
  if (!strcmp(backend, "egl-xcb")) {
    xcb_dpy = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(xcb_dpy)) {
      printf("xcb_connect failed\n");
      goto out;
    }

    win_width = xcb_setup_roots_iterator(xcb_get_setup(xcb_dpy)).data->width_in_pixels;
    win_height = xcb_setup_roots_iterator(xcb_get_setup(xcb_dpy)).data->height_in_pixels;
  }
  #endif
  #if defined(EGL_DRM)
  if (!strcmp(backend, "egl-drm")) {
    drm_fd = open(getenv("DRICARD") ? getenv("DRICARD") : "/dev/dri/card0", O_RDWR);
    if (drm_fd == -1) {
      printf("open %s failed: %m\n", getenv("DRICARD") ? getenv("DRICARD") : "/dev/dri/card0");
      goto out;
    }

    #ifdef HAVE_DRI
    if (getenv("NO_GBM")) {
      drm_dpy = calloc(1, sizeof(struct drm_display));
      if (!drm_dpy) {
        printf("drm_display calloc failed: %m\n");
        goto out;
      }

      drm_dpy->fd = drm_fd;
      drm_dpy->name = "drm";
      if (getenv("DRI_DRIVER")) {
        drm_dpy->driver_name = getenv("DRI_DRIVER");
      }
      else {
        #if DRI_MAJOR_VERSION > 10 || (DRI_MAJOR_VERSION == 10 && DRI_MINOR_VERSION >= 3)
        drm_dpy->driver_name = "kms_swrast";
        #else
        printf("DRI_DRIVER is not set\n");
        goto out;
        #endif
      }

      sprintf(drm_driver_path, "%s/%s_dri.so", DRI_DRIVERDIR, drm_dpy->driver_name);
      drm_dpy->driver = dlopen(drm_driver_path, RTLD_LAZY);
      if (!drm_dpy->driver) {
        printf("%s DRI driver not found\n", drm_dpy->driver_name);
        goto out;
      }

      drm_driver_extensions = dlsym(drm_dpy->driver, "__driDriverExtensions");
      if (!drm_dpy->driver) {
        printf("DRI DriverExtensions not found\n");
        goto out;
      }

      drm_dpy->core = drm_driver_extensions[0];
      drm_dpy->dri2 = (struct __DRIdri2ExtensionRec *)drm_driver_extensions[2];
      drm_dpy->screen = drm_dpy->dri2->createNewScreen2(0, drm_dpy->fd, drm_extensions, NULL, &drm_dpy->driver_configs, NULL);
      if (!drm_dpy->screen) {
        printf("DRI createNewScreen2 failed\n");
        goto out;
      }

      drm_dpy->flush = (struct __DRI2flushExtensionRec *)drm_dpy->core->getExtensions(drm_dpy->screen)[1];
      drm_dpy->image = (struct __DRIimageExtensionRec *)drm_dpy->core->getExtensions(drm_dpy->screen)[2];
      drm_dpy->bo_create = drm_bo_create;
      drm_dpy->bo_destroy = drm_bo_destroy;
    }
    else
    #endif
    {
      drm_dpy = gbm_create_device(drm_fd);
      if (!drm_dpy) {
        printf("gbm_create_device failed\n");
        goto out;
      }
    }

    drm_resources = drmModeGetResources(drm_fd);
    if (!drm_resources) {
      printf("drmModeGetResources failed: %m\n");
      goto out;
    }

    for (opt = 0; opt < drm_resources->count_connectors; opt++) {
      drm_connector = drmModeGetConnector(drm_fd, drm_resources->connectors[opt]);
      if (!drm_connector) {
        printf("drmModeGetConnector %d failed: %m\n", opt);
        continue;
      }
      else {
        if (drm_connector->connection == DRM_MODE_CONNECTED)
          break;
        else {
          drmModeFreeConnector(drm_connector);
          drm_connector = NULL;
        }
      }
    }

    if (!drm_connector)
      goto out;

    if (drm_connector->encoder_id) {
      drm_encoder = drmModeGetEncoder(drm_fd, drm_connector->encoder_id);
      if (!drm_encoder) {
        printf("drmModeGetEncoder failed: %m\n");
        goto out;
      }
    }
    else {
      for (opt = 0; opt < drm_resources->count_encoders; opt++) {
        drm_encoder = drmModeGetEncoder(drm_fd, drm_connector->encoders[opt]);
        if (!drm_encoder) {
          printf("drmModeGetEncoder %d failed: %m\n", opt);
          continue;
        }
        else
          break;
      }

      if (!drm_encoder)
        goto out;
    }

    if (drm_encoder->crtc_id) {
      drm_crtc = drmModeGetCrtc(drm_fd, drm_encoder->crtc_id);
      if (!drm_crtc) {
        printf("drmModeGetCrtc failed: %m\n");
        goto out;
      }
    }
    else {
      for (opt = 0; opt < drm_resources->count_crtcs; opt++) {
        if (drm_encoder->possible_crtcs & (1 << opt))
          break;
      }

      drm_crtc = drmModeGetCrtc(drm_fd, drm_resources->crtcs[opt]);
      if (!drm_crtc) {
        printf("drmModeGetCrtc %d failed: %m\n", opt);
        goto out;
      }
    }

    win_width = drm_connector->modes[0].hdisplay;
    win_height = drm_connector->modes[0].vdisplay;
  }
  #endif
  #if defined(EGL_RPI)
  if (!strcmp(backend, "egl-rpi")) {
    bcm_host_init();

    rpi_dpy = vc_dispmanx_display_open(DISPMANX_ID_MAIN_LCD);
    if (rpi_dpy == DISPMANX_NO_HANDLE) {
      printf("vc_dispmanx_element_add failed\n");
      goto out;
    }

    memset(&rpi_info, 0, sizeof(DISPMANX_MODEINFO_T));
    err = vc_dispmanx_display_get_info(rpi_dpy, &rpi_info);
    if (err == -1) {
      printf("vc_dispmanx_display_get_info failed\n");
      goto out;
    }

    win_width = rpi_info.width;
    win_height = rpi_info.height;
  }
  #endif
  #if defined(WAFFLE)
  if (!strcmp(backend, "waffle")) {
    if (!getenv("PLATFORM")) {
      printf("\nPLATFORM is not set:\n"
             "  19 -> GLX\n"
             "  20 -> EGL-Wayland\n"
             "  21 -> EGL-Xlib\n"
             "  22 -> EGL-DRM\n"
             "  ...\n\n");
      goto out;
    }

    waffle_init_attr[0] = WAFFLE_PLATFORM;
    waffle_init_attr[1] = atoi(getenv("PLATFORM"));
    waffle_init_attr[2] = WAFFLE_NONE;
    err = waffle_init(waffle_init_attr);
    if (!err) {
      printf("waffle_init failed: 0x%x\n", waffle_error_get_code());
      goto out;
    }

    waffle_dpy = waffle_display_connect(NULL);
    if (!waffle_dpy) {
      printf("waffle_display_connect failed: 0x%x\n", waffle_error_get_code());
      goto out;
    }

    if (!getenv("WIDTH") || !getenv("HEIGHT")) {
      printf("WIDTH or HEIGHT is not set\n");
      goto out;
    }
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

  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_XCB) || defined(EGL_DRM)
  #ifdef EGL_EXT_platform_base
  #if defined(EGL_X11)
  if (!strcmp(backend, "egl-x11")) {
    egl_extension_name = "EGL_EXT_platform_x11";
  }
  #endif
  #if defined(EGL_DIRECTFB)
  if (!strcmp(backend, "egl-directfb")) {
    egl_extension_name = "EGL_EXT_platform_directfb";
  }
  #endif
  #if defined(EGL_FBDEV)
  if (!strcmp(backend, "egl-fbdev")) {
    egl_extension_name = "EGL_EXT_platform_fbdev";
  }
  #endif
  #if defined(EGL_WAYLAND)
  if (!strcmp(backend, "egl-wayland")) {
    egl_extension_name = "EGL_EXT_platform_wayland";
  }
  #endif
  #if defined(EGL_XCB)
  if (!strcmp(backend, "egl-xcb")) {
    egl_extension_name = "EGL_EXT_platform_xcb";
  }
  #endif
  #if defined(EGL_DRM)
  if (!strcmp(backend, "egl-drm")) {
    egl_extension_name = "EGL_KHR_platform_gbm";
  }
  #endif

  if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev") || !strcmp(backend, "egl-wayland") || !strcmp(backend, "egl-xcb") || !strcmp(backend, "egl-drm")) {
    egl_extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (!getenv("NO_EGL_EXT_PLATFORM") && egl_extensions && strstr(egl_extensions, egl_extension_name)) {
      eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");
      eglCreatePlatformWindowSurfaceEXT = (PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC)eglGetProcAddress("eglCreatePlatformWindowSurfaceEXT");
    }
  }
  #endif
  #endif

  #if defined(EGL_X11)
  if (!strcmp(backend, "egl-x11")) {
    #if defined(EGL_EXT_platform_base) && defined(EGL_PLATFORM_X11_EXT)
    if (eglGetPlatformDisplayEXT) {
      egl_dpy = eglGetPlatformDisplayEXT(EGL_PLATFORM_X11_EXT, x11_dpy, NULL);
    }
    else
    #endif
    {
      setenv("EGL_PLATFORM", "x11", 1);
      egl_dpy = eglGetDisplay((EGLNativeDisplayType)x11_dpy);
    }
  }
  #endif
  #if defined(EGL_DIRECTFB)
  if (!strcmp(backend, "egl-directfb")) {
    #if defined(EGL_EXT_platform_base) && defined(EGL_PLATFORM_DIRECTFB_EXT)
    if (eglGetPlatformDisplayEXT) {
      egl_dpy = eglGetPlatformDisplayEXT(EGL_PLATFORM_DIRECTFB_EXT, dfb_dpy, NULL);
    }
    else
    #endif
    {
      setenv("EGL_PLATFORM", "directfb", 1);
      egl_dpy = eglGetDisplay((EGLNativeDisplayType)dfb_dpy);
    }
  }
  #endif
  #if defined(EGL_FBDEV)
  if (!strcmp(backend, "egl-fbdev")) {
    #if defined(EGL_EXT_platform_base) && defined(EGL_PLATFORM_FBDEV_EXT)
    if (eglGetPlatformDisplayEXT) {
      egl_dpy = eglGetPlatformDisplayEXT(EGL_PLATFORM_FBDEV_EXT, &fb_dpy, NULL);
    }
    else
    #endif
    {
      setenv("EGL_PLATFORM", "fbdev", 1);
      egl_dpy = eglGetDisplay((EGLNativeDisplayType)(long)fb_dpy);
    }
  }
  #endif
  #if defined(EGL_WAYLAND)
  if (!strcmp(backend, "egl-wayland")) {
    #if defined(EGL_EXT_platform_base) && defined(EGL_PLATFORM_WAYLAND_EXT)
    if (eglGetPlatformDisplayEXT) {
      egl_dpy = eglGetPlatformDisplayEXT(EGL_PLATFORM_WAYLAND_EXT, wl_dpy, NULL);
    }
    else
    #endif
    {
      setenv("EGL_PLATFORM", "wayland", 1);
      egl_dpy = eglGetDisplay((EGLNativeDisplayType)wl_dpy);
    }
  }
  #endif
  #if defined(EGL_XCB)
  if (!strcmp(backend, "egl-xcb")) {
    #if defined(EGL_EXT_platform_base) && defined(EGL_PLATFORM_XCB_EXT)
    if (eglGetPlatformDisplayEXT) {
      egl_dpy = eglGetPlatformDisplayEXT(EGL_PLATFORM_XCB_EXT, xcb_dpy, NULL);
    }
    else
    #endif
    {
      setenv("EGL_PLATFORM", "xcb", 1);
      egl_dpy = eglGetDisplay((EGLNativeDisplayType)xcb_dpy);
    }
  }
  #endif
  #if defined(EGL_DRM)
  if (!strcmp(backend, "egl-drm")) {
    #if defined(EGL_EXT_platform_base) && defined(EGL_PLATFORM_GBM_KHR)
    if (eglGetPlatformDisplayEXT) {
      egl_dpy = eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_KHR, drm_dpy, NULL);
    }
    else
    #endif
    {
      setenv("EGL_PLATFORM", "drm", 1);
      egl_dpy = eglGetDisplay((EGLNativeDisplayType)drm_dpy);
    }
  }
  #endif
  #if defined(EGL_RPI)
  if (!strcmp(backend, "egl-rpi")) {
    egl_dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  }
  #endif
  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_XCB) || defined(EGL_DRM) || defined(EGL_RPI)
  if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev") || !strcmp(backend, "egl-wayland") || !strcmp(backend, "egl-xcb") || !strcmp(backend, "egl-drm") || !strcmp(backend, "egl-rpi")) {
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
      printf("mmap failed: %m\n");
      goto out;
    }

    fb_buffer = glFBDevCreateBuffer(&fb_finfo, &fb_vinfo, fb_visual, fb_addr, NULL, fb_finfo.smem_len);
    if (!fb_buffer) {
      printf("glFBDevCreateBuffer failed\n");
      goto out;
    }
  }
  #endif

  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_XCB) || defined(EGL_DRM) || defined(EGL_RPI)
  if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev") || !strcmp(backend, "egl-wayland") || !strcmp(backend, "egl-xcb") || !strcmp(backend, "egl-drm") || !strcmp(backend, "egl-rpi")) {
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

    opt = 0;
    egl_config_attr[opt++] = EGL_RED_SIZE;
    egl_config_attr[opt++] = 1;
    egl_config_attr[opt++] = EGL_GREEN_SIZE;
    egl_config_attr[opt++] = 1;
    egl_config_attr[opt++] = EGL_BLUE_SIZE;
    egl_config_attr[opt++] = 1;
    egl_config_attr[opt++] = EGL_DEPTH_SIZE;
    egl_config_attr[opt++] = 1;
    egl_config_attr[opt++] = EGL_RENDERABLE_TYPE;
    egl_config_attr[opt++] = !gears_engine_version(gears_engine) ? EGL_OPENGL_BIT : gears_engine_version(gears_engine) == 1 ? EGL_OPENGL_ES_BIT : EGL_OPENGL_ES2_BIT;
    egl_config_attr[opt] = EGL_NONE;
    err = eglChooseConfig(egl_dpy, egl_config_attr, NULL, 0, &egl_configs_count);
    if (!err || !egl_configs_count) {
      printf("eglChooseConfig failed: 0x%x, %d\n", eglGetError(), egl_configs_count);
      goto out;
    }

    egl_configs = calloc(egl_configs_count, sizeof(EGLConfig));
    if (!egl_configs) {
      printf("calloc failed: %m\n");
      goto out;
    }

    err = eglChooseConfig(egl_dpy, egl_config_attr, egl_configs, egl_configs_count, &egl_configs_count);
    if (!err) {
      printf("eglChooseConfig failed: 0x%x\n", eglGetError());
      goto out;
    }

    egl_config = egl_configs[0];
  }
  #endif

  #if defined(WAFFLE)
  if (!strcmp(backend, "waffle")) {
    opt = 0;
    waffle_config_attr[opt++] = WAFFLE_RED_SIZE;
    waffle_config_attr[opt++] = 1;
    waffle_config_attr[opt++] = WAFFLE_GREEN_SIZE;
    waffle_config_attr[opt++] = 1;
    waffle_config_attr[opt++] = WAFFLE_BLUE_SIZE;
    waffle_config_attr[opt++] = 1;
    waffle_config_attr[opt++] = WAFFLE_ALPHA_SIZE;
    waffle_config_attr[opt++] = 1;
    waffle_config_attr[opt++] = WAFFLE_DEPTH_SIZE;
    waffle_config_attr[opt++] = 1;
    waffle_config_attr[opt++] = WAFFLE_CONTEXT_API;
    waffle_config_attr[opt++] = !gears_engine_version(gears_engine) ? WAFFLE_CONTEXT_OPENGL : gears_engine_version(gears_engine) == 1 ? WAFFLE_CONTEXT_OPENGL_ES1 : WAFFLE_CONTEXT_OPENGL_ES2;
    waffle_config_attr[opt++] = WAFFLE_NONE;
    waffle_config = waffle_config_choose(waffle_dpy, waffle_config_attr);
    if (!waffle_config) {
      printf("waffle_config_choose failed: 0x%x\n", waffle_error_get_code());
      goto out;
    }
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

    XMoveWindow(x11_dpy, x11_win, win_posx, win_posy);

    x11_event_mask = ExposureMask | KeyPressMask;
    XSelectInput(x11_dpy, x11_win, x11_event_mask);
  }
  #endif
  #if defined(GL_DIRECTFB) || defined(EGL_DIRECTFB)
  if (!strcmp(backend, "gl-directfb") || !strcmp(backend, "egl-directfb")) {
    memset(&dfb_desc, 0, sizeof(DFBWindowDescription));
    dfb_desc.flags = DWDESC_SURFACE_CAPS | DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_POSX | DWDESC_POSY;
    dfb_desc.surface_caps = dfb_attr;
    if (getenv("DSCAPS_GL")) {
      dfb_desc.surface_caps |= DSCAPS_GL;
    }
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
    fb_win = calloc(1, sizeof(struct fb_window));
    if (!fb_win) {
      printf("fb_window calloc failed: %m\n");
      goto out;
    }

    fb_win->width = win_width;
    fb_win->height = win_height;
    fb_win->posx = win_posx;
    fb_win->posy = win_posy;

    if (getenv("KEYBOARD")) {
      fb_keyboard = open(getenv("KEYBOARD"), O_RDONLY | O_NONBLOCK);
    }
    else {
      fb_input_dir = opendir("/dev/input");
      if (!fb_input_dir) {
        printf("opendir /dev/input failed: %m\n");
        goto out;
      }

      c = alloca(64);

      while ((fb_input_dev = readdir(fb_input_dir))) {
        if (fb_input_dev->d_type == DT_CHR) {
          sprintf(c, "/dev/input/%s", fb_input_dev->d_name);
          fb_keyboard = open(c, O_RDONLY | O_NONBLOCK);
          if (fb_keyboard == -1)
            continue;

          err = ioctl(fb_keyboard, EVIOCGBIT(EV_KEY, sizeof(fb_key_bits)), fb_key_bits);
          if (err == -1)
            continue;

          if (fb_key_bits[KEY_ENTER / 8] & (1 << (KEY_ENTER % 8)))
            break;

          close(fb_keyboard);
          fb_keyboard = -1;
        }
      }
    }

    if (fb_keyboard == -1) {
      printf("open keyboard event device failed\n");
      goto out;
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

    wl_shell_surface = wl_shell_get_shell_surface(wl_data.wl_shell, wl_surface);
    if (!wl_shell_surface) {
      printf("wl_shell_get_shell_surface failed\n");
      goto out;
    }

    wl_shell_surface_set_toplevel(wl_shell_surface);

    #ifdef HAVE_WL_SHELL_SURFACE_SET_POSITION
    wl_shell_surface_set_position(wl_shell_surface, win_posx, win_posy);
    #endif

    if (getenv("NO_WL_EGL_WINDOW")) {
      wl_win = calloc(1, sizeof(struct wl_window));
      if (!wl_win) {
        printf("wl_window calloc failed: %m\n");
        goto out;
      }

      wl_win->surface = wl_surface;
      wl_win->width = win_width;
      wl_win->height = win_height;
    }
    else {
      wl_win = wl_egl_window_create(wl_surface, win_width, win_height);
      if (!wl_win) {
        printf("wl_egl_window_create failed\n");
        goto out;
      }
    }

    wl_data.xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!wl_data.xkb_context) {
      printf("xkb_context_new failed\n");
      goto out;
    }
  }
  #endif
  #if defined(EGL_XCB)
  if (!strcmp(backend, "egl-xcb")) {
    xcb_win = xcb_generate_id(xcb_dpy);
    xcb_event_mask = XCB_EVENT_MASK_KEY_PRESS;
    xcb_value_list[0] = 0;
    xcb_value_list[1] = xcb_event_mask;
    xcb_cookie = xcb_create_window_checked(xcb_dpy, XCB_COPY_FROM_PARENT, xcb_win, xcb_setup_roots_iterator(xcb_get_setup(xcb_dpy)).data->root, win_posx, win_posy, win_width, win_height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, xcb_setup_roots_iterator(xcb_get_setup(xcb_dpy)).data->root_visual, XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK, xcb_value_list);
    xcb_win = xcb_request_check(xcb_dpy, xcb_cookie) ? -1 : xcb_win;
    if (xcb_win == -1) {
      printf("xcb_create_window failed\n");
      goto out;
    }

    xcb_map_window(xcb_dpy, xcb_win);

    xcb_value_list[0] = win_posx;
    xcb_value_list[1] = win_posy;
    xcb_configure_window(xcb_dpy, xcb_win, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, xcb_value_list);

    xcb_flush(xcb_dpy);
  }
  #endif
  #if defined(EGL_DRM)
  if (!strcmp(backend, "egl-drm")) {
    #ifdef HAVE_DRI
    if (getenv("NO_GBM")) {
      drm_win = calloc(1, sizeof(struct drm_surface));
      if (!drm_win) {
        printf("drm_surface calloc failed: %m\n");
        goto out;
      }

      drm_win->display = drm_dpy;
      drm_win->width = drm_connector->modes[0].hdisplay;
      drm_win->height = drm_connector->modes[0].vdisplay;
    }
    else
    #endif
    {
      drm_win = gbm_surface_create(drm_dpy, drm_connector->modes[0].hdisplay, drm_connector->modes[0].vdisplay, GBM_FORMAT_XRGB8888, GBM_BO_USE_SCANOUT);
      if (!drm_win) {
        printf("gbm_surface_create failed\n");
        goto out;
      }
    }

    if (getenv("KEYBOARD")) {
      drm_keyboard = open(getenv("KEYBOARD"), O_RDONLY | O_NONBLOCK);
    }
    else {
      drm_input_dir = opendir("/dev/input");
      if (!drm_input_dir) {
        printf("opendir /dev/input failed: %m\n");
        goto out;
      }

      c = alloca(64);

      while ((drm_input_dev = readdir(drm_input_dir))) {
        if (drm_input_dev->d_type == DT_CHR) {
          sprintf(c, "/dev/input/%s", drm_input_dev->d_name);
          drm_keyboard = open(c, O_RDONLY | O_NONBLOCK);
          if (drm_keyboard == -1)
            continue;

          err = libevdev_new_from_fd(drm_keyboard, &drm_evdev);
          if (err < 0)
            continue;

          if (libevdev_has_event_code(drm_evdev, EV_KEY, KEY_ENTER))
            break;

          libevdev_free(drm_evdev);
          close(drm_keyboard);
          drm_keyboard = -1;
        }
      }
    }

    if (drm_keyboard == -1) {
      printf("open keyboard event device failed\n");
      goto out;
    }
  }
  #endif
  #if defined(EGL_RPI)
  if (!strcmp(backend, "egl-rpi")) {
    rpi_update = vc_dispmanx_update_start(0);
    if (rpi_update == DISPMANX_NO_HANDLE) {
      printf("vc_dispmanx_update_start failed\n");
      goto out;
    }

    rpi_dst_rect.x = win_posx;
    rpi_dst_rect.y = win_posy;
    rpi_dst_rect.width = win_width;
    rpi_dst_rect.height = win_height;

    rpi_src_rect.x = 0;
    rpi_src_rect.y = 0;
    rpi_src_rect.width  = win_width << 16;
    rpi_src_rect.height = win_height << 16;

    rpi_element = vc_dispmanx_element_add(rpi_update, rpi_dpy,  0, &rpi_dst_rect, DISPMANX_NO_HANDLE, &rpi_src_rect, DISPMANX_PROTECTION_NONE, NULL, NULL, DISPMANX_NO_ROTATE);
    if (rpi_element == DISPMANX_NO_HANDLE) {
      printf("vc_dispmanx_element_add failed\n");
      goto out;
    }

    err = vc_dispmanx_update_submit_sync(rpi_update);
    if (err == -1) {
      printf("vc_dispmanx_update_submit_sync failed\n");
      goto out;
    }

    rpi_win = calloc(1, sizeof(DISPMANX_WINDOW_T));
    if (!rpi_win) {
      printf("DISPMANX_WINDOW_T calloc failed: %m\n");
      goto out;
    }

    rpi_win->element = rpi_element;
    rpi_win->width = win_width;
    rpi_win->height = win_height;

    rpi_termios = calloc(1, sizeof(struct termios));
    if (!rpi_termios) {
      printf("rpi_termios calloc failed: %m\n");
      goto out;
    }

    err = tcgetattr(STDIN_FILENO, rpi_termios);
    if (err == -1) {
      printf("tcgetattr failed: %m\n");
      goto out;
    }

    memcpy(&rpi_termios_new, rpi_termios, sizeof(struct termios));
    rpi_termios_new.c_lflag &= ~(ICANON | ECHO);

    err = tcsetattr(STDIN_FILENO, TCSANOW, &rpi_termios_new);
    if (err == -1) {
      printf("tcsetattr failed: %m\n");
      goto out;
    }

    rpi_fdflags = fcntl(STDIN_FILENO, F_GETFL);
    if (rpi_fdflags == -1) {
      printf("fcntl F_GETFL failed: %m\n");
      goto out;
    }

    err = fcntl(STDIN_FILENO, F_SETFL, rpi_fdflags | O_NONBLOCK);
    if (err == -1) {
      printf("fcntl F_SETFL failed: %m\n");
      goto out;
    }
  }
  #endif

  #if defined(EGL_X11)
  if (!strcmp(backend, "egl-x11")) {
    #if defined(EGL_EXT_platform_base) && defined(EGL_PLATFORM_X11_EXT)
    if (eglCreatePlatformWindowSurfaceEXT) {
      egl_win = eglCreatePlatformWindowSurfaceEXT(egl_dpy, egl_config, &x11_win, NULL);
    }
    else
    #endif
    {
      egl_win = eglCreateWindowSurface(egl_dpy, egl_config, (EGLNativeWindowType)x11_win, NULL);
    }
  }
  #endif
  #if defined(EGL_DIRECTFB)
  if (!strcmp(backend, "egl-directfb")) {
    #if defined(EGL_EXT_platform_base) && defined(EGL_PLATFORM_DIRECTFB_EXT)
    if (eglCreatePlatformWindowSurfaceEXT) {
      egl_win = eglCreatePlatformWindowSurfaceEXT(egl_dpy, egl_config, dfb_win, NULL);
    }
    else
    #endif
    {
      egl_win = eglCreateWindowSurface(egl_dpy, egl_config, (EGLNativeWindowType)dfb_win, NULL);
    }
  }
  #endif
  #if defined(EGL_FBDEV)
  if (!strcmp(backend, "egl-fbdev")) {
    #if defined(EGL_EXT_platform_base) && defined(EGL_PLATFORM_FBDEV_EXT)
    if (eglCreatePlatformWindowSurfaceEXT) {
      egl_win = eglCreatePlatformWindowSurfaceEXT(egl_dpy, egl_config, fb_win, NULL);
    }
    else
    #endif
    {
      egl_win = eglCreateWindowSurface(egl_dpy, egl_config, (EGLNativeWindowType)fb_win, NULL);
    }
  }
  #endif
  #if defined(EGL_WAYLAND)
  if (!strcmp(backend, "egl-wayland")) {
    #if defined(EGL_EXT_platform_base) && defined(EGL_PLATFORM_WAYLAND_EXT)
    if (eglCreatePlatformWindowSurfaceEXT) {
      egl_win = eglCreatePlatformWindowSurfaceEXT(egl_dpy, egl_config, wl_win, NULL);
    }
    else
    #endif
    {
      egl_win = eglCreateWindowSurface(egl_dpy, egl_config, (EGLNativeWindowType)wl_win, NULL);
    }
  }
  #endif
  #if defined(EGL_XCB)
  if (!strcmp(backend, "egl-xcb")) {
    #if defined(EGL_EXT_platform_base) && defined(EGL_PLATFORM_XCB_EXT)
    if (eglCreatePlatformWindowSurfaceEXT) {
      egl_win = eglCreatePlatformWindowSurfaceEXT(egl_dpy, egl_config, &xcb_win, NULL);
    }
    else
    #endif
    {
      egl_win = eglCreateWindowSurface(egl_dpy, egl_config, (EGLNativeWindowType)(long)xcb_win, NULL);
    }
  }
  #endif
  #if defined(EGL_DRM)
  if (!strcmp(backend, "egl-drm")) {
    #if defined(EGL_EXT_platform_base) && defined(EGL_PLATFORM_GBM_KHR)
    if (eglCreatePlatformWindowSurfaceEXT) {
      egl_win = eglCreatePlatformWindowSurfaceEXT(egl_dpy, egl_config, drm_win, NULL);
    }
    else
    #endif
    {
      egl_win = eglCreateWindowSurface(egl_dpy, egl_config, (EGLNativeWindowType)drm_win, NULL);
    }
  }
  #endif
  #if defined(EGL_RPI)
  if (!strcmp(backend, "egl-rpi")) {
    egl_win = eglCreateWindowSurface(egl_dpy, egl_config, (EGLNativeWindowType)rpi_win, NULL);
  }
  #endif
  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_XCB) || defined(EGL_DRM) || defined(EGL_RPI)
  if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev") || !strcmp(backend, "egl-wayland") || !strcmp(backend, "egl-xcb") || !strcmp(backend, "egl-drm") || !strcmp(backend, "egl-rpi")) {
    if (!egl_win) {
      printf("eglCreateWindowSurface failed: 0x%x\n", eglGetError());
      goto out;
    }
  }
  #endif

  #if defined(WAFFLE)
  if (!strcmp(backend, "waffle")) {
    waffle_win = waffle_window_create(waffle_config, win_width, win_height);
    if (!waffle_win) {
      printf("waffle_window_create failed: 0x%x\n", waffle_error_get_code());
      goto out;
    }

    waffle_window_show(waffle_win);

    waffle_input = libinput_path_create_context(&waffle_input_interface, NULL);
    if (!waffle_input) {
      printf("libinput_path_create_context failed\n");
      goto out;
    }

    if (!libinput_path_add_device(waffle_input, getenv("KEYBOARD") ? getenv("KEYBOARD") : "/dev/input/event0")) {
      printf("libinput_path_add_device %s failed\n", getenv("KEYBOARD") ? getenv("KEYBOARD") : "/dev/input/event0");
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
    c = alloca(2);
    sprintf(c, "%d", gears_engine_version(gears_engine));
    DirectFBSetOption("gles", c);

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
    fb_ctx = glFBDevCreateContext(fb_visual, NULL);
    if (!fb_ctx) {
      printf("glFBDevCreateContext failed\n");
      goto out;
    }

    glFBDevSetWindow(fb_buffer, fb_win);
    err = glFBDevMakeCurrent(fb_ctx, fb_buffer, fb_buffer);
    if (!err) {
      printf("glFBDevMakeCurrent failed\n");
      goto out;
    }
  }
  #endif

  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_XCB) || defined(EGL_DRM) || defined(EGL_RPI)
  if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev") || !strcmp(backend, "egl-wayland") || !strcmp(backend, "egl-xcb") || !strcmp(backend, "egl-drm") || !strcmp(backend, "egl-rpi")) {
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

  #if defined(WAFFLE)
  if (!strcmp(backend, "waffle")) {
    waffle_ctx = waffle_context_create(waffle_config, NULL);
    if (!waffle_ctx) {
      printf("waffle_context_create failed: 0x%x\n", waffle_error_get_code());
      goto out;
    }

    err = waffle_make_current(waffle_dpy, waffle_win, waffle_ctx);
    if (!err) {
      printf("waffle_make_current failed: 0x%x\n", waffle_error_get_code());
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
        printf("gettimeofday failed: %m\n");
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
        if (getenv("DSCAPS_GL")) {
          dfb_ctx->SwapBuffers(dfb_ctx);
        }
        else {
          dfb_win->Flip(dfb_win, NULL, DSFLIP_WAITFORSYNC);
        }
      }
      #endif
      #if defined(GL_FBDEV)
      if (!strcmp(backend, "gl-fbdev")) {
        glFBDevSwapBuffers(fb_buffer);
      }
      #endif

      #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_XCB) || defined(EGL_DRM) || defined(EGL_RPI)
      if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev") || !strcmp(backend, "egl-wayland") || !strcmp(backend, "egl-xcb") || !strcmp(backend, "egl-drm") || !strcmp(backend, "egl-rpi")) {
        eglSwapBuffers(egl_dpy, egl_win);
      }
      #endif

      #if defined(WAFFLE)
      if (!strcmp(backend, "waffle")) {
        waffle_window_swap_buffers(waffle_win);
      }
      #endif
    }

    #if defined(GL_X11) || defined(EGL_X11)
    if (!strcmp(backend, "gl-x11") || !strcmp(backend, "egl-x11")) {
      if (!animate && redisplay) {
        redisplay = 0;
      }

      memset(&x11_event, 0, sizeof(XEvent));
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
      if (redisplay) {
        if (getenv("DSCAPS_GL")) {
          dfb_win->Flip(dfb_win, NULL, DSFLIP_WAITFORSYNC);
        }
        if (!animate) {
          redisplay = 0;
        }
      }

      memset(&dfb_event, 0, sizeof(DFBWindowEvent));
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

      memset(&fb_event, 0, sizeof(struct input_event));
      if (read(fb_keyboard, &fb_event, sizeof(struct input_event)) > 0 && fb_event.type == EV_KEY) {
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
    #if defined(EGL_XCB)
    if (!strcmp(backend, "egl-xcb")) {
      if (!animate && redisplay) {
        redisplay = 0;
      }

      xcb_event = xcb_poll_for_event(xcb_dpy);
      if (xcb_event) {
        if ((xcb_event->response_type & 0x7f) == XCB_KEY_PRESS) {
          xcb_keyboard_handle_key(xcb_event);
        }
        free(xcb_event);
      }
    }
    #endif
    #if defined(EGL_DRM)
    if (!strcmp(backend, "egl-drm")) {
      if (redisplay) {
        #ifdef HAVE_DRI
        if (getenv("NO_GBM")) {
          drm_bo = drm_dpy->surface_lock_front_buffer(drm_win);
        }
        else
        #endif
        {
          drm_bo = gbm_surface_lock_front_buffer(drm_win);
        }
        if (drm_bo) {
          #ifdef HAVE_DRI
          if (getenv("NO_GBM")) {
            drm_fb_id = (uintptr_t)drm_bo->user_data;
            if (!drm_fb_id) {
              drmModeAddFB(drm_fd, drm_bo->width, drm_bo->height, 24, 32, drm_bo->stride, drm_bo->handle, &drm_fb_id);
              drmModeSetCrtc(drm_fd, drm_crtc->crtc_id, drm_fb_id, 0, 0, &drm_connector->connector_id, 1, &drm_connector->modes[0]);
              drm_bo->user_data = (void *)(uintptr_t)drm_fb_id;
              drm_bo->destroy_user_data = drm_destroy_user_data;
            }
          }
          else
          #endif
          {
            drm_fb_id = (uintptr_t)gbm_bo_get_user_data(drm_bo);
            if (!drm_fb_id) {
              drmModeAddFB(drm_fd, gbm_bo_get_width(drm_bo), gbm_bo_get_height(drm_bo), 24, 32, gbm_bo_get_stride(drm_bo), gbm_bo_get_handle(drm_bo).u32, &drm_fb_id);
              drmModeSetCrtc(drm_fd, drm_crtc->crtc_id, drm_fb_id, 0, 0, &drm_connector->connector_id, 1, &drm_connector->modes[0]);
              gbm_bo_set_user_data(drm_bo, (void *)(uintptr_t)drm_fb_id, gbm_destroy_user_data);
            }
          }
          drmModePageFlip(drm_fd, drm_crtc->crtc_id, drm_fb_id, DRM_MODE_PAGE_FLIP_EVENT, NULL);
          drmHandleEvent(drm_fd, &drm_context);
          #ifdef HAVE_DRI
          if (getenv("NO_GBM")) {
            drm_dpy->surface_release_buffer(drm_win, drm_bo);
          }
          else
          #endif
          {
            gbm_surface_release_buffer(drm_win, drm_bo);
          }
        }
        if (!animate) {
          redisplay = 0;
        }
      }

      memset(&drm_event, 0, sizeof(struct input_event));
      if (!libevdev_next_event(drm_evdev, LIBEVDEV_READ_FLAG_NORMAL, &drm_event) && drm_event.type == EV_KEY) {
        if (drm_event.value) {
          drm_keyboard_handle_key(&drm_event);
        }
      }
    }
    #endif
    #if defined(EGL_RPI)
    if (!strcmp(backend, "egl-rpi")) {
      if (!animate && redisplay) {
        redisplay = 0;
      }

      memset(rpi_event, 0, sizeof(rpi_event));
      if (read(STDIN_FILENO, rpi_event, 4) > 0) {
        if (rpi_event[0]) {
          rpi_keyboard_handle_key(rpi_event);
        }
      }
    }
    #endif
    #if defined(WAFFLE)
    if (!strcmp(backend, "waffle")) {
      if (!animate && redisplay) {
        redisplay = 0;
      }

      libinput_dispatch(waffle_input);
      waffle_event = libinput_get_event(waffle_input);
      if (waffle_event && libinput_event_get_type(waffle_event) == LIBINPUT_EVENT_KEYBOARD_KEY) {
        if (libinput_event_keyboard_get_key_state((struct libinput_event_keyboard *)waffle_event)) {
          waffle_keyboard_handle_key((struct libinput_event_keyboard *)waffle_event);
        }
        libinput_event_destroy(waffle_event);
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

  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_XCB) || defined(EGL_DRM) || defined(EGL_RPI)
  if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev") || !strcmp(backend, "egl-wayland") || !strcmp(backend, "egl-xcb") || !strcmp(backend, "egl-drm") || !strcmp(backend, "egl-rpi")) {
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

  /* destroy context */

  #if defined(WAFFLE)
  if (!strcmp(backend, "waffle")) {
    if (waffle_ctx) {
      waffle_make_current(waffle_dpy, NULL, NULL);
      waffle_context_destroy(waffle_ctx);
    }
  }
  #endif

  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_XCB) || defined(EGL_DRM) || defined(EGL_RPI)
  if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev") || !strcmp(backend, "egl-wayland") || !strcmp(backend, "egl-xcb") || !strcmp(backend, "egl-drm") || !strcmp(backend, "egl-rpi")) {
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

  #if defined(WAFFLE)
  if (!strcmp(backend, "waffle")) {
    if (waffle_input) {
      libinput_unref(waffle_input);
    }

    if (waffle_win) {
      waffle_window_destroy(waffle_win);
    }

    if (waffle_config) {
      waffle_config_destroy(waffle_config);
    }

    if (waffle_dpy) {
      waffle_display_disconnect(waffle_dpy);
    }
  }
  #endif

  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_XCB) || defined(EGL_DRM) || defined(EGL_RPI)
  if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev") || !strcmp(backend, "egl-wayland") || !strcmp(backend, "egl-xcb") || !strcmp(backend, "egl-drm") || !strcmp(backend, "egl-rpi")) {
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

  #if defined(EGL_X11) || defined(EGL_DIRECTFB) || defined(EGL_FBDEV) || defined(EGL_WAYLAND) || defined(EGL_XCB) || defined(EGL_DRM)
  if (!strcmp(backend, "egl-x11") || !strcmp(backend, "egl-directfb") || !strcmp(backend, "egl-fbdev") || !strcmp(backend, "egl-wayland") || !strcmp(backend, "egl-xcb") || !strcmp(backend, "egl-drm")) {
    #ifdef EGL_EXT_platform_base
    if (eglGetPlatformDisplayEXT) {
      eglGetPlatformDisplayEXT = NULL;
    }

    if (eglCreatePlatformWindowSurfaceEXT) {
      eglCreatePlatformWindowSurfaceEXT = NULL;
    }
    #endif
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
    if (x11_event_mask) {
      XSelectInput(x11_dpy, x11_win, NoEventMask);
    }

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
    if (dfb_event_buffer) {
      dfb_event_buffer->Release(dfb_event_buffer);
    }

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
    if (fb_keyboard != -1) {
      close(fb_keyboard);
    }

    if (fb_input_dir) {
      closedir(fb_input_dir);
    }

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
    if (wl_data.xkb_state) {
      xkb_state_unref(wl_data.xkb_state);
    }

    if (wl_data.xkb_keymap) {
      xkb_map_unref(wl_data.xkb_keymap);
    }

    if (wl_data.xkb_context) {
      xkb_context_unref(wl_data.xkb_context);
    }

    if (wl_win) {
      if (getenv("NO_WL_EGL_WINDOW")) {
        free(wl_win);
      }
      else {
        wl_egl_window_destroy(wl_win);
      }
    }

    if (wl_shell_surface) {
      wl_shell_surface_destroy(wl_shell_surface);
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
  #if defined(EGL_XCB)
  if (!strcmp(backend, "egl-xcb")) {
    if (xcb_event_mask) {
      xcb_event_mask = XCB_EVENT_MASK_NO_EVENT;
      xcb_change_window_attributes(xcb_dpy, xcb_win, XCB_CW_EVENT_MASK, &xcb_event_mask);
    }

    if (xcb_win != -1) {
      xcb_destroy_window(xcb_dpy, xcb_win);
    }

    if (xcb_dpy) {
      xcb_disconnect(xcb_dpy);
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

    if (drm_input_dir) {
      closedir(drm_input_dir);
    }

    if (drm_win) {
      #ifdef HAVE_DRI
      if (getenv("NO_GBM")) {
        free(drm_win);
      }
      else
      #endif
      {
        gbm_surface_destroy(drm_win);
      }
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
      #ifdef HAVE_DRI
      if (getenv("NO_GBM")) {
        if (drm_dpy->screen) {
          for (opt = 0; drm_dpy->driver_configs[opt]; opt++)
            free(drm_dpy->driver_configs[opt]);
          free(drm_dpy->driver_configs);

          drm_dpy->core->destroyScreen(drm_dpy->screen);
        }

        if (drm_dpy->driver) {
          dlclose(drm_dpy->driver);
        }

        free(drm_dpy);
      }
      else
      #endif
      {
        gbm_device_destroy(drm_dpy);
      }
    }

    if (drm_fd != -1) {
      close(drm_fd);
    }
  }
  #endif
  #if defined(EGL_RPI)
  if (!strcmp(backend, "egl-rpi")) {
    if (rpi_fdflags != -1) {
      fcntl(STDIN_FILENO, F_SETFL, rpi_fdflags);
    }

    if (rpi_termios) {
      tcsetattr(STDIN_FILENO, TCSANOW, rpi_termios);
      free(rpi_termios);
    }

    if (rpi_element != DISPMANX_NO_HANDLE) {
      if ((rpi_update = vc_dispmanx_update_start(0)) != DISPMANX_NO_HANDLE) {
        vc_dispmanx_element_remove(rpi_update, rpi_element);
        vc_dispmanx_update_submit_sync(rpi_update);
      }
    }

    if (rpi_win) {
      free(rpi_win);
    }

    if (rpi_dpy != DISPMANX_NO_HANDLE) {
      vc_dispmanx_display_close(rpi_dpy);
    }

    bcm_host_deinit();
  }
  #endif

  gears_engine_free(gears_engine);

  return ret;
}
