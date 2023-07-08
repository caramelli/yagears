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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include <vulkan/vulkan.h>

#if defined(GLFW)
#include <GLFW/glfw3.h>
#endif
#if defined(SDL)
#include <SDL.h>
#include <SDL_vulkan.h>
#endif

#include "vulkan_gears.h"

/******************************************************************************/

static char *toolkit = NULL;
static VkSwapchainKHR vk_swapchain = VK_NULL_HANDLE;
static VkQueue vk_queue = VK_NULL_HANDLE;
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

#if defined(GLFW)
static void glfwDisplay(GLFWwindow *window)
{
  if (animate) { if (frames) rotate(); else t_rate = t_rot = current_time(); }
  vk_gears_draw(gears, view_tz, view_rx, view_ry, model_rz, vk_queue);
  if (animate) frames++;

  uint32_t vk_index = 0;
  VkPresentInfoKHR vk_present_info;
  memset(&vk_present_info, 0, sizeof(VkPresentInfoKHR));
  vk_present_info.swapchainCount = 1;
  vk_present_info.pSwapchains = &vk_swapchain;
  vk_present_info.pImageIndices = &vk_index;
  vkQueuePresentKHR(vk_queue, &vk_present_info);
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
    case GLFW_KEY_T:
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
    glfwDisplay(window);
  }

}
#endif

#if defined(SDL)
static void SDL_Display(SDL_Window *window)
{
  if (animate) { if (frames) rotate(); else t_rate = t_rot = current_time(); }
  vk_gears_draw(gears, view_tz, view_rx, view_ry, model_rz, vk_queue);
  if (animate) frames++;

  uint32_t vk_index = 0;
  VkPresentInfoKHR vk_present_info;
  memset(&vk_present_info, 0, sizeof(VkPresentInfoKHR));
  vk_present_info.swapchainCount = 1;
  vk_present_info.pSwapchains = &vk_swapchain;
  vk_present_info.pImageIndices = &vk_index;
  vkQueuePresentKHR(vk_queue, &vk_present_info);
}

static void SDL_Idle(SDL_Window *window)
{
  if (animate) {
    if (!frames) return;
    SDL_Display(window);
  }
  else {
    if (frames) frames = 0;
  }
}

static void SDL_KeyDownEvent(SDL_Window *window, SDL_Event *event, void *data)
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
        SDL_Display(window);
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
    case SDLK_t:
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
    SDL_Display(window);
  }
}
#endif

/******************************************************************************/

