#ifndef MINPDF_H
#define MINPDF_H

/* minpdf - a zero-runtime-dependency PDF writer (C11, MIT) */
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MINPDF_VERSION_MAJOR 0
#define MINPDF_VERSION_MINOR 1
#define MINPDF_VERSION_PATCH 0

typedef struct minpdf_document minpdf_document;

typedef enum minpdf_status {
  MINPDF_OK = 0,
  MINPDF_ERROR_ARGUMENT,
  MINPDF_ERROR_MEMORY,
  MINPDF_ERROR_FORMAT,
  MINPDF_ERROR_UNSUPPORTED,
  MINPDF_ERROR_IO,
  MINPDF_ERROR_STATE
} minpdf_status;

typedef enum minpdf_align {
  MINPDF_ALIGN_LEFT,
  MINPDF_ALIGN_CENTER,
  MINPDF_ALIGN_RIGHT
} minpdf_align;
typedef enum minpdf_weight {
  MINPDF_WEIGHT_NORMAL,
  MINPDF_WEIGHT_BOLD,
  MINPDF_WEIGHT_ITALIC,
  MINPDF_WEIGHT_BOLD_ITALIC
} minpdf_weight;

typedef struct minpdf_size {
  float width, height;
} minpdf_size;
typedef struct minpdf_color {
  float r, g, b;
} minpdf_color;
typedef struct minpdf_buffer {
  unsigned char *data;
  size_t length;
} minpdf_buffer;
typedef struct minpdf_text_options {
  float size;
  minpdf_color color;
  minpdf_align align;
  minpdf_weight weight;
  float max_width;
  const char *font;
} minpdf_text_options;
typedef struct minpdf_rect_options {
  minpdf_color fill, stroke;
  float line_width, radius;
  int has_fill, has_stroke;
} minpdf_rect_options;
typedef struct minpdf_line_options {
  minpdf_color color;
  float width;
  const float *dash;
  size_t dash_count;
} minpdf_line_options;
typedef struct minpdf_image_options {
  float width, height;
} minpdf_image_options;
typedef struct minpdf_link_options {
  minpdf_color color;
  int underline;
  float size;
} minpdf_link_options;
typedef struct minpdf_table_column {
  const char *header;
  float width;
  minpdf_align align;
} minpdf_table_column;
typedef struct minpdf_table_options {
  const minpdf_table_column *columns;
  size_t column_count;
  minpdf_color header_background, header_color, border_color;
  float font_size, padding;
} minpdf_table_options;
typedef struct minpdf_metadata {
  const char *title, *author, *subject, *keywords, *creator, *creation_date;
} minpdf_metadata;

extern const minpdf_size MINPDF_A3, MINPDF_A4, MINPDF_A5, MINPDF_LETTER;
minpdf_color minpdf_rgb(unsigned char r, unsigned char g, unsigned char b);
minpdf_text_options minpdf_text_defaults(void);
minpdf_rect_options minpdf_rect_defaults(void);
minpdf_line_options minpdf_line_defaults(void);
minpdf_image_options minpdf_image_defaults(void);
minpdf_link_options minpdf_link_defaults(void);
minpdf_table_options minpdf_table_defaults(const minpdf_table_column *columns,
                                           size_t count);

minpdf_document *minpdf_create(minpdf_size default_size);
void minpdf_destroy(minpdf_document *document);
const char *minpdf_error(const minpdf_document *document);
minpdf_status minpdf_set_metadata(minpdf_document *document,
                                  const minpdf_metadata *metadata);
minpdf_status minpdf_add_page(minpdf_document *document, minpdf_size size);
minpdf_status minpdf_register_font(minpdf_document *document, const char *name,
                                   const void *data, size_t length);
minpdf_status minpdf_text(minpdf_document *document, const char *utf8, float x,
                          float y, const minpdf_text_options *options);
minpdf_status minpdf_rect(minpdf_document *document, float x, float y,
                          float width, float height,
                          const minpdf_rect_options *options);
minpdf_status minpdf_line(minpdf_document *document, float x1, float y1,
                          float x2, float y2,
                          const minpdf_line_options *options);
minpdf_status minpdf_circle(minpdf_document *document, float cx, float cy,
                            float radius, const minpdf_rect_options *options);
minpdf_status minpdf_image(minpdf_document *document, const void *data,
                           size_t length, float x, float y,
                           const minpdf_image_options *options);
minpdf_status minpdf_link(minpdf_document *document, const char *label,
                          const char *url, float x, float y,
                          const minpdf_link_options *options);
minpdf_status minpdf_table(minpdf_document *document, const char *const *cells,
                           size_t rows, float x, float y,
                           const minpdf_table_options *options);
minpdf_status minpdf_build(minpdf_document *document, minpdf_buffer *output);
minpdf_status minpdf_save(minpdf_document *document, const char *path);
void minpdf_buffer_free(minpdf_buffer *buffer);

#ifdef __cplusplus
}
#endif
#endif

#ifdef MINPDF_IMPLEMENTATION
#ifndef MINPDF_IMPLEMENTATION_ONCE
#define MINPDF_IMPLEMENTATION_ONCE

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Override these before defining MINPDF_IMPLEMENTATION to instrument or
   replace the engine allocator. All three functions must use one allocator. */
#ifndef MINPDF_MALLOC
#define MINPDF_MALLOC malloc
#endif
#ifndef MINPDF_REALLOC
#define MINPDF_REALLOC realloc
#endif
#ifndef MINPDF_CALLOC
#define MINPDF_CALLOC calloc
#endif
#ifndef MINPDF_FREE
#define MINPDF_FREE free
#endif
#define malloc MINPDF_MALLOC
#define realloc MINPDF_REALLOC
#define calloc MINPDF_CALLOC
#define free MINPDF_FREE

/* The public wrapper is emitted after the single-line text primitive. */
#define minpdf_text mp_text_line
#define minpdf_rect mp_rect_basic

const minpdf_size MINPDF_A3 = {842, 1191}, MINPDF_A4 = {595, 842},
                  MINPDF_A5 = {420, 595}, MINPDF_LETTER = {612, 792};

typedef struct mp_buf {
  unsigned char *p;
  size_t n, cap;
} mp_buf;
typedef struct mp_image {
  unsigned char *data, *alpha;
  size_t length, alpha_length;
  unsigned w, h;
  float x, y, dw, dh;
  int jpeg, colors;
  unsigned object_id;
  char name[20];
} mp_image;
typedef struct mp_link {
  char *url;
  float x, y, w, h;
  unsigned object_id;
} mp_link;
typedef struct mp_page {
  float w, h;
  mp_buf content;
  mp_image *images;
  size_t image_count, image_cap;
  mp_link *links;
  size_t link_count, link_cap;
  unsigned fonts;
  uint64_t custom_fonts;
  unsigned content_id;
} mp_page;
typedef struct mp_glyph_use {
  uint16_t glyph;
  uint32_t unicode;
} mp_glyph_use;
typedef struct mp_font {
  char *name;
  unsigned char *data;
  size_t length;
  uint32_t cmap, hmtx;
  uint16_t cmap_format, units, ascent, num_glyphs, num_hmetrics;
  int16_t descent, bbox[4];
  mp_glyph_use *used;
  size_t used_count, used_cap;
  unsigned object_id;
} mp_font;
struct minpdf_document {
  minpdf_size default_size;
  mp_page *pages;
  size_t page_count, page_cap;
  mp_font *fonts;
  size_t font_count, font_cap;
  minpdf_metadata meta;
  char *meta_values[6];
  char error[192];
};

static int mp_grow(mp_buf *b, size_t add) {
  size_t need = b->n + add, cap;
  unsigned char *p;
  if (need <= b->cap)
    return 1;
  cap = b->cap ? b->cap : 256;
  while (cap < need) {
    if (cap > SIZE_MAX / 2)
      return 0;
    cap *= 2;
  }
  p = (unsigned char *)realloc(b->p, cap);
  if (!p)
    return 0;
  b->p = p;
  b->cap = cap;
  return 1;
}
static int mp_put(mp_buf *b, const void *p, size_t n) {
  if (!mp_grow(b, n))
    return 0;
  memcpy(b->p + b->n, p, n);
  b->n += n;
  return 1;
}
static int mp_printf(mp_buf *b, const char *fmt, ...) {
  va_list ap;
  int n;
  size_t avail;
  if (!mp_grow(b, 128))
    return 0;
  avail = b->cap - b->n;
  va_start(ap, fmt);
  n = vsnprintf((char *)b->p + b->n, avail, fmt, ap);
  va_end(ap);
  if (n < 0)
    return 0;
  if ((size_t)n >= avail) {
    if (!mp_grow(b, (size_t)n + 1))
      return 0;
    va_start(ap, fmt);
    vsnprintf((char *)b->p + b->n, (size_t)n + 1, fmt, ap);
    va_end(ap);
  }
  b->n += (size_t)n;
  return 1;
}
static char *mp_strdup(const char *s) {
  size_t n;
  if (!s)
    return NULL;
  n = strlen(s) + 1;
  char *p = (char *)malloc(n);
  if (p)
    memcpy(p, s, n);
  return p;
}
static minpdf_status mp_fail(minpdf_document *d, minpdf_status s,
                             const char *m) {
  if (d) {
    snprintf(d->error, sizeof d->error, "%s", m ? m : "error");
  }
  return s;
}
static mp_page *mp_page_now(minpdf_document *d) {
  if (!d)
    return NULL;
  if (!d->page_count && minpdf_add_page(d, d->default_size) != MINPDF_OK)
    return NULL;
  return &d->pages[d->page_count - 1];
}
static int mp_color(mp_buf *b, minpdf_color c, int stroke) {
  return mp_printf(b, "%.3f %.3f %.3f %s\n", c.r, c.g, c.b,
                   stroke ? "RG" : "rg");
}
static size_t mp_utf8_count(const char *s) {
  size_t n = 0;
  while (s && *s) {
    unsigned char c = (unsigned char)*s;
    n++;
    s += (c < 0x80) ? 1 : (c < 0xe0 ? 2 : (c < 0xf0 ? 3 : 4));
  }
  return n;
}
static float mp_measure(const char *s, float z) {
  return (float)mp_utf8_count(s) * z * .52f;
}
static int mp_pdf_string(mp_buf *b, const char *s) {
  const unsigned char *p = (const unsigned char *)s;
  if (!mp_put(b, "(", 1))
    return 0;
  for (; *p; p++) {
    unsigned char c = *p;
    if (c == '(' || c == ')' || c == '\\') {
      if (!mp_put(b, "\\", 1))
        return 0;
    }
    if (c < 32 || c > 126) {
      if (!mp_printf(b, "\\%03o", c))
        return 0;
    } else if (!mp_put(b, p, 1))
      return 0;
  }
  return mp_put(b, ")", 1);
}
static uint16_t mp_u16(const unsigned char *p) {
  return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}
