#define MINPDF_IMPLEMENTATION
#include "../../include/minpdf.h"
#include <stdio.h>

static const unsigned char PNG_RGBA[] = {
    0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,
    0x49,0x48,0x44,0x52,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,
    0x08,0x06,0x00,0x00,0x00,0x1f,0x15,0xc4,0x89,0x00,0x00,0x00,
    0x0d,0x49,0x44,0x41,0x54,0x78,0xda,0x63,0xfc,0xcf,0xc0,0x50,
    0x0f,0x00,0x05,0x83,0x02,0x7f,0x96,0xa2,0xfc,0x59,0x00,0x00,
    0x00,0x00,0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82};

static int check(minpdf_document *pdf, minpdf_status status) {
  if (status == MINPDF_OK)
    return 1;
  fprintf(stderr, "minpdf: %s\n", minpdf_error(pdf));
  return 0;
}

int main(void) {
  const char *cells[] = {"Ada Lovelace", "Engineer", "Grace Hopper", "Admiral",
                         "Linus Torvalds", "Maintainer"};
  minpdf_table_column columns[] = {{"Name", 220, MINPDF_ALIGN_LEFT},
                                   {"Role", 225, MINPDF_ALIGN_RIGHT}};
  minpdf_table_options table = minpdf_table_defaults(columns, 2);
  minpdf_metadata metadata = {"minpdf Cross-Language Showcase", "Actile",
                              "Identical feature showcase for every binding",
                              "minpdf,pdf,showcase", "minpdf", "D:20260101000000Z"};
  minpdf_document *pdf = minpdf_create(MINPDF_A4);
  minpdf_text_options text = minpdf_text_defaults();
  minpdf_rect_options shape = minpdf_rect_defaults();
  minpdf_line_options line = minpdf_line_defaults();
  minpdf_image_options image = {64, 64};
  minpdf_link_options link = minpdf_link_defaults();
  int ok = pdf != NULL;
  if (!ok)
    return 1;

  /* Bindings express these defaults through rgb(), so set them explicitly to
     keep the serialized output byte-for-byte identical. */
  table.header_background = minpdf_rgb(240, 240, 240);
  table.header_color = minpdf_rgb(0, 0, 0);
  table.border_color = minpdf_rgb(204, 204, 204);

  ok &= check(pdf, minpdf_set_metadata(pdf, &metadata));
  text.size = 26; text.weight = MINPDF_WEIGHT_BOLD;
  text.color = minpdf_rgb(30, 64, 175);
  ok &= check(pdf, minpdf_text(pdf, "minpdf Showcase", 48, 62, &text));
  text.size = 11; text.weight = MINPDF_WEIGHT_NORMAL;
  text.color = minpdf_rgb(71, 85, 105);
  ok &= check(pdf, minpdf_text(pdf, "One layout, four language bindings", 48, 84, &text));

  shape.has_fill = shape.has_stroke = 1;
  shape.fill = minpdf_rgb(239, 246, 255);
  shape.stroke = minpdf_rgb(147, 197, 253);
  shape.line_width = 1; shape.radius = 10;
  ok &= check(pdf, minpdf_rect(pdf, 48, 110, 499, 100, &shape));
  shape.fill = minpdf_rgb(37, 99, 235);
  shape.stroke = minpdf_rgb(30, 64, 175);
  ok &= check(pdf, minpdf_circle(pdf, 100, 160, 28, &shape));
  text.size = 14; text.weight = MINPDF_WEIGHT_BOLD;
  text.color = minpdf_rgb(30, 41, 59);
  ok &= check(pdf, minpdf_text(pdf, "Shapes, fills, strokes and rounded corners", 148, 150, &text));
  text.size = 10; text.weight = MINPDF_WEIGHT_ITALIC;
  text.color = minpdf_rgb(71, 85, 105);
  ok &= check(pdf, minpdf_text(pdf, "All coordinates and styles match across examples.", 148, 174, &text));

  line.color = minpdf_rgb(148, 163, 184); line.width = 1;
  ok &= check(pdf, minpdf_line(pdf, 48, 232, 547, 232, &line));
  text.size = 16; text.weight = MINPDF_WEIGHT_BOLD;
  text.color = minpdf_rgb(15, 23, 42);
  ok &= check(pdf, minpdf_text(pdf, "Team", 48, 264, &text));
  ok &= check(pdf, minpdf_table(pdf, cells, 3, 48, 282, &table));

  text.size = 16;
  ok &= check(pdf, minpdf_text(pdf, "Embedded RGBA image", 48, 430, &text));
  ok &= check(pdf, minpdf_image(pdf, PNG_RGBA, sizeof PNG_RGBA, 48, 450, &image));
  text.size = 11; text.weight = MINPDF_WEIGHT_NORMAL;
  text.color = minpdf_rgb(71, 85, 105);
  ok &= check(pdf, minpdf_text(pdf, "The red square is a scaled 1x1 PNG with alpha.", 128, 486, &text));

  link.color = minpdf_rgb(37, 99, 235); link.size = 12; link.underline = 1;
  ok &= check(pdf, minpdf_link(pdf, "Visit minpdf documentation", "https://github.com/actiledev/minpdf", 48, 550, &link));
  text.size = 12; text.align = MINPDF_ALIGN_CENTER; text.weight = MINPDF_WEIGHT_BOLD;
  text.color = minpdf_rgb(15, 23, 42);
  ok &= check(pdf, minpdf_text(pdf, "Centered text", 297.5f, 610, &text));
  text.align = MINPDF_ALIGN_RIGHT; text.weight = MINPDF_WEIGHT_BOLD_ITALIC;
  ok &= check(pdf, minpdf_text(pdf, "Right-aligned bold italic", 547, 635, &text));

  ok &= check(pdf, minpdf_add_page(pdf, MINPDF_LETTER));
  text.align = MINPDF_ALIGN_LEFT; text.weight = MINPDF_WEIGHT_BOLD;
  text.size = 24; text.color = minpdf_rgb(30, 64, 175);
  ok &= check(pdf, minpdf_text(pdf, "Page 2", 48, 62, &text));
  text.weight = MINPDF_WEIGHT_NORMAL; text.size = 12;
  text.color = minpdf_rgb(51, 65, 85);
  ok &= check(pdf, minpdf_text(pdf, "This Letter-sized page demonstrates mixed page sizes.", 48, 92, &text));
  ok &= check(pdf, minpdf_save(pdf, "examples/results/minpdf-showcase-c.pdf"));
  minpdf_destroy(pdf);
  return ok ? 0 : 1;
}
