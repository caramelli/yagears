/*
  yagears                  Yet Another Gears OpenGL / Vulkan demo
  Copyright (C) 2013-2022  Nicolas Caramelli

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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct gears_engine gears_engine_t;

int gears_engine_nb();
char *gears_engine_name(int opt);
gears_engine_t *gears_engine_new(char *name);
int gears_engine_version(gears_engine_t *gears_engine);
int gears_engine_init(gears_engine_t *gears_engine, int width, int height);
void gears_engine_draw(gears_engine_t *gears_engine, float view_tz, float view_rx, float view_ry, float model_rz);
void gears_engine_term(gears_engine_t *gears_engine);
void gears_engine_free(gears_engine_t *gears_engine);

#ifdef __cplusplus
}
#endif
