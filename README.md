# minpdf

**Fast, portable PDF generation from one zero-runtime-dependency C11 file.**

minpdf provides a compact, consistent PDF generation API for JavaScript/TypeScript,
browsers, Python, Go, C, and C++. Every binding uses the same `minpdf` engine,
so page geometry, font handling, image decoding, errors, and serialized output
consistent between languages.

```text
minpdf is still under active development, so the APIs are subject to change.
```

## Why minpdf?

- **One implementation:** PDF logic lives in the amalgamated C11 file.
- **Zero runtime dependencies:** no third-party runtime packages or linked
  libraries.
- **Cross-language design:** the same pages, coordinates, colors, and drawing
  operations in every supported language.
- **Native and browser-ready:** native bindings where appropriate and the same
  engine compiled to WebAssembly for browsers, workers, Node.js, and Bun.
- **Unicode-aware:** embedded TrueType fonts use real glyph metrics and
  searchable ToUnicode maps.
- **Small surface area:** create, draw, serialize, and release without a browser
  renderer or a large layout framework.
- **Deterministic:** identical calls and inputs produce identical output bytes
  when metadata is fixed.

## Installation

### JavaScript and TypeScript

Choose your package manager:

```sh
npm install minpdf
```

```sh
pnpm install minpdf
```

```sh
yarn add minpdf
```

```sh
bun add minpdf
```

Node.js 18+, Bun, modern ESM bundlers, workers, and evergreen browsers are
supported. The installed package has no runtime npm dependencies.

### Python

With pip:

```sh
pip install minpdf
```

Or through the current interpreter:

```sh
python -m pip install minpdf
```

With uv:

```sh
uv add minpdf
```

For a standalone environment managed by uv:

```sh
uv pip install minpdf
```

Prebuilt abi3 wheels support CPython 3.9 and newer. Source installations require
a C compiler and setuptools, but installed code has no Python runtime
dependencies.

### Go

```sh
go get github.com/actiledev/minpdf
```

Import the module as `minpdf`:

```go
import "github.com/actiledev/minpdf"
```

The Go package uses cgo and requires `CGO_ENABLED=1` plus a working C compiler.
It has no third-party Go module dependencies.

### C and C++

Copy [include/minpdf.h](include/minpdf.h) into your project and define
`MINPDF_IMPLEMENTATION` in exactly one C translation unit, or compile
[core.c](core.c) as a static/shared library.

```sh
cc -std=c11 -O2 app.c -o app
```

CMake and pkg-config installation metadata are also included.

## Quick start

### JavaScript / TypeScript

```ts
import { PDF, SIZES, rgb } from "minpdf";

const doc = await PDF.create(SIZES.A4);
try {
  doc
    .metadata({ title: "Hello minpdf", author: "minpdf" })
    .text("Hello, PDF!", 50, 55, { size: 24, weight: "bold" })
    .rect(50, 85, 240, 70, {
      fill: rgb(52, 152, 219),
      radius: 10,
    });

  await doc.save("hello.pdf"); // Node.js or Bun
  // doc.download("hello.pdf"); // Browser
} finally {
  doc.close();
}
```

### Python

```python
from minpdf import PDF, A4, TextOptions, RectOptions, rgb

with PDF(A4) as doc:
    (doc.metadata(title="Hello minpdf", author="minpdf")
        .text("Hello, PDF!", 50, 55,
              TextOptions(size=24, weight="bold"))
        .rect(50, 85, 240, 70,
              RectOptions(fill=rgb(52, 152, 219), radius=10)))
    doc.save("hello.pdf")
```

### Go

```go
package main

import (
    "log"
    "github.com/actiledev/minpdf"
)

func main() {
    doc, err := minpdf.New(minpdf.A4)
    if err != nil { log.Fatal(err) }
    defer doc.Close()

    if err := doc.Text("Hello, PDF!", 50, 55, &minpdf.TextOptions{
        Size: 24,
        Color: minpdf.Black,
        Weight: minpdf.WeightBold,
    }); err != nil { log.Fatal(err) }

    if err := doc.Save("hello.pdf"); err != nil { log.Fatal(err) }
}
```

### C

```c
#define MINPDF_IMPLEMENTATION
#include "minpdf.h"

int main(void) {
  minpdf_document *doc = minpdf_create(MINPDF_A4);
  if (!doc) return 1;

  minpdf_text_options title = minpdf_text_defaults();
  title.size = 24;
  title.weight = MINPDF_WEIGHT_BOLD;

  if (minpdf_text(doc, "Hello, PDF!", 50, 55, &title) != MINPDF_OK ||
      minpdf_save(doc, "hello.pdf") != MINPDF_OK) {
    minpdf_destroy(doc);
    return 1;
  }

  minpdf_destroy(doc);
  return 0;
}
```

## Feature matrix