static int16_t mp_i16(const unsigned char *p) { return (int16_t)mp_u16(p); }
static uint32_t mp_u32(const unsigned char *p) {
  return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
         ((uint32_t)p[2] << 8) | p[3];
}
static int mp_font_table(mp_font *f, const char tag[4], uint32_t *off,
                         uint32_t *len) {
  uint16_t n;
  size_t i;
  if (f->length < 12)
    return 0;
  n = mp_u16(f->data + 4);
  if (12 + (size_t)n * 16 > f->length)
    return 0;
  for (i = 0; i < n; i++) {
    const unsigned char *r = f->data + 12 + i * 16;
    if (!memcmp(r, tag, 4)) {
      *off = mp_u32(r + 8);
      *len = mp_u32(r + 12);
      return (size_t)*off + *len <= f->length;
    }
  }
  return 0;
}
static int mp_parse_font(mp_font *f) {
  uint32_t head, headn, hhea, hhean, maxp, maxpn, cmap, cmapn, hmtx, hmtxn;
  uint16_t count, i;
  uint32_t chosen = 0;
  if (!mp_font_table(f, "head", &head, &headn) || headn < 54 ||
      !mp_font_table(f, "hhea", &hhea, &hhean) || hhean < 36 ||
      !mp_font_table(f, "maxp", &maxp, &maxpn) || maxpn < 6 ||
      !mp_font_table(f, "cmap", &cmap, &cmapn) || cmapn < 12 ||
      !mp_font_table(f, "hmtx", &hmtx, &hmtxn))
    return 0;
  f->units = mp_u16(f->data + head + 18);
  f->bbox[0] = mp_i16(f->data + head + 36);
  f->bbox[1] = mp_i16(f->data + head + 38);
  f->bbox[2] = mp_i16(f->data + head + 40);
  f->bbox[3] = mp_i16(f->data + head + 42);
  f->ascent = mp_u16(f->data + hhea + 4);
  f->descent = mp_i16(f->data + hhea + 6);
  f->num_hmetrics = mp_u16(f->data + hhea + 34);
  f->num_glyphs = mp_u16(f->data + maxp + 4);
  f->hmtx = hmtx;
  count = mp_u16(f->data + cmap + 2);
  if (4 + (size_t)count * 8 > cmapn)
    return 0;
  for (i = 0; i < count; i++) {
    const unsigned char *r = f->data + cmap + 4 + i * 8;
    uint16_t platform = mp_u16(r), encoding = mp_u16(r + 2);
    uint32_t sub = mp_u32(r + 4);
    if (sub + 2 > cmapn)
      continue;
    uint16_t format = mp_u16(f->data + cmap + sub);
    if ((platform == 3 && (encoding == 10 || encoding == 1)) || platform == 0) {
      if (format == 12) {
        chosen = cmap + sub;
        f->cmap_format = 12;
        break;
      }
      if (format == 4 && !chosen) {
        chosen = cmap + sub;
        f->cmap_format = 4;
      }
    }
  }
  f->cmap = chosen;
  return f->units && f->cmap && f->num_hmetrics &&
         f->hmtx + (size_t)f->num_hmetrics * 4 <= f->length;
}
static uint16_t mp_glyph(mp_font *f, uint32_t cp) {
  const unsigned char *d = f->data + f->cmap;
  if (f->cmap_format == 12) {
    uint32_t groups = mp_u32(d + 12), i;
    if (f->cmap + 16 + (size_t)groups * 12 > f->length)
      return 0;
    for (i = 0; i < groups; i++) {
      uint32_t a = mp_u32(d + 16 + i * 12), z = mp_u32(d + 20 + i * 12);
      if (cp >= a && cp <= z)
        return (uint16_t)(mp_u32(d + 24 + i * 12) + (cp - a));
    }
    return 0;
  } else {
    uint16_t segs = mp_u16(d + 6) / 2, i;
    const unsigned char *end = d + 14;
    const unsigned char *start = end + 2 * segs + 2;
    const unsigned char *delta = start + 2 * segs;
    const unsigned char *range = delta + 2 * segs;
    if (cp > 65535 || range + 2 * segs > f->data + f->length)
      return 0;
    for (i = 0; i < segs; i++)
      if (cp <= mp_u16(end + 2 * i)) {
        uint16_t s = mp_u16(start + 2 * i), ro = mp_u16(range + 2 * i), g;
        if (cp < s)
          return 0;
        if (!ro)
          return (uint16_t)(cp + mp_u16(delta + 2 * i));
        const unsigned char *q = range + 2 * i + ro + 2 * (cp - s);
        if (q + 2 > f->data + f->length)
          return 0;
        g = mp_u16(q);
        return g ? (uint16_t)(g + mp_u16(delta + 2 * i)) : 0;
      }
    return 0;
  }
}
static uint16_t mp_advance(mp_font *f, uint16_t glyph) {
  uint16_t i =
      glyph < f->num_hmetrics ? glyph : (uint16_t)(f->num_hmetrics - 1);
  return mp_u16(f->data + f->hmtx + 4 * (size_t)i);
}
static int mp_use_glyph(mp_font *f, uint16_t glyph, uint32_t cp) {
  size_t i;
  for (i = 0; i < f->used_count; i++)
    if (f->used[i].glyph == glyph && f->used[i].unicode == cp)
      return 1;
  if (f->used_count == f->used_cap) {
    size_t c = f->used_cap ? f->used_cap * 2 : 32;
    void *q = realloc(f->used, c * sizeof *f->used);
    if (!q)
      return 0;
    f->used = (mp_glyph_use *)q;
    f->used_cap = c;
  }
  f->used[f->used_count++] = (mp_glyph_use){glyph, cp};
  return 1;
}
static int mp_utf8_next(const char **sp, uint32_t *cp) {
  const unsigned char *s = (const unsigned char *)*sp, c = *s;
  if (!c)
    return 0;
  if (c < 128) {
    *cp = c;
    *sp += 1;
    return 1;
  }
  if (c >= 194 && c < 224 && (s[1] & 192) == 128) {
    *cp = ((c & 31) << 6) | (s[1] & 63);
    *sp += 2;
    return 1;
  }
  if (c >= 224 && c < 240 && (s[1] & 192) == 128 && (s[2] & 192) == 128) {
    *cp = ((c & 15) << 12) | ((s[1] & 63) << 6) | (s[2] & 63);
    if (*cp >= 0xd800 && *cp <= 0xdfff)
      return -1;
    *sp += 3;
    return 1;
  }
  if (c >= 240 && c < 245 && (s[1] & 192) == 128 && (s[2] & 192) == 128 &&
      (s[3] & 192) == 128) {
    *cp = ((c & 7) << 18) | ((s[1] & 63) << 12) | ((s[2] & 63) << 6) |
          (s[3] & 63);
    if (*cp > 0x10ffff)
      return -1;
    *sp += 4;
    return 1;
  }
  return -1;
}

minpdf_color minpdf_rgb(unsigned char r, unsigned char g, unsigned char b) {
  minpdf_color c = {(float)r / 255, (float)g / 255, (float)b / 255};
  return c;
}
minpdf_text_options minpdf_text_defaults(void) {
  minpdf_text_options o = {
      12, {0, 0, 0}, MINPDF_ALIGN_LEFT, MINPDF_WEIGHT_NORMAL, 0, NULL};
  return o;
}
minpdf_rect_options minpdf_rect_defaults(void) {
  minpdf_rect_options o = {{0, 0, 0}, {0, 0, 0}, 1, 0, 0, 0};
  return o;
}
minpdf_line_options minpdf_line_defaults(void) {
  minpdf_line_options o = {{0, 0, 0}, 1, NULL, 0};
  return o;
}
minpdf_image_options minpdf_image_defaults(void) {
  minpdf_image_options o = {0, 0};
  return o;
}
minpdf_link_options minpdf_link_defaults(void) {
  minpdf_link_options o = {{0, 0, .933f}, 1, 12};
  return o;
}
minpdf_table_options minpdf_table_defaults(const minpdf_table_column *c,
                                           size_t n) {
  minpdf_table_options o = {
      c, n, {.94f, .94f, .94f}, {0, 0, 0}, {.8f, .8f, .8f}, 10, 8};
  return o;
}

