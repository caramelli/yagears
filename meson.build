# yagears                  Yet Another Gears OpenGL / Vulkan demo
# Copyright (C) 2013-2023  Nicolas Caramelli
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

project('yagears', 'c', 'cpp', version: '2.7', meson_version: '>= 0.57')

config_h = configuration_data()

cc = meson.get_compiler('c')

fs = import('fs')

add_global_arguments('-D_GNU_SOURCE', language: 'c')
add_global_link_arguments('-lm', language: 'c')

# Configuration options

if get_option('engine-ctor')
  add_global_arguments('-DENGINE_CTOR', language: 'c')
endif

enable_gl = get_option('gl')
enable_glesv1_cm = get_option('glesv1_cm')
enable_glesv2 = get_option('glesv2')
with_pgl = get_option('pgl')

enable_gl_x11 = get_option('gl-x11')
enable_gl_directfb = get_option('gl-directfb')
enable_gl_fbdev = get_option('gl-fbdev')
enable_egl_x11 = get_option('egl-x11')
enable_egl_directfb = get_option('egl-directfb')
enable_egl_fbdev = get_option('egl-fbdev')
enable_egl_wayland = get_option('egl-wayland')
enable_egl_xcb = get_option('egl-xcb')
enable_egl_drm = get_option('egl-drm')
enable_egl_rpi = get_option('egl-rpi')
enable_waffle = get_option('waffle')

enable_vk_x11 = get_option('vk-x11')
enable_vk_directfb = get_option('vk-directfb')
enable_vk_fbdev = get_option('vk-fbdev')
enable_vk_wayland = get_option('vk-wayland')
enable_vk_xcb = get_option('vk-xcb')
enable_vk_d2d = get_option('vk-d2d')

enable_efl = get_option('efl')
enable_fltk = get_option('fltk')
enable_glfw = get_option('glfw')
enable_glut = get_option('glut')
enable_gtk = get_option('gtk')
enable_qt = get_option('qt')
enable_sdl = get_option('sdl')
enable_sfml = get_option('sfml')
enable_wx = get_option('wx')

with_gtk = get_option('with-gtk')
with_qt = get_option('with-qt')
with_sdl = get_option('with-sdl')

enable_png = get_option('png')
enable_tiff = get_option('tiff')

# OpenGL Engines

pgl_dep = []
if with_pgl != 'false'
  if enable_gl or enable_glesv1_cm or enable_glesv2
    error('OpenGL 1.0, OpenGL ES 1.1 CM and OpenGL ES 2.0 engines incompatible with PortableGL')
  endif

  if fs.is_file(with_pgl)
    pgl_dep = declare_dependency(compile_args: ['-DPGL_H="@0@"'.format(with_pgl), get_option('pgl-cflags').split(' ')])
  else
    error('PortableGL platform header not found')
  endif
endif

gl_dep = []
if enable_gl
  gl_dep = dependency('gl', required: false)
  if gl_dep.found()
    if enable_glesv1_cm
      gl_dep = declare_dependency(compile_args: ['-DGLESV1_CM_H=<GL/gl.h>', '-DGLESV1_CM_LIB="libGL.so"', '-DGL_GLEXT_PROTOTYPES'], dependencies: gl_dep)
    endif
    if enable_glesv2
      xxd = find_program('xxd', required: false)
      if xxd.found()
        gl_dep = declare_dependency(compile_args: ['-DGLESV2_H=<GL/gl.h>', '-DGLESV2_LIB="libGL.so"', '-DGL_GLEXT_PROTOTYPES'], dependencies: gl_dep)
      else
        enable_glesv2 = false
      endif
    endif
    if enable_glesv1_cm or enable_glesv2
      gl_dep = declare_dependency(link_args: '-ldl', dependencies: gl_dep)
    endif
  else
    enable_gl = false
  endif
endif

