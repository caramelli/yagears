/*
  yagears                  Yet Another Gears OpenGL / Vulkan demo
  Copyright (C) 2013-2023  Nicolas Caramelli

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

#if defined(VK_X11)
#include <X11/Xutil.h>
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#if defined(VK_DIRECTFB)
#include <directfb.h>
#define VK_USE_PLATFORM_DIRECTFB_EXT
#endif
#if defined(VK_FBDEV)
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#define VK_USE_PLATFORM_FBDEV_EXT
#endif
#if defined(VK_WAYLAND)
#include <sys/mman.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif
#if defined(VK_XCB)
#include <xcb/xcb.h>
#define VK_USE_PLATFORM_XCB_KHR
#endif
#if defined(VK_D2D)
#include <fcntl.h>
#include <libevdev/libevdev.h>
#endif
#include <vulkan/vulkan.h>

#include "vulkan_gears.h"

/******************************************************************************/

static char *wsi = NULL;
static gears_t *gears = NULL;

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

#if defined(VK_X11)
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

#if defined(VK_DIRECTFB)
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

#if defined(VK_FBDEV)
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

#if defined(VK_WAYLAND)
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

#if defined(VK_XCB)
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

#if defined(VK_D2D)
static void d2d_keyboard_handle_key(struct input_event *event)
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

/******************************************************************************/

