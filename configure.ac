# yagears                  Yet Another Gears OpenGL / Vulkan demo
# Copyright (C) 2013-2024  Nicolas Caramelli
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

AC_INIT(yagears, 2.8)

AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE
LT_INIT
AC_PROG_CC
AC_PROG_CXX
AC_SYS_LARGEFILE
PKG_PROG_PKG_CONFIG

CFLAGS="$CFLAGS -Wall -D_GNU_SOURCE"
LIBS="$LIBS -lm"

# Configuration options

AC_ARG_ENABLE(engine-ctor,
              AS_HELP_STRING(--disable-engine-ctor, disable use of constructor attribute to register an engine),,
              CFLAGS="$CFLAGS -DENGINE_CTOR")

AC_ARG_ENABLE(gl,
              AS_HELP_STRING(--disable-gl, disable OpenGL 1.0 Engine),,
              enable_gl=yes)
AC_ARG_ENABLE(glesv1_cm,
              AS_HELP_STRING(--disable-glesv1_cm, disable OpenGL ES 1.1 CM Engine),,
              enable_glesv1_cm=yes)
AC_ARG_ENABLE(glesv2,
              AS_HELP_STRING(--disable-glesv2, disable OpenGL ES 2.0 Engine),,
              enable_glesv2=yes)
AC_ARG_WITH(pgl,
            AS_HELP_STRING(--with-pgl=PATH, PATH to PortableGL platform header),,
            with_pgl=no)
AC_ARG_WITH(pgl-cflags,
            AS_HELP_STRING(--with-pgl-cflags=CFLAGS, CFLAGS for PortableGL platform header),,
            with_pgl_cflags=)

AC_ARG_ENABLE(gl-x11,
              AS_HELP_STRING(--disable-gl-x11, disable OpenGL extension for Xlib Backend),,
              enable_gl_x11=yes)
AC_ARG_ENABLE(gl-directfb,
              AS_HELP_STRING(--disable-gl-directfb, disable OpenGL extension for DirectFB Backend),,
              enable_gl_directfb=yes)
AC_ARG_ENABLE(gl-fbdev,
              AS_HELP_STRING(--disable-gl-fbdev, disable OpenGL extension for Linux Framebuffer Backend),,
              enable_gl_fbdev=yes)
AC_ARG_ENABLE(egl-x11,
              AS_HELP_STRING(--disable-egl-x11, disable EGL interface for Xlib Backend),,
              enable_egl_x11=yes)
AC_ARG_ENABLE(egl-directfb,
              AS_HELP_STRING(--disable-egl-directfb, disable EGL interface for DirectFB Backend),,
              enable_egl_directfb=yes)
AC_ARG_ENABLE(egl-fbdev,
              AS_HELP_STRING(--disable-egl-fbdev, disable EGL interface for Linux Framebuffer Backend),,
              enable_egl_fbdev=yes)
AC_ARG_ENABLE(egl-wayland,
              AS_HELP_STRING(--disable-egl-wayland, disable EGL interface for Wayland Backend),,
              enable_egl_wayland=yes)
AC_ARG_ENABLE(egl-xcb,
              AS_HELP_STRING(--disable-egl-xcb, disable EGL interface for XCB Backend),,
              enable_egl_xcb=yes)
AC_ARG_ENABLE(egl-drm,
              AS_HELP_STRING(--disable-egl-drm, disable EGL interface for DRM Backend),,
              enable_egl_drm=yes)
AC_ARG_ENABLE(egl-rpi,
              AS_HELP_STRING(--disable-egl-rpi, disable EGL interface for Raspberry Pi Dispmanx Backend),,
              enable_egl_rpi=yes)
AC_ARG_ENABLE(waffle,
              AS_HELP_STRING(--disable-waffle, disable Waffle cross-platform wrapper),,
              enable_waffle=yes)

AC_ARG_ENABLE(vk-x11,
              AS_HELP_STRING(--disable-vk-x11, disable Vulkan extension for Xlib WSI),,
              enable_vk_x11=yes)
AC_ARG_ENABLE(vk-directfb,
              AS_HELP_STRING(--disable-vk-directfb, disable Vulkan extension for DirectFB WSI),,
              enable_vk_directfb=yes)