glesv1_cm_dep = []
glesv2_dep = []
if not enable_gl
  if enable_glesv1_cm
    glesv1_cm_dep = dependency('glesv1_cm', required: false)
    if glesv1_cm_dep.found()
      glesv1_cm_dep = declare_dependency(compile_args: run_command('pkg-config', '--cflags', 'glesv1_cm').stdout().strip().split(' '))
      glesv1_cm_dep = declare_dependency(compile_args: ['-DGLESV1_CM_H=<GLES/gl.h>', '-DGLESV1_CM_LIB="libGLESv1_CM.so.1"'], link_args: '-ldl', dependencies: glesv1_cm_dep)
    else
      enable_glesv1_cm = false
    endif
  endif

  if enable_glesv2
    glesv2_dep = dependency('glesv2', required: false)
    xxd = find_program('xxd', required: false)
    if glesv2_dep.found() and xxd.found()
      glesv2_dep = declare_dependency(compile_args: run_command('pkg-config', '--cflags', 'glesv2').stdout().strip().split(' '))
      glesv2_dep = declare_dependency(compile_args: ['-DGLESV2_H=<GLES2/gl2.h>', '-DGLESV2_LIB="libGLESv2.so.2"'], link_args: '-ldl', dependencies: glesv2_dep)
    else
      enable_glesv2 = false
    endif
  endif
endif

if not enable_gl and not enable_glesv1_cm and not enable_glesv2 and with_pgl == 'false'
  error('No OpenGL Engines found')
endif

# OpenGL Backends

x11_dep = []
directfb_dep = []
if enable_gl or with_pgl != 'false'
  if enable_gl_x11
    libdir = run_command('pkg-config', '--variable=libdir', 'gl').stdout().strip()
    if cc.has_function('glXCreateContext', dependencies: cc.find_library('GL', dirs: libdir))
      x11_dep = dependency('x11', required: false)
      if not x11_dep.found()
        enable_gl_x11 = false
      endif
    else
      enable_gl_x11 = false
    endif
  endif

  if enable_gl_directfb
    message('Checking for DirectFBGL module')
    moduledir = run_command('pkg-config', '--variable=moduledir', 'directfb-internal').stdout().strip()
    if run_command('test', '-d', moduledir + '/interfaces/IDirectFBGL').returncode() == 0
      message('DirectFBGL found')
      directfb_dep = dependency('directfb', required: false)
      if not directfb_dep.found()
        enable_gl_directfb = false
      endif
    else
      message('DirectFBGL not found')
      enable_gl_directfb = false
    endif
  endif

  if enable_gl_fbdev
    libdir = run_command('pkg-config', '--variable=libdir', 'gl').stdout().strip()
    if not cc.has_function('glFBDevCreateContext', dependencies: cc.find_library('GL', dirs: libdir))
      enable_gl_fbdev = false
    endif
  endif
else
  enable_gl_x11 = false
  enable_gl_directfb = false
  enable_gl_fbdev = false
endif
config_h.set('GL_X11', enable_gl_x11, description: 'Support for GLX')
config_h.set('GL_DIRECTFB', enable_gl_directfb , description: 'Support for DirectFBGL')
config_h.set('GL_FBDEV', enable_gl_fbdev, description: 'Support for GLFBDev')