int main(int argc, char *argv[])
{
  int err = 0, ret = EXIT_FAILURE;
  const char *wsi_arg = NULL;
  char wsis[64], *c;
  int opt, t_rate = 0, t_rot = 0, t, frames = 0;
  struct timeval tv;

  #if defined(VK_X11)
  Display *x11_dpy = NULL;
  Window x11_win = 0;
  int x11_event_mask = NoEventMask;
  XEvent x11_event;
  VkXlibSurfaceCreateInfoKHR x11_surface_create_info;
  #endif
  #if defined(VK_DIRECTFB)
  IDirectFB *dfb_dpy = NULL;
  IDirectFBSurface *dfb_win = NULL;
  IDirectFBDisplayLayer *dfb_layer = NULL;
  DFBDisplayLayerConfig dfb_layer_config;
  DFBWindowDescription dfb_desc;
  IDirectFBWindow *dfb_window = NULL;
  IDirectFBEventBuffer *dfb_event_buffer = NULL;
  DFBWindowEventType dfb_event_mask = DWET_ALL;
  DFBWindowEvent dfb_event;
  VkDirectFBSurfaceCreateInfoEXT dfb_surface_create_info;
  #endif
  #if defined(VK_FBDEV)
  int fb_dpy = -1;
  struct fb_window *fb_win = NULL;
  struct fb_fix_screeninfo fb_finfo;
  struct fb_var_screeninfo fb_vinfo;
  int fb_keyboard = -1;
  struct input_event fb_event;
  VkFBDevSurfaceCreateInfoEXT fb_surface_create_info;
  #endif
  #if defined(VK_WAYLAND)
  struct wl_display *wl_dpy = NULL;
  struct wl_surface *wl_win = NULL;
  struct wl_data wl_data;
  struct wl_shell_surface *wl_shell_surface = NULL;
  VkWaylandSurfaceCreateInfoKHR wl_surface_create_info;
  #endif
  #if defined(VK_XCB)
  xcb_connection_t *xcb_dpy = NULL;
  xcb_window_t xcb_win = -1;
  xcb_void_cookie_t xcb_cookie;
  uint32_t xcb_value_list[2];
  xcb_event_mask_t xcb_event_mask = XCB_EVENT_MASK_NO_EVENT;
  xcb_generic_event_t *xcb_event = NULL;
  VkXcbSurfaceCreateInfoKHR xcb_surface_create_info;
  #endif
  #if defined(VK_D2D)
  VkDisplayModeKHR d2d_dpy = VK_NULL_HANDLE;
  VkExtent2D d2d_win = { 0, 0 };
  uint32_t d2d_properties_count, d2d_mode_properties_count;
  VkDisplayPropertiesKHR *d2d_properties = NULL;
  VkDisplayModePropertiesKHR *d2d_mode_properties = NULL;
  int d2d_keyboard = -1;
  struct libevdev *d2d_evdev = NULL;
  struct input_event d2d_event;
  VkDisplaySurfaceCreateInfoKHR d2d_surface_create_info;
  #endif
  const char *vk_extension_name = NULL;
  VkInstanceCreateInfo vk_instance_create_info;
  VkInstance vk_instance = VK_NULL_HANDLE;
  uint32_t vk_physical_devices_count;
  VkPhysicalDevice *vk_physical_devices = NULL, vk_physical_device = VK_NULL_HANDLE;
  VkSurfaceKHR vk_surface = VK_NULL_HANDLE;
  VkDeviceCreateInfo vk_device_create_info;
  VkDeviceQueueCreateInfo vk_device_queue_create_info;
  VkDevice vk_device = VK_NULL_HANDLE;
  VkQueue vk_queue = VK_NULL_HANDLE;
  VkSwapchainCreateInfoKHR vk_swapchain_create_info;
  VkSwapchainKHR vk_swapchain = VK_NULL_HANDLE;
  VkPresentInfoKHR vk_present_info;
  uint32_t vk_index = 0;

  /* process command line */

  memset(wsis, 0, sizeof(wsis));
  #if defined(VK_X11)
  strcat(wsis, "vk-x11 ");
  #endif
  #if defined(VK_DIRECTFB)
  strcat(wsis, "vk-directfb ");
  #endif
  #if defined(VK_FBDEV)
  strcat(wsis, "vk-fbdev ");
  #endif
  #if defined(VK_WAYLAND)
  strcat(wsis, "vk-wayland ");
  #endif
  #if defined(VK_XCB)
  strcat(wsis, "vk-xcb ");
  #endif
  #if defined(VK_D2D)
  strcat(wsis, "vk-d2d ");
  #endif

  while ((opt = getopt(argc, argv, "w:h")) != -1) {
    switch (opt) {
      case 'w':
        wsi_arg = optarg;
        break;
      case 'h':
      default:
        break;
    }
  }

  if (argc != 3 || !wsi_arg) {
    printf("\n\tUsage: %s -w WSI\n\n", argv[0]);
    printf("\t\tWSIs: %s\n\n", wsis);
    return EXIT_FAILURE;
  }

  wsi = wsis;
  while ((c = strchr(wsi, ' '))) {
    *c = '\0';
    if (!strcmp(wsi, wsi_arg))
      break;
    else
      wsi = c + 1;
  }

  if (!c) {
    printf("%s: WSI unknown\n", wsi_arg);
    return EXIT_FAILURE;
  }

  /* create instance and set physical device */

  #if defined(VK_X11)
  if (!strcmp(wsi, "vk-x11")) {
    vk_extension_name = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
  }
  #endif
  #if defined(VK_DIRECTFB)
  if (!strcmp(wsi, "vk-directfb")) {
    vk_extension_name = VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME;
  }
  #endif
  #if defined(VK_FBDEV)
  if (!strcmp(wsi, "vk-fbdev")) {
    vk_extension_name = VK_EXT_FBDEV_SURFACE_EXTENSION_NAME;
  }
  #endif
  #if defined(VK_WAYLAND)
  if (!strcmp(wsi, "vk-wayland")) {
    vk_extension_name = VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
  }
  #endif
  #if defined(VK_XCB)
  if (!strcmp(wsi, "vk-xcb")) {
    vk_extension_name = VK_KHR_XCB_SURFACE_EXTENSION_NAME;
  }
  #endif
  #if defined(VK_D2D)
  if (!strcmp(wsi, "vk-d2d")) {
    vk_extension_name = VK_KHR_DISPLAY_EXTENSION_NAME;
  }
  #endif

  memset(&vk_instance_create_info, 0, sizeof(VkInstanceCreateInfo));
  vk_instance_create_info.enabledExtensionCount = 1;
  vk_instance_create_info.ppEnabledExtensionNames = &vk_extension_name;
  err = vkCreateInstance(&vk_instance_create_info, NULL, &vk_instance);
  if (err) {
    printf("vkCreateInstance failed: %d\n", err);
    goto out;
  }

  err = vkEnumeratePhysicalDevices(vk_instance, &vk_physical_devices_count, NULL);
  if (err || !vk_physical_devices_count) {
    printf("vkEnumeratePhysicalDevices failed: %d, %d\n", err, vk_physical_devices_count);
    goto out;
  }

  vk_physical_devices = calloc(vk_physical_devices_count, sizeof(VkPhysicalDevice));
  if (!vk_physical_devices) {
    printf("calloc failed: %m\n");
    goto out;
  }

  err = vkEnumeratePhysicalDevices(vk_instance, &vk_physical_devices_count, vk_physical_devices);
  if (err) {
    printf("vkEnumeratePhysicalDevices failed: %d\n", err);
    goto out;
  }

  vk_physical_device = vk_physical_devices[0];

  /* open display */

  #if defined(VK_X11)
  if (!strcmp(wsi, "vk-x11")) {
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
  #if defined(VK_DIRECTFB)
  if (!strcmp(wsi, "vk-directfb")) {
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
  #if defined(VK_FBDEV)
  if (!strcmp(wsi, "vk-fbdev")) {
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
  #if defined(VK_WAYLAND)
  if (!strcmp(wsi, "vk-wayland")) {
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
  #if defined(VK_XCB)
  if (!strcmp(wsi, "vk-xcb")) {
    xcb_dpy = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(xcb_dpy)) {
      printf("xcb_connect failed\n");
      goto out;
    }

    win_width = xcb_setup_roots_iterator(xcb_get_setup(xcb_dpy)).data->width_in_pixels;
    win_height = xcb_setup_roots_iterator(xcb_get_setup(xcb_dpy)).data->height_in_pixels;
  }
  #endif
  #if defined(VK_D2D)
  if (!strcmp(wsi, "vk-d2d")) {
    err = vkGetPhysicalDeviceDisplayPropertiesKHR(vk_physical_device, &d2d_properties_count, NULL);
    if (err || !d2d_properties_count) {
      printf("vkGetPhysicalDeviceDisplayPropertiesKHR failed: %d, %d\n", err, vk_physical_devices_count);
      goto out;
    }

    d2d_properties = calloc(vk_physical_devices_count, sizeof(VkDisplayPropertiesKHR));
    if (!d2d_properties) {
      printf("calloc failed: %m\n");
      goto out;
    }

    err = vkGetPhysicalDeviceDisplayPropertiesKHR(vk_physical_device, &d2d_properties_count, d2d_properties);
    if (err) {
      printf("vkGetPhysicalDeviceDisplayPropertiesKHR failed: %d\n", err);
      goto out;
    }

    err = vkGetDisplayModePropertiesKHR(vk_physical_device, d2d_properties[0].display, &d2d_mode_properties_count, NULL);
    if (err || !d2d_mode_properties_count) {
      printf("vkGetPhysicalDeviceDisplayPropertiesKHR failed: %d, %d\n", err, vk_physical_devices_count);
      goto out;
    }

    d2d_mode_properties = calloc(d2d_mode_properties_count, sizeof(VkDisplayModePropertiesKHR));
    if (!d2d_mode_properties) {
      printf("calloc failed: %m\n");
      goto out;
    }

    err = vkGetDisplayModePropertiesKHR(vk_physical_device, d2d_properties[0].display, &d2d_mode_properties_count, d2d_mode_properties);
    if (err) {
      printf("vkGetDisplayModePropertiesKHR failed: %d\n", err);
      goto out;
    }

    d2d_dpy = d2d_mode_properties[0].displayMode;

    win_width = d2d_mode_properties[0].parameters.visibleRegion.width;
    win_height = d2d_mode_properties[0].parameters.visibleRegion.height;
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

  /* create window associated to the display */

  #if defined(VK_X11)
  if (!strcmp(wsi, "vk-x11")) {
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
  #if defined(VK_DIRECTFB)
  if (!strcmp(wsi, "vk-directfb")) {
    memset(&dfb_desc, 0, sizeof(DFBWindowDescription));
    dfb_desc.flags = DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_POSX | DWDESC_POSY;
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
  #if defined(VK_FBDEV)
  if (!strcmp(wsi, "vk-fbdev")) {
    fb_win = calloc(1, sizeof(struct fb_window));
    if (!fb_win) {
      printf("fb_window calloc failed: %m\n");
      goto out;
    }

    fb_win->width = win_width;
    fb_win->height = win_height;
    fb_win->posx = win_posx;
    fb_win->posy = win_posy;

    fb_keyboard = open(getenv("KEYBOARD") ? getenv("KEYBOARD") : "/dev/input/event0", O_RDONLY | O_NONBLOCK);
    if (fb_keyboard == -1) {
      printf("open %s failed: %m\n", getenv("KEYBOARD") ? getenv("KEYBOARD") : "/dev/input/event0");
      goto out;
    }
  }
  #endif
  #if defined(VK_WAYLAND)
  if (!strcmp(wsi, "vk-wayland")) {
    wl_win = wl_compositor_create_surface(wl_data.wl_compositor);
    if (!wl_win) {
      printf("wl_compositor_create_surface failed\n");
      goto out;
    }

    wl_shell_surface = wl_shell_get_shell_surface(wl_data.wl_shell, wl_win);
    if (!wl_shell_surface) {
      printf("wl_shell_get_shell_surface failed\n");
      goto out;
    }

    wl_shell_surface_set_toplevel(wl_shell_surface);

    #ifdef HAVE_WL_SHELL_SURFACE_SET_POSITION
    wl_shell_surface_set_position(wl_shell_surface, win_posx, win_posy);
    #endif

    wl_data.xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!wl_data.xkb_context) {
      printf("xkb_context_new failed\n");
      goto out;
    }
  }
  #endif
  #if defined(VK_XCB)
  if (!strcmp(wsi, "vk-xcb")) {
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
  #if defined(VK_D2D)
  if (!strcmp(wsi, "vk-d2d")) {
    d2d_win = d2d_mode_properties[0].parameters.visibleRegion;

    d2d_keyboard = open(getenv("KEYBOARD") ? getenv("KEYBOARD") : "/dev/input/event0", O_RDONLY | O_NONBLOCK);
    if (d2d_keyboard == -1) {
      printf("open %s failed: %m\n", getenv("KEYBOARD") ? getenv("KEYBOARD") : "/dev/input/event0");
      goto out;
    }

    err = libevdev_new_from_fd(d2d_keyboard, &d2d_evdev);
    if (err < 0) {
      printf("libevdev_new_from_fd failed: %m\n");
      goto out;
    }
  }
  #endif

  /* create surface */

  #if defined(VK_X11)
  if (!strcmp(wsi, "vk-x11")) {
    memset(&x11_surface_create_info, 0, sizeof(VkXlibSurfaceCreateInfoKHR));
    x11_surface_create_info.dpy = x11_dpy;
    x11_surface_create_info.window = x11_win;
    err = vkCreateXlibSurfaceKHR(vk_instance, &x11_surface_create_info, NULL, &vk_surface);
    if (err) {
      printf("vkCreateXlibSurfaceKHR failed: %d\n", err);
      goto out;
    }
  }
  #endif
  #if defined(VK_DIRECTFB)
  if (!strcmp(wsi, "vk-directfb")) {
    memset(&dfb_surface_create_info, 0, sizeof(VkDirectFBSurfaceCreateInfoEXT));
    dfb_surface_create_info.dfb = dfb_dpy;
    dfb_surface_create_info.surface = dfb_win;
    err = vkCreateDirectFBSurfaceEXT(vk_instance, &dfb_surface_create_info, NULL, &vk_surface);
    if (err) {
      printf("vkCreateDirectFBSurfaceEXT failed: %d\n", err);
      goto out;
    }
  }
  #endif
  #if defined(VK_FBDEV)
  if (!strcmp(wsi, "vk-fbdev")) {
    memset(&fb_surface_create_info, 0, sizeof(VkFBDevSurfaceCreateInfoEXT));
    fb_surface_create_info.fd = fb_dpy;
    fb_surface_create_info.window = fb_win;
    err = vkCreateFBDevSurfaceEXT(vk_instance, &fb_surface_create_info, NULL, &vk_surface);
    if (err) {
      printf("vkCreateFBDevSurfaceEXT failed: %d\n", err);
      goto out;
    }
  }
  #endif
  #if defined(VK_WAYLAND)
  if (!strcmp(wsi, "vk-wayland")) {
    memset(&wl_surface_create_info, 0, sizeof(VkWaylandSurfaceCreateInfoKHR));
    wl_surface_create_info.display = wl_dpy;
    wl_surface_create_info.surface = wl_win;
    err = vkCreateWaylandSurfaceKHR(vk_instance, &wl_surface_create_info, NULL, &vk_surface);
    if (err) {
      printf("vkCreateWaylandSurfaceKHR failed: %d\n", err);
      goto out;
    }
  }
  #endif
  #if defined(VK_XCB)
  if (!strcmp(wsi, "vk-xcb")) {
    memset(&xcb_surface_create_info, 0, sizeof(VkXcbSurfaceCreateInfoKHR));
    xcb_surface_create_info.connection = xcb_dpy;
    xcb_surface_create_info.window = xcb_win;
    err = vkCreateXcbSurfaceKHR(vk_instance, &xcb_surface_create_info, NULL, &vk_surface);
    if (err) {
      printf("vkCreateXcbSurfaceKHR failed: %d\n", err);
      goto out;
    }
  }
  #endif
  #if defined(VK_D2D)
  if (!strcmp(wsi, "vk-d2d")) {
    memset(&d2d_surface_create_info, 0, sizeof(VkDisplaySurfaceCreateInfoKHR));
    d2d_surface_create_info.displayMode = d2d_dpy;
    d2d_surface_create_info.imageExtent = d2d_win;
    err = vkCreateDisplayPlaneSurfaceKHR(vk_instance, &d2d_surface_create_info, NULL, &vk_surface);
    if (err) {
      printf("vkCreateXcbSurfaceKHR failed: %d\n", err);
      goto out;
    }
  }
  #endif

  /* create logical device */

  memset(&vk_device_create_info, 0, sizeof(VkDeviceCreateInfo));
  vk_device_create_info.queueCreateInfoCount = 1;
  memset(&vk_device_queue_create_info, 0, sizeof(VkDeviceQueueCreateInfo));
  vk_device_queue_create_info.queueCount = 1;
  vk_device_create_info.pQueueCreateInfos = &vk_device_queue_create_info;
  vk_device_create_info.enabledExtensionCount = 1;
  vk_extension_name = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
  vk_device_create_info.ppEnabledExtensionNames = &vk_extension_name;
  err = vkCreateDevice(vk_physical_device, &vk_device_create_info, NULL, &vk_device);
  if (err) {
    printf("vkCreateDevice failed: %d\n", err);
    goto out;
  }

  /* get queue */

  vkGetDeviceQueue(vk_device, 0, 0, &vk_queue);

  /* create swapchain */

  memset(&vk_swapchain_create_info, 0, sizeof(VkSwapchainCreateInfoKHR));
  vk_swapchain_create_info.surface = vk_surface;
  vk_swapchain_create_info.minImageCount = 1;
  vk_swapchain_create_info.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
  vk_swapchain_create_info.imageExtent.width = win_width;
  vk_swapchain_create_info.imageExtent.height = win_height;
  vk_swapchain_create_info.imageArrayLayers = 1;
  err = vkCreateSwapchainKHR(vk_device, &vk_swapchain_create_info, NULL, &vk_swapchain);
  if (err) {
    printf("vkCreateSwapchainKHR failed: %d\n", err);
    goto out;
  }

  /* drawing (main event loop) */

  gears = vk_gears_init(win_width, win_height, vk_device, vk_swapchain);
  if (!gears) {
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
      vk_gears_draw(gears, view_tz, view_rx, view_ry, model_rz, vk_queue);

      if (animate) {
        frames++;
      }

      memset(&vk_present_info, 0, sizeof(VkPresentInfoKHR));
      vk_present_info.swapchainCount = 1;
      vk_present_info.pSwapchains = &vk_swapchain;
      vk_present_info.pImageIndices = &vk_index;
      err = vkQueuePresentKHR(vk_queue, &vk_present_info);
      if (err) {
        printf("vkQueuePresentKHR failed: %d\n", err);
        goto out;
      }
    }

    #if defined(VK_X11)
    if (!strcmp(wsi, "vk-x11")) {
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
    #if defined(VK_DIRECTFB)
    if (!strcmp(wsi, "vk-directfb")) {
      if (!animate && redisplay) {
        redisplay = 0;
      }

      memset(&dfb_event, 0, sizeof(DFBWindowEvent));
      if (!dfb_event_buffer->GetEvent(dfb_event_buffer, (DFBEvent *)&dfb_event)) {
        if (dfb_event.type == DWET_KEYDOWN) {
          dfb_keyboard_handle_key(&dfb_event);
        }
      }
    }
    #endif
    #if defined(VK_FBDEV)
    if (!strcmp(wsi, "vk-fbdev")) {
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
    #if defined(VK_WAYLAND)
    if (!strcmp(wsi, "vk-wayland")) {
      if (!animate && redisplay) {
        redisplay = 0;
      }

      wl_display_dispatch(wl_dpy);
    }
    #endif
    #if defined(VK_XCB)
    if (!strcmp(wsi, "vk-xcb")) {
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
    #if defined(VK_D2D)
    if (!strcmp(wsi, "vk-d2d")) {
      if (!animate && redisplay) {
        redisplay = 0;
      }

      memset(&d2d_event, 0, sizeof(struct input_event));
      if (!libevdev_next_event(d2d_evdev, LIBEVDEV_READ_FLAG_NORMAL, &d2d_event) && d2d_event.type == EV_KEY) {
        if (d2d_event.value) {
          d2d_keyboard_handle_key(&d2d_event);
        }
      }
    }
    #endif
  }

  vk_gears_term(gears);

  ret = EXIT_SUCCESS;

out:

  /* destroy swapchain, device, surface and instance */

  if (vk_swapchain) {
    vkDestroySwapchainKHR(vk_device, vk_swapchain, NULL);
  }

  if (vk_device) {
    vkDestroyDevice(vk_device, NULL);
  }

  if (vk_surface) {
    vkDestroySurfaceKHR(vk_instance, vk_surface, NULL);
  }

  if (vk_physical_devices) {
    free(vk_physical_devices);
  }

  if (vk_instance) {
    vkDestroyInstance(vk_instance, NULL);
  }

  /* destroy window and close display */

  #if defined(VK_X11)
  if (!strcmp(wsi, "vk-x11")) {
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
  #if defined(VK_DIRECTFB)
  if (!strcmp(wsi, "vk-directfb")) {
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
  #if defined(VK_FBDEV)
  if (!strcmp(wsi, "vk-fbdev")) {
    if (fb_keyboard != -1) {
      close(fb_keyboard);
    }

    if (fb_win) {
      free(fb_win);
    }

    if (fb_dpy != -1) {
      close(fb_dpy);
    }
  }
  #endif
  #if defined(VK_WAYLAND)
  if (!strcmp(wsi, "vk-wayland")) {
    if (wl_data.xkb_state) {
      xkb_state_unref(wl_data.xkb_state);
    }

    if (wl_data.xkb_keymap) {
      xkb_map_unref(wl_data.xkb_keymap);
    }

    if (wl_data.xkb_context) {
      xkb_context_unref(wl_data.xkb_context);
    }

    if (wl_shell_surface) {
      wl_shell_surface_destroy(wl_shell_surface);
    }

    if (wl_win) {
      wl_surface_destroy(wl_win);
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
  #if defined(VK_XCB)
  if (!strcmp(wsi, "vk-xcb")) {
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
  #if defined(VK_D2D)
  if (!strcmp(wsi, "vk-d2d")) {
    if (d2d_evdev) {
      libevdev_free(d2d_evdev);
    }

    if (d2d_keyboard != -1) {
      close(d2d_keyboard);
    }

    if (!d2d_win.width || !d2d_win.height) {
      memset(&d2d_win, 0, sizeof(VkExtent2D));
    }

    if (d2d_mode_properties) {
      free(d2d_mode_properties);
    }

    if (d2d_properties) {
      free(d2d_properties);
    }

    if (d2d_dpy) {
      d2d_dpy = VK_NULL_HANDLE;
    }
  }
  #endif

  return ret;
}
