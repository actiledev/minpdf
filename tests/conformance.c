#define MINPDF_IMPLEMENTATION
#include "../include/minpdf.h"

#include <stdio.h>

static const unsigned char png[] = {
    0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,
    0x49,0x48,0x44,0x52,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,
    0x08,0x06,0x00,0x00,0x00,0x1f,0x15,0xc4,0x89,0x00,0x00,0x00,
    0x0d,0x49,0x44,0x41,0x54,0x78,0xda,0x63,0xfc,0xcf,0xc0,0x50,
    0x0f,0x00,0x05,0x83,0x02,0x7f,0x96,0xa2,0xfc,0x59,0x00,0x00,
    0x00,0x00,0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82};

int main(void) {
  const char *cells[] = {"Ada", "Engineer", "Linus", "Maintainer"};
  minpdf_table_column columns[] = {{"Name", 100, MINPDF_ALIGN_LEFT},
                                   {"Role", 120, MINPDF_ALIGN_RIGHT}};
  minpdf_table_options table = minpdf_table_defaults(columns, 2);
  minpdf_text_options text = minpdf_text_defaults();
  minpdf_rect_options rect = minpdf_rect_defaults();
  minpdf_line_options line = minpdf_line_defaults();
  minpdf_metadata metadata = {"Conformance", "minpdf", NULL, NULL,
                              "minpdf", "D:20260802000000Z"};
  minpdf_document *document = minpdf_create(MINPDF_A4);
  minpdf_buffer output = {0};
  float dash[] = {3, 2};
  int status;
  int invalid_image_status;
  int invalid_page_status;
  size_t index;

  if (!document)
    return 2;
  invalid_page_status = minpdf_add_page(document, (minpdf_size){0, 100});
  invalid_image_status = minpdf_image(document, "bad", 3, 0, 0, NULL);
  printf("errors:%d:%d:%s\n", invalid_page_status, invalid_image_status,
         minpdf_error(document));
  text.size = 18;
  text.weight = MINPDF_WEIGHT_BOLD;
  rect.has_fill = rect.has_stroke = 1;
  rect.fill = minpdf_rgb(52, 152, 219);
  rect.stroke = minpdf_rgb(0, 0, 0);
  line.dash = dash;
  line.dash_count = 2;

  (void)minpdf_set_metadata(document, &metadata);
  (void)minpdf_text(document, "Native and Wasm", 50, 60, &text);
  (void)minpdf_rect(document, 50, 90, 200, 60, &rect);
  (void)minpdf_circle(document, 300, 100, 20, &rect);
  (void)minpdf_line(document, 10, 20, 100, 20, &line);
  (void)minpdf_link(document, "Site", "https://example.com?a=1&b=2", 50,
                    180, NULL);
  (void)minpdf_table(document, cells, 2, 50, 220, &table);
  (void)minpdf_image(document, png, sizeof png, 10, 400, NULL);
  (void)minpdf_add_page(document, MINPDF_LETTER);
  (void)minpdf_text(document, "Second page", 20, 30, NULL);
  status = minpdf_build(document, &output);
  if (status != MINPDF_OK) {
    minpdf_destroy(document);
    return 3;
  }
  for (index = 0; index < output.length; index++)
    printf("%02x", output.data[index]);
  putchar('\n');
  minpdf_buffer_free(&output);
  minpdf_destroy(document);
  return 0;
}