egl_dep = []
wayland_dep = []
wayland_egl_dep = []
xcb_dep = []
drm_dep = []
rpi_dep = []
if enable_egl_x11 or enable_egl_directfb or enable_egl_fbdev or enable_egl_wayland or enable_egl_xcb or enable_egl_drm or enable_egl_rpi
  egl_dep = dependency('egl', required: false)
  if egl_dep.found()
    egl_dep = declare_dependency(compile_args: ['-DEGL_NO_PLATFORM_SPECIFIC_TYPES', '-DEGL_EGLEXT_PROTOTYPES'], dependencies: egl_dep)

    if enable_egl_x11
      if not enable_gl_x11
        x11_dep = dependency('x11', required: false)
        if not x11_dep.found()
          enable_egl_x11 = false
        endif
      endif
    endif

    if enable_egl_directfb
      if not enable_gl_directfb
        directfb_dep = dependency('directfb', required: false)
        if not directfb_dep.found()
          enable_egl_directfb = false
        endif
      endif
    endif

    if enable_egl_wayland
      wayland_dep = [dependency('wayland-client', required: false), dependency('xkbcommon', required: false)]
      foreach dep : wayland_dep
        if not dep.found()
          enable_egl_wayland = false
        endif
      endforeach
      if enable_egl_wayland
        message('Checking for function "wl_shell_surface_set_position"')
        code = '''
                  #include <wayland-client.h>
                  int main()
                  {
                    wl_shell_surface_set_position(NULL, 0, 0);
                    return 0;
                  }
               '''
        if cc.links(code, dependencies: wayland_dep)
          message('"wl_shell_surface_set_position" found')
          config_h.set('HAVE_WL_SHELL_SURFACE_SET_POSITION', true, description: 'Have function wl_shell_surface_set_position')
        else
          message('"wl_shell_surface_set_position" not found')
          config_h.set('HAVE_WL_SHELL_SURFACE_SET_POSITION', false, description: 'Have function wl_shell_surface_set_position')
        endif

        wayland_egl_dep = [wayland_dep, cc.find_library('wayland-egl', dirs: run_command('pkg-config', '--variable=libdir', 'wayland-client').stdout().strip())]
      endif
    endif

    if enable_egl_xcb
      xcb_dep = dependency('xcb', required: false)
      if not xcb_dep.found()
        enable_egl_xcb = false
      endif
    endif

    if enable_egl_drm
      drm_dep = [dependency('gbm', required: false), dependency('libdrm', required: false), dependency('libevdev', required: false)]
      foreach dep : drm_dep
        if not dep.found()
          enable_egl_drm = false
        endif
      endforeach
      if enable_egl_drm
        dri_dep = dependency('dri', required: false)
        if dri_dep.found()
          dridriverdir = run_command('pkg-config', '--variable=dridriverdir', 'dri').stdout().strip()
          modversion = run_command('pkg-config', '--modversion', 'dri')
          drimajorversion = modversion.stdout().split('.')[0]
          driminorversion = modversion.stdout().split('.')[1]
          drm_dep = declare_dependency(compile_args: ['-DDRI_DRIVERDIR="@0@"'.format(dridriverdir), '-DDRI_MAJOR_VERSION=@0@'.format(drimajorversion), '-DDRI_MINOR_VERSION=@0@'.format(driminorversion)], link_args: '-ldl', dependencies: drm_dep)
          config_h.set('HAVE_DRI', true, description: 'Have Direct Rendering Infrastructure')
        endif
      endif
    endif

    if enable_egl_rpi
      rpi_dep = dependency('bcm_host', required: false)
      if rpi_dep.found()
        rpi_dep = [rpi_dep, cc.find_library('vchostif', dirs: run_command('pkg-config', '--variable=libdir', 'bcm_host').stdout().strip())]
      else
        enable_egl_rpi = false
      endif
    endif
  else
    enable_egl_x11 = false
    enable_egl_directfb = false
    enable_egl_fbdev = false
    enable_egl_wayland = false
    enable_egl_xcb = false
    enable_egl_drm = false
    enable_egl_rpi = false
  endif
endif
config_h.set('EGL_X11', enable_egl_x11, description: 'Support for EGL with Xlib platform')
config_h.set('EGL_DIRECTFB', enable_egl_directfb, description: 'Support for EGL with DirectFB platform')
config_h.set('EGL_FBDEV', enable_egl_fbdev, description: 'Support for EGL with Linux FBDev platform')
config_h.set('EGL_WAYLAND', enable_egl_wayland, description: 'Support for EGL with Wayland platform')
config_h.set('EGL_XCB', enable_egl_xcb, description: 'Support for EGL with XCB platform')
config_h.set('EGL_DRM', enable_egl_drm, description: 'Support for EGL with DRM platform')
config_h.set('EGL_RPI', enable_egl_rpi, description: 'Support for EGL with Raspberry Pi Dispmanx platform')

waffle_dep = []
if enable_waffle
  waffle_dep = [dependency('libinput', required: false), dependency('waffle-1', required: false)]
  foreach dep : waffle_dep
    if not dep.found()
      enable_waffle = false
    endif
  endforeach
