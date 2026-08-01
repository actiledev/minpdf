# C and C++ guide

[Docs](README.md) · [Installation](installation.md) · [API](api.md) · [C example](../examples/c)

## Install

Copy [`include/minpdf.h`](../include/minpdf.h) into your project. Define `MINPDF_IMPLEMENTATION` in one C translation unit:

```c
#define MINPDF_IMPLEMENTATION
#include "minpdf.h"
```

Other files include `minpdf.h` without the macro. For C++, compile the implementation as C11 and link it with the C++ application.

The full example is in [`examples/c`](../examples/c). Run it from the repository root:

```sh
make
./build/minpdf-showcase
```

## Basic example

```c
#define MINPDF_IMPLEMENTATION
#include "minpdf.h"
#include <stdio.h>
#include <stdlib.h>

static int check(minpdf_document *doc, minpdf_status status) {
  if (status == MINPDF_OK) return 1;
  fprintf(stderr, "minpdf: %s\n", minpdf_error(doc));
  return 0;
}

int main(void) {
  minpdf_document *doc = minpdf_create(MINPDF_A4);
  if (!doc) return 1;

  minpdf_metadata metadata = {0};
  metadata.title = "Example report";
  metadata.author = "Actile";
  if (!check(doc, minpdf_set_metadata(doc, &metadata))) goto fail;

  minpdf_text_options title = minpdf_text_defaults();
  title.size = 24;
  title.weight = MINPDF_WEIGHT_BOLD;
  if (!check(doc, minpdf_text(doc, "Example report", 50, 55, &title))) goto fail;

  minpdf_rect_options panel = minpdf_rect_defaults();
  panel.has_fill = 1;
  panel.fill = minpdf_rgb(240, 244, 248);
  panel.radius = 8;
  if (!check(doc, minpdf_rect(doc, 50, 85, 495, 80, &panel))) goto fail;

  if (!check(doc, minpdf_save(doc, "report.pdf"))) goto fail;
  minpdf_destroy(doc);
  return 0;

fail:
  minpdf_destroy(doc);
  return 1;
}
```

## Options and defaults

Always initialize option structs with their default helpers. Zero-initializing
is not equivalent for every option:

```c
minpdf_line_options line = minpdf_line_defaults();
line.color = minpdf_rgb(231, 76, 60);
line.width = 2;
float dash[] = {6, 3};
line.dash = dash;
line.dash_count = 2;
minpdf_line(doc, 50, 200, 300, 200, &line);
```

Passing `NULL` for an options pointer uses engine defaults.

## Text and fonts

```c
FILE *file = fopen("NotoSans-Regular.ttf", "rb");
fseek(file, 0, SEEK_END);
long length = ftell(file);
rewind(file);
unsigned char *font_data = malloc((size_t)length);
fread(font_data, 1, (size_t)length, file);
fclose(file);

check(doc, minpdf_register_font(doc, "body", font_data, (size_t)length));
free(font_data); /* minpdf copied it */

minpdf_text_options body = minpdf_text_defaults();
body.font = "body";
body.max_width = 400;
check(doc, minpdf_text(doc, "UTF-8: Héllo, κόσμε", 50, 240, &body));
```

## Tables

Cells are a row-major array with `rows × column_count` entries:

```c
minpdf_table_column columns[] = {
  {"Item", 240, MINPDF_ALIGN_LEFT},
  {"Amount", 100, MINPDF_ALIGN_RIGHT},
};
const char *cells[] = {"Design", "$500", "Development", "$1,200"};
minpdf_table_options table = minpdf_table_defaults(columns, 2);
check(doc, minpdf_table(doc, cells, 2, 50, 300, &table));
```

## Memory output

```c
minpdf_buffer output = {0};
if (check(doc, minpdf_build(doc, &output))) {
  fwrite(output.data, 1, output.length, stdout);
  minpdf_buffer_free(&output);
}
```

Every successful `minpdf_build` buffer must be released with
`minpdf_buffer_free`. `minpdf_save` manages its temporary buffer internally.

## Status values

- `MINPDF_ERROR_ARGUMENT`: invalid size, pointer, name, or duplicate font
- `MINPDF_ERROR_MEMORY`: allocation failed
- `MINPDF_ERROR_FORMAT`: malformed UTF-8, font, image, or PNG stream
- `MINPDF_ERROR_UNSUPPORTED`: unsupported glyph, image variant, or font limit
- `MINPDF_ERROR_IO`: file open/write failed
- `MINPDF_ERROR_STATE`: reserved for invalid PDF state

Call `minpdf_error(doc)` after a failure to read the error message.
