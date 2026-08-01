# Installation

## C and C++

The simplest distribution is the amalgamated header:

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

## Go

```sh
go get github.com/actiledev/minpdf
```

The Go binding uses cgo. `CGO_ENABLED=1` and a working C compiler are required.
No third-party Go module is used.

## Python

```sh
pip install minpdf
```

```sh
python -m pip install minpdf
```

With uv:

```sh
uv add minpdf
```

```sh
uv pip install minpdf
```

Prebuilt abi3 wheels target CPython 3.9 and newer. Building from the source
distribution requires a C compiler and setuptools. minpdf has no Python runtime
dependencies.

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