int main(int argc, char *argv[])
{
  int err = 0, ret = EXIT_FAILURE;
  char toolkits[64], *toolkit_arg = NULL, *c;
  int opt;
  #if defined(GLFW)
  GLFWwindow *glfw_win;
  #endif
  #if defined(SDL)
  SDL_Window *sdl_win;
  int sdl_idle_id;
  #endif
  uint32_t vk_extension_count;
  const char **vk_extension_names = NULL;
  VkInstanceCreateInfo vk_instance_create_info;
  VkInstance vk_instance = VK_NULL_HANDLE;
  uint32_t vk_physical_devices_count;
  VkPhysicalDevice *vk_physical_devices = NULL, vk_physical_device = VK_NULL_HANDLE;
  VkSurfaceKHR vk_surface = VK_NULL_HANDLE;
  VkDeviceCreateInfo vk_device_create_info;
  VkDeviceQueueCreateInfo vk_device_queue_create_info;
  VkDevice vk_device = VK_NULL_HANDLE;
  VkSwapchainCreateInfoKHR vk_swapchain_create_info;

  /* process command line */

  memset(toolkits, 0, sizeof(toolkits));
  #if defined(GLFW)
  strcat(toolkits, "glfw ");
  #endif
  #if defined(SDL)
  strcat(toolkits, "sdl ");
  #endif

  while ((opt = getopt(argc, argv, "t:h")) != -1) {
    switch (opt) {
      case 't':
        toolkit_arg = optarg;
        break;
      case 'h':
      default:
        break;
    }
  }

  if (argc != 3 || !toolkit_arg) {
    printf("\n\tUsage: %s -t Toolkit\n\n", argv[0]);
    printf("\t\tToolkits: %s\n\n", toolkits);
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

  /* Toolkit init */

  #if defined(GLFW)
  if (!strcmp(toolkit, "glfw")) {
    glfwInit();

    const GLFWvidmode *glfw_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    win_width = glfw_mode->width;
    win_height = glfw_mode->height;
  }
  #endif

  #if defined(SDL)
  if (!strcmp(toolkit, "sdl")) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_DisplayMode sdl_mode;
    SDL_GetDisplayMode(0, 0, &sdl_mode);
    win_width = sdl_mode.w;
    win_height = sdl_mode.h;
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

  #if defined(GLFW)
  if (!strcmp(toolkit, "glfw")) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfw_win = glfwCreateWindow(win_width, win_height, "yagears", NULL, NULL);
    glfwSetWindowPos(glfw_win, win_posx, win_posy);
    glfwSetKeyCallback(glfw_win, glfwKeyCallback);
  }
  #endif

  #if defined(SDL)
  if (!strcmp(toolkit, "sdl")) {
    sdl_win = SDL_CreateWindow("yagears", win_posx, win_posy, win_width, win_height, SDL_WINDOW_VULKAN);
    sdl_idle_id = 1;
  }
  #endif

  /* create instance and set physical device */

  #if defined(GLFW)
  if (!strcmp(toolkit, "glfw")) {
    vk_extension_names = glfwGetRequiredInstanceExtensions(&vk_extension_count);
  }
  #endif

  #if defined(SDL)
  if (!strcmp(toolkit, "sdl")) {
    SDL_Vulkan_GetInstanceExtensions(sdl_win, &vk_extension_count, NULL);
    vk_extension_names = (const char **)calloc(vk_extension_count, sizeof(const char *));
    SDL_Vulkan_GetInstanceExtensions(sdl_win, &vk_extension_count, vk_extension_names);
  }
  #endif

  memset(&vk_instance_create_info, 0, sizeof(VkInstanceCreateInfo));
  vk_instance_create_info.enabledExtensionCount = vk_extension_count;
  vk_instance_create_info.ppEnabledExtensionNames = vk_extension_names;
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

  vk_physical_devices = (VkPhysicalDevice *)calloc(vk_physical_devices_count, sizeof(VkPhysicalDevice));

  err = vkEnumeratePhysicalDevices(vk_instance, &vk_physical_devices_count, vk_physical_devices);
  if (err) {
    printf("vkEnumeratePhysicalDevices failed: %d\n", err);
    goto out;
  }

  vk_physical_device = vk_physical_devices[0];

  /* create surface */

  #if defined(GLFW)
  if (!strcmp(toolkit, "glfw")) {
    glfwCreateWindowSurface(vk_instance, glfw_win, NULL, &vk_surface);
  }
  #endif

  #if defined(SDL)
  if (!strcmp(toolkit, "sdl")) {
    SDL_Vulkan_CreateSurface(sdl_win, vk_instance, &vk_surface);
  }
  #endif

  /* create logical device */

  memset(&vk_device_create_info, 0, sizeof(VkDeviceCreateInfo));
  vk_device_create_info.queueCreateInfoCount = 1;
  memset(&vk_device_queue_create_info, 0, sizeof(VkDeviceQueueCreateInfo));
  vk_device_queue_create_info.queueCount = 1;
  vk_device_create_info.pQueueCreateInfos = &vk_device_queue_create_info;
  vk_device_create_info.enabledExtensionCount = 1;
  vk_extension_names[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
  vk_device_create_info.ppEnabledExtensionNames = vk_extension_names;
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

  #if defined(SDL)
  if (!strcmp(toolkit, "sdl")) {
    SDL_Display(sdl_win);
    while (1) {
      if (sdl_idle_id)
        SDL_Idle(sdl_win);
      SDL_Event event;
      while (SDL_PollEvent(&event))
        SDL_KeyDownEvent(sdl_win, &event, &sdl_idle_id);
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

  /* Toolkit term */

  #if defined(GLFW)
  if (!strcmp(toolkit, "glfw")) {
    glfwDestroyWindow(glfw_win);
    glfwTerminate();
  }
  #endif

  #if defined(SDL)
  if (!strcmp(toolkit, "sdl")) {
    free(vk_extension_names);
    SDL_DestroyWindow(sdl_win);
    SDL_Quit();
  }
  #endif

  return ret;
}
