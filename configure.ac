AC_INIT(yagears, 1.0)

AC_CONFIG_HEADERS(config.h)

AM_INIT_AUTOMAKE

LT_INIT

AC_PROG_CC
AC_PROG_CXX

AC_SYS_LARGEFILE

PKG_PROG_PKG_CONFIG

CFLAGS="$CFLAGS -Wall -D_GNU_SOURCE"
LIBS="$LIBS -lm"

AC_ARG_ENABLE(gl,
              AS_HELP_STRING(--disable-gl, disable OpenGL engine),
              enable_gl=no, enable_gl=yes)
AC_ARG_ENABLE(glesv1_cm,
              AS_HELP_STRING(--disable-glesv1_cm, disable OpenGL ES 1.1 CM engine),
              enable_glesv1_cm=no, enable_glesv1_cm=yes)
AC_ARG_ENABLE(glesv2,
              AS_HELP_STRING(--disable-glesv2, disable OpenGL ES 2.0 engine),
              enable_glesv2=no, enable_glesv2=yes)

if test x$enable_gl = xyes; then
  PKG_CHECK_MODULES(GL, gl, , enable_gl=no)
  if test x$enable_gl = xyes; then
    if test x$enable_glesv1_cm = xyes; then
      GL_CFLAGS="$GL_CFLAGS -DGLESV1_CM_H=\<GL/gl.h\> -DGL_GLEXT_PROTOTYPES"
    fi
    if test x$enable_glesv2 = xyes; then
      GL_CFLAGS="$GL_CFLAGS -DGLESV2_H=\<GL/gl.h\> -DGL_GLEXT_PROTOTYPES"
    fi
  fi
fi

if test x$enable_gl = xno; then
  if test x$enable_glesv1_cm = xyes; then
    PKG_CHECK_MODULES(GLESV1_CM, glesv1_cm, , enable_glesv1_cm=no)
    if test x$enable_glesv1_cm = xyes; then
      GLESV1_CM_CFLAGS="$GLESV1_CM_CFLAGS -DGLESV1_CM_H=\<GLES/gl.h\>"
    fi
  fi
  if test x$enable_glesv2 = xyes; then
    PKG_CHECK_MODULES(GLESV2, glesv2, , enable_glesv2=no)
    if test x$enable_glesv2 = xyes; then
      GLESV2_CFLAGS="$GLESV2_CFLAGS -DGLESV2_H=\<GLES2/gl2.h\>"
    fi
  fi
fi

if test x$enable_gl = xno -a x$enable_glesv1_cm = xno -a x$enable_glesv2 = xno; then
  AC_MSG_ERROR(No engines found)
else
  AM_CONDITIONAL(GL, test x$enable_gl = xyes)
  AM_CONDITIONAL(GLESV1_CM, test x$enable_glesv1_cm = xyes)
  AM_CONDITIONAL(GLESV2, test x$enable_glesv2 = xyes)
fi

AC_ARG_ENABLE(gl-x11,
              AS_HELP_STRING(--disable-gl-x11, disable OpenGL X11 rendering backend),
              enable_gl_x11=no, enable_gl_x11=yes)
AC_ARG_ENABLE(gl-directfb,
              AS_HELP_STRING(--disable-gl-directfb, disable OpenGL DirectFB rendering backend),
              enable_gl_directfb=no, enable_gl_directfb=yes)
AC_ARG_ENABLE(gl-fbdev,
              AS_HELP_STRING(--disable-gl-fbdev, disable OpenGL Framebuffer rendering backend),
              enable_gl_fbdev=no, enable_gl_fbdev=yes)
AC_ARG_ENABLE(egl-x11,
              AS_HELP_STRING(--disable-egl-x11, disable X11 EGL rendering backend),
              enable_egl_x11=no, enable_egl_x11=yes)
AC_ARG_ENABLE(egl-directfb,
              AS_HELP_STRING(--disable-egl-directfb, disable DirectFB EGL rendering backend),
              enable_egl_directfb=no, enable_egl_directfb=yes)
AC_ARG_ENABLE(egl-fbdev,
              AS_HELP_STRING(--disable-egl-fbdev, disable Linux Framebuffer EGL rendering backend),
              enable_egl_fbdev=no, enable_egl_fbdev=yes)
