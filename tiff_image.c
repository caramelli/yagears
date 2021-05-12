/*
  yagears                  Yet Another Gears OpenGL / Vulkan demo
  Copyright (C) 2013-2021  Nicolas Caramelli

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

#include <stdlib.h>
#include <tiffio.h>
#include "loader.h"

extern struct list loader_list;

/******************************************************************************/

struct handle {
  TIFF *tiff;
  int width;
  int height;
};

static handle_t *tiff_init(char *filename, int *width, int *height)
{
  handle_t *handle = NULL;

  handle = calloc(1, sizeof(handle_t));
  if (!handle) {
    printf("calloc handle failed\n");
    return NULL;
  }

  handle->tiff = TIFFOpen(filename, "r");
  if (!handle->tiff) {
    printf("TIFFOpen failed\n");
    return NULL;
  }

  TIFFGetField(handle->tiff, TIFFTAG_IMAGEWIDTH, &handle->width);
  TIFFGetField(handle->tiff, TIFFTAG_IMAGELENGTH, &handle->height);

  *width = handle->width;
  *height = handle->width;

  return handle;
}

static void tiff_read(handle_t *handle, unsigned char *pixel_data)
{
  TIFFReadRGBAImage(handle->tiff, handle->width, handle->height, (unsigned int *)pixel_data, 0);
}

void tiff_term(handle_t *handle)
{
  TIFFClose(handle->tiff);
  free(handle);
}

/******************************************************************************/

static loader_t tiffbe_loader = {
  "MM",
  2,
  tiff_init,
  tiff_read,
  tiff_term
};

static loader_t tiffle_loader = {
  "II",
  2,
  tiff_init,
  tiff_read,
  tiff_term
};

static void __attribute__((constructor)) loader_ctor()
{
  list_add(&tiffbe_loader.entry, &loader_list);
  list_add(&tiffle_loader.entry, &loader_list);
}
