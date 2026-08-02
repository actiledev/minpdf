#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef union tracked_header {
  size_t size;
  max_align_t alignment;
} tracked_header;

static size_t live_bytes;
static size_t peak_bytes;

static void *tracked_malloc(size_t size) {
  tracked_header *header = (tracked_header *)malloc(sizeof *header + size);
  if (!header)
    return NULL;
  header->size = size;
  live_bytes += size;
  if (live_bytes > peak_bytes)
    peak_bytes = live_bytes;
  return header + 1;
}

static void tracked_free(void *pointer) {
  tracked_header *header;
  if (!pointer)
    return;
  header = (tracked_header *)pointer - 1;
  live_bytes -= header->size;
  free(header);
}

static void *tracked_calloc(size_t count, size_t size) {
  size_t total;
  void *pointer;
  if (size && count > SIZE_MAX / size)
    return NULL;
  total = count * size;
  pointer = tracked_malloc(total);
  if (pointer)
    memset(pointer, 0, total);
  return pointer;
}

static void *tracked_realloc(void *pointer, size_t size) {
  tracked_header *header;
  size_t previous = 0;
  if (!pointer)
    return tracked_malloc(size);
  header = (tracked_header *)pointer - 1;
  previous = header->size;
  header = (tracked_header *)realloc(header, sizeof *header + size);
  if (!header)
    return NULL;
  header->size = size;
  live_bytes = live_bytes - previous + size;
  if (live_bytes > peak_bytes)
    peak_bytes = live_bytes;
  return header + 1;
}

#define MINPDF_MALLOC tracked_malloc
#define MINPDF_REALLOC tracked_realloc
#define MINPDF_CALLOC tracked_calloc
#define MINPDF_FREE tracked_free
#define MINPDF_IMPLEMENTATION
#include "../include/minpdf.h"

typedef void (*bench_fn)(void);

typedef struct benchmark_case {
  const char *name;
  bench_fn function;
  int iterations;
} benchmark_case;

static volatile size_t output_bytes;

static void finish(minpdf_document *document) {
  minpdf_buffer output = {0};
  if (minpdf_build(document, &output) == MINPDF_OK)
    output_bytes += output.length;
  minpdf_buffer_free(&output);
  minpdf_destroy(document);
}

static void text_document(void) {
  minpdf_document *document = minpdf_create(MINPDF_A4);
  int row;
  for (row = 0; row < 100; row++)
    (void)minpdf_text(document, "Benchmark row", 40, 30 + row * 7, NULL);
  finish(document);
}

static void wrapped_document(void) {
  minpdf_document *document = minpdf_create(MINPDF_A4);
  minpdf_text_options text = minpdf_text_defaults();
  int row;
  text.max_width = 220;
  for (row = 0; row < 30; row++)
    (void)minpdf_text(document,
                      "A representative wrapped sentence with several words",
                      40, 30 + row * 18, &text);
  finish(document);
}

static void multipage_document(void) {
  minpdf_document *document = minpdf_create(MINPDF_A4);
  int page, row;
  for (page = 0; page < 10; page++) {
    if (page)
      (void)minpdf_add_page(document, MINPDF_A4);
    for (row = 0; row < 20; row++)
      (void)minpdf_text(document, "Multipage benchmark", 40, 30 + row * 18,
                        NULL);
  }
  finish(document);
}

static void table_document(void) {
  const char *cells[60];
  minpdf_table_column columns[] = {{"Name", 160, MINPDF_ALIGN_LEFT},
                                   {"Role", 160, MINPDF_ALIGN_LEFT},
                                   {"Score", 80, MINPDF_ALIGN_RIGHT}};
  minpdf_table_options table = minpdf_table_defaults(columns, 3);
  minpdf_document *document = minpdf_create(MINPDF_A4);
  int index;
  for (index = 0; index < 60; index += 3) {
    cells[index] = "Ada Lovelace";
    cells[index + 1] = "Engineer";
    cells[index + 2] = "100";
  }
  (void)minpdf_table(document, cells, 20, 40, 40, &table);
  finish(document);
}

static void png_document(void) {
  static const unsigned char png[] = {
      0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,
      0x49,0x48,0x44,0x52,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,
      0x08,0x06,0x00,0x00,0x00,0x1f,0x15,0xc4,0x89,0x00,0x00,0x00,
      0x0d,0x49,0x44,0x41,0x54,0x78,0xda,0x63,0xfc,0xcf,0xc0,0x50,
      0x0f,0x00,0x05,0x83,0x02,0x7f,0x96,0xa2,0xfc,0x59,0x00,0x00,
      0x00,0x00,0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82};
  minpdf_document *document = minpdf_create(MINPDF_A4);
  (void)minpdf_image(document, png, sizeof png, 40, 40, NULL);
  finish(document);
}

static double run(bench_fn function, int iterations) {
  clock_t start = clock();
  int index;
  for (index = 0; index < iterations; index++)
    function();
  return (double)(clock() - start) / CLOCKS_PER_SEC;
}

static void benchmark(const benchmark_case *item, int json, int first) {
  double samples[7], median;
  size_t measured_peak;
  int index, cursor;
  (void)run(item->function, item->iterations / 10 + 1);
  peak_bytes = live_bytes;
  output_bytes = 0;
  for (index = 0; index < 7; index++)
    samples[index] = run(item->function, item->iterations);
  measured_peak = peak_bytes;
  for (index = 1; index < 7; index++) {
    double value = samples[index];
    for (cursor = index; cursor > 0 && samples[cursor - 1] > value; cursor--)
      samples[cursor] = samples[cursor - 1];
    samples[cursor] = value;
  }
  median = samples[3];
  if (json) {
    printf("%s{\"name\":\"%s\",\"iterations\":%d,\"median_seconds\":%.6f,"
           "\"peak_live_bytes\":%zu,\"output_bytes\":%zu}",
           first ? "" : ",", item->name, item->iterations, median,
           measured_peak, (size_t)output_bytes);
  } else {
    printf("%-10s %5d documents: %.3f seconds, peak %zu bytes\n", item->name,
           item->iterations, median, measured_peak);
  }
}

int main(int argc, char **argv) {
  benchmark_case cases[] = {{"text", text_document, 1000},
                            {"wrapped", wrapped_document, 300},
                            {"multipage", multipage_document, 300},
                            {"table", table_document, 300},
                            {"png", png_document, 1000}};
  const char *selected = NULL;
  int json = 0, first = 1;
  size_t index;
  for (index = 1; index < (size_t)argc; index++) {
    if (strcmp(argv[index], "--json") == 0)
      json = 1;
    else
      selected = argv[index];
  }
  if (json)
    printf("{\"schema_version\":1,\"benchmarks\":[");
  for (index = 0; index < sizeof cases / sizeof cases[0]; index++) {
    if (!selected || strcmp(selected, cases[index].name) == 0) {
      benchmark(&cases[index], json, first);
      first = 0;
    }
  }
  if (json)
    printf("]}\n");
  if (selected && first) {
    fprintf(stderr, "unknown benchmark: %s\n", selected);
    return 2;
  }
  if (live_bytes != 0) {
    fprintf(stderr, "benchmark leaked %zu tracked bytes\n", live_bytes);
    return 3;
  }
  return 0;
}
