<div align="center">

# minpdf

Ultra fast, zero depedency, portable PDF engine written in C for JS/TS, client browser, Python, Golang and more.

[Documentation](docs/README.md) · [Installation](docs/installation.md) · [API](docs/api.md) · [Roadmap](docs/roadmap.md) · [Contributing](CONTRIBUTING.md)

[![CI](https://github.com/actiledev/minpdf/actions/workflows/ci.yml/badge.svg)](https://github.com/actiledev/minpdf/actions/workflows/ci.yml)
[![npm](https://img.shields.io/npm/v/minpdf?logo=npm&logoColor=white)](https://www.npmjs.com/package/minpdf)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

[![C](https://img.shields.io/badge/C-00599C?logo=c&logoColor=white)](docs/c.md)
[![C++](https://img.shields.io/badge/C++-%2300599C.svg?logo=c%2B%2B&logoColor=white)](docs/c.md)
[![Go](https://img.shields.io/badge/Go-%2300ADD8.svg?&logo=go&logoColor=white)](docs/go.md)
[![Python](https://img.shields.io/badge/Python-3776AB?logo=python&logoColor=fff)](docs/python.md)
[![Javascript](https://img.shields.io/badge/JavaScript-F7DF1E?logo=javascript&logoColor=000)](docs/javascript.md)
[![TypeScript](https://img.shields.io/badge/TypeScript-3178C6?logo=typescript&logoColor=fff)](docs/javascript.md)
[![WebAssembly](https://img.shields.io/badge/WebAssembly-654FF0?logo=webassembly&logoColor=fff)](docs/browser.md)

</div>

> [!WARNING]
> minpdf is under development. APIs may change.

The bindings use the same PDF engine and follow the same API. The library has no third-party runtime dependencies.

## Installation

### JavaScript and TypeScript

```sh
npm install minpdf
```

Node.js 18+, Bun, modern browsers, workers, and ESM bundlers are supported.

### Python

```sh
python -m pip install minpdf-core
```

Python 3.9+

### Go

```sh
go get github.com/actiledev/minpdf
```

Requires `CGO_ENABLED=1` and a C compiler.

### C and C++

Copy [`include/minpdf.h`](include/minpdf.h) into your project and define `MINPDF_IMPLEMENTATION` in one C translation unit, or compile [`core.c`](core.c) as a library.

```sh
cc -std=c11 -O2 app.c -o app
```

See the [installation guide](docs/installation.md) for more details.

## Example

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

  await doc.save("hello.pdf");
} finally {
  doc.close();
}
```

Examples for other targets:

- [C and C++](docs/c.md)
- [Go](docs/go.md)
- [Python](docs/python.md)
- [JavaScript and TypeScript](docs/javascript.md)
- [Browsers](docs/browser.md)

## Features

- PDF 1.4 output
- A3, A4, A5, Letter, and custom page sizes
- Text alignment and word wrapping
- Built-in Helvetica fonts and embedded TrueType fonts
- UTF-8 text with searchable Unicode maps
- Rectangles, circles, lines, and simple tables
- JPEG and non-interlaced 8-bit PNG images
- Links, metadata, and multiple pages
- File output, in-memory bytes, browser blobs, and downloads

minpdf creates new PDFs. It does not read, edit, merge, encrypt, sign, or render existing PDFs. See the [roadmap](docs/roadmap.md) for planned work.

## Documentation

- [Documentation index](docs/README.md)
- [Core concepts](docs/concepts.md)
- [Shared API](docs/api.md)
- [Text and fonts](docs/text-and-fonts.md)
- [Images](docs/images.md)
- [Tables](docs/tables.md)
- [Output and errors](docs/output-and-errors.md)
- [Performance and development](docs/development.md)

## Contributing

Read [CONTRIBUTING.md](CONTRIBUTING.md) before opening a pull request. This project uses the [Contributor Covenant](CODE_OF_CONDUCT.md). Report vulnerabilities as described in [SECURITY.md](SECURITY.md).

Licensed under the [MIT License](LICENSE).