endif
config_h.set('WAFFLE', enable_waffle, description: 'Support for Waffle cross-platform wrapper')

if not enable_gl_x11 and not enable_gl_directfb and not enable_gl_fbdev and not enable_egl_x11 and not enable_egl_directfb and not enable_egl_fbdev and not enable_egl_wayland and not enable_egl_xcb and not enable_egl_drm and not enable_egl_rpi and not enable_waffle
  error('No OpenGL Backends found')
endif

# Vulkan WSIs

vulkan_dep = []
d2d_dep = []
if enable_vk_x11 or enable_vk_directfb or enable_vk_fbdev or enable_vk_wayland or enable_vk_xcb or enable_vk_d2d
  vulkan_dep = dependency('vulkan', required: false)
  glslang_validator = find_program('glslangValidator', required: false)
  if vulkan_dep.found() and glslang_validator.found()
    libdir = run_command('pkg-config', '--variable=libdir', 'vulkan').stdout().strip()

    if enable_vk_x11
      if cc.has_function('vkCreateXlibSurfaceKHR', dependencies: cc.find_library('vulkan', dirs: libdir))
        if not enable_gl_x11 and not enable_egl_x11
          x11_dep = dependency('x11', required: false)
          if not x11_dep.found()
            enable_vk_x11 = false
          endif
        endif
      else
        enable_vk_x11 = false
      endif
    endif

    if enable_vk_directfb
      if cc.has_function('vkCreateDirectFBSurfaceEXT', dependencies: cc.find_library('vulkan', dirs: libdir))
        if not enable_gl_directfb and not enable_egl_directfb
          directfb_dep = dependency('directfb', required: false)
          if not directfb_dep.found()
            enable_vk_directfb = false
          endif
        endif
      else
        enable_vk_directfb = false
      endif
    endif

    if enable_vk_fbdev
      if not cc.has_function('vkCreateFBDevSurfaceEXT', dependencies: cc.find_library('vulkan', dirs: libdir))
        enable_vk_fbdev = false
      endif
    endif

    if enable_vk_wayland
      if cc.has_function('vkCreateWaylandSurfaceKHR', dependencies: cc.find_library('vulkan', dirs: libdir))
        if not enable_egl_wayland
          wayland_dep = [dependency('wayland-client', required: false), dependency('xkbcommon', required: false)]
          foreach dep : wayland_dep
            if not dep.found()
              enable_vk_wayland = false
            endif
          endforeach
          if enable_vk_wayland
            message('Checking for function "wl_shell_surface_set_position"')
            code = '''
                      #include <wayland-client.h>
                      int main()
                      {
                        wl_shell_surface_set_position(NULL, 0, 0);
                        return 0;
                      }
                   '''
            if cc.links(code, dependencies: wayland_dep)
              message('"wl_shell_surface_set_position" found')
              config_h.set('HAVE_WL_SHELL_SURFACE_SET_POSITION', true, description: 'Have function wl_shell_surface_set_position')
            else
              message('"wl_shell_surface_set_position" not found')
              config_h.set('HAVE_WL_SHELL_SURFACE_SET_POSITION', false, description: 'Have function wl_shell_surface_set_position')
            endif
          endif
        endif
      else
        enable_vk_wayland = false
      endif
    endif

    if enable_vk_xcb
      if cc.has_function('vkCreateXcbSurfaceKHR', dependencies: cc.find_library('vulkan', dirs: libdir))
        if not enable_egl_xcb
          xcb_dep = dependency('xcb', required: false)
          if not xcb_dep.found()
            enable_vk_xcb = false
          endif
        endif
      else
        enable_vk_xcb = false
      endif
    endif

    if enable_vk_d2d
      if cc.has_function('vkCreateDisplayPlaneSurfaceKHR', dependencies: cc.find_library('vulkan', dirs: libdir))
        d2d_dep = dependency('libevdev', required: false)
        if not d2d_dep.found()
          enable_vk_d2d = false
        endif
      else
        enable_vk_d2d = false
      endif
    endif
  else
    enable_vk_x11 = false
    enable_vk_directfb = false
    enable_vk_fbdev = false
    enable_vk_wayland = false
    enable_vk_xcb = false
    enable_vk_d2d = false
  endif