AC_ARG_ENABLE(vk-fbdev,
              AS_HELP_STRING(--disable-vk-fbdev, disable Vulkan extension for Linux Framebuffer WSI),,
              enable_vk_fbdev=yes)
AC_ARG_ENABLE(vk-wayland,
              AS_HELP_STRING(--disable-vk-wayland, disable Vulkan extension for Wayland WSI),,
              enable_vk_wayland=yes)
AC_ARG_ENABLE(vk-xcb,
              AS_HELP_STRING(--disable-vk-xcb, disable Vulkan extension for XCB WSI),,
              enable_vk_xcb=yes)
AC_ARG_ENABLE(vk-d2d,
              AS_HELP_STRING(--disable-vk-d2d, disable Vulkan extension for Direct-to-Display WSI),,
              enable_vk_d2d=yes)

AC_ARG_ENABLE(efl,
              AS_HELP_STRING(--disable-efl, disable EFL GUI Toolkit),,
              enable_efl=yes)
AC_ARG_ENABLE(fltk,
              AS_HELP_STRING(--disable-fltk, disable FLTK GUI Toolkit),,
              enable_fltk=yes)
AC_ARG_ENABLE(glfw,
              AS_HELP_STRING(--disable-glfw, disable GLFW GUI Toolkit),,
              enable_glfw=yes)
AC_ARG_ENABLE(glut,
              AS_HELP_STRING(--disable-glut, disable GLUT GUI Toolkit),,
              enable_glut=yes)
AC_ARG_ENABLE(gtk,
              AS_HELP_STRING(--disable-gtk, disable GTK+ GUI Toolkit),,
              enable_gtk=yes)
AC_ARG_ENABLE(qt,
              AS_HELP_STRING(--disable-qt, disable Qt GUI Toolkit),,
              enable_qt=yes)
AC_ARG_ENABLE(sdl,
              AS_HELP_STRING(--disable-sdl, disable SDL GUI Toolkit),,
              enable_sdl=yes)
AC_ARG_ENABLE(sfml,
              AS_HELP_STRING(--disable-sfml, disable SFML GUI Toolkit),,
              enable_sfml=yes)
AC_ARG_ENABLE(wx,
              AS_HELP_STRING(--disable-wx, disable wxWidgets GUI Toolkit),,
              enable_wx=yes)

AC_ARG_WITH(gtk,
            AS_HELP_STRING(--with-gtk=2|3, GTK+ GUI Toolkit version (default: 3)),
            [case "$with_gtk" in 2|3) ;; *) AC_MSG_ERROR(Invalid GTK+ GUI Toolkit version) ;; esac],
            with_gtk=3)
AC_ARG_WITH(qt,
            AS_HELP_STRING(--with-qt=4|5, Qt GUI Toolkit version (default: 5)),
            [case "$with_qt" in 4|5) ;; *) AC_MSG_ERROR(Invalid Qt GUI Toolkit version) ;; esac],
            with_qt=5)
AC_ARG_WITH(sdl,
            AS_HELP_STRING(--with-sdl=1|2, SDL GUI Toolkit version (default: 2)),
            [case "$with_sdl" in 1|2) ;; *) AC_MSG_ERROR(Invalid SDL GUI Toolkit version) ;; esac],
            with_sdl=2)

AC_ARG_ENABLE(png,
              AS_HELP_STRING(--disable-png, disable PNG loader),,
              enable_png=yes)
AC_ARG_ENABLE(tiff,
              AS_HELP_STRING(--disable-tiff, disable TIFF loader),,
              enable_tiff=yes)

# OpenGL Engines

if test x$with_pgl != xno; then
  enable_gl=no
  enable_glesv1_cm=no
  enable_glesv2=no

  if test -f $with_pgl; then
    PGL_CFLAGS="-DPGL_H=\\\"$with_pgl\\\" $with_pgl_cflags"
    AC_SUBST(PGL_CFLAGS)
  else
    AC_MSG_ERROR(PortableGL platform header not found)
  fi
fi

