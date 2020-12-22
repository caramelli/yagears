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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image_loader.h"
#include "loader.h"

#include "tux_image.c"

struct list loader_list = LIST_INIT(loader_list);

static int magic_read(char *filename, char *magic, int magic_size)
{
  FILE *file = NULL;

  file = fopen(filename, "r");
  if (!file) {
    printf("fopen %s failed\n", filename);
    return -1;
  }

  memset(magic, 0, magic_size);
  if (fread(magic, magic_size, 1, file) != 1) {
    printf("fread failed\n");
    return -1;
  }

  fclose(file);

  return 0;
}

static int image_read(char *filename, loader_t *loader, unsigned char *image_data, int *image_width, int *image_height)
{
  handle_t *handle = loader->init(filename, image_width, image_height);
  if (!handle) {
    return -1;
  }
  else if (image_data) {
    loader->read(handle, image_data);
  }
  loader->fini(handle);

  return 0;
}

void image_load(char *filename, unsigned char *image_data, int *image_width, int *image_height)
{
  char magic[4];
  loader_t *loader = NULL;
  struct list *loader_entry = NULL;

  if (filename) {
    if (magic_read(filename, magic, sizeof(magic)) != -1) {
      LIST_FOR_EACH(loader_entry, &loader_list) {
        loader = LIST_ENTRY(loader_entry, loader_t, entry);
        if (!memcmp(magic, loader->magic, loader->magic_size)) {
          break;
        }
        loader = NULL;
      }
    }

    if (loader) {
      if (image_read(filename, loader, image_data, image_width, image_height) == -1) {
        loader = NULL;
      }
    }
  }

  if (!loader) {
    *image_width = tux_image.width;
    *image_height = tux_image.height;
    if (image_data) {
      memcpy(image_data, (unsigned char *)tux_image.pixel_data, tux_image.width * tux_image.height * 4);
    }
  }
}
