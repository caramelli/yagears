AC_INIT(yagears, 0.3)

AC_CONFIG_HEADERS(config.h)

AM_INIT_AUTOMAKE

AC_PROG_CC

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
  PKG_CHECK_MODULES(GL, gl, AC_DEFINE(GL, 1, Support for OpenGL engine), enable_gl=no)
fi

if test x$enable_glesv1_cm = xyes; then
  PKG_CHECK_MODULES(GLESV1_CM, glesv1_cm, AC_DEFINE(GLESV1_CM, 1, Support for OpenGL ES 1.1 CM engine), enable_glesv1_cm=no)
fi

if test x$enable_glesv2 = xyes; then
  PKG_CHECK_MODULES(GLESV2, glesv2, AC_DEFINE(GLESV2, 1, Support for OpenGL ES 2.0 engine), enable_glesv2=no)
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

if test x$enable_gl = xyes; then
  if test x$enable_gl_x11 = xyes; then
    AC_CHECK_LIB(GL, glXCreateContext,
                 AC_DEFINE(GL_X11, 1, Support for OpenGL Extension to Xlib),
                 enable_gl_x11=no)
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
    AC_CHECK_LIB(GL, glFBDevCreateContext,
                 AC_DEFINE(GL_FBDEV, 1, Support for OpenGL Extension to Linux FBDev),
                 enable_gl_fbdev=no)
  fi
else
  enable_gl_x11=no
  enable_gl_directfb=no
  enable_gl_fbdev=no
fi

if test x$enable_egl_x11 = xyes || test x$enable_egl_directfb = xyes || test x$enable_egl_fbdev = xyes; then
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
  else
    enable_egl_x11=no
    enable_egl_directfb=no
    enable_egl_fbdev=no
  fi
fi

if test x$enable_gl_x11 = xno -a x$enable_gl_directfb = xno -a x$enable_gl_fbdev = xno -a \
        x$enable_egl_x11 = xno -a x$enable_egl_directfb = xno -a x$enable_egl_fbdev = xno; then
  AC_MSG_ERROR(No rendering backends found)
fi

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
echo

echo
echo "Engines:"
echo
echo "  OpenGL                           $enable_gl"
echo "  OpenGL ES 1.1 CM                 $enable_glesv1_cm"
echo "  OpenGL ES 2.0                    $enable_glesv2"
echo