if test x$enable_gl = xyes; then
  PKG_CHECK_MODULES(GL, gl, , enable_gl=no)
  if test x$enable_gl = xyes; then
    if test x$enable_glesv1_cm = xyes; then
      GL_CFLAGS="$GL_CFLAGS -DGLESV1_CM_H=\<GL/gl.h\> -DGLESV1_CM_LIB=\\\"libGL.so.1\\\" -DGL_GLEXT_PROTOTYPES"
    fi
    if test x$enable_glesv2 = xyes; then
      AC_PATH_TOOL(XXD, xxd)
      if test -n "$XXD"; then
        GL_CFLAGS="$GL_CFLAGS -DGLESV2_H=\<GL/gl.h\> -DGLESV2_LIB=\\\"libGL.so.1\\\" -DGL_GLEXT_PROTOTYPES"
      else
        enable_glesv2=no
      fi
    fi
    if test x$enable_glesv1_cm = xyes -o x$enable_glesv2 = xyes; then
      GL_LIBS="$GL_LIBS -ldl"
    fi
  fi
fi

if test x$enable_gl = xno; then
  if test x$enable_glesv1_cm = xyes; then
    PKG_CHECK_MODULES(GLESV1_CM, glesv1_cm, , enable_glesv1_cm=no)
    if test x$enable_glesv1_cm = xyes; then
      GLESV1_CM_CFLAGS="$GLESV1_CM_CFLAGS -DGLESV1_CM_H=\<GLES/gl.h\> -DGLESV1_CM_LIB=\\\"libGLESv1_CM.so.1\\\""
      GLESV1_CM_LIBS=-ldl
    fi
  fi

  if test x$enable_glesv2 = xyes; then
    PKG_CHECK_MODULES(GLESV2, glesv2, , enable_glesv2=no)
    if test x$enable_glesv2 = xyes; then
      AC_PATH_TOOL(XXD, xxd)
      if test -n "$XXD"; then
        GLESV2_CFLAGS="$GLESV2_CFLAGS -DGLESV2_H=\<GLES2/gl2.h\> -DGLESV2_LIB=\\\"libGLESv2.so.2\\\""
        GLESV2_LIBS=-ldl
      else
        enable_glesv2=no
      fi
    fi
  fi
fi

if test x$enable_gl = xno -a x$enable_glesv1_cm = xno -a x$enable_glesv2 = xno -a x$with_pgl = xno; then
  AC_MSG_WARN(No OpenGL Engines found)
fi

# OpenGL Backends

if test x$enable_gl = xyes; then
  if test x$enable_gl_x11 = xyes; then
    save_LIBS=$LIBS
    LIBS="$LIBS `$PKG_CONFIG --libs-only-L gl`"
    AC_CHECK_LIB(GL, glXCreateContext, , enable_gl_x11=no)
    LIBS=$save_LIBS
    if test x$enable_gl_x11 = xyes; then
      PKG_CHECK_MODULES(X11, x11, , enable_gl_x11=no)
    fi
  fi
else
  enable_gl_x11=no
fi
if test x$enable_gl_x11 = xyes; then
  AC_DEFINE(GL_X11, , Support for GLX)
fi

if test x$enable_gl_directfb = xyes; then
  AC_MSG_CHECKING(for DirectFBGL module)
  MODULEDIR=`$PKG_CONFIG --variable=moduledir directfb-internal 2> /dev/null`
  if test -d $MODULEDIR/interfaces/IDirectFBGL; then
    AC_MSG_RESULT(yes)
    PKG_CHECK_MODULES(DIRECTFB, directfb, , enable_gl_directfb=no)
  else
    AC_MSG_RESULT(no)
    enable_gl_directfb=no
  fi
fi
if test x$enable_gl_directfb = xyes; then
  AC_DEFINE(GL_DIRECTFB, , Support for DirectFBGL)
fi

if test x$enable_gl = xyes; then
  if test x$enable_gl_fbdev = xyes; then
    save_LIBS=$LIBS
    LIBS="$LIBS `$PKG_CONFIG --libs-only-L gl`"
    AC_CHECK_LIB(GL, glFBDevCreateContext, , enable_gl_fbdev=no)
    LIBS=$save_LIBS
  fi
else
  enable_gl_fbdev=no
fi
if test x$enable_gl_fbdev = xyes; then
  AC_DEFINE(GL_FBDEV, , Support for GLFBDev)
fi