AC_ARG_ENABLE(egl-wayland,
              AS_HELP_STRING(--disable-egl-wayland, disable Wayland EGL rendering backend),
              enable_egl_wayland=no, enable_egl_wayland=yes)
AC_ARG_ENABLE(egl-drm,
              AS_HELP_STRING(--disable-egl-drm, disable DRM EGL rendering backend),
              enable_egl_drm=no, enable_egl_drm=yes)
AC_ARG_ENABLE(egl-rpi,
              AS_HELP_STRING(--disable-egl-rpi, disable Raspberry Pi Dispmanx EGL rendering backend),
              enable_egl_rpi=no, enable_egl_rpi=yes)

if test x$enable_gl = xyes; then
  if test x$enable_gl_x11 = xyes; then
    save_LIBS=$LIBS
    LIBS="$LIBS `$PKG_CONFIG --libs-only-L gl`"
    AC_CHECK_LIB(GL, glXCreateContext, , enable_gl_x11=no)
    LIBS=$save_LIBS
    if test x$enable_gl_x11 = xyes; then
      PKG_CHECK_MODULES(X11, x11,
                        AC_DEFINE(GL_X11, 1, Support for OpenGL Extension to Xlib),
                        enable_gl_x11=no)
    fi
  fi

  if test x$enable_gl_directfb = xyes; then
    AC_MSG_CHECKING(for DirectFBGL module)
    MODULEDIR=`$PKG_CONFIG --variable=moduledir directfb-internal 2> /dev/null`
    if test -d $MODULEDIR/interfaces/IDirectFBGL; then
      AC_MSG_RESULT(yes)
      PKG_CHECK_MODULES(DIRECTFB, directfb,
                        AC_DEFINE(GL_DIRECTFB, 1, Support for OpenGL Extension to DirectFB),
                        enable_gl_directfb=no)
    else
      AC_MSG_RESULT(no)
      enable_gl_directfb=no
    fi
  fi

  if test x$enable_gl_fbdev = xyes; then
    save_LIBS=$LIBS
    LIBS="$LIBS `$PKG_CONFIG --libs-only-L gl`"
    AC_CHECK_LIB(GL, glFBDevCreateContext,
                 AC_DEFINE(GL_FBDEV, 1, Support for OpenGL Extension to Linux FBDev),
                 enable_gl_fbdev=no)
    LIBS=$save_LIBS
  fi
else
  enable_gl_x11=no
  enable_gl_directfb=no
  enable_gl_fbdev=no
fi

