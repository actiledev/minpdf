#!/bin/sh
set -eu

if ! command -v emcc >/dev/null 2>&1; then
  echo "error: Emscripten compiler 'emcc' was not found." >&2
  echo "Install it on macOS with: brew install emscripten" >&2
  echo "Official SDK instructions: https://emscripten.org/docs/getting_started/downloads.html" >&2
  exit 127
fi

mkdir -p dist
emcc ../../core.c wasm/bridge.c -I../../include -Oz -flto \
  -s MODULARIZE=1 -s EXPORT_ES6=1 -s ENVIRONMENT=web,worker,node \
  -s ALLOW_MEMORY_GROWTH=1 -s FILESYSTEM=0 -s MALLOC=emmalloc \
  -s EXPORTED_RUNTIME_METHODS='["UTF8ToString","lengthBytesUTF8","stringToUTF8","HEAPU8","HEAPU32","HEAPF32"]' \
  -s EXPORTED_FUNCTIONS='["_malloc","_free","_mpw_create","_mpw_destroy","_mpw_error","_mpw_page","_mpw_text","_mpw_rect","_mpw_line","_mpw_circle","_mpw_image","_mpw_font","_mpw_link","_mpw_metadata","_mpw_build","_mpw_free"]' \
  -o dist/engine.js
