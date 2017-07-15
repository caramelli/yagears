/*
  yagears                  Yet Another Gears OpenGL demo
  Copyright (C) 2013-2017  Nicolas Caramelli

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

struct list {
  struct list *next;
  struct list *prev;
};

#define LIST_INIT(list) { &list, &list }

static inline void list_add(struct list *entry, struct list *list)
{
  entry->next = list->next;
  entry->prev = list;
  list->next->prev = entry;
  list->next = entry;
}

#define LIST_FOR_EACH(entry, list) \
  for (entry = (list)->next; entry != list; entry = entry->next)

#define LIST_ENTRY(entry, type, member) \
  (type *)((char *)entry - (char *)&((type *)NULL)->member);
