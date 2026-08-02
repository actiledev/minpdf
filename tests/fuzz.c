#define MINPDF_IMPLEMENTATION
#include "../include/minpdf.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static char *fuzz_string(const uint8_t *data, size_t size) {
  char *value = (char *)malloc(size + 1);
  if (!value)
    return NULL;
  if (size)
    memcpy(value, data, size);
  value[size] = '\0';
  return value;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  minpdf_document *document;
  minpdf_buffer output = {0};

  if (size > 1024 * 1024)
    return 0;
  document = minpdf_create(MINPDF_A4);
  if (!document)
    return 0;

#if defined(MINPDF_FUZZ_PARSERS)
  if (size && (data[0] & 1))
    (void)minpdf_register_font(document, "fuzz", data + 1, size - 1);
  else
    (void)minpdf_image(document, data, size, 0, 0, NULL);
#elif defined(MINPDF_FUZZ_SERIALIZER)
  {
    size_t cursor = 0;
    unsigned operations = 0;
    while (cursor < size && operations++ < 64) {
      uint8_t opcode = data[cursor++];
      size_t remaining = size - cursor;
      size_t length = remaining > 128 ? 128 : remaining;
      char *value = fuzz_string(data + cursor, length);
      minpdf_text_options text = minpdf_text_defaults();
      minpdf_metadata metadata = {0};
      if (!value)
        break;
      switch (opcode % 6) {
      case 0:
        (void)minpdf_text(document, value, (float)(opcode * 2),
                          (float)operations, &text);
        break;
      case 1:
        metadata.title = value;
        metadata.author = value;
        (void)minpdf_set_metadata(document, &metadata);
        break;
      case 2:
        (void)minpdf_link(document, value, value, 1, 2, NULL);
        break;
      case 3:
        (void)minpdf_rect(document, 1, 2, opcode, operations, NULL);
        break;
      case 4:
        (void)minpdf_add_page(document,
                              (minpdf_size){100 + opcode, 100 + opcode});
        break;
      default:
        (void)minpdf_build(document, &output);
        minpdf_buffer_free(&output);
        break;
      }
      free(value);
      cursor += length;
    }
  }
#else
#error "select a fuzz target"
#endif

  (void)minpdf_build(document, &output);
  minpdf_buffer_free(&output);
  minpdf_destroy(document);
  return 0;
}

#ifdef MINPDF_FUZZ_STANDALONE
int main(void) {
  uint8_t input[512] = {0};
  unsigned run;
  size_t index;
  for (run = 0; run < 1000; run++) {
    for (index = 0; index < sizeof input; index++)
      input[index] = (uint8_t)(run * 33u + index * 17u + (run >> 3));
    (void)LLVMFuzzerTestOneInput(input, run % sizeof input);
  }
  return 0;
}
#endif