if test x$enable_egl_x11 = xyes -o x$enable_egl_directfb = xyes -o x$enable_egl_fbdev = xyes -o x$enable_egl_wayland = xyes -o x$enable_egl_xcb = xyes -o x$enable_egl_drm -o x$enable_egl_rpi; then
  if test x$with_pgl = xno; then
    PKG_CHECK_MODULES(EGL, egl, have_egl=yes, have_egl=no)
  fi
  if test x$have_egl = xyes; then
    EGL_CFLAGS="$EGL_CFLAGS -DEGL_NO_PLATFORM_SPECIFIC_TYPES -DEGL_EGLEXT_PROTOTYPES"

    if test x$enable_egl_x11 = xyes; then
      if test x$enable_gl_x11 = xno; then
        PKG_CHECK_MODULES(X11, x11, , enable_egl_x11=no)
      fi
    fi

    if test x$enable_egl_directfb = xyes; then
      if test x$enable_gl_directfb = xno; then
        PKG_CHECK_MODULES(DIRECTFB, directfb, , enable_egl_directfb=no)
      fi
    fi

    if test x$enable_egl_wayland = xyes; then
      PKG_CHECK_MODULES(WAYLAND, wayland-client xkbcommon, , enable_egl_wayland=no)
      if test x$enable_egl_wayland = xyes; then
        AC_MSG_CHECKING(for wl_shell_surface_set_position)
        save_CFLAGS=$CFLAGS
        CFLAGS="$CFLAGS $WAYLAND_CFLAGS"
        save_LIBS=$LIBS
        LIBS="$LIBS $WAYLAND_LIBS"
        AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <wayland-client.h>], wl_shell_surface_set_position(NULL, 0, 0))], AC_DEFINE(HAVE_WL_SHELL_SURFACE_SET_POSITION, 1, Have function wl_shell_surface_set_position) AC_MSG_RESULT(yes), AC_MSG_RESULT(no))
        CFLAGS=$save_CFLAGS
        LIBS=$save_LIBS

        WAYLAND_EGL_CFLAGS=$WAYLAND_CFLAGS
        WAYLAND_EGL_LIBS="$WAYLAND_LIBS -lwayland-egl"
        AC_SUBST(WAYLAND_EGL_CFLAGS)
        AC_SUBST(WAYLAND_EGL_LIBS)
      fi
    fi

    if test x$enable_egl_xcb = xyes; then
      PKG_CHECK_MODULES(XCB, xcb, , enable_egl_xcb=no)
    fi

    if test x$enable_egl_drm = xyes; then
      PKG_CHECK_MODULES(DRM, gbm libdrm libevdev, , enable_egl_drm=no)
      if test x$enable_egl_drm = xyes; then
        PKG_CHECK_MODULES(DRI, dri, have_dri=yes, have_dri=no)
        if test x$have_dri = xyes; then
          DRIDRIVERDIR=`$PKG_CONFIG --variable=dridriverdir dri`
          MODVERSION=`$PKG_CONFIG --modversion dri`
          DRIMAJORVERSION=`echo $MODVERSION | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
          DRIMINORVERSION=`echo $MODVERSION | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
          DRM_CFLAGS="$DRM_CFLAGS -DDRI_DRIVERDIR=\\\"$DRIDRIVERDIR\\\" -DDRI_MAJOR_VERSION=$DRIMAJORVERSION -DDRI_MINOR_VERSION=$DRIMINORVERSION"
          DRM_LIBS="$DRM_LIBS -ldl"
          AC_DEFINE(HAVE_DRI, 1, Have Direct Rendering Infrastructure)
        fi
      fi
    fi

    if test x$enable_egl_rpi = xyes; then
      PKG_CHECK_MODULES(RPI, bcm_host, , enable_egl_rpi=no)
      if test x$enable_egl_rpi = xyes; then
        RPI_LIBS="$RPI_LIBS -lvchostif"
      fi
    fi
  else
    enable_egl_x11=no
    enable_egl_directfb=no
    enable_egl_fbdev=no
    enable_egl_wayland=no
    enable_egl_xcb=no
    enable_egl_drm=no
    enable_egl_rpi=no
  fi
fi
if test x$enable_egl_x11 = xyes; then
  AC_DEFINE(EGL_X11, , Support for EGL with Xlib platform)
fi
if test x$enable_egl_directfb = xyes; then
  AC_DEFINE(EGL_DIRECTFB, , Support for EGL with DirectFB platform)
