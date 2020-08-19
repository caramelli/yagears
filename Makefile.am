BUILT_SOURCES =

if GL
GL_SOURCE = gl_gears.c
endif

if GLESV1_CM
GLESV1_CM_SOURCE = glesv1_cm_gears.c
endif

if GLESV2
vert.xxd: glesv2_gears.vert
	sh $(top_srcdir)/xxd.sh $< $@
frag.xxd: glesv2_gears.frag
	sh $(top_srcdir)/xxd.sh $< $@

BUILT_SOURCES += vert.xxd frag.xxd

GLESV2_SOURCE = glesv2_gears.c
endif

if PNG
PNG_SOURCE = png_image.c
endif

if TIFF
TIFF_SOURCE = tiff_image.c
endif

noinst_LTLIBRARIES     = libyagears2.la
libyagears2_la_SOURCES = gears_engine.c $(GL_SOURCE) $(GLESV1_CM_SOURCE) $(GLESV2_SOURCE) image_loader.c $(PNG_SOURCE) $(TIFF_SOURCE)
libyagears2_la_CFLAGS  = @GL_CFLAGS@ @GLESV1_CM_CFLAGS@ @GLESV2_CFLAGS@ @PNG_CFLAGS@ @TIFF_CFLAGS@
libyagears2_la_LIBADD  = @GL_LIBS@ @GLESV1_CM_LIBS@ @GLESV2_LIBS@ @PNG_LIBS@ @TIFF_LIBS@

bin_PROGRAMS = yagears2
yagears2_SOURCES = main.c
yagears2_CFLAGS  = @EGL_CFLAGS@ @X11_CFLAGS@ @DIRECTFB_CFLAGS@ @WAYLAND_CFLAGS@ @XCB_CFLAGS@ @DRM_CFLAGS@ @RPI_CFLAGS@
yagears2_LDFLAGS = -Wl,--whole-archive,.libs/libyagears2.a,--no-whole-archive
yagears2_LDADD   = libyagears2.la @EGL_LIBS@ @X11_LIBS@ @DIRECTFB_LIBS@ @WAYLAND_LIBS@ @XCB_LIBS@ @DRM_LIBS@ @RPI_LIBS@

if VK
vert.spv: vulkan_gears.vert
	$(GLSLANG_VALIDATOR) -V -x $<
frag.spv: vulkan_gears.frag
	$(GLSLANG_VALIDATOR) -V -x $<

BUILT_SOURCES += vert.spv frag.spv

bin_PROGRAMS       += yagears2-vk
yagears2_vk_SOURCES = vk.c vulkan_gears.c image_loader.c $(PNG_SOURCE) $(TIFF_SOURCE)
yagears2_vk_CFLAGS  = @VULKAN_CFLAGS@ @PNG_CFLAGS@ @TIFF_CFLAGS@ @X11_CFLAGS@ @DIRECTFB_CFLAGS@ @WAYLAND_CFLAGS@ @XCB_CFLAGS@
yagears2_vk_LDADD   = @VULKAN_LIBS@ @PNG_LIBS@ @TIFF_LIBS@ @X11_LIBS@ @DIRECTFB_LIBS@ @WAYLAND_LIBS@ @XCB_LIBS@
endif

if MOSAIC
bin_PROGRAMS           += yagears2-mosaic
yagears2_mosaic_SOURCES = mosaic.c
yagears2_mosaic_CFLAGS  = @GLUT_CFLAGS@
yagears2_mosaic_LDFLAGS = -Wl,--whole-archive,.libs/libyagears2.a,--no-whole-archive
yagears2_mosaic_LDADD   = libyagears2.la @GLUT_LIBS@
endif

if GUI
bin_PROGRAMS         += yagears2-gui
yagears2_gui_SOURCES  = gui.cc
yagears2_gui_CXXFLAGS = @EFL_CFLAGS@ @FLTK_CFLAGS@ @GLFW_CFLAGS@ @GLUT_CFLAGS@ @GTK_CFLAGS@ @QT_CFLAGS@ @SDL_CFLAGS@ @SFML_CFLAGS@ @WX_CFLAGS@
yagears2_gui_LDFLAGS  = -Wl,--whole-archive,.libs/libyagears2.a,--no-whole-archive
yagears2_gui_LDADD    = libyagears2.la @EFL_LIBS@ @FLTK_LIBS@ @GLFW_LIBS@ @GLUT_LIBS@ @GTK_LIBS@ @QT_LIBS@ @SDL_LIBS@ @SFML_LIBS@ @WX_LIBS@
endif
