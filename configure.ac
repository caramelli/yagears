AC_INIT(yagears, 0.1)

AM_INIT_AUTOMAKE

AC_PROG_CC

CFLAGS="$CFLAGS -Wall"

PKG_CHECK_MODULES(GL, gl)

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

if test x$enable_gl_x11 = xyes; then
  AC_CHECK_LIB(GL, glXCreateContext, CPPFLAGS="$CPPFLAGS -DGL_X11", enable_gl_x11=no)
fi

if test x$enable_gl_directfb = xyes; then
  AC_MSG_CHECKING(for DirectFBGL module)
  MODULEDIR=`$PKG_CONFIG --variable=moduledir directfb-internal`
  if test -d $MODULEDIR/interfaces/IDirectFBGL; then
    AC_MSG_RESULT(yes)
    PKG_CHECK_MODULES(DIRECTFB, directfb, CPPFLAGS="$CPPFLAGS -DGL_DIRECTFB", enable_gl_directfb=no)
  else
    AC_MSG_RESULT(no)
    enable_gl_directfb=no
  fi
fi

if test x$enable_gl_fbdev = xyes; then
  AC_CHECK_LIB(GL, glFBDevCreateContext, CPPFLAGS="$CPPFLAGS -DGL_FBDEV", enable_gl_fbdev=no)
fi

if test x$enable_egl_x11 = xyes || test x$enable_egl_directfb = xyes || test x$enable_egl_fbdev = xyes; then
  PKG_CHECK_MODULES(EGL, egl, enable_egl=yes, enable_egl=no)
  if test x$enable_egl = xyes; then
    if test x$enable_egl_x11 = xyes; then
      PKG_CHECK_MODULES(X11, x11, CPPFLAGS="$CPPFLAGS -DEGL_X11", enable_egl_x11=no)
    fi
    if test x$enable_egl_directfb = xyes; then
      if test x$enable_gl_directfb = xyes; then
        CPPFLAGS="$CPPFLAGS -DEGL_DIRECTFB"
      else
        PKG_CHECK_MODULES(DIRECTFB, directfb, CPPFLAGS="$CPPFLAGS -DEGL_DIRECTFB", enable_egl_directfb=no)
      fi
    fi
    if test x$enable_egl_fbdev = xyes; then
      CPPFLAGS="$CPPFLAGS -DEGL_FBDEV"
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