fi
if test x$enable_egl_fbdev = xyes; then
  AC_DEFINE(EGL_FBDEV, , Support for EGL with Linux FBDev platform)
fi
if test x$enable_egl_wayland = xyes; then
  AC_DEFINE(EGL_WAYLAND, , Support for EGL with Wayland platform)
fi
if test x$enable_egl_xcb = xyes; then
  AC_DEFINE(EGL_XCB, , Support for EGL with XCB platform)
fi
if test x$enable_egl_drm = xyes; then
  AC_DEFINE(EGL_DRM, , Support for EGL with DRM platform)
fi
if test x$enable_egl_rpi = xyes; then
  AC_DEFINE(EGL_RPI, , Support for EGL with Raspberry Pi Dispmanx platform)
fi

if test x$enable_waffle = xyes && test x$with_pgl = xno; then
  PKG_CHECK_MODULES(WAFFLE, libinput waffle-1, , enable_waffle=no)
else
  enable_waffle=no
fi
if test x$enable_waffle = xyes; then
  AC_DEFINE(WAFFLE, , Support for Waffle cross-platform wrapper)
fi

if test x$enable_gl_x11 = xno -a x$enable_gl_directfb = xno -a x$enable_gl_fbdev = xno -a x$enable_egl_x11 = xno -a x$enable_egl_directfb = xno -a x$enable_egl_fbdev = xno -a x$enable_egl_wayland = xno -a x$enable_egl_xcb = xno -a x$enable_egl_drm = xno -a x$enable_egl_rpi = xno -a x$enable_waffle = xno; then
  AC_MSG_WARN(No OpenGL Backends found)
fi

# Vulkan WSIs

if test x$enable_vk_x11 = xyes -o x$enable_vk_directfb = xyes -o x$enable_vk_fbdev = xyes -o x$enable_vk_wayland = xyes -o x$enable_vk_xcb = xyes -o x$enable_vk_d2d = xyes; then
  PKG_CHECK_MODULES(VULKAN, vulkan, enable_vulkan=yes, enable_vulkan=no)
  AC_PATH_TOOL(GLSLANG_VALIDATOR, glslangValidator)
  if test x$enable_vulkan = xyes -a -n "$GLSLANG_VALIDATOR"; then
    save_LIBS=$LIBS
    LIBS="$LIBS `$PKG_CONFIG --libs-only-L vulkan`"

    if test x$enable_vk_x11 = xyes; then
      AC_CHECK_LIB(vulkan, vkCreateXlibSurfaceKHR, , enable_vk_x11=no)
      if test x$enable_vk_x11 = xyes; then
        if test x$enable_gl_x11 = xno -a x$enable_egl_x11 = xno; then
          PKG_CHECK_MODULES(X11, x11, , enable_vk_x11=no)
        fi
      fi
    fi

    if test x$enable_vk_directfb = xyes; then
      AC_CHECK_LIB(vulkan, vkCreateDirectFBSurfaceEXT, , enable_vk_directfb=no)
      if test x$enable_vk_directfb = xyes; then
        if test x$enable_gl_directfb = xno -a x$enable_egl_directfb = xno; then
          PKG_CHECK_MODULES(DIRECTFB, directfb, , enable_vk_directfb=no)
        fi
      fi
    fi

    if test x$enable_vk_fbdev = xyes; then
      AC_CHECK_LIB(vulkan, vkCreateFBDevSurfaceEXT, , enable_vk_fbdev=no)
    fi

    if test x$enable_vk_wayland = xyes; then
      AC_CHECK_LIB(vulkan, vkCreateWaylandSurfaceKHR, , enable_vk_wayland=no)
      if test x$enable_vk_wayland = xyes; then
        if test x$enable_egl_wayland = xno; then
          PKG_CHECK_MODULES(WAYLAND, wayland-client xkbcommon, , enable_vk_wayland=no)
          if test x$enable_vk_wayland = xyes; then
            AC_MSG_CHECKING(for wl_shell_surface_set_position)
            save_CFLAGS=$CFLAGS
            CFLAGS="$CFLAGS $WAYLAND_CFLAGS"
            LIBS="$save_LIBS $WAYLAND_LIBS"
            AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <wayland-client.h>], wl_shell_surface_set_position(NULL, 0, 0))], AC_DEFINE(HAVE_WL_SHELL_SURFACE_SET_POSITION, 1, Have function wl_shell_surface_set_position) AC_MSG_RESULT(yes), AC_MSG_RESULT(no))
            CFLAGS=$save_CFLAGS
            LIBS="$save_LIBS `$PKG_CONFIG --libs-only-L vulkan`"
          fi
        fi
      fi
    fi

    if test x$enable_vk_xcb = xyes; then
      AC_CHECK_LIB(vulkan, vkCreateXcbSurfaceKHR, , enable_vk_xcb=no)
      if test x$enable_vk_xcb = xyes; then
        if test x$enable_egl_xcb = xno; then
          PKG_CHECK_MODULES(XCB, xcb, , enable_vk_xcb=no)
        fi
      fi
    fi

    if test x$enable_vk_d2d = xyes; then
      AC_CHECK_LIB(vulkan, vkCreateDisplayPlaneSurfaceKHR, , enable_vk_d2d=no)
      if test x$enable_vk_d2d = xyes; then
        PKG_CHECK_MODULES(D2D, libevdev, , enable_vk_d2d=no)
      fi
    fi

    LIBS=$save_LIBS
  else
    enable_vk_x11=no
    enable_vk_directfb=no
    enable_vk_fbdev=no
    enable_vk_wayland=no
    enable_vk_xcb=no
    enable_vk_d2d=no
  fi