endif
config_h.set('VK_X11', enable_vk_x11, description: 'Support for Xlib WSI')
config_h.set('VK_DIRECTFB', enable_vk_directfb , description: 'Support for DirectFB WSI')
config_h.set('VK_FBDEV', enable_vk_fbdev, description: 'Support for FBDev WSI')
config_h.set('VK_WAYLAND', enable_vk_wayland, description: 'Support for Wayland WSI')
config_h.set('VK_XCB', enable_vk_xcb, description: 'Support for XCB WSI')
config_h.set('VK_D2D', enable_vk_d2d, description: 'Support for D2D WSI')

if not enable_vk_x11 and not enable_vk_directfb and not enable_vk_fbdev and not enable_vk_wayland and not enable_vk_xcb and not enable_vk_d2d
  warning('No Vulkan WSIs found')
endif

# Graphical User Interface Toolkits

efl_dep = []
if enable_efl
  efl_dep = dependency('elementary', required: false)
  if not efl_dep.found()
    enable_efl = false
  endif
endif
config_h.set('EFL', enable_efl, description: 'Support for EFL graphical user interface')

fltk_dep = []
if enable_fltk
  fltk_config = find_program('fltk-config', required: false)
  if fltk_config.found()
    fltk_dep = declare_dependency(compile_args: run_command(fltk_config, '--cflags').stdout().strip().split(' '), link_args: run_command(fltk_config, '--use-gl', '--ldflags').stdout().strip().split(' '))
  else
    enable_fltk = false
  endif
endif
config_h.set('FLTK', enable_fltk, description: 'Support for FLTK graphical user interface')

glfw_dep = []
if enable_glfw
  glfw_dep = dependency('glfw3', required: false)
  if not glfw_dep.found()
    enable_glfw = false
  endif
endif
config_h.set('GLFW', enable_glfw, description: 'Support for GLFW graphical user interface')

glut_dep = []
if enable_glut
  glut_dep = dependency('glut', required: false)
  if not glut_dep.found()
    glut_dep = cc.find_library('glut', required: false)
    if not cc.has_function('glutExit', dependencies: glut_dep)
      enable_glut = false
    endif
  endif
endif
config_h.set('GLUT', enable_glut, description: 'Support for GLUT graphical user interface')

gtk_dep = []
if enable_gtk
  message('Checking for GTK+ version: ' + with_gtk)
  if with_gtk == '3'
    gtk_dep = dependency('gtk+-3.0', version: '>= 3.16.0', required: false)
    if not gtk_dep.found()
      enable_gtk = false
    endif
  else
    enable_gtk = false
  endif
  if not enable_gtk
    if with_gtk == '3'
      gtk_dep = dependency('gtkgl-3.0', required: false)
      if gtk_dep.found()
        enable_gtk = true
      else
        enable_gtk = false
      endif
    endif
    if not enable_gtk
      gtk_dep = dependency('gtkgl-2.0', required: false)
      if gtk_dep.found()
        enable_gtk = true
      else
        enable_gtk = false
      endif
    endif
  endif
endif
config_h.set('GTK', enable_gtk, description: 'Support for GTK+ graphical user interface')

qt_dep = []
if enable_qt
  message('Checking for Qt version: ' + with_qt)
  if with_qt == '5'
    qt_dep = dependency('Qt5OpenGL', required: false)
    if not qt_dep.found()
      enable_qt = false
    endif
  else
    enable_qt = false
  endif
  if not enable_qt
    qt_dep = dependency('QtOpenGL', required: false)
    if qt_dep.found()
      enable_qt = true
    else
      enable_qt = false
    endif
  endif
endif
config_h.set('QT', enable_qt, description: 'Support for Qt graphical user interface')

sdl_dep = []
if enable_sdl
  message('Checking for SDL version: ' + with_sdl)
  if with_sdl == '2'
    sdl_dep = dependency('sdl2', required: false)
    if not sdl_dep.found()
      enable_sdl = false
    endif
  else
    enable_sdl = false
  endif
  if not enable_sdl
    sdl_dep = dependency('sdl', required: false)
    if sdl_dep.found()
      enable_sdl = true
    else
      enable_sdl = false
    endif
  endif
