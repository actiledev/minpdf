#define MINPDF_IMPLEMENTATION
#include "../include/minpdf.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int contains(const unsigned char *data, size_t length,
                    const char *needle) {
  size_t n = strlen(needle), i;
  for (i = 0; i + n <= length; ++i)
    if (memcmp(data + i, needle, n) == 0)
      return 1;
  return 0;
}

static minpdf_buffer build(minpdf_document *pdf) {
  minpdf_buffer bytes = {0};
  assert(minpdf_build(pdf, &bytes) == MINPDF_OK);
  assert(bytes.length > 100);
  assert(memcmp(bytes.data, "%PDF-1.4", 8) == 0);
  assert(contains(bytes.data, bytes.length, "xref"));
  assert(contains(bytes.data, bytes.length, "%%EOF"));
  return bytes;
}

static void assert_repeated_build(minpdf_document *pdf,
                                  const minpdf_buffer *first) {
  minpdf_buffer second = build(pdf);
  assert(second.length == first->length);
  assert(memcmp(second.data, first->data, first->length) == 0);
  minpdf_buffer_free(&second);
}

static void test_defaults_and_colors(void) {
  minpdf_color c = minpdf_rgb(255, 128, 0);
  minpdf_text_options text = minpdf_text_defaults();
  minpdf_rect_options rect = minpdf_rect_defaults();
  minpdf_line_options line = minpdf_line_defaults();
  minpdf_image_options image = minpdf_image_defaults();
  minpdf_link_options link = minpdf_link_defaults();
  assert(c.r == 1.0f && c.g > 0.50f && c.g < 0.51f && c.b == 0.0f);
  assert(text.size == 12 && text.align == MINPDF_ALIGN_LEFT);
  assert(text.weight == MINPDF_WEIGHT_NORMAL && text.font == NULL);
  assert(!rect.has_fill && !rect.has_stroke && rect.line_width == 1);
  assert(line.width == 1 && line.dash == NULL && line.dash_count == 0);
  assert(image.width == 0 && image.height == 0);
  assert(link.underline && link.size == 12);
}

static void test_document_content(void) {
  minpdf_document *pdf = minpdf_create(MINPDF_A4);
  minpdf_text_options text = minpdf_text_defaults();
  minpdf_rect_options rect = minpdf_rect_defaults();
  minpdf_line_options line = minpdf_line_defaults();
  minpdf_link_options link = minpdf_link_defaults();
  minpdf_metadata meta = {"A (title)", "Ada", NULL, NULL, "minpdf", NULL};
  minpdf_buffer bytes;
  float dash[] = {3, 2};
  assert(pdf);
  assert(minpdf_set_metadata(pdf, &meta) == MINPDF_OK);
  text.size = 18;
  text.weight = MINPDF_WEIGHT_BOLD;
  text.color = minpdf_rgb(10, 20, 30);
  assert(minpdf_text(pdf, "Hello (PDF) \\ world", 50, 60, &text) == MINPDF_OK);
  rect.has_fill = rect.has_stroke = 1;
  rect.fill = minpdf_rgb(52, 152, 219);
  rect.stroke = minpdf_rgb(0, 0, 0);
  rect.radius = 4;
  assert(minpdf_rect(pdf, 50, 90, 200, 60, &rect) == MINPDF_OK);
  assert(minpdf_circle(pdf, 300, 100, 20, &rect) == MINPDF_OK);
  line.dash = dash;
  line.dash_count = 2;
  assert(minpdf_line(pdf, 10, 20, 100, 20, &line) == MINPDF_OK);
  link.underline = 0;
  assert(minpdf_link(pdf, "Site", "https://example.com?a=1&b=2", 50, 180,
                     &link) == MINPDF_OK);
  assert(minpdf_add_page(pdf, MINPDF_LETTER) == MINPDF_OK);
  assert(minpdf_text(pdf, "Second page", 20, 30, NULL) == MINPDF_OK);
  bytes = build(pdf);
  assert(contains(bytes.data, bytes.length, "/Count 2"));
  assert(contains(bytes.data, bytes.length, "/Helvetica-Bold"));
  assert(contains(bytes.data, bytes.length, "Hello \\(PDF\\) \\\\ world"));
  assert(contains(bytes.data, bytes.length, "/Title(A \\(title\\))"));
  assert(contains(bytes.data, bytes.length, "/URI(https://example.com?a=1&b=2)"));
  assert(contains(bytes.data, bytes.length, "[3.00 2.00 ] 0 d"));
  assert_repeated_build(pdf, &bytes);
  minpdf_buffer_free(&bytes);
  assert(bytes.data == NULL && bytes.length == 0);
  minpdf_destroy(pdf);
}