if test x$enable_egl_x11 = xyes || test x$enable_egl_directfb = xyes || test x$enable_egl_fbdev = xyes ||
   test x$enable_egl_wayland = xyes || test x$enable_egl_drm || test x$enable_egl_rpi; then
  PKG_CHECK_MODULES(EGL, egl, enable_egl=yes, enable_egl=no)
  if test x$enable_egl = xyes; then
    if test x$enable_egl_x11 = xyes; then
      if test x$enable_gl_x11 = xyes; then
        AC_DEFINE(EGL_X11, 1, Support for EGL with Xlib platform)
      else
        PKG_CHECK_MODULES(X11, x11,
                          AC_DEFINE(EGL_X11, 1, Support for EGL with Xlib platform),
                          enable_egl_x11=no)
      fi
    fi
    if test x$enable_egl_directfb = xyes; then
      if test x$enable_gl_directfb = xyes; then
        AC_DEFINE(EGL_DIRECTFB, 1, Support for EGL with DirectFB platform)
      else
        PKG_CHECK_MODULES(DIRECTFB, directfb,
                          AC_DEFINE(EGL_DIRECTFB, 1, Support for EGL with DirectFB platform),
                          enable_egl_directfb=no)
      fi
    fi
    if test x$enable_egl_fbdev = xyes; then
      AC_DEFINE(EGL_FBDEV, 1, Support for EGL with Linux FBDev platform)
    fi
    if test x$enable_egl_wayland = xyes; then
      PKG_CHECK_MODULES(WAYLAND, wayland-client xkbcommon,
                        AC_DEFINE(EGL_WAYLAND, 1, Support for EGL with Wayland platform),
                        enable_egl_wayland=no)
      if test x$enable_egl_wayland = xyes; then
        AC_MSG_CHECKING(for wl_shell_surface_set_position)
        save_CFLAGS=$CFLAGS
        CFLAGS="$CFLAGS `$PKG_CONFIG --cflags wayland-client`"
        save_LIBS=$LIBS
        LIBS="$LIBS `$PKG_CONFIG --libs wayland-client`"
        AC_TRY_LINK([#include <wayland-client.h>], wl_shell_surface_set_position(NULL, 0, 0), AC_DEFINE(HAVE_WL_SHELL_SURFACE_SET_POSITION, 1, Have function wl_shell_surface_set_position) AC_MSG_RESULT(yes), AC_MSG_RESULT(no))
        CFLAGS=$save_CFLAGS
        LIBS=$save_LIBS
      fi
    fi
    if test x$enable_egl_drm = xyes; then
      PKG_CHECK_MODULES(DRM, gbm libdrm libevdev,
                        AC_DEFINE(EGL_DRM, 1, Support for EGL with DRM platform),
                        enable_egl_drm=no)
      if test x$enable_egl_drm = xyes; then
        DRM_CFLAGS="$DRM_CFLAGS -DDRI_DRIVERDIR=\\\"`$PKG_CONFIG --variable=dridriverdir dri`\\\" -DDRI_MAJOR_VERSION=`$PKG_CONFIG --modversion dri | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'` -DDRI_MINOR_VERSION=`$PKG_CONFIG --modversion dri | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`"
        DRM_LIBS="$DRM_LIBS -ldl"
      fi
    fi
    if test x$enable_egl_rpi = xyes; then
      PKG_CHECK_MODULES(RPI, bcm_host,
                        AC_DEFINE(EGL_RPI, 1, Support for EGL with Raspberry Pi Dispmanx platform),
                        enable_egl_rpi=no)
    fi
  else
    enable_egl_x11=no
    enable_egl_directfb=no
    enable_egl_fbdev=no
    enable_egl_wayland=no
    enable_egl_drm=no
    enable_egl_rpi=no
  fi
fi

if test x$enable_gl_x11 = xno -a x$enable_gl_directfb = xno -a x$enable_gl_fbdev = xno -a \
        x$enable_egl_x11 = xno -a x$enable_egl_directfb = xno -a x$enable_egl_fbdev = xno -a \
        x$enable_egl_wayland = xno -a x$enable_egl_drm = xno -a x$enable_egl_rpi = xno; then
  AC_MSG_ERROR(No rendering backends found)
fi

AC_ARG_ENABLE(efl,
              AS_HELP_STRING(--disable-efl, disable EFL GUI toolkit),
              enable_efl=no, enable_efl=yes)
AC_ARG_ENABLE(fltk,
              AS_HELP_STRING(--disable-fltk, disable FLTK GUI toolkit),
              enable_fltk=no, enable_fltk=yes)
AC_ARG_ENABLE(glfw,
              AS_HELP_STRING(--disable-glfw, disable GLFW GUI toolkit),
              enable_glfw=no, enable_glfw=yes)
AC_ARG_ENABLE(glut,
              AS_HELP_STRING(--disable-glut, disable GLUT GUI toolkit),
              enable_glut=no, enable_glut=yes)
AC_ARG_ENABLE(gtk,
              AS_HELP_STRING(--disable-gtk, disable GTK+ GUI toolkit),
              enable_gtk=no, enable_gtk=yes)
AC_ARG_ENABLE(qt,
              AS_HELP_STRING(--disable-qt, disable Qt GUI toolkit),
              enable_qt=no, enable_qt=yes)
AC_ARG_ENABLE(sdl,
              AS_HELP_STRING(--disable-sdl, disable SDL GUI toolkit),
              enable_sdl=no, enable_sdl=yes)
AC_ARG_ENABLE(sfml,
              AS_HELP_STRING(--disable-sfml, disable SFML GUI toolkit),
              enable_sfml=no, enable_sfml=yes)
AC_ARG_ENABLE(wx,
              AS_HELP_STRING(--disable-wx, disable wxWidgets GUI toolkit),
              enable_wx=no, enable_wx=yes)

if test x$enable_efl = xyes; then
  PKG_CHECK_MODULES(EFL, elementary, AC_DEFINE(EFL, 1, Support for EFL graphical user interface), enable_efl=no)
fi

if test x$enable_fltk = xyes; then
  AC_PATH_PROG(FLTK_CONFIG, fltk-config, no)
  if test x$FLTK_CONFIG != xno; then
    FLTK_CFLAGS=`$FLTK_CONFIG --cflags`
    FLTK_LIBS=`$FLTK_CONFIG --use-gl --ldflags`
    AC_SUBST(FLTK_CFLAGS)
    AC_SUBST(FLTK_LIBS)
    AC_DEFINE(FLTK, 1, Support for FLTK graphical user interface)
  else
    enable_fltk=no
  fi
fi

if test x$enable_glfw = xyes; then
  PKG_CHECK_MODULES(GLFW, glfw3, AC_DEFINE(GLFW, 1, Support for GLFW graphical user interface), enable_glfw=no)
fi

if test x$enable_glut = xyes; then
  PKG_CHECK_MODULES(GLUT, glut, , enable_glut=no)
  if test x$enable_glut = xno; then
    AC_CHECK_LIB(glut, glutCreateWindow, enable_glut=yes, enable_glut=no)
  fi
  if test x$enable_glut = xyes; then
    GLUT_LIBS=-lglut
    AC_DEFINE(GLUT, 1, Support for GLUT graphical user interface)
  fi
fi

if test x$enable_gtk = xyes; then
  AC_MSG_CHECKING(for GTK+ version)
  AC_ARG_WITH(gtk,
              AS_HELP_STRING(--with-gtk=2|3, GTK+ GUI toolkit version (default: 2)),
              [case "$with_gtk" in 2|3) ;; *) AC_MSG_ERROR([invalid GTK+ GUI toolkit version]) ;; esac],
              with_gtk=2)
  AC_MSG_RESULT([$with_gtk])
  if test x$with_gtk = x2; then
    PKG_CHECK_MODULES(GTK, gtkgl-2.0, , enable_gtk=no)
  else
    enable_gtk=no
  fi
  if test x$enable_gtk = xno; then
    PKG_CHECK_MODULES(GTK, gtkgl-3.0, enable_gtk=yes, enable_gtk=no)
    if test x$enable_gtk = xno; then
      PKG_CHECK_MODULES(GTK, gtk+-3.0 >= 3.16.0, enable_gtk=yes, enable_gtk=no)
    fi
  fi
  if test x$enable_gtk = xyes; then
    AC_DEFINE(GTK, 1, Support for GTK+ graphical user interface)
  fi
