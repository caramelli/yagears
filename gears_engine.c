/*
  yagears                  Yet Another Gears OpenGL / Vulkan demo
  Copyright (C) 2013-2024  Nicolas Caramelli

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
#include "gears_engine.h"
#include "engine.h"

struct list engine_list = LIST_INIT(engine_list);

struct gears_engine {
  engine_t *engine;
  gears_t *gears;
};

int gears_engine_nb()
{
  int nb = 0;
  struct list *engine_entry = NULL;

  LIST_FOR_EACH(engine_entry, &engine_list) {
    nb++;
  }

  return nb;
}

char *gears_engine_name(int opt)
{
  int nb = 0;
  engine_t *engine = NULL;
  struct list *engine_entry = NULL;

  LIST_FOR_EACH(engine_entry, &engine_list) {
    engine = LIST_ENTRY(engine_entry, engine_t, entry);
    if (nb == opt) {
      return engine->name;
    }
    nb++;
  }

  return NULL;
}

gears_engine_t *gears_engine_new(char *name)
{
  gears_engine_t *gears_engine = NULL;
  engine_t *engine = NULL;
  struct list *engine_entry = NULL;

  LIST_FOR_EACH(engine_entry, &engine_list) {
    engine = LIST_ENTRY(engine_entry, engine_t, entry);
    if (!strcmp(engine->name, name)) {
      gears_engine = calloc(1, sizeof(gears_engine_t));
      if (!gears_engine) {
        printf("calloc gears_engine failed\n");
        return NULL;
      }
      gears_engine->engine = engine;
      return gears_engine;
    }
  }

  return NULL;
}

int gears_engine_init(gears_engine_t *gears_engine, int width, int height)
{
  if (!gears_engine) {
    return -1;
  }

  gears_engine->gears = gears_engine->engine->init(width, height);
  if (!gears_engine->gears) {
    return -1;
  }

  return 0;
}

int gears_engine_version(gears_engine_t *gears_engine)
{
  if (!gears_engine) {
    return -1;
  }

  return gears_engine->engine->version;
}

void gears_engine_draw(gears_engine_t *gears_engine, float view_tz, float view_rx, float view_ry, float model_rz)
{
  if (!gears_engine) {
    return;
  }

  gears_engine->engine->draw(gears_engine->gears, view_tz, view_rx, view_ry, model_rz);
}

void gears_engine_term(gears_engine_t *gears_engine)
{
  if (!gears_engine) {
    return;
  }

  gears_engine->engine->term(gears_engine->gears);
  gears_engine->gears = NULL;
}

void gears_engine_free(gears_engine_t *gears_engine)
{
  if (!gears_engine) {
    return;
  }

  free(gears_engine);
}
