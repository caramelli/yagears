if GL
GL_SOURCE = gl_gears.c
endif
if GLESV1_CM
GLESV1_CM_SOURCE = glesv1_cm_gears.c
endif
if GLESV2
GLESV2_SOURCE = glesv2_gears.c
endif

if PNG
PNG_SOURCE = png_image.c
endif
if TIFF
TIFF_SOURCE = tiff_image.c
endif

noinst_LTLIBRARIES = libyagears.la
libyagears_la_SOURCES = gears_engine.c $(GL_SOURCE) $(GLESV1_CM_SOURCE) $(GLESV2_SOURCE) image_loader.c $(PNG_SOURCE) $(TIFF_SOURCE)
libyagears_la_CFLAGS = @GL_CFLAGS@ @GLESV1_CM_CFLAGS@ @GLESV2_CFLAGS@ @PNG_CFLAGS@ @TIFF_CFLAGS@
libyagears_la_LIBADD = @GL_LIBS@ @GLESV1_CM_LIBS@ @GLESV2_LIBS@ @PNG_LIBS@ @TIFF_LIBS@

bin_PROGRAMS = yagears
yagears_SOURCES = main.c
yagears_CFLAGS = @EGL_CFLAGS@ @X11_CFLAGS@ @DIRECTFB_CFLAGS@ @WAYLAND_CFLAGS@ @DRM_CFLAGS@ @RPI_CFLAGS@
yagears_LDFLAGS = -Wl,--whole-archive,.libs/libyagears.a,--no-whole-archive
yagears_LDADD = libyagears.la @EGL_LIBS@ @X11_LIBS@ @DIRECTFB_LIBS@ @WAYLAND_LIBS@ @DRM_LIBS@ @RPI_LIBS@

if HAVE_GLUT
bin_PROGRAMS += yagears-mosaic
yagears_mosaic_SOURCES = mosaic.c
yagears_mosaic_CFLAGS = @GLUT_CFLAGS@
yagears_mosaic_LDFLAGS = -Wl,--whole-archive,.libs/libyagears.a,--no-whole-archive
yagears_mosaic_LDADD = libyagears.la @GLUT_LIBS@
endif

if HAVE_GUI
bin_PROGRAMS += yagears-gui
yagears_gui_SOURCES = gui.cc
yagears_gui_CXXFLAGS = @EFL_CFLAGS@ @FLTK_CFLAGS@ @GLFW_CFLAGS@ @GLUT_CFLAGS@ @GTK_CFLAGS@ @QT_CFLAGS@ @SDL_CFLAGS@ @SFML_CFLAGS@ @WX_CFLAGS@
yagears_gui_LDFLAGS = -Wl,--whole-archive,.libs/libyagears.a,--no-whole-archive
yagears_gui_LDADD = libyagears.la @EFL_LIBS@ @FLTK_LIBS@ @GLFW_LIBS@ @GLUT_LIBS@ @GTK_LIBS@ @QT_LIBS@ @SDL_LIBS@ @SFML_LIBS@ @WX_LIBS@
endif