fi

if test x$enable_qt = xyes; then
  AC_MSG_CHECKING(for Qt version)
  AC_ARG_WITH(qt,
              AS_HELP_STRING(--with-qt=4|5, Qt GUI toolkit version (default: 4)),
              [case "$with_qt" in 4|5) ;; *) AC_MSG_ERROR([invalid Qt GUI toolkit version]) ;; esac],
              with_qt=4)
  AC_MSG_RESULT([$with_qt])
  if test x$with_qt = x4; then
    PKG_CHECK_MODULES(QT, QtOpenGL, , enable_qt=no)
  else
    enable_qt=no
  fi
  if test x$enable_qt = xno; then
    PKG_CHECK_MODULES(QT, Qt5OpenGL, enable_qt=yes, enable_qt=no)
  fi
  if test x$enable_qt = xyes; then
    AC_DEFINE(QT, 1, Support for Qt graphical user interface)
  fi
fi

if test x$enable_sdl = xyes; then
  AC_MSG_CHECKING(for SDL version)
  AC_ARG_WITH(sdl,
              AS_HELP_STRING(--with-sdl=1|2, SDL GUI toolkit version (default: 1)),
              [case "$with_sdl" in 1|2) ;; *) AC_MSG_ERROR([invalid SDL GUI toolkit version]) ;; esac],
              with_sdl=1)
  AC_MSG_RESULT([$with_sdl])
  if test x$with_sdl = x1; then
    PKG_CHECK_MODULES(SDL, sdl, , enable_sdl=no)
  else
    enable_sdl=no
  fi
  if test x$enable_sdl = xno; then
    PKG_CHECK_MODULES(SDL, sdl2, enable_sdl=yes, enable_sdl=no)
  fi
  if test x$enable_sdl = xyes; then
    AC_DEFINE(SDL, 1, Support for SDL graphical user interface)
  fi