fi
if test x$enable_vk_x11 = xyes; then
  AC_DEFINE(VK_X11, , Support for Xlib WSI)
fi
if test x$enable_vk_directfb = xyes; then
  AC_DEFINE(VK_DIRECTFB, , Support for DirectFB WSI)
fi
if test x$enable_vk_fbdev = xyes; then
  AC_DEFINE(VK_FBDEV, , Support for FBDev WSI)
fi
if test x$enable_vk_wayland = xyes; then
  AC_DEFINE(VK_WAYLAND, , Support for Wayland WSI)
fi
if test x$enable_vk_xcb = xyes; then
  AC_DEFINE(VK_XCB, , Support for XCB WSI)
fi
if test x$enable_vk_d2d = xyes; then
  AC_DEFINE(VK_D2D, , Support for D2D WSI)
fi

if test x$enable_vk_x11 = xno -a x$enable_vk_directfb = xno -a x$enable_vk_fbdev = xno -a x$enable_vk_wayland = xno -a x$enable_vk_xcb = xno -a x$enable_vk_d2d = xno; then
  AC_MSG_WARN(No Vulkan WSIs found)
fi

# Graphical User Interface Toolkits

if test x$enable_gl = xno -a x$enable_glesv1_cm = xno -a x$enable_glesv2 = xno; then
  enable_glut=no
fi

if test x$with_pgl != xno -a x$enable_vk_x11 = xno -a x$enable_vk_directfb = xno -a x$enable_vk_fbdev = xno -a x$enable_vk_wayland = xno -a x$enable_vk_xcb = xno -a x$enable_vk_d2d = xno; then
  enable_efl=no
  enable_fltk=no
  enable_glfw=no
  enable_gtk=no
  enable_qt=no
  enable_sdl=no
  enable_sfml=no
  enable_wx=no
fi

if test x$enable_efl = xyes; then
  PKG_CHECK_MODULES(EFL, elementary, , enable_efl=no)
fi
if test x$enable_efl = xyes; then
  AC_DEFINE(EFL, , Support for EFL graphical user interface)
fi

if test x$enable_fltk = xyes; then
  AC_PATH_PROG(FLTK_CONFIG, fltk-config, no)
  if test x$FLTK_CONFIG != xno; then
    FLTK_CFLAGS=`$FLTK_CONFIG --cflags`
    FLTK_LIBS=`$FLTK_CONFIG --use-gl --ldflags`
    AC_SUBST(FLTK_CFLAGS)
    AC_SUBST(FLTK_LIBS)
  else
    enable_fltk=no
  fi
fi
if test x$enable_fltk = xyes; then
  AC_DEFINE(FLTK, , Support for FLTK graphical user interface)
fi

if test x$enable_glfw = xyes; then
  PKG_CHECK_MODULES(GLFW, glfw3, , enable_glfw=no)