endif
config_h.set('SDL', enable_sdl, description: 'Support for SDL graphical user interface')

sfml_dep = []
if enable_sfml
  sfml_dep = dependency('sfml-graphics', required: false)
  if not sfml_dep.found()
    enable_sfml = false
  endif
endif
config_h.set('SFML', enable_sfml, description: 'Support for SFML graphical user interface')

wx_dep = []
if enable_wx
  wx_config = find_program('wx-config', required: false)
  if wx_config.found()
    wx_dep = declare_dependency(compile_args: run_command(wx_config, '--cflags').stdout().strip().split(' '), link_args: run_command(wx_config, '--libs', 'core,gl').stdout().strip().split(' '))
  else
    enable_wx = false
  endif
endif
config_h.set('WX', enable_wx, description: 'Support for wxWidgets graphical user interface')

if not enable_efl and not enable_fltk and not enable_glfw and not enable_glut and not enable_gtk and not enable_qt and not enable_sdl and not enable_sfml and not enable_wx
  warning('No Graphical User Interface Toolkits found')
endif

# Image libraries

png_dep = []
if enable_png
  png_dep = dependency('libpng', required: false)
  if not png_dep.found()
    enable_png = false
  endif
endif

tiff_dep = []
if enable_tiff
  tiff_dep = dependency('libtiff', required: false)
  if not tiff_dep.found()
    enable_tiff = false
  endif
endif

# Build configuration

configure_file(configuration: config_h, output: 'config.h')

message('')
message('OpenGL Engines:')
message('')
message('  OpenGL 1.0                        @0@'.format(enable_gl))
message('  OpenGL ES 1.1 CM                  @0@'.format(enable_glesv1_cm))
message('  OpenGL ES 2.0                     @0@'.format(enable_glesv2))
message('  PortableGL                        @0@'.format(with_pgl))
message('')

message('')
message('OpenGL Backends:')
message('')
message('  OpenGL extension for Xlib         @0@'.format(enable_gl_x11))
message('  OpenGL extension for DirectFB     @0@'.format(enable_gl_directfb))
message('  OpenGL extension for Linux FBDev  @0@'.format(enable_gl_fbdev))
message('     EGL interface for Xlib         @0@'.format(enable_egl_x11))
message('     EGL interface for DirectFB     @0@'.format(enable_egl_directfb))
message('     EGL interface for Linux FBDev  @0@'.format(enable_egl_fbdev))
message('     EGL interface for Wayland      @0@'.format(enable_egl_wayland))
message('     EGL interface for XCB          @0@'.format(enable_egl_xcb))
message('     EGL interface for DRM          @0@'.format(enable_egl_drm))
message('     EGL interface for RPi Dispmanx @0@'.format(enable_egl_rpi))
message('  Waffle cross-platform wrapper     @0@'.format(enable_waffle))
message('')

message('')
message('Vulkan WSIs:')
message('')
message('  Vulkan extension for Xlib         @0@'.format(enable_vk_x11))
message('  Vulkan extension for DirectFB     @0@'.format(enable_vk_directfb))
message('  Vulkan extension for Linux FBDev  @0@'.format(enable_vk_fbdev))
message('  Vulkan extension for Wayland      @0@'.format(enable_vk_wayland))
message('  Vulkan extension for XCB          @0@'.format(enable_vk_xcb))
message('  Vulkan extension for D2D          @0@'.format(enable_vk_d2d))
message('')

message('')
message('Graphical User Interface Toolkits:')
message('')
message('  EFL                               @0@'.format(enable_efl))
message('  FLTK                              @0@'.format(enable_fltk))
message('  GLFW                              @0@'.format(enable_glfw))
message('  GLUT                              @0@'.format(enable_glut))
message('  GTK                               @0@'.format(enable_gtk))
message('  QT                                @0@'.format(enable_qt))
message('  SDL                               @0@'.format(enable_sdl))
message('  SFML                              @0@'.format(enable_sfml))
message('  WX                                @0@'.format(enable_wx))
message('')

