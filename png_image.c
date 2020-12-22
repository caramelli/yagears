/*
  yagears                  Yet Another Gears OpenGL demo
  Copyright (C) 2013-2020  Nicolas Caramelli

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
#include <png.h>
#include "loader.h"

extern struct list loader_list;

/******************************************************************************/

struct handle {
  png_structp png;
  png_infop info;
  FILE *file;
};

static handle_t *png_init(char *filename, int *width, int *height)
{
  handle_t *handle = NULL;

  handle = calloc(1, sizeof(handle_t));
  if (!handle) {
    printf("calloc handle failed\n");
    return NULL;
  }

  handle->png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!handle->png) {
    printf("png_create_read_struct failed\n");
    return NULL;
  }

  handle->info = png_create_info_struct(handle->png);
  if (!handle->info) {
    printf("png_create_info_struct failed\n");
    return NULL;
  }

  handle->file = fopen(filename, "r");
  if (!handle->file) {
    printf("fopen failed\n");
    return NULL;
  }

  png_init_io(handle->png, handle->file);
  png_read_info(handle->png, handle->info);

  *width = png_get_image_width(handle->png, handle->info);
  *height = png_get_image_height(handle->png, handle->info);

  return handle;
}

static void png_read(handle_t *handle, unsigned char *pixel_data)
{
  int i;
  png_bytep byte[png_get_image_height(handle->png, handle->info)];

  for (i = 0; i < png_get_image_height(handle->png, handle->info); i++) {
    byte[i] = pixel_data + i * png_get_image_width(handle->png, handle->info) * 4;
  }

  png_read_image(handle->png, byte);
}

static void png_term(handle_t *handle)
{
  fclose(handle->file);
  png_destroy_read_struct(&handle->png, &handle->info, NULL);
  free(handle);
}

/******************************************************************************/

static loader_t png_loader = {
  "\x89PNG",
  4,
  png_init,
  png_read,
  png_term
};

static void __attribute__((constructor)) loader_ctor()
{
  list_add(&png_loader.entry, &loader_list);
}
