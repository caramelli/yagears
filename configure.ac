AC_INIT(yagears, 0.6)

AC_CONFIG_HEADERS(config.h)

AM_INIT_AUTOMAKE

AC_PROG_CC
AC_PROG_CXX

AM_PROG_CC_C_O

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
      GL_CFLAGS="$GL_CFLAGS -DGLESV1_CM_H=\<GL/gl.h\>"
    fi
    if test x$enable_glesv2 = xyes; then
      GL_CFLAGS="$GL_CFLAGS -DGLESV2_H=\<GL/gl.h\>"
    fi
  fi
else
  if test x$enable_glesv1_cm = xyes; then
    PKG_CHECK_MODULES(GLESV1_CM, glesv1_cm, , enable_glesv1_cm=no)
    GLESV1_CM_CFLAGS="$GLESV1_CM_CFLAGS -DGLESV1_CM_H=\<GLES/gl.h\>"
  fi
  if test x$enable_glesv2 = xyes; then
    PKG_CHECK_MODULES(GLESV2, glesv2, , enable_glesv2=no)
    GLESV2_CFLAGS="$GLESV2_CFLAGS -DGLESV2_H=\<GLES2/gl2.h\>"
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

if test x$enable_gl = xyes; then
  if test x$enable_gl_x11 = xyes; then
    save_LIBS=$LIBS
    LIBS="$LIBS `$PKG_CONFIG --libs-only-L gl`"
    AC_CHECK_LIB(GL, glXCreateContext,
                 AC_DEFINE(GL_X11, 1, Support for OpenGL Extension to Xlib),
                 enable_gl_x11=no)
    LIBS=$save_LIBS
  fi

  if test x$enable_gl_directfb = xyes; then
    AC_MSG_CHECKING(for DirectFBGL module)
    MODULEDIR=`$PKG_CONFIG --variable=moduledir directfb-internal`
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
   test x$enable_egl_wayland = xyes; then
  PKG_CHECK_MODULES(EGL, egl, enable_egl=yes, enable_egl=no)
  if test x$enable_egl = xyes; then
    if test x$enable_egl_x11 = xyes; then
      PKG_CHECK_MODULES(X11, x11,
                        AC_DEFINE(EGL_X11, 1, Support for EGL with Xlib platform),
                        enable_egl_x11=no)
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
    fi
  else
    enable_egl_x11=no
    enable_egl_directfb=no
    enable_egl_fbdev=no
    enable_egl_wayland=no
  fi
fi

if test x$enable_gl_x11 = xno -a x$enable_gl_directfb = xno -a x$enable_gl_fbdev = xno -a \
        x$enable_egl_x11 = xno -a x$enable_egl_directfb = xno -a x$enable_egl_fbdev = xno -a \
        x$enable_egl_wayland = xno; then
  AC_MSG_ERROR(No rendering backends found)
fi

AC_ARG_ENABLE(efl,
              AS_HELP_STRING(--disable-efl, disable EFL GUI toolkit),
              enable_efl=no, enable_efl=yes)
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

if test x$enable_efl = xyes; then
  PKG_CHECK_MODULES(EFL, elementary, AC_DEFINE(EFL, 1, Support for EFL graphical user interface), enable_efl=no)
fi

if test x$enable_glut = xyes; then
  PKG_CHECK_MODULES(GLUT, glut, AC_DEFINE(GLUT, 1, Support for GLUT graphical user interface), enable_glut=no)
fi

if test x$enable_gtk = xyes; then
  PKG_CHECK_MODULES(GTK, gtkgl, AC_DEFINE(GTK, 1, Support for GTK+ graphical user interface), enable_gtk=no)
fi

if test x$enable_qt = xyes; then
  PKG_CHECK_MODULES(QT, QtGui QtOpenGL, AC_DEFINE(QT, 1, Support for Qt graphical user interface), enable_qt=no)
fi

if test x$enable_sdl = xyes; then
  PKG_CHECK_MODULES(SDL, sdl, AC_DEFINE(SDL, 1, Support for SDL graphical user interface), enable_sdl=no)
fi

if test x$enable_efl = xno -a x$enable_glut = xno -a x$enable_gtk = xno -a x$enable_qt = xno -a x$enable_sdl = xno; then
  AC_MSG_WARN(No graphical user interface toolkits found)
fi

AM_CONDITIONAL(HAVE_GUI, test x$enable_efl = xyes ||
                         test x$enable_glut = xyes ||
                         test x$enable_gtk = xyes ||
                         test x$enable_qt = xyes ||
                         test x$enable_sdl = xyes)

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
echo

echo
echo "Toolkits:"
echo
echo "  EFL                              $enable_efl"
echo "  GLUT                             $enable_glut"
echo "  GTK                              $enable_gtk"
echo "  QT                               $enable_qt"
echo "  SDL                              $enable_sdl"
echo

echo
echo "Engines:"
echo
echo "  OpenGL                           $enable_gl"
echo "  OpenGL ES 1.1 CM                 $enable_glesv1_cm"
echo "  OpenGL ES 2.0                    $enable_glesv2"
echo