minpdf_document *minpdf_create(minpdf_size s) {
  minpdf_document *d;
  if (s.width <= 0 || s.height <= 0)
    return NULL;
  d = (minpdf_document *)calloc(1, sizeof *d);
  if (!d)
    return NULL;
  d->default_size = s;
  if (minpdf_add_page(d, s) != MINPDF_OK) {
    minpdf_destroy(d);
    return NULL;
  }
  return d;
}
void minpdf_destroy(minpdf_document *d) {
  size_t i, j;
  if (!d)
    return;
  for (i = 0; i < d->page_count; i++) {
    mp_page *p = &d->pages[i];
    free(p->content.p);
    for (j = 0; j < p->image_count; j++) {
      free(p->images[j].data);
      free(p->images[j].alpha);
    }
    for (j = 0; j < p->link_count; j++)
      free(p->links[j].url);
    free(p->images);
    free(p->links);
  }
  for (i = 0; i < d->font_count; i++) {
    free(d->fonts[i].name);
    free(d->fonts[i].data);
    free(d->fonts[i].used);
  }
  for (i = 0; i < 6; i++)
    free(d->meta_values[i]);
  free(d->fonts);
  free(d->pages);
  free(d);
}
const char *minpdf_error(const minpdf_document *d) {
  return d ? d->error : "invalid document";
}
minpdf_status minpdf_add_page(minpdf_document *d, minpdf_size s) {
  mp_page *p;
  if (!d || s.width <= 0 || s.height <= 0)
    return mp_fail(d, MINPDF_ERROR_ARGUMENT, "invalid page size");
  if (d->page_count == d->page_cap) {
    size_t c = d->page_cap ? d->page_cap * 2 : 4;
    void *q = realloc(d->pages, c * sizeof *d->pages);
    if (!q)
      return mp_fail(d, MINPDF_ERROR_MEMORY, "out of memory");
    d->pages = (mp_page *)q;
    d->page_cap = c;
  }
  p = &d->pages[d->page_count++];
  memset(p, 0, sizeof *p);
  p->w = s.width;
  p->h = s.height;
  return MINPDF_OK;
}
minpdf_status minpdf_set_metadata(minpdf_document *d,
                                  const minpdf_metadata *m) {
  const char *in[6];
  char **out;
  size_t i;
  if (!d || !m)
    return mp_fail(d, MINPDF_ERROR_ARGUMENT, "invalid metadata");
  in[0] = m->title;
  in[1] = m->author;
  in[2] = m->subject;
  in[3] = m->keywords;
  in[4] = m->creator;
  in[5] = m->creation_date;
  for (i = 0; i < 6; i++) {
    free(d->meta_values[i]);
    d->meta_values[i] = in[i] ? mp_strdup(in[i]) : NULL;
    if (in[i] && !d->meta_values[i])
      return mp_fail(d, MINPDF_ERROR_MEMORY, "out of memory");
  }
  out = d->meta_values;
  d->meta.title = out[0];
  d->meta.author = out[1];
  d->meta.subject = out[2];
  d->meta.keywords = out[3];
  d->meta.creator = out[4];
  d->meta.creation_date = out[5];
  return MINPDF_OK;
}
minpdf_status minpdf_register_font(minpdf_document *d, const char *name,
                                   const void *data, size_t n) {
  mp_font *f;
  void *q;
  size_t i;
  if (!d || !name || !*name || !data || n < 12)
    return mp_fail(d, MINPDF_ERROR_ARGUMENT, "invalid font");
  if (d->font_count >= 64)
    return mp_fail(d, MINPDF_ERROR_UNSUPPORTED,
                   "at most 64 custom fonts are supported");
  for (i = 0; i < d->font_count; i++)
    if (!strcmp(d->fonts[i].name, name))
      return mp_fail(d, MINPDF_ERROR_ARGUMENT, "font name already registered");
  if (d->font_count == d->font_cap) {
    size_t c = d->font_cap ? d->font_cap * 2 : 4;
    q = realloc(d->fonts, c * sizeof *d->fonts);
    if (!q)
      return mp_fail(d, MINPDF_ERROR_MEMORY, "out of memory");
    d->fonts = (mp_font *)q;
    d->font_cap = c;
  }
  f = &d->fonts[d->font_count];
  memset(f, 0, sizeof *f);
  f->name = mp_strdup(name);
  f->data = (unsigned char *)malloc(n);
  if (!f->name || !f->data) {
    free(f->name);
    free(f->data);
    return mp_fail(d, MINPDF_ERROR_MEMORY, "out of memory");
  }
  memcpy(f->data, data, n);
  f->length = n;
  if (!mp_parse_font(f)) {
    free(f->name);
    free(f->data);
    memset(f, 0, sizeof *f);
    return mp_fail(d, MINPDF_ERROR_FORMAT,
                   "unsupported or malformed TrueType font");
  }
  d->font_count++;
  return MINPDF_OK;
}

minpdf_status minpdf_text(minpdf_document *d, const char *t, float x, float y,
                          const minpdf_text_options *in) {
  mp_page *p;
  minpdf_text_options o;
  const char *font;
  float tx, width;
  size_t fi;
  if (!d || !t)
    return mp_fail(d, MINPDF_ERROR_ARGUMENT, "invalid text");
  p = mp_page_now(d);
  if (!p)
    return MINPDF_ERROR_MEMORY;
  o = in ? *in : minpdf_text_defaults();
  if (o.size <= 0)
    o.size = 12;
  if (o.font && *o.font) {
    mp_font *f = NULL;
    mp_buf encoded = {0};
    const char *s = t;
    uint32_t cp;
    for (fi = 0; fi < d->font_count; fi++)
      if (!strcmp(d->fonts[fi].name, o.font)) {
        f = &d->fonts[fi];
        break;
      }
    if (!f)
      return mp_fail(d, MINPDF_ERROR_ARGUMENT, "font is not registered");
    width = 0;
    while (*s) {
      int ok = mp_utf8_next(&s, &cp);
      uint16_t g;
      if (ok < 0) {
        free(encoded.p);
        return mp_fail(d, MINPDF_ERROR_FORMAT, "invalid UTF-8 text");
      }
      g = mp_glyph(f, cp);
      if (!g && cp) {
        free(encoded.p);
        return mp_fail(d, MINPDF_ERROR_UNSUPPORTED,
                       "font does not contain a requested glyph");
      }
      if (!mp_use_glyph(f, g, cp) || !mp_printf(&encoded, "%04X", g)) {
        free(encoded.p);
        return mp_fail(d, MINPDF_ERROR_MEMORY, "out of memory");
      }
      width += (float)mp_advance(f, g) * o.size / f->units;
    }
    p->custom_fonts |= (uint64_t)1 << fi;
    tx = x;
    if (o.align == MINPDF_ALIGN_CENTER)
      tx -= width / 2;
    else if (o.align == MINPDF_ALIGN_RIGHT)
      tx -= width;
    if (!mp_put(&p->content, "q\nBT\n", 5) ||
        !mp_color(&p->content, o.color, 0) ||
        !mp_printf(&p->content, "/FC%zu %.2f Tf\n%.2f %.2f Td\n<", fi, o.size,
                   tx, p->h - y) ||
        !mp_put(&p->content, encoded.p, encoded.n) ||
        !mp_put(&p->content, "> Tj\nET\nQ\n", 10)) {
      free(encoded.p);
      return mp_fail(d, MINPDF_ERROR_MEMORY, "out of memory");
    }
    free(encoded.p);
    return MINPDF_OK;
  }
  font = o.weight == MINPDF_WEIGHT_BOLD          ? "Helvetica-Bold"
         : o.weight == MINPDF_WEIGHT_ITALIC      ? "Helvetica-Oblique"
         : o.weight == MINPDF_WEIGHT_BOLD_ITALIC ? "Helvetica-BoldOblique"
                                                 : "Helvetica";
  for (const unsigned char *s = (const unsigned char *)t; *s; s++)
    if (*s >= 128)
      return mp_fail(d, MINPDF_ERROR_UNSUPPORTED,
                     "non-ASCII text requires a registered TrueType font");
  p->fonts |= 1u << o.weight;
  width = mp_measure(t, o.size);
  tx = x;
  if (o.align == MINPDF_ALIGN_CENTER)
    tx -= width / 2;
  else if (o.align == MINPDF_ALIGN_RIGHT)
    tx -= width;
  if (!mp_put(&p->content, "q\nBT\n", 5) ||
      !mp_color(&p->content, o.color, 0) ||
      !mp_printf(&p->content, "/%s %.2f Tf\n%.2f %.2f Td\n", font, o.size, tx,
                 p->h - y) ||
      !mp_pdf_string(&p->content, t) || !mp_put(&p->content, " Tj\nET\nQ\n", 9))
    return mp_fail(d, MINPDF_ERROR_MEMORY, "out of memory");
  return MINPDF_OK;
}
minpdf_status minpdf_rect(minpdf_document *d, float x, float y, float w,
                          float h, const minpdf_rect_options *in) {
  mp_page *p;
  minpdf_rect_options o;
  if (!d || w < 0 || h < 0)
    return mp_fail(d, MINPDF_ERROR_ARGUMENT, "invalid rectangle");
  p = mp_page_now(d);
  o = in ? *in : minpdf_rect_defaults();
  if (!mp_put(&p->content, "q\n", 2))
    goto oom;
  if (o.has_fill && !mp_color(&p->content, o.fill, 0))
    goto oom;
  if (o.has_stroke && !mp_color(&p->content, o.stroke, 1))
    goto oom;
  if (!mp_printf(&p->content, "%.2f w\n%.2f %.2f %.2f %.2f re\n%s\nQ\n",
                 o.line_width > 0 ? o.line_width : 1, x, p->h - y - h, w, h,
                 o.has_fill ? (o.has_stroke ? "B" : "f")
                            : (o.has_stroke ? "S" : "n")))
    goto oom;
  return MINPDF_OK;
oom:
  return mp_fail(d, MINPDF_ERROR_MEMORY, "out of memory");
}
minpdf_status minpdf_line(minpdf_document *d, float x1, float y1, float x2,
                          float y2, const minpdf_line_options *in) {
  mp_page *p;
  minpdf_line_options o;
  size_t i;
  if (!d)
    return MINPDF_ERROR_ARGUMENT;
  p = mp_page_now(d);
  o = in ? *in : minpdf_line_defaults();
  if (!mp_put(&p->content, "q\n", 2) || !mp_color(&p->content, o.color, 1) ||
      !mp_printf(&p->content, "%.2f w\n", o.width > 0 ? o.width : 1))
    goto oom;
  if (o.dash_count) {
    if (!mp_put(&p->content, "[", 1))
      goto oom;
    for (i = 0; i < o.dash_count; i++)
      if (!mp_printf(&p->content, "%.2f ", o.dash[i]))
        goto oom;
    if (!mp_put(&p->content, "] 0 d\n", 6))
      goto oom;
  }
  if (!mp_printf(&p->content, "%.2f %.2f m\n%.2f %.2f l\nS\nQ\n", x1, p->h - y1,
                 x2, p->h - y2))
    goto oom;
  return MINPDF_OK;
oom:
  return mp_fail(d, MINPDF_ERROR_MEMORY, "out of memory");
}
minpdf_status minpdf_circle(minpdf_document *d, float cx, float cy, float r,
                            const minpdf_rect_options *in) {
  mp_page *p;
  minpdf_rect_options o;
  float y, k = .55228475f;
  if (!d || r < 0)
    return mp_fail(d, MINPDF_ERROR_ARGUMENT, "invalid circle");
  p = mp_page_now(d);
  o = in ? *in : minpdf_rect_defaults();
  y = p->h - cy;
  if (!mp_put(&p->content, "q\n", 2))
    goto oom;
  if (o.has_fill && !mp_color(&p->content, o.fill, 0))
    goto oom;
  if (o.has_stroke && !mp_color(&p->content, o.stroke, 1))
    goto oom;
  if (!mp_printf(
          &p->content,
          "%.2f w\n%.2f %.2f m\n%.2f %.2f %.2f %.2f %.2f %.2f c\n%.2f %.2f "
          "%.2f %.2f %.2f %.2f c\n%.2f %.2f %.2f %.2f %.2f %.2f c\n%.2f %.2f "
          "%.2f %.2f %.2f %.2f c\n%s\nQ\n",
          o.line_width > 0 ? o.line_width : 1, cx + r, y, cx + r, y + r * k,
          cx + r * k, y + r, cx, y + r, cx - r * k, y + r, cx - r, y + r * k,
          cx - r, y, cx - r, y - r * k, cx - r * k, y - r, cx, y - r,
          cx + r * k, y - r, cx + r, y - r * k, cx + r, y,
          o.has_fill ? (o.has_stroke ? "B" : "f") : (o.has_stroke ? "S" : "n")))
    goto oom;
  return MINPDF_OK;
oom:
  return mp_fail(d, MINPDF_ERROR_MEMORY, "out of memory");
}

