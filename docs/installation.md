# Installation

[Docs](README.md) · [C/C++](c.md) · [Go](go.md) · [Python](python.md) · [JavaScript](javascript.md) · [Browser](browser.md)

Install the package for your language. All packages use the same C engine.

## C and C++

Copy `include/minpdf.h` into your project and compile with C11:

```sh
cc -std=c11 -O2 app.c -o app
```

Define `MINPDF_IMPLEMENTATION` in exactly one translation unit. Alternatively,
compile `core.c` and link it into the application. CMake and pkg-config install
metadata are included:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix /usr/local
```

Example: [`examples/c`](../examples/c)

## Go

```sh
go get github.com/actiledev/minpdf
```

The Go binding uses cgo. `CGO_ENABLED=1` and a working C compiler are required.
No third-party Go module is used.

Example: [`examples/go`](../examples/go)

## Python

The first `minpdf-core` PyPI release is being prepared. Until it is published,
install the package directly from the repository:

```sh
python -m pip install "minpdf-core @ git+https://github.com/actiledev/minpdf.git"
```

After the PyPI release, either of these commands will install the distribution:

```sh
pip install minpdf-core
python -m pip install minpdf-core
```

With uv:

```sh
uv add minpdf-core
```

```sh
uv pip install minpdf-core
```

The release workflow builds abi3 wheels targeting CPython 3.9 and newer. The
distribution name is `minpdf-core`; Python code imports it as `minpdf`. Building
from the source distribution requires a C compiler and setuptools. minpdf has
no Python runtime dependencies.

Example: [`examples/python`](../examples/python)

## JavaScript, TypeScript, and browsers

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

The package contains the C engine compiled to WebAssembly and TypeScript
declarations. Node.js 18+, Bun, modern ESM bundlers, workers, and evergreen
browsers are supported. There are no runtime npm dependencies.

Example: [`examples/javascript`](../examples/javascript)

## Building the repository

```sh
make test
go test ./...
python -m pip install -e .

cd bindings/javascript
npm install
npm run build
```

The JavaScript build requires Emscripten (`emcc`). Compilers, setuptools,
TypeScript, and test tools are development dependencies, not runtime
dependencies of an installed library.
