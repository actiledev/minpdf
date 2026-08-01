#define MINPDF_IMPLEMENTATION
#include "../include/minpdf.h"
#include <stdio.h>
#include <time.h>

typedef void (*bench_fn)(void);

static volatile size_t output_bytes;

static void finish(minpdf_document *doc) {
  minpdf_buffer out = {0};
  if (minpdf_build(doc, &out) == MINPDF_OK)
    output_bytes += out.length;
  minpdf_buffer_free(&out);
  minpdf_destroy(doc);
}

static void text_document(void) {
  minpdf_document *doc = minpdf_create(MINPDF_A4);
  int row;
  for (row = 0; row < 100; row++)
    minpdf_text(doc, "Benchmark row", 40, 30 + row * 7, NULL);
  finish(doc);
}

static void wrapped_document(void) {
  minpdf_document *doc = minpdf_create(MINPDF_A4);
  minpdf_text_options text = minpdf_text_defaults();
  int row;
  text.max_width = 220;
  for (row = 0; row < 30; row++)
    minpdf_text(doc, "A representative wrapped sentence with several words",
                40, 30 + row * 18, &text);
  finish(doc);
}

static void multipage_document(void) {
  minpdf_document *doc = minpdf_create(MINPDF_A4);
  int page, row;
  for (page = 0; page < 10; page++) {
    if (page)
      minpdf_add_page(doc, MINPDF_A4);
    for (row = 0; row < 20; row++)
      minpdf_text(doc, "Multipage benchmark", 40, 30 + row * 18, NULL);
  }
  finish(doc);
}

static void table_document(void) {
  const char *cells[60];
  minpdf_table_column columns[] = {{"Name", 160, MINPDF_ALIGN_LEFT},
                                   {"Role", 160, MINPDF_ALIGN_LEFT},
                                   {"Score", 80, MINPDF_ALIGN_RIGHT}};
  minpdf_table_options table = minpdf_table_defaults(columns, 3);
  minpdf_document *doc = minpdf_create(MINPDF_A4);
  int i;
  for (i = 0; i < 60; i += 3) {
    cells[i] = "Ada Lovelace";
    cells[i + 1] = "Engineer";
    cells[i + 2] = "100";
  }
  minpdf_table(doc, cells, 20, 40, 40, &table);
  finish(doc);
}

static void png_document(void) {
  static const unsigned char png[] = {
      0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,
      0x49,0x48,0x44,0x52,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,
      0x08,0x06,0x00,0x00,0x00,0x1f,0x15,0xc4,0x89,0x00,0x00,0x00,
      0x0d,0x49,0x44,0x41,0x54,0x78,0xda,0x63,0xfc,0xcf,0xc0,0x50,
      0x0f,0x00,0x05,0x83,0x02,0x7f,0x96,0xa2,0xfc,0x59,0x00,0x00,
      0x00,0x00,0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82};
  minpdf_document *doc = minpdf_create(MINPDF_A4);
  minpdf_image(doc, png, sizeof png, 40, 40, NULL);
  finish(doc);
}

static double run(bench_fn fn, int iterations) {
  clock_t start = clock();
  int i;
  for (i = 0; i < iterations; i++)
    fn();
  return (double)(clock() - start) / CLOCKS_PER_SEC;
}

static void benchmark(const char *name, bench_fn fn, int iterations) {
  double samples[7], median;
  int i, j;
  (void)run(fn, iterations / 10 + 1);
  for (i = 0; i < 7; i++)
    samples[i] = run(fn, iterations);
  for (i = 1; i < 7; i++) {
    double value = samples[i];
    for (j = i; j > 0 && samples[j - 1] > value; j--)
      samples[j] = samples[j - 1];
    samples[j] = value;
  }
  median = samples[3];
  printf("%-10s %5d documents: %.3f seconds\n", name, iterations, median);
}

int main(void) {
  benchmark("text", text_document, 1000);
  benchmark("wrapped", wrapped_document, 300);
  benchmark("multipage", multipage_document, 300);
  benchmark("table", table_document, 300);
  benchmark("png", png_document, 1000);
  printf("output bytes consumed: %zu\n", output_bytes);
  return 0;
}
