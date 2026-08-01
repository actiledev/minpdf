#include "minpdf.h"
#include <stdlib.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define MPW EMSCRIPTEN_KEEPALIVE
#else
#define MPW
#endif

MPW minpdf_document *mpw_create(float w, float h) {
  return minpdf_create((minpdf_size){w, h});
}
MPW void mpw_destroy(minpdf_document *d) { minpdf_destroy(d); }
MPW const char *mpw_error(minpdf_document *d) { return minpdf_error(d); }
MPW int mpw_page(minpdf_document *d, float w, float h) {
  return minpdf_add_page(d, (minpdf_size){w, h});
}
MPW int mpw_text(minpdf_document *d, const char *t, float x, float y,
                 float size, float r, float g, float b, int align, int weight,
                 float maxw, const char *font) {
  minpdf_text_options o = minpdf_text_defaults();
  o.size = size;
  o.color = (minpdf_color){r, g, b};
  o.align = (minpdf_align)align;
  o.weight = (minpdf_weight)weight;
  o.max_width = maxw;
  o.font = font && *font ? font : NULL;
  return minpdf_text(d, t, x, y, &o);
}
MPW int mpw_rect(minpdf_document *d, float x, float y, float w, float h,
                 float fr, float fg, float fb, float sr, float sg, float sb,
                 float lw, float radius, int fill, int stroke) {
  minpdf_rect_options o = minpdf_rect_defaults();
  o.fill = (minpdf_color){fr, fg, fb};
  o.stroke = (minpdf_color){sr, sg, sb};
  o.line_width = lw;
  o.radius = radius;
  o.has_fill = fill;
  o.has_stroke = stroke;
  return minpdf_rect(d, x, y, w, h, &o);
}
MPW int mpw_line(minpdf_document *d, float x1, float y1, float x2, float y2,
                 float r, float g, float b, float w, const float *dash,
                 size_t dash_count) {
  minpdf_line_options o = minpdf_line_defaults();
  o.color = (minpdf_color){r, g, b};
  o.width = w;
  o.dash = dash;
  o.dash_count = dash_count;
  return minpdf_line(d, x1, y1, x2, y2, &o);
}
MPW int mpw_circle(minpdf_document *d, float x, float y, float radius, float fr,
                   float fg, float fb, float sr, float sg, float sb, float lw,
                   int fill, int stroke) {
  minpdf_rect_options o = minpdf_rect_defaults();
  o.fill = (minpdf_color){fr, fg, fb};
  o.stroke = (minpdf_color){sr, sg, sb};
  o.line_width = lw;
  o.has_fill = fill;
  o.has_stroke = stroke;
  return minpdf_circle(d, x, y, radius, &o);
}
MPW int mpw_image(minpdf_document *d, const void *p, size_t n, float x, float y,
                  float w, float h) {
  minpdf_image_options o = {w, h};
  return minpdf_image(d, p, n, x, y, &o);
}
MPW int mpw_font(minpdf_document *d, const char *name, const void *p,
                 size_t n) {
  return minpdf_register_font(d, name, p, n);
}
MPW int mpw_link(minpdf_document *d, const char *label, const char *url,
                 float x, float y, float r, float g, float b, float size,
                 int underline) {
  minpdf_link_options o = {{r, g, b}, underline, size};
  return minpdf_link(d, label, url, x, y, &o);
}
MPW int mpw_metadata(minpdf_document *d, const char *t, const char *a,
                     const char *s, const char *k, const char *c,
                     const char *date) {
  minpdf_metadata m = {t, a, s, k, c, date};
  return minpdf_set_metadata(d, &m);
}
MPW unsigned char *mpw_build(minpdf_document *d, size_t *length) {
  minpdf_buffer b = {0};
  if (minpdf_build(d, &b) != MINPDF_OK)
    return NULL;
  *length = b.length;
  return b.data;
}
MPW void mpw_free(void *p) { free(p); }