fi
if test x$enable_glfw = xyes; then
  AC_DEFINE(GLFW, , Support for GLFW graphical user interface)
fi

if test x$enable_glut = xyes; then
  PKG_CHECK_MODULES(GLUT, glut, , enable_glut=no)
  if test x$enable_glut = xno; then
    AC_CHECK_LIB(glut, glutExit, enable_glut=yes, enable_glut=no)
    if test x$enable_glut = xyes; then
      GLUT_LIBS=-lglut
    fi
  fi
fi
if test x$enable_glut = xyes; then
  AC_DEFINE(GLUT, , Support for GLUT graphical user interface)
fi

if test x$enable_gtk = xyes; then
  AC_MSG_CHECKING(for GTK+ version)
  AC_MSG_RESULT([$with_gtk])
  if test x$with_gtk = x3; then
    PKG_CHECK_MODULES(GTK, gtk+-3.0 >= 3.16.0, , enable_gtk=no)
  else
    enable_gtk=no
  fi
  if test x$enable_gtk = xno; then
    if test x$with_gtk = x3; then
      PKG_CHECK_MODULES(GTK, gtkgl-3.0, enable_gtk=yes, enable_gtk=no)
    fi
    if test x$enable_gtk = xno; then
      PKG_CHECK_MODULES(GTK, gtkgl-2.0, enable_gtk=yes, enable_gtk=no)
    fi
  fi
fi
if test x$enable_gtk = xyes; then
  AC_DEFINE(GTK, , Support for GTK+ graphical user interface)
fi

if test x$enable_qt = xyes; then
  AC_MSG_CHECKING(for Qt version)
  AC_MSG_RESULT([$with_qt])
  if test x$with_qt = x5; then
    PKG_CHECK_MODULES(QT, Qt5OpenGL, , enable_qt=no)
  else
    enable_qt=no
  fi
  if test x$enable_qt = xno; then
    PKG_CHECK_MODULES(QT, QtOpenGL, enable_qt=yes, enable_qt=no)
  fi
fi
if test x$enable_qt = xyes; then
  AC_DEFINE(QT, , Support for Qt graphical user interface)
fi

if test x$enable_sdl = xyes; then
  AC_MSG_CHECKING(for SDL version)
  AC_MSG_RESULT([$with_sdl])
  if test x$with_sdl = x2; then
    PKG_CHECK_MODULES(SDL, sdl2, , enable_sdl=no)
    if test x$enable_sdl = xno; then
      with_sdl=1
    fi
  else
    enable_sdl=no
  fi
  if test x$enable_sdl = xno; then
    PKG_CHECK_MODULES(SDL, sdl, enable_sdl=yes, enable_sdl=no)
  fi
fi
if test x$enable_sdl = xyes; then
  AC_DEFINE(SDL, , Support for SDL graphical user interface)
fi

if test x$enable_sfml = xyes; then
  PKG_CHECK_MODULES(SFML, sfml-graphics, , enable_sfml=no)
fi
if test x$enable_sfml = xyes; then
  AC_DEFINE(SFML, , Support for SFML graphical user interface)
fi

if test x$enable_wx = xyes; then
  AC_PATH_PROG(WX_CONFIG, wx-config, no)
  if test x$WX_CONFIG != xno; then
    WX_CFLAGS=`$WX_CONFIG --cflags`
    WX_LIBS=`$WX_CONFIG --libs core,gl`
    AC_SUBST(WX_CFLAGS)
    AC_SUBST(WX_LIBS)
  else
    enable_wx=no
  fi
fi
if test x$enable_wx = xyes; then
  AC_DEFINE(WX, 1, Support for wxWidgets graphical user interface)
fi

if test x$enable_efl = xno -a x$enable_fltk = xno -a x$enable_glfw = xno -a x$enable_glut = xno -a x$enable_gtk = xno -a x$enable_qt = xno -a x$enable_sdl = xno -a x$enable_sfml = xno -a x$enable_wx = xno; then
  AC_MSG_WARN(No Graphical User Interface Toolkits found)
fi

# Image libraries

if test x$enable_png = xyes; then
  PKG_CHECK_MODULES(PNG, libpng, , enable_png=no)
fi

if test x$enable_tiff = xyes; then
  PKG_CHECK_MODULES(TIFF, libtiff, , enable_tiff=no)