static void test_table_and_image(void) {
  static const unsigned char png[] = {
      0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,
      0x49,0x48,0x44,0x52,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,
      0x08,0x06,0x00,0x00,0x00,0x1f,0x15,0xc4,0x89,0x00,0x00,0x00,
      0x0d,0x49,0x44,0x41,0x54,0x78,0xda,0x63,0xfc,0xcf,0xc0,0x50,
      0x0f,0x00,0x05,0x83,0x02,0x7f,0x96,0xa2,0xfc,0x59,0x00,0x00,
      0x00,0x00,0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82};
  const char *cells[] = {"Ada", "Engineer", "Linus", "Maintainer"};
  minpdf_table_column columns[] = {{"Name", 100, MINPDF_ALIGN_LEFT},
                                   {"Role", 0, MINPDF_ALIGN_RIGHT}};
  minpdf_table_options table = minpdf_table_defaults(columns, 2);
  minpdf_document *pdf = minpdf_create(MINPDF_A4);
  minpdf_buffer bytes;
  assert(minpdf_table(pdf, cells, 2, 50, 50, &table) == MINPDF_OK);
  assert(minpdf_image(pdf, png, sizeof png, 10, 180, NULL) == MINPDF_OK);
  bytes = build(pdf);
  assert(contains(bytes.data, bytes.length, "(Name)"));
  assert(contains(bytes.data, bytes.length, "(Maintainer)"));
  assert(contains(bytes.data, bytes.length, "/Subtype/Image"));
  assert(contains(bytes.data, bytes.length, "/SMask"));
  minpdf_buffer_free(&bytes);
  minpdf_destroy(pdf);
}

static void test_errors(void) {
  minpdf_document *pdf = minpdf_create(MINPDF_A4);
  minpdf_text_options text = minpdf_text_defaults();
  minpdf_table_options table = {0};
  unsigned char junk[12] = {0};
  assert(minpdf_add_page(pdf, (minpdf_size){0, 100}) == MINPDF_ERROR_ARGUMENT);
  assert(strcmp(minpdf_error(pdf), "invalid page size") == 0);
  assert(minpdf_rect(pdf, 0, 0, -1, 1, NULL) == MINPDF_ERROR_ARGUMENT);
  assert(minpdf_circle(pdf, 0, 0, -1, NULL) == MINPDF_ERROR_ARGUMENT);
  assert(minpdf_image(pdf, junk, sizeof junk, 0, 0, NULL) ==
         MINPDF_ERROR_FORMAT);
  assert(minpdf_register_font(pdf, "bad", junk, sizeof junk) ==
         MINPDF_ERROR_FORMAT);
  text.font = "missing";
  assert(minpdf_text(pdf, "x", 0, 0, &text) == MINPDF_ERROR_ARGUMENT);
  text.font = NULL;
  assert(minpdf_text(pdf, "caf\xc3\xa9", 0, 0, &text) ==
         MINPDF_ERROR_UNSUPPORTED);
  assert(minpdf_table(pdf, NULL, 0, 0, 0, &table) == MINPDF_ERROR_ARGUMENT);
  assert(minpdf_build(pdf, NULL) == MINPDF_ERROR_ARGUMENT);
  assert(minpdf_save(pdf, "") == MINPDF_ERROR_IO);
  minpdf_destroy(pdf);
  assert(strcmp(minpdf_error(NULL), "invalid document") == 0);
}

int main(void) {
  test_defaults_and_colors();
  test_document_content();
  test_table_and_image();
  test_errors();
  return 0;
}
