bin_PROGRAMS = yagears
yagears_SOURCES = main.c
if GL
yagears_SOURCES += gl_gears.c
endif
if GLESV1_CM
yagears_SOURCES += glesv1_cm_gears.c
endif
if GLESV2
yagears_SOURCES += glesv2_gears.c
endif
INCLUDES = @GL_CFLAGS@ @GLESV1_CM_CFLAGS@ @GLESV2_CFLAGS@ @EGL_CFLAGS@ @X11_CFLAGS@ @DIRECTFB_CFLAGS@
yagears_LDADD = @GL_LIBS@ @GLESV1_CM_LIBS@ @GLESV2_LIBS@ @EGL_LIBS@ @X11_LIBS@ @DIRECTFB_LIBS@