fi

if test x$enable_sfml = xyes; then
  PKG_CHECK_MODULES(SFML, sfml-graphics, AC_DEFINE(SFML, 1, Support for SFML graphical user interface), enable_sfml=no)
fi

if test x$enable_wx = xyes; then
  AC_PATH_PROG(WX_CONFIG, wx-config, no)
  if test x$WX_CONFIG != xno; then
    WX_CFLAGS=`$WX_CONFIG --cflags`
    WX_LIBS=`$WX_CONFIG --libs core,gl`
    AC_SUBST(WX_CFLAGS)
    AC_SUBST(WX_LIBS)
    AC_DEFINE(WX, 1, Support for wxWidgets graphical user interface)
  else
    enable_wx=no
  fi
fi

if test x$enable_efl = xno -a x$enable_fltk = xno -a x$enable_glfw = xno -a x$enable_glut = xno -a x$enable_gtk = xno -a x$enable_qt = xno -a x$enable_sdl = xno -a x$enable_sfml = xno -a x$enable_wx = xno; then
  AC_MSG_WARN(No graphical user interface toolkits found)
fi

AM_CONDITIONAL(HAVE_GLUT, test x$enable_glut = xyes)
AM_CONDITIONAL(HAVE_GUI, test x$enable_efl = xyes ||
                         test x$enable_fltk = xyes ||
                         test x$enable_glfw = xyes ||
                         test x$enable_glut = xyes ||
                         test x$enable_gtk = xyes ||
                         test x$enable_qt = xyes ||
                         test x$enable_sdl = xyes ||
                         test x$enable_sfml = xyes ||
                         test x$enable_wx = xyes)

AC_ARG_ENABLE(png,
              AS_HELP_STRING(--disable-png, disable PNG loader),
              enable_png=no, enable_png=yes)
AC_ARG_ENABLE(tiff,
              AS_HELP_STRING(--disable-tiff, disable TIFF loader),
              enable_tiff=no, enable_tiff=yes)

if test x$enable_png = xyes; then
  PKG_CHECK_MODULES(PNG, libpng, , enable_png=no)
fi

if test x$enable_tiff = xyes; then
  PKG_CHECK_MODULES(TIFF, libtiff, , enable_tiff=no)
fi

AM_CONDITIONAL(PNG, test x$enable_png = xyes)
AM_CONDITIONAL(TIFF, test x$enable_tiff = xyes)

AC_OUTPUT(Makefile)

echo
echo "Backends:"
echo
echo "  OpenGL Extension to Xlib         $enable_gl_x11"
echo "  OpenGL Extension to DirectFB     $enable_gl_directfb"
echo "  OpenGL Extension to Linux FBDev  $enable_gl_fbdev"
echo "  EGL with Xlib platform           $enable_egl_x11"
echo "  EGL with DirectFB platform       $enable_egl_directfb"
echo "  EGL with Linux FBDev platform    $enable_egl_fbdev"
echo "  EGL with Wayland platform        $enable_egl_wayland"
echo "  EGL with DRM platform            $enable_egl_drm"
echo "  EGL with RPi Dispmanx platform   $enable_egl_rpi"
echo

echo
echo "Toolkits:"
echo
echo "  EFL                              $enable_efl"
echo "  FLTK                             $enable_fltk"
echo "  GLFW                             $enable_glfw"
echo "  GLUT                             $enable_glut"
echo "  GTK                              $enable_gtk"
echo "  QT                               $enable_qt"
echo "  SDL                              $enable_sdl"
echo "  SFML                             $enable_sfml"
echo "  WX                               $enable_wx"
echo

echo
echo "Engines:"
echo
echo "  OpenGL                           $enable_gl"
echo "  OpenGL ES 1.1 CM                 $enable_glesv1_cm"
echo "  OpenGL ES 2.0                    $enable_glesv2"
echo