| Capability                        | C/C++ | Go  | Python | Node/Bun | Browser |
| --------------------------------- | :---: | :-: | :----: | :------: | :-----: |
| Text and alignment                |  ✅   | ✅  |   ✅   |    ✅    |   ✅    |
| Word wrapping                     |  ✅   | ✅  |   ✅   |    ✅    |   ✅    |
| UTF-8 with embedded TTF           |  ✅   | ✅  |   ✅   |    ✅    |   ✅    |
| Searchable Unicode maps           |  ✅   | ✅  |   ✅   |    ✅    |   ✅    |
| Rectangles and rounded rectangles |  ✅   | ✅  |   ✅   |    ✅    |   ✅    |
| Circles and lines                 |  ✅   | ✅  |   ✅   |    ✅    |   ✅    |
| Dashed lines                      |  ✅   | ✅  |   ✅   |    ✅    |   ✅    |
| JPEG images                       |  ✅   | ✅  |   ✅   |    ✅    |   ✅    |
| PNG palette and transparency      |  ✅   | ✅  |   ✅   |    ✅    |   ✅    |
| Links and metadata                |  ✅   | ✅  |   ✅   |    ✅    |   ✅    |
| Simple tables                     |  ✅   | ✅  |   ✅   |    ✅    |   ✅    |
| Mixed-size pages                  |  ✅   | ✅  |   ✅   |    ✅    |   ✅    |
| In-memory bytes                   |  ✅   | ✅  |   ✅   |    ✅    |   ✅    |
| Direct filesystem save            |  ✅   | ✅  |   ✅   |    ✅    |    —    |
| Blob, object URL, and download    |   —   |  —  |   —    |    —     |   ✅    |
| Zero runtime dependencies         |  ✅   | ✅  |   ✅   |    ✅    |   ✅    |

## Comparison table

This release-planning table requires measured results and versioned audits before publishing performance claims.

| Library    | Core model      | Browser | Native bindings | Runtime dependencies | Bundle/package size |
| ---------- | --------------- | :-----: | :-------------: | :------------------: | ------------------: |
| **minpdf** | Single C11 file | ✅ WASM |  C, Go, Python  |          0           |                 TBD |
| jsPDF      | JavaScript      |   TBD   |       TBD       |         TBD          |                 TBD |
| pdf-lib    | JavaScript      |   TBD   |       TBD       |         TBD          |                 TBD |
| PDFKit     | JavaScript      |   TBD   |       TBD       |         TBD          |                 TBD |
| ReportLab  | Python          |   TBD   |       TBD       |         TBD          |                 TBD |
| gofpdf     | Go              |   TBD   |       TBD       |         TBD          |                 TBD |

## Benchmark comparison

The repository includes C and Go benchmark harnesses. Run benchmarks on identical fixtures and hardware before filling values.

| Scenario                    | minpdf C | minpdf Go | minpdf JS/WASM | minpdf Python | jsPDF | pdf-lib | PDFKit |
| --------------------------- | -------: | --------: | -------------: | ------------: | ----: | ------: | -----: |
| 1-page PDF with text        |      TBD |       TBD |            TBD |           TBD |   TBD |     TBD |    TBD |
| 100-row table               |      TBD |       TBD |            TBD |           TBD |   TBD |     TBD |    TBD |
| 10-page report              |      TBD |       TBD |            TBD |           TBD |   TBD |     TBD |    TBD |
| JPEG-heavy PDF              |      TBD |       TBD |            TBD |           TBD |   TBD |     TBD |    TBD |
| Unicode + embedded TTF      |      TBD |       TBD |            TBD |           TBD |   TBD |     TBD |    TBD |
| Peak memory: 10-page report |      TBD |       TBD |            TBD |           TBD |   TBD |     TBD |    TBD |
| Generated PDF size          |      TBD |       TBD |            TBD |           TBD |   TBD |     TBD |    TBD |

Run the included baselines with:

```sh
make benchmark
go test -bench BenchmarkDocument -benchmem ./...
```

See [Performance and development](docs/development.md) for the measurement
rules and release matrix.

## Supported formats and limits

- Produces PDF 1.4 files.
- Built-in Helvetica variants are ASCII-only; register a TrueType font for
  accented or non-Latin text.
- TrueType fonts use cmap mapping, glyph advances, and ToUnicode output.
- Complex-script shaping, bidi, kerning, fallback, and font subsetting are not
  implemented in version 0.1.0.
- JPEG is embedded without recompression.
- PNG supports non-interlaced 8-bit color types 0, 2, 3, 4, and 6, including
  palette and alpha transparency.
- Tables use fixed-height, single-line cells and do not paginate.
- minpdf generates new PDFs. Reading, editing, merging, encrypting, signing, or
  rendering existing PDFs is not supported.

## Architecture

```text
include/minpdf.h
├── PDF page model
├── PDF 1.4 serializer
├── UTF-8 and TrueType parser
├── JPEG metadata reader
├── PNG filters and internal DEFLATE decoder
└── C ABI
    ├── core.c                    C/C++ compiled library
    ├── minpdf.go                Go/cgo binding
    ├── bindings/python          CPython abi3 binding
    └── bindings/javascript      TypeScript + WebAssembly binding
```

Font and image inputs stay copied in engine memory until released. The engine
serializes the full PDF in memory before transferring bytes to host runtimes.

## Documentation

- [Roadmap](docs/roadmap.md)
- [Documentation](docs/README.md)
- [Installation](docs/installation.md)
- [Core concepts](docs/concepts.md)
- [Shared API reference](docs/api.md)
- [C and C++](docs/c.md)
- [Go](docs/go.md)
- [Python](docs/python.md)
- [JavaScript and TypeScript](docs/javascript.md)
- [Browser deployment](docs/browser.md)
- [Text and fonts](docs/text-and-fonts.md)
- [Images](docs/images.md)
- [Tables](docs/tables.md)
- [Output, errors, and ownership](docs/output-and-errors.md)
- [Complete invoice example](docs/example.md)

## Building from source

```sh
make test
go test ./...

python -m pip install -e .
python -m pytest bindings/python

cd bindings/javascript
npm install
npm run build
```

The JavaScript build requires Emscripten. Compiler, packaging, and test tooling
are development dependencies; installed libraries remain dependency-free at
runtime.

## Contributing

Please include tests for C engine behavior and every affected binding. Changes
to shared behavior should update the cross-language documentation and examples.
Run the native, Go, Python, and TypeScript checks before opening a pull request.

## License

MIT: see [LICENSE](LICENSE).