# Build rules

GL = enable_gl
GLESV1_CM = enable_glesv1_cm
GLESV2 = enable_glesv2
PGL = with_pgl != 'false'

if enable_vk_x11 or enable_vk_directfb or enable_vk_fbdev or enable_vk_wayland or enable_vk_xcb or enable_vk_d2d
  VK = true
else
  VK = false
endif

MOSAIC = enable_glut

if enable_efl or enable_fltk or enable_glfw or enable_glut or enable_gtk or enable_qt or enable_sdl or enable_sfml or enable_wx
  GUI = true
else
  GUI = false
endif

if (enable_vk_x11 or enable_vk_directfb or enable_vk_fbdev or enable_vk_wayland or enable_vk_xcb or enable_vk_d2d) and (enable_glfw or enable_sdl)
  VK_GUI = true
else
  VK_GUI = false
endif

PNG = enable_png
TIFF = enable_tiff

gl_source = []
if GL
gl_source = 'gl_gears.c'
endif

glesv1_cm_source = []
if GLESV1_CM
glesv1_cm_source = 'glesv1_cm_gears.c'
endif

glesv2_source = []
vert_xxd_file = []
frag_xxd_file = []
if GLESV2
vert_xxd_file = custom_target('vert_xxd', command: [files('xxd.sh'), '@INPUT@', '@OUTPUT@'], input: 'glesv2_gears.vert', output: 'vert.xxd')
frag_xxd_file = custom_target('frag_xxd', command: [files('xxd.sh'), '@INPUT@', '@OUTPUT@'], input: 'glesv2_gears.frag', output: 'frag.xxd')

glesv2_source = 'glesv2_gears.c'
endif

pgl_source = []
if PGL
pgl_source = 'pgl_gears.c'
endif

png_source = []
if PNG
png_source = 'png_image.c'
endif

tiff_source = []
if TIFF
tiff_source = 'tiff_image.c'
endif

libyagears = static_library('yagears',
                            'gears_engine.c', gl_source, glesv1_cm_source, glesv2_source, vert_xxd_file, frag_xxd_file, pgl_source, 'image_loader.c', png_source, tiff_source,
                            dependencies: [gl_dep, glesv1_cm_dep, glesv2_dep, pgl_dep, png_dep, tiff_dep])

executable('yagears2',
           'main.c',
           dependencies: [waffle_dep, egl_dep, x11_dep, directfb_dep, wayland_egl_dep, xcb_dep, drm_dep, rpi_dep],
           link_whole: libyagears,
           install: true)

if VK
vert_spv_file = custom_target('vert_spv', command: [glslang_validator, '@INPUT@', '-V', '-x'], input: 'vulkan_gears.vert', output: 'vert.spv')
frag_spv_file = custom_target('frag_spv', command: [glslang_validator, '@INPUT@', '-V', '-x'], input: 'vulkan_gears.frag', output: 'frag.spv')

executable('yagears2-vk',
           'vk.c', 'vulkan_gears.c', vert_spv_file, frag_spv_file, 'image_loader.c', png_source, tiff_source,
           dependencies: [vulkan_dep, png_dep, tiff_dep, x11_dep, directfb_dep, wayland_dep, xcb_dep, d2d_dep],
           install: true)
endif

if MOSAIC
executable('yagears2-mosaic',
           'mosaic.c',
           dependencies: glut_dep,
           link_whole: libyagears,
           install: true)
endif

if GUI
executable('yagears2-gui',
           'gui.cc',
           dependencies: [efl_dep, fltk_dep, glfw_dep, glut_dep, gtk_dep, qt_dep, sdl_dep, sfml_dep, wx_dep],
           link_whole: libyagears,
           install: true)
endif

if VK_GUI
executable('yagears2-vk-gui',
           'vk-gui.cc', 'vulkan_gears.c', 'image_loader.c', png_source, tiff_source,
           dependencies: [vulkan_dep, png_dep, tiff_dep, glfw_dep, sdl_dep],
           install: true)
endif