fi

# Build configuration

echo
echo "OpenGL Engines:"
echo
echo "  OpenGL 1.0                        $enable_gl"
echo "  OpenGL ES 1.1 CM                  $enable_glesv1_cm"
echo "  OpenGL ES 2.0                     $enable_glesv2"
echo "  PortableGL                        $with_pgl"
echo

echo
echo "OpenGL Backends:"
echo
echo "  OpenGL extension for Xlib         $enable_gl_x11"
echo "  OpenGL extension for DirectFB     $enable_gl_directfb"
echo "  OpenGL extension for Linux FBDev  $enable_gl_fbdev"
echo "  EGL    interface for Xlib         $enable_egl_x11"
echo "  EGL    interface for DirectFB     $enable_egl_directfb"
echo "  EGL    interface for Linux FBDev  $enable_egl_fbdev"
echo "  EGL    interface for Wayland      $enable_egl_wayland"
echo "  EGL    interface for XCB          $enable_egl_xcb"
echo "  EGL    interface for DRM          $enable_egl_drm"
echo "  EGL    interface for RPi Dispmanx $enable_egl_rpi"
echo "  Waffle cross-platform wrapper     $enable_waffle"
echo

echo
echo "Vulkan WSIs:"
echo
echo "  Vulkan extension for Xlib         $enable_vk_x11"
echo "  Vulkan extension for DirectFB     $enable_vk_directfb"
echo "  Vulkan extension for Linux FBDev  $enable_vk_fbdev"
echo "  Vulkan extension for Wayland      $enable_vk_wayland"
echo "  Vulkan extension for XCB          $enable_vk_xcb"
echo "  Vulkan extension for D2D          $enable_vk_d2d"
echo

echo
echo "Graphical User Interface Toolkits:"
echo
echo "  EFL                               $enable_efl"
echo "  FLTK                              $enable_fltk"
echo "  GLFW                              $enable_glfw"
echo "  GLUT                              $enable_glut"
echo "  GTK                               $enable_gtk"
echo "  QT                                $enable_qt"
echo "  SDL                               $enable_sdl"
echo "  SFML                              $enable_sfml"
echo "  WX                                $enable_wx"
echo

# Build rules

AM_CONDITIONAL(PNG, test x$enable_png = xyes)
AM_CONDITIONAL(TIFF, test x$enable_tiff = xyes)

AM_CONDITIONAL(GL, test x$enable_gl = xyes)
AM_CONDITIONAL(GLESV1_CM, test x$enable_glesv1_cm = xyes)
AM_CONDITIONAL(GLESV2, test x$enable_glesv2 = xyes)
AM_CONDITIONAL(PGL, test x$with_pgl != xno)
AM_CONDITIONAL(OPENGL, test x$enable_gl = xyes -o x$enable_glesv1_cm = xyes -o x$enable_glesv2 = xyes -o x$with_pgl != xno)

AM_CONDITIONAL(VK, test x$enable_vk_x11 = xyes -o x$enable_vk_directfb = xyes -o x$enable_vk_fbdev = xyes -o x$enable_vk_wayland = xyes -o x$enable_vk_xcb = xyes -o x$enable_vk_d2d = xyes)

AM_CONDITIONAL(MOSAIC, test x$enable_glut = xyes)

AM_CONDITIONAL(GUI, test x$enable_gl = xyes -o x$enable_glesv1_cm = xyes -o x$enable_glesv2 = xyes && test x$enable_efl = xyes -o x$enable_fltk = xyes -o x$enable_glfw = xyes -o x$enable_glut = xyes -o x$enable_gtk = xyes -o x$enable_qt = xyes -o x$enable_sdl = xyes -o x$enable_sfml = xyes -o x$enable_wx = xyes)

AM_CONDITIONAL(VK_GUI, test x$enable_vk_x11 = xyes -o x$enable_vk_directfb = xyes -o x$enable_vk_fbdev = xyes -o x$enable_vk_wayland = xyes -o x$enable_vk_xcb = xyes -o x$enable_vk_d2d = xyes && test x$enable_glfw = xyes -o x$enable_sdl = xyes -a x$with_sdl = x2 -o x$enable_sfml = xyes)

AC_CONFIG_FILES(Makefile)

AC_OUTPUT