/* JPEG dimensions; PNG decoding is added below through its native zlib stream.
 */
static int mp_jpeg_size(const unsigned char *d, size_t n, unsigned *w,
                        unsigned *h) {
  size_t p = 2;
  if (n < 4 || d[0] != 255 || d[1] != 216)
    return 0;
  while (p + 9 < n) {
    unsigned m, l;
    if (d[p++] != 255)
      continue;
    while (p < n && d[p] == 255)
      p++;
    if (p >= n)
      return 0;
    m = d[p++];
    if (m == 217 || m == 218)
      break;
    if (p + 2 > n)
      return 0;
    l = ((unsigned)d[p] << 8) | d[p + 1];
    if (l < 2 || p + l > n)
      return 0;
    if (m >= 192 && m <= 207 && m != 196 && m != 200 && m != 204) {
      *h = ((unsigned)d[p + 3] << 8) | d[p + 4];
      *w = ((unsigned)d[p + 5] << 8) | d[p + 6];
      return *w && *h;
    }
    p += l;
  }
  return 0;
}
static uint32_t mp_be32(const unsigned char *p) {
  return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
         ((uint32_t)p[2] << 8) | p[3];
}
typedef struct mp_bits {
  const unsigned char *p;
  size_t n, pos;
  uint32_t bits;
  unsigned count;
} mp_bits;
typedef struct mp_huff {
  uint16_t code[320], symbol[320];
  unsigned char length[320];
  unsigned count;
} mp_huff;
static int mp_bits_read(mp_bits *b, unsigned n, uint32_t *out) {
  while (b->count < n) {
    if (b->pos >= b->n)
      return 0;
    b->bits |= (uint32_t)b->p[b->pos++] << b->count;
    b->count += 8;
  }
  *out = b->bits & ((1u << n) - 1);
  b->bits >>= n;
  b->count -= n;
  return 1;
}
static uint16_t mp_reverse(uint16_t v, unsigned n) {
  uint16_t r = 0;
  while (n--) {
    r = (uint16_t)((r << 1) | (v & 1));
    v >>= 1;
  }
  return r;
}
static int mp_huff_build(mp_huff *h, const unsigned char *lens, unsigned n) {
  unsigned count[16] = {0}, next[16] = {0}, i, b, code = 0;
  h->count = 0;
  for (i = 0; i < n; i++) {
    if (lens[i] > 15)
      return 0;
    if (lens[i])
      count[lens[i]]++;
  }
  for (b = 1; b <= 15; b++) {
    code = (code + count[b - 1]) << 1;
    next[b] = code;
  }
  for (i = 0; i < n; i++)
    if (lens[i]) {
      unsigned k = h->count++;
      h->length[k] = lens[i];
      h->code[k] = mp_reverse((uint16_t)next[lens[i]]++, lens[i]);
      h->symbol[k] = (uint16_t)i;
    }
  return h->count > 0;
}
static int mp_huff_read(mp_bits *b, const mp_huff *h, uint32_t *out) {
  uint32_t code = 0, bit;
  unsigned len, i;
  for (len = 1; len <= 15; len++) {
    if (!mp_bits_read(b, 1, &bit))
      return 0;
    code |= bit << (len - 1);
    for (i = 0; i < h->count; i++)
      if (h->length[i] == len && h->code[i] == code) {
        *out = h->symbol[i];
        return 1;
      }
  }
  return 0;
}
static int mp_inflate(const unsigned char *src, size_t n, unsigned char *out,
                      size_t cap, size_t *written) {
  static const uint16_t lb[29] = {3,  4,  5,  6,   7,   8,   9,   10,  11, 13,
                                  15, 17, 19, 23,  27,  31,  35,  43,  51, 59,
                                  67, 83, 99, 115, 131, 163, 195, 227, 258};
  static const unsigned char le[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
                                       1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
                                       4, 4, 4, 4, 5, 5, 5, 5, 0};
  static const uint16_t db[30] = {
      1,    2,    3,    4,    5,    7,    9,    13,    17,    25,
      33,   49,   65,   97,   129,  193,  257,  385,   513,   769,
      1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
  static const unsigned char de[30] = {0, 0, 0,  0,  1,  1,  2,  2,  3,  3,
                                       4, 4, 5,  5,  6,  6,  7,  7,  8,  8,
                                       9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
  mp_bits br;
  size_t op = 0;
  if (n < 6 || (src[0] & 15) != 8 || (((unsigned)src[0] << 8) | src[1]) % 31)
    return 0;
  br = (mp_bits){src + 2, n - 6, 0, 0, 0};
  for (;;) {
    uint32_t final, type;
    if (!mp_bits_read(&br, 1, &final) || !mp_bits_read(&br, 2, &type))
      return 0;
    if (type == 0) {
      uint32_t len, nlen;
      br.bits = 0;
      br.count = 0;
      if (br.pos + 4 > br.n)
        return 0;
      len = (uint32_t)br.p[br.pos] | ((uint32_t)br.p[br.pos + 1] << 8);
      nlen = (uint32_t)br.p[br.pos + 2] | ((uint32_t)br.p[br.pos + 3] << 8);
      br.pos += 4;
      if ((len ^ 0xffffu) != nlen || br.pos + len > br.n || op + len > cap)
        return 0;
      memcpy(out + op, br.p + br.pos, len);
      op += len;
      br.pos += len;
    } else if (type == 1 || type == 2) {
      unsigned char ll[288] = {0}, dl[32] = {0};
      mp_huff lh, dh;
      if (type == 1) {
        unsigned i;
        for (i = 0; i <= 143; i++)
          ll[i] = 8;
        for (; i <= 255; i++)
          ll[i] = 9;
        for (; i <= 279; i++)
          ll[i] = 7;
        for (; i <= 287; i++)
          ll[i] = 8;
        for (i = 0; i < 32; i++)
          dl[i] = 5;
      } else {
        static const unsigned char order[19] = {
            16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
        uint32_t hlit, hdist, hclen, sym, extra;
        unsigned char cl[19] = {0}, all[320] = {0};
        mp_huff ch;
        unsigned i, total;
        if (!mp_bits_read(&br, 5, &hlit) || !mp_bits_read(&br, 5, &hdist) ||
            !mp_bits_read(&br, 4, &hclen))
          return 0;
        hlit += 257;
        hdist += 1;
        hclen += 4;
        for (i = 0; i < hclen; i++) {
          if (!mp_bits_read(&br, 3, &extra))
            return 0;
          cl[order[i]] = (unsigned char)extra;
        }
        if (!mp_huff_build(&ch, cl, 19))
          return 0;
        total = (unsigned)(hlit + hdist);
        for (i = 0; i < total;) {
          if (!mp_huff_read(&br, &ch, &sym))
            return 0;
          if (sym < 16)
            all[i++] = (unsigned char)sym;
          else {
            unsigned repeat;
            unsigned char value = 0;
            if (sym == 16) {
              if (!i || !mp_bits_read(&br, 2, &extra))
                return 0;
              repeat = 3 + extra;
              value = all[i - 1];
            } else if (sym == 17) {
              if (!mp_bits_read(&br, 3, &extra))
                return 0;
              repeat = 3 + extra;
            } else if (sym == 18) {
              if (!mp_bits_read(&br, 7, &extra))
                return 0;
              repeat = 11 + extra;
            } else
              return 0;
            if (i + repeat > total)
              return 0;
            while (repeat--)
              all[i++] = value;
          }
        }
        memcpy(ll, all, hlit);
        memcpy(dl, all + hlit, hdist);
      }
      if (!mp_huff_build(&lh, ll, 288) || !mp_huff_build(&dh, dl, 32))
        return 0;
      for (;;) {
        uint32_t sym, extra, len, dist;
        if (!mp_huff_read(&br, &lh, &sym))
          return 0;
        if (sym < 256) {
          if (op >= cap)
            return 0;
          out[op++] = (unsigned char)sym;
        } else if (sym == 256)
          break;
        else {
          if (sym < 257 || sym > 285)
            return 0;
          sym -= 257;
          len = lb[sym];
          if (le[sym]) {
            if (!mp_bits_read(&br, le[sym], &extra))
              return 0;
            len += extra;
          }
          if (!mp_huff_read(&br, &dh, &sym) || sym >= 30)
            return 0;
          dist = db[sym];
          if (de[sym]) {
            if (!mp_bits_read(&br, de[sym], &extra))
              return 0;
            dist += extra;
          }
          if (!dist || dist > op || op + len > cap)
            return 0;
          while (len--) {
            out[op] = out[op - dist];
            op++;
          }
        }
      }
    } else
      return 0;
    if (final)
      break;
  }
  *written = op;
  return 1;
}
static unsigned char mp_paeth(unsigned char a, unsigned char b,
                              unsigned char c) {
  int p = (int)a + b - c, pa = abs(p - a), pb = abs(p - b), pc = abs(p - c);
  return pa <= pb && pa <= pc ? a : (pb <= pc ? b : c);
}
static int mp_png_decode(const unsigned char *d, size_t n, unsigned char **rgb,
                         unsigned char **alpha, size_t *rgbn, size_t *alphan,
                         unsigned *w, unsigned *h) {
  size_t p = 8, rawlen, row, y, x, outn;
  unsigned type, bpp;
  mp_buf id = {0}, palette = {0}, trans = {0};
  unsigned char *raw = NULL;
  if (n < 33 || memcmp(d, "\x89PNG\r\n\x1a\n", 8))
    return 0;
  *w = mp_be32(d + 16);
  *h = mp_be32(d + 20);
  type = d[25];
  if (!*w || !*h || d[24] != 8 || d[28] != 0 ||
      (type != 0 && type != 2 && type != 3 && type != 4 && type != 6))
    return -1;
  bpp = type == 6 ? 4 : (type == 2 ? 3 : (type == 4 ? 2 : 1));
  while (p + 12 <= n) {
    uint32_t z = mp_be32(d + p);
    if (p + 12 + (size_t)z > n)
      goto bad;
    if (!memcmp(d + p + 4, "IDAT", 4)) {
      if (!mp_put(&id, d + p + 8, z))
        goto oom;
    } else if (!memcmp(d + p + 4, "PLTE", 4)) {
      if (!mp_put(&palette, d + p + 8, z))
        goto oom;
    } else if (!memcmp(d + p + 4, "tRNS", 4)) {
      if (!mp_put(&trans, d + p + 8, z))
        goto oom;
    } else if (!memcmp(d + p + 4, "IEND", 4))
      break;
    p += 12 + (size_t)z;
  }
  if (!id.n || (type == 3 && (!palette.n || palette.n % 3)))
    goto bad;
  if (*w > SIZE_MAX / bpp || *w * bpp + 1 > SIZE_MAX / (*h))
    goto bad;
  rawlen = ((size_t)*w * bpp + 1) * (*h);
  raw = (unsigned char *)malloc(rawlen);
  if (!raw)
    goto oom;
  if (!mp_inflate(id.p, id.n, raw, rawlen, &outn) || outn != rawlen)
    goto bad;
  row = (size_t)*w * bpp + 1;
  for (y = 0; y < *h; y++) {
    unsigned filter = raw[y * row];
    if (filter > 4)
      goto bad;
    for (x = 0; x < (size_t)*w * bpp; x++) {
      size_t q = y * row + 1 + x;
      unsigned char a = x >= bpp ? raw[q - bpp] : 0, b = y ? raw[q - row] : 0,
                    c = (y && x >= bpp) ? raw[q - row - bpp] : 0;
      if (filter == 1)
        raw[q] = (unsigned char)(raw[q] + a);
      else if (filter == 2)
        raw[q] = (unsigned char)(raw[q] + b);
      else if (filter == 3)
        raw[q] = (unsigned char)(raw[q] + ((unsigned)a + b) / 2);
      else if (filter == 4)
        raw[q] = (unsigned char)(raw[q] + mp_paeth(a, b, c));
    }
  }
  if ((size_t)*w * (*h) > SIZE_MAX / 3)
    goto bad;
  *rgbn = (size_t)*w * (*h) * 3;
  *alphan = (size_t)*w * (*h);
  *rgb = (unsigned char *)malloc(*rgbn);
  *alpha = (type == 4 || type == 6 || trans.n)
               ? (unsigned char *)malloc(*alphan)
               : NULL;
  if (!*rgb || ((type == 4 || type == 6 || trans.n) && !*alpha))
    goto oom;
  for (y = 0; y < *h; y++)
    for (x = 0; x < *w; x++) {
      size_t q = y * row + 1 + x * bpp, z = y * (size_t)*w + x, ri = z * 3;
      if (type == 0) {
        (*rgb)[ri] = (*rgb)[ri + 1] = (*rgb)[ri + 2] = raw[q];
        if (*alpha)
          (*alpha)[z] = (trans.n >= 2 && raw[q] == mp_u16(trans.p)) ? 0 : 255;
      } else if (type == 2) {
        (*rgb)[ri] = raw[q];
        (*rgb)[ri + 1] = raw[q + 1];
        (*rgb)[ri + 2] = raw[q + 2];
        if (*alpha)
          (*alpha)[z] = (trans.n >= 6 && raw[q] == trans.p[1] &&
                         raw[q + 1] == trans.p[3] && raw[q + 2] == trans.p[5])
                            ? 0
                            : 255;
      } else if (type == 3) {
        unsigned ix = raw[q];
        if ((size_t)ix * 3 + 2 >= palette.n)
          goto bad;
        (*rgb)[ri] = palette.p[ix * 3];
        (*rgb)[ri + 1] = palette.p[ix * 3 + 1];
        (*rgb)[ri + 2] = palette.p[ix * 3 + 2];
        if (*alpha)
          (*alpha)[z] = ix < trans.n ? trans.p[ix] : 255;
      } else if (type == 4) {
        (*rgb)[ri] = (*rgb)[ri + 1] = (*rgb)[ri + 2] = raw[q];
        (*alpha)[z] = raw[q + 1];
      } else {
        (*rgb)[ri] = raw[q];
        (*rgb)[ri + 1] = raw[q + 1];
        (*rgb)[ri + 2] = raw[q + 2];
        (*alpha)[z] = raw[q + 3];
      }
    }
  free(raw);
  free(id.p);
  free(palette.p);
  free(trans.p);
  return 1;
oom:
  free(*rgb);
  free(*alpha);
  *rgb = *alpha = NULL;
  free(raw);
  free(id.p);
  free(palette.p);
  free(trans.p);
  return -2;
bad:
  free(*rgb);
  free(*alpha);
  *rgb = *alpha = NULL;
  free(raw);
  free(id.p);
  free(palette.p);
  free(trans.p);
  return 0;
}
static int mp_png_passthrough(const unsigned char *d, size_t n, mp_buf *idat,
                              unsigned *w, unsigned *h, int *colors) {
  size_t p = 8;
  int transparent = 0;
  if (n < 33 || memcmp(d, "\x89PNG\r\n\x1a\n", 8))
    return 0;
  *w = mp_be32(d + 16);
  *h = mp_be32(d + 20);
  if (d[24] != 8 || d[28] != 0 || (d[25] != 0 && d[25] != 2))
    return -1;
  *colors = d[25] == 2 ? 3 : 1;
  while (p + 12 <= n) {
    uint32_t z = mp_be32(d + p);
    if (p + 12 + (size_t)z > n)
      return 0;
    if (!memcmp(d + p + 4, "IDAT", 4) && !mp_put(idat, d + p + 8, z))
      return -2;
    if (!memcmp(d + p + 4, "tRNS", 4))
      transparent = 1;
    if (!memcmp(d + p + 4, "IEND", 4))
      break;
    p += 12 + (size_t)z;
  }
  return transparent ? -1 : (idat->n ? 1 : 0);
}
minpdf_status minpdf_image(minpdf_document *d, const void *vp, size_t n,
                           float x, float y, const minpdf_image_options *in) {
  mp_page *p;
  mp_image *i;
  minpdf_image_options o;
  unsigned w = 0, h = 0;
  int png, colors = 3, decoded = 0;
  mp_buf id = {0};
  unsigned char *rgb = NULL, *alpha = NULL;
  size_t rgbn = 0, alphan = 0;
  const unsigned char *data = (const unsigned char *)vp;
  if (!d || !data || !n)
    return mp_fail(d, MINPDF_ERROR_ARGUMENT, "invalid image");
  p = mp_page_now(d);
  o = in ? *in : minpdf_image_defaults();
  if (mp_jpeg_size(data, n, &w, &h))
    png = 0;
  else {
    png = mp_png_passthrough(data, n, &id, &w, &h, &colors);
    if (png == -1) {
      int z;
      free(id.p);
      id.p = NULL;
      id.n = id.cap = 0;
      z = mp_png_decode(data, n, &rgb, &alpha, &rgbn, &alphan, &w, &h);
      if (z <= 0)
        return mp_fail(d, z == -2 ? MINPDF_ERROR_MEMORY : MINPDF_ERROR_FORMAT,
                       z == -2 ? "out of memory"
                               : "invalid or unsupported PNG");
      decoded = 1;
      png = 1;
      colors = 3;
    } else if (png <= 0) {
      free(id.p);
      return mp_fail(d, MINPDF_ERROR_FORMAT, "invalid image");
    }
  }
  if (p->image_count == p->image_cap) {
    size_t c = p->image_cap ? p->image_cap * 2 : 4;
    void *q = realloc(p->images, c * sizeof *p->images);
    if (!q) {
      free(id.p);
      free(rgb);
      free(alpha);
      return mp_fail(d, MINPDF_ERROR_MEMORY, "out of memory");
    }
    p->images = (mp_image *)q;
    p->image_cap = c;
  }
  i = &p->images[p->image_count++];
  memset(i, 0, sizeof *i);
  i->w = w;
  i->h = h;
  i->x = x;
  i->dw = o.width > 0 ? o.width : (float)w;
  i->dh = o.height > 0 ? o.height : (float)h;
  i->y = p->h - y - i->dh;
  i->jpeg = png ? (decoded ? 2 : 0) : 1;
  i->colors = colors;
  snprintf(i->name, sizeof i->name, "Im%zu", p->image_count);
  if (decoded) {
    i->data = rgb;
    i->length = rgbn;
    i->alpha = alpha;
    i->alpha_length = alphan;
  } else if (png) {
    i->data = id.p;
    i->length = id.n;
  } else {
    i->data = (unsigned char *)malloc(n);
    if (!i->data)
      return mp_fail(d, MINPDF_ERROR_MEMORY, "out of memory");
    memcpy(i->data, data, n);
    i->length = n;
  }
  if (!mp_printf(&p->content, "q\n%.2f 0 0 %.2f %.2f %.2f cm\n/%s Do\nQ\n",
                 i->dw, i->dh, i->x, i->y, i->name))
    return mp_fail(d, MINPDF_ERROR_MEMORY, "out of memory");
  return MINPDF_OK;
}
minpdf_status minpdf_link(minpdf_document *d, const char *label,
                          const char *url, float x, float y,
                          const minpdf_link_options *in) {
  mp_page *p;
  mp_link *l;
  minpdf_link_options o;
  minpdf_text_options t;
  minpdf_line_options ln;
  minpdf_status s;
  if (!d || !label || !url)
    return mp_fail(d, MINPDF_ERROR_ARGUMENT, "invalid link");
  p = mp_page_now(d);
  o = in ? *in : minpdf_link_defaults();
  t = minpdf_text_defaults();
  t.color = o.color;
  t.size = o.size > 0 ? o.size : 12;
  if ((s = minpdf_text(d, label, x, y, &t)) != MINPDF_OK)
    return s;
  if (o.underline) {
    ln = minpdf_line_defaults();
    ln.color = o.color;
    ln.width = .5f;
    if ((s = minpdf_line(d, x, y + 2, x + mp_measure(label, t.size), y + 2,
                         &ln)) != MINPDF_OK)
      return s;
  }
  if (p->link_count == p->link_cap) {
    size_t c = p->link_cap ? p->link_cap * 2 : 4;
    void *q = realloc(p->links, c * sizeof *p->links);
    if (!q)
      return mp_fail(d, MINPDF_ERROR_MEMORY, "out of memory");
    p->links = (mp_link *)q;
    p->link_cap = c;
  }
  l = &p->links[p->link_count++];
  l->url = mp_strdup(url);
  if (!l->url)
    return mp_fail(d, MINPDF_ERROR_MEMORY, "out of memory");
  l->x = x;
  l->y = p->h - y - t.size;
  l->w = mp_measure(label, t.size);
  l->h = t.size;
  return MINPDF_OK;
}
minpdf_status minpdf_table(minpdf_document *d, const char *const *cells,
                           size_t rows, float x, float y,
                           const minpdf_table_options *in) {
  minpdf_table_options o;
  size_t r, c;
  float *widths, total = 0, cy, fs, pad;
  minpdf_rect_options ro;
  minpdf_text_options to;
  minpdf_status s;
  if (!d || !in || !in->columns || !in->column_count || (!cells && rows))
    return mp_fail(d, MINPDF_ERROR_ARGUMENT, "invalid table");
  o = *in;
  fs = o.font_size > 0 ? o.font_size : 10;
  pad = o.padding >= 0 ? o.padding : 8;
  widths = (float *)malloc(o.column_count * sizeof *widths);
  if (!widths)
    return mp_fail(d, MINPDF_ERROR_MEMORY, "out of memory");
  for (c = 0; c < o.column_count; c++) {
    float w = o.columns[c].width;
    if (w <= 0) {
      w = mp_measure(o.columns[c].header ? o.columns[c].header : "", fs);
      for (r = 0; r < rows; r++) {
        float z = mp_measure(
            cells[r * o.column_count + c] ? cells[r * o.column_count + c] : "",
            fs);
        if (z > w)
          w = z;
      }
      w += 2 * pad;
    }
    widths[c] = w;
    total += w;
  }
  ro = minpdf_rect_defaults();
  ro.has_fill = 1;
  ro.fill = o.header_background;
  if ((s = minpdf_rect(d, x, y, total, fs + 2 * pad, &ro)) != MINPDF_OK)
    goto done;
  to = minpdf_text_defaults();
  to.size = fs;
  to.weight = MINPDF_WEIGHT_BOLD;
  to.color = o.header_color;
  {
    float cx = x;
    for (c = 0; c < o.column_count; c++) {
      to.align = o.columns[c].align;
      float tx =
          cx + (to.align == MINPDF_ALIGN_CENTER
                    ? widths[c] / 2
                    : (to.align == MINPDF_ALIGN_RIGHT ? widths[c] - pad : pad));
      if ((s = minpdf_text(d, o.columns[c].header ? o.columns[c].header : "",
                           tx, y + pad + fs * .8f, &to)) != MINPDF_OK)
        goto done;
      cx += widths[c];
    }
  }
  cy = y + fs + 2 * pad;
  to.weight = MINPDF_WEIGHT_NORMAL;
  to.color = minpdf_rgb(0, 0, 0);
  for (r = 0; r < rows; r++) {
    ro.fill = (r & 1) ? minpdf_rgb(249, 249, 249) : minpdf_rgb(255, 255, 255);
    if ((s = minpdf_rect(d, x, cy, total, fs + 2 * pad, &ro)) != MINPDF_OK)
      goto done;
    float cx = x;
    for (c = 0; c < o.column_count; c++) {
      to.align = o.columns[c].align;
      float tx =
          cx + (to.align == MINPDF_ALIGN_CENTER
                    ? widths[c] / 2
                    : (to.align == MINPDF_ALIGN_RIGHT ? widths[c] - pad : pad));
      if ((s = minpdf_text(d,
                           cells[r * o.column_count + c]
                               ? cells[r * o.column_count + c]
                               : "",
                           tx, cy + pad + fs * .8f, &to)) != MINPDF_OK)
        goto done;
      cx += widths[c];
    }
    cy += fs + 2 * pad;
  }
  ro = minpdf_rect_defaults();
  ro.has_stroke = 1;
  ro.stroke = o.border_color;
  s = minpdf_rect(d, x, y, total, (rows + 1) * (fs + 2 * pad), &ro);
done:
  free(widths);
  return s;
}

static int mp_obj(mp_buf *out, size_t **offs, size_t *cap, unsigned *oid) {
  if ((size_t)(*oid + 2) > *cap) {
    size_t c = *cap ? *cap * 2 : 64;
    size_t *q = (size_t *)realloc(*offs, c * sizeof **offs);
    if (!q)
      return 0;
    *offs = q;
    *cap = c;
  }
  (*oid)++;
  (*offs)[*oid] = out->n;
  return mp_printf(out, "%u 0 obj\n", *oid);
}
minpdf_status minpdf_build(minpdf_document *d, minpdf_buffer *res) {
  mp_buf o = {0};
  size_t *off = NULL, offcap = 0, i, j;
  unsigned oid = 0, fontids[4] = {0}, pagesid, infoid = 0, catalog, xref;
  const char *fn[4] = {"Helvetica", "Helvetica-Bold", "Helvetica-Oblique",
                       "Helvetica-BoldOblique"};
  if (!d || !res)
    return mp_fail(d, MINPDF_ERROR_ARGUMENT, "invalid output");
  res->data = NULL;
  res->length = 0;
  if (!mp_put(&o, "%PDF-1.4\n%\xB5\xB5\xB5\xB5\n", 15))
    goto oom;
  for (i = 0; i < 4; i++) {
    int used = 0;
    for (j = 0; j < d->page_count; j++)
      if (d->pages[j].fonts & (1u << i))
        used = 1;
    if (used) {
      if (!mp_obj(&o, &off, &offcap, &oid))
        goto oom;
      fontids[i] = oid;
      if (!mp_printf(&o,
                     "<</Type/Font/Subtype/Type1/BaseFont/%s/Encoding/"
                     "WinAnsiEncoding>>\nendobj\n",
                     fn[i]))
        goto oom;
    }
  }
  for (i = 0; i < d->font_count; i++) {
    mp_font *f = &d->fonts[i];
    unsigned fileid, descid, cidid, mapid;
    mp_buf map = {0};
    double scale = 1000.0 / f->units;
    if (!f->used_count)
      continue;
    if (!mp_obj(&o, &off, &offcap, &oid))
      goto oom;
    fileid = oid;
    if (!mp_printf(&o, "<</Length %zu/Length1 %zu>>\nstream\n", f->length,
                   f->length) ||
        !mp_put(&o, f->data, f->length) ||
        !mp_put(&o, "\nendstream\nendobj\n", 18))
      goto oom;
    if (!mp_obj(&o, &off, &offcap, &oid))
      goto oom;
    descid = oid;
    if (!mp_printf(&o,
                   "<</Type/FontDescriptor/FontName/MinPDF%zu/Flags "
                   "32/FontBBox[%d %d %d %d]/ItalicAngle 0/Ascent %d/Descent "
                   "%d/CapHeight %d/StemV 80/FontFile2 %u 0 R>>\nendobj\n",
                   i, (int)(f->bbox[0] * scale), (int)(f->bbox[1] * scale),
                   (int)(f->bbox[2] * scale), (int)(f->bbox[3] * scale),
                   (int)(f->ascent * scale), (int)(f->descent * scale),
                   (int)(f->ascent * scale), fileid))
      goto oom;
    if (!mp_obj(&o, &off, &offcap, &oid))
      goto oom;
    cidid = oid;
    if (!mp_printf(
            &o,
            "<</Type/Font/Subtype/CIDFontType2/BaseFont/MinPDF%zu/"
            "CIDSystemInfo<</Registry(Adobe)/Ordering(Identity)/Supplement "
            "0>>/FontDescriptor %u 0 R/CIDToGIDMap/Identity/DW 1000/W[",
            i, descid))
      goto oom;
    for (j = 0; j < f->used_count; j++) {
      size_t k;
      int first = 1;
      for (k = 0; k < j; k++)
        if (f->used[k].glyph == f->used[j].glyph)
          first = 0;
      if (first &&
          !mp_printf(&o, "%u[%u]", f->used[j].glyph,
                     (unsigned)((uint32_t)mp_advance(f, f->used[j].glyph) *
                                1000 / f->units)))
        goto oom;
    }
    if (!mp_printf(&o, "]>>\nendobj\n"))
      goto oom;
    if (!mp_printf(
            &map,
            "/CIDInit/ProcSet findresource begin\n12 dict "
            "begin\nbegincmap\n/CIDSystemInfo<</Registry(Adobe)/Ordering(UCS)/"
            "Supplement 0>>def\n/CMapName/MinPDF%zu def\n/CMapType 2 def\n1 "
            "begincodespacerange\n<0000><FFFF>\nendcodespacerange\n",
            i))
      goto oom;
    for (j = 0; j < f->used_count; j += 100) {
      size_t k, end = j + 100 < f->used_count ? j + 100 : f->used_count;
      if (!mp_printf(&map, "%zu beginbfchar\n", end - j))
        goto oom;
      for (k = j; k < end; k++) {
        uint32_t cp = f->used[k].unicode;
        if (cp <= 0xffff) {
          if (!mp_printf(&map, "<%04X><%04X>\n", f->used[k].glyph,
                         (unsigned)cp))
            goto oom;
        } else {
          uint32_t v = cp - 0x10000, hi = 0xd800 + (v >> 10),
                   lo = 0xdc00 + (v & 1023);
          if (!mp_printf(&map, "<%04X><%04X%04X>\n", f->used[k].glyph,
                         (unsigned)hi, (unsigned)lo))
            goto oom;
        }
      }
      if (!mp_printf(&map, "endbfchar\n"))
        goto oom;
    }
    if (!mp_printf(&map, "endcmap\nCMapName currentdict/CMap defineresource "
                         "pop\nend\nend\n"))
      goto oom;
    if (!mp_obj(&o, &off, &offcap, &oid)) {
      free(map.p);
      goto oom;
    }
    mapid = oid;
    if (!mp_printf(&o, "<</Length %zu>>\nstream\n", map.n) ||
        !mp_put(&o, map.p, map.n) || !mp_put(&o, "endstream\nendobj\n", 17)) {
      free(map.p);
      goto oom;
    }
    free(map.p);
    if (!mp_obj(&o, &off, &offcap, &oid))
      goto oom;
    f->object_id = oid;
    if (!mp_printf(
            &o,
            "<</Type/Font/Subtype/Type0/BaseFont/MinPDF%zu/Encoding/Identity-H/"
            "DescendantFonts[%u 0 R]/ToUnicode %u 0 R>>\nendobj\n",
            i, cidid, mapid))
      goto oom;
  }
  for (i = 0; i < d->page_count; i++) {
    mp_page *p = &d->pages[i];
    for (j = 0; j < p->image_count; j++) {
      mp_image *im = &p->images[j];
      unsigned smask = 0;
      if (im->alpha) {
        if (!mp_obj(&o, &off, &offcap, &oid))
          goto oom;
        smask = oid;
        if (!mp_printf(&o,
                       "<</Type/XObject/Subtype/Image/Width %u/Height "
                       "%u/ColorSpace/DeviceGray/BitsPerComponent 8/Length "
                       "%zu>>\nstream\n",
                       im->w, im->h, im->alpha_length) ||
            !mp_put(&o, im->alpha, im->alpha_length) ||
            !mp_put(&o, "\nendstream\nendobj\n", 18))
          goto oom;
      }
      if (!mp_obj(&o, &off, &offcap, &oid))
        goto oom;
      im->object_id = oid;
      if (!mp_printf(&o,
                     "<</Type/XObject/Subtype/Image/Width %u/Height "
                     "%u/ColorSpace/%s/BitsPerComponent 8",
                     im->w, im->h,
                     im->colors == 1 ? "DeviceGray" : "DeviceRGB"))
        goto oom;
      if (im->jpeg == 1 && !mp_printf(&o, "/Filter/DCTDecode"))
        goto oom;
      if (im->jpeg == 0 &&
          !mp_printf(&o,
                     "/Filter/FlateDecode/DecodeParms<</Predictor 15/Colors "
                     "%d/BitsPerComponent 8/Columns %u>>",
                     im->colors, im->w))
        goto oom;
      if (smask && !mp_printf(&o, "/SMask %u 0 R", smask))
        goto oom;
      if (!mp_printf(&o, "/Length %zu>>\nstream\n", im->length) ||
          !mp_put(&o, im->data, im->length) ||
          !mp_put(&o, "\nendstream\nendobj\n", 18))
        goto oom;
    }
    if (!mp_obj(&o, &off, &offcap, &oid))
      goto oom;
    p->content_id = oid;
    if (!mp_printf(&o, "<</Length %zu>>\nstream\n", p->content.n) ||
        !mp_put(&o, p->content.p, p->content.n) ||
        !mp_put(&o, "endstream\nendobj\n", 17))
      goto oom;
    for (j = 0; j < p->link_count; j++) {
      mp_link *l = &p->links[j];
      if (!mp_obj(&o, &off, &offcap, &oid))
        goto oom;
      l->object_id = oid;
      if (!mp_printf(&o,
                     "<</Type/Annot/Subtype/Link/Rect[%.2f %.2f %.2f "
                     "%.2f]/Border[0 0 0]/A<</S/URI/URI",
                     l->x, l->y, l->x + l->w, l->y + l->h) ||
          !mp_pdf_string(&o, l->url) || !mp_put(&o, ">>>>\nendobj\n", 12))
        goto oom;
    }
  }
  if (!mp_obj(&o, &off, &offcap, &oid))
    goto oom;
  pagesid = oid;
  if (!mp_printf(&o, "<</Type/Pages/Kids["))
    goto oom;
  for (i = 0; i < d->page_count; i++)
    if (!mp_printf(&o, "%u 0 R ", pagesid + 1 + (unsigned)i))
      goto oom;
  if (!mp_printf(&o, "]/Count %zu>>\nendobj\n", d->page_count))
    goto oom;
  for (i = 0; i < d->page_count; i++) {
    mp_page *p = &d->pages[i];
    if (!mp_obj(&o, &off, &offcap, &oid))
      goto oom;
    if (!mp_printf(&o,
                   "<</Type/Page/Parent %u 0 R/MediaBox[0 0 %.2f "
                   "%.2f]/Contents %u 0 R/Resources<<",
                   pagesid, p->w, p->h, p->content_id))
      goto oom;
    if (p->fonts || p->custom_fonts) {
      if (!mp_put(&o, "/Font<<", 7))
        goto oom;
      for (j = 0; j < 4; j++)
        if ((p->fonts & (1u << j)) &&
            !mp_printf(&o, "/%s %u 0 R", fn[j], fontids[j]))
          goto oom;
      for (j = 0; j < d->font_count; j++)
        if ((p->custom_fonts & ((uint64_t)1 << j)) &&
            !mp_printf(&o, "/FC%zu %u 0 R", j, d->fonts[j].object_id))
          goto oom;
      if (!mp_put(&o, ">>", 2))
        goto oom;
    }
    if (p->image_count) {
      if (!mp_put(&o, "/XObject<<", 10))
        goto oom;
      for (j = 0; j < p->image_count; j++)
        if (!mp_printf(&o, "/%s %u 0 R", p->images[j].name,
                       p->images[j].object_id))
          goto oom;
      if (!mp_put(&o, ">>", 2))
        goto oom;
    }
    if (!mp_put(&o, ">>", 2))
      goto oom;
    if (p->link_count) {
      if (!mp_put(&o, "/Annots[", 8))
        goto oom;
      for (j = 0; j < p->link_count; j++)
        if (!mp_printf(&o, "%u 0 R ", p->links[j].object_id))
          goto oom;
      if (!mp_put(&o, "]", 1))
        goto oom;
    }
    if (!mp_put(&o, ">>\nendobj\n", 10))
      goto oom;
  }
  if (d->meta.title || d->meta.author || d->meta.subject || d->meta.keywords ||
      d->meta.creator || d->meta.creation_date) {
    const char *keys[6] = {"Title",    "Author",  "Subject",
                           "Keywords", "Creator", "CreationDate"};
    const char *values[6] = {d->meta.title,   d->meta.author,
                             d->meta.subject, d->meta.keywords,
                             d->meta.creator, d->meta.creation_date};
    if (!mp_obj(&o, &off, &offcap, &oid))
      goto oom;
    infoid = oid;
    if (!mp_put(&o, "<<", 2))
      goto oom;
    for (i = 0; i < 6; i++) {
      if (values[i] &&
          (!mp_printf(&o, "/%s", keys[i]) || !mp_pdf_string(&o, values[i])))
        goto oom;
    }
    if (!mp_put(&o, "/Producer(minpdf)>>\nendobj\n", 27))
      goto oom;
  }
  if (!mp_obj(&o, &off, &offcap, &oid))
    goto oom;
  catalog = oid;
  if (!mp_printf(&o, "<</Type/Catalog/Pages %u 0 R>>\nendobj\n", pagesid))
    goto oom;
  xref = (unsigned)o.n;
  if (!mp_printf(&o, "xref\n0 %u\n0000000000 65535 f \n", oid + 1))
    goto oom;
  for (i = 1; i <= oid; i++)
    if (!mp_printf(&o, "%010zu 00000 n \n", off[i]))
      goto oom;
  if (!mp_printf(&o, "trailer\n<</Size %u/Root %u 0 R", oid + 1, catalog))
    goto oom;
  if (infoid && !mp_printf(&o, "/Info %u 0 R", infoid))
    goto oom;
  if (!mp_printf(&o, ">>\nstartxref\n%u\n%%%%EOF\n", xref))
    goto oom;
  res->data = o.p;
  res->length = o.n;
  free(off);
  return MINPDF_OK;
oom:
  free(o.p);
  free(off);
  return mp_fail(d, MINPDF_ERROR_MEMORY, "out of memory");
}
minpdf_status minpdf_save(minpdf_document *d, const char *path) {
  minpdf_buffer b;
  FILE *f;
  size_t n;
  minpdf_status s;
  if (!path)
    return mp_fail(d, MINPDF_ERROR_ARGUMENT, "invalid path");
  if ((s = minpdf_build(d, &b)) != MINPDF_OK)
    return s;
  f = fopen(path, "wb");
  if (!f) {
    minpdf_buffer_free(&b);
    return mp_fail(d, MINPDF_ERROR_IO, "cannot open output file");
  }
  n = fwrite(b.data, 1, b.length, f);
  if (fclose(f) || n != b.length) {
    minpdf_buffer_free(&b);
    return mp_fail(d, MINPDF_ERROR_IO, "cannot write output file");
  }
  minpdf_buffer_free(&b);
  return MINPDF_OK;
}
void minpdf_buffer_free(minpdf_buffer *b) {
  if (b) {
    free(b->data);
    b->data = NULL;
    b->length = 0;
  }
}
#undef minpdf_text
#undef minpdf_rect
minpdf_status minpdf_rect(minpdf_document *d, float x, float y, float w,
                          float h, const minpdf_rect_options *in) {
  mp_page *p;
  minpdf_rect_options o;
  float r, k = .55228475f, py;
  if (!in || in->radius <= 0)
    return mp_rect_basic(d, x, y, w, h, in);
  if (!d || w < 0 || h < 0)
    return mp_fail(d, MINPDF_ERROR_ARGUMENT, "invalid rectangle");
  p = mp_page_now(d);
  o = *in;
  r = o.radius;
  if (r > w / 2)
    r = w / 2;
  if (r > h / 2)
    r = h / 2;
  py = p->h - y - h;
  if (!mp_put(&p->content, "q\n", 2))
    goto oom;
  if (o.has_fill && !mp_color(&p->content, o.fill, 0))
    goto oom;
  if (o.has_stroke && !mp_color(&p->content, o.stroke, 1))
    goto oom;
  if (!mp_printf(&p->content,
                 "%.2f w\n%.2f %.2f m\n%.2f %.2f l\n%.2f %.2f %.2f %.2f %.2f "
                 "%.2f c\n%.2f %.2f l\n%.2f %.2f %.2f %.2f %.2f %.2f c\n%.2f "
                 "%.2f l\n%.2f %.2f %.2f %.2f %.2f %.2f c\n%.2f %.2f l\n%.2f "
                 "%.2f %.2f %.2f %.2f %.2f c\nh\n%s\nQ\n",
                 o.line_width > 0 ? o.line_width : 1, x + r, py, x + w - r, py,
                 x + w - r + r * k, py, x + w, py + r - r * k, x + w, py + r,
                 x + w, py + h - r, x + w, py + h - r + r * k,
                 x + w - r + r * k, py + h, x + w - r, py + h, x + r, py + h,
                 x + r - r * k, py + h, x, py + h - r + r * k, x, py + h - r, x,
                 py + r, x, py + r - r * k, x + r - r * k, py, x + r, py,
                 o.has_fill ? (o.has_stroke ? "B" : "f")
                            : (o.has_stroke ? "S" : "n")))
    goto oom;
  return MINPDF_OK;
oom:
  return mp_fail(d, MINPDF_ERROR_MEMORY, "out of memory");
}
static float mp_wrap_width(minpdf_document *d, const char *t,
                           const minpdf_text_options *o) {
  size_t i;
  if (o->font && *o->font) {
    mp_font *f = NULL;
    const char *s = t;
    uint32_t cp;
    float width = 0;
    for (i = 0; i < d->font_count; i++)
      if (!strcmp(d->fonts[i].name, o->font)) {
        f = &d->fonts[i];
        break;
      }
    if (!f)
      return -1;
    while (*s) {
      if (mp_utf8_next(&s, &cp) < 0)
        return -1;
      width += (float)mp_advance(f, mp_glyph(f, cp)) * o->size / f->units;
    }
    return width;
  }
  return mp_measure(t, o->size);
}
minpdf_status minpdf_text(minpdf_document *d, const char *t, float x, float y,
                          const minpdf_text_options *in) {
  minpdf_text_options o;
  mp_buf line = {0};
  const char *p;
  float py;
  minpdf_status status = MINPDF_OK;
  if (!d || !t)
    return mp_fail(d, MINPDF_ERROR_ARGUMENT, "invalid text");
  o = in ? *in : minpdf_text_defaults();
  if (o.size <= 0)
    o.size = 12;
  if (o.max_width <= 0)
    return mp_text_line(d, t, x, y, &o);
  o.max_width = 0;
  p = t;
  py = y;
  while (*p) {
    const char *start;
    size_t len, old = line.n;
    while (*p == ' ')
      p++;
    if (!*p)
      break;
    start = p;
    while (*p && *p != ' ')
      p++;
    len = (size_t)(p - start);
    if (line.n && !mp_put(&line, " ", 1))
      goto oom;
    if (!mp_put(&line, start, len) || !mp_grow(&line, 1))
      goto oom;
    line.p[line.n] = 0;
    if (old && mp_wrap_width(d, (char *)line.p, &o) > in->max_width) {
      line.n = old;
      line.p[line.n] = 0;
      if ((status = mp_text_line(d, (char *)line.p, x, py, &o)) != MINPDF_OK)
        goto done;
      py += o.size * 1.2f;
      line.n = 0;
      if (!mp_put(&line, start, len) || !mp_grow(&line, 1))
        goto oom;
      line.p[line.n] = 0;
    }
  }
  if (line.n)
    status = mp_text_line(d, (char *)line.p, x, py, &o);
  goto done;
oom:
  status = mp_fail(d, MINPDF_ERROR_MEMORY, "out of memory");
done:
  free(line.p);
  return status;
}
#undef malloc
#undef realloc
#undef calloc
#undef free
#endif
#endif
