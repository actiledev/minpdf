CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Wpedantic

.PHONY: all test benchmark fuzz-smoke conformance clean
all: build/minpdf-showcase

build:
	mkdir -p build

build/minpdf-showcase: examples/c/showcase.c include/minpdf.h | build
	$(CC) $(CFLAGS) $< -o $@

build/test-core: tests/test_core.c include/minpdf.h | build
	$(CC) $(CFLAGS) $< -o $@

test: build/test-core
	./build/test-core
	$(CC) $(CFLAGS) -c core.c -o build/core.o
	$(CXX) -std=c++17 -Wall -Wextra -Wpedantic tests/test_cpp.cpp build/core.o -o build/test-cpp
	./build/test-cpp

build/benchmark: tests/benchmark.c include/minpdf.h | build
	$(CC) $(CFLAGS) $< -o $@

benchmark: build/benchmark
	./build/benchmark

build/fuzz-parsers: tests/fuzz.c include/minpdf.h | build
	clang -std=c11 -O1 -g -fsanitize=fuzzer,address,undefined \
		-DMINPDF_FUZZ_PARSERS $< -o $@

build/fuzz-serializer: tests/fuzz.c include/minpdf.h | build
	clang -std=c11 -O1 -g -fsanitize=fuzzer,address,undefined \
		-DMINPDF_FUZZ_SERIALIZER $< -o $@

build/fuzz-parsers-smoke: tests/fuzz.c include/minpdf.h | build
	$(CC) -std=c11 -O1 -g -fsanitize=address,undefined \
		-DMINPDF_FUZZ_PARSERS -DMINPDF_FUZZ_STANDALONE $< -o $@

build/fuzz-serializer-smoke: tests/fuzz.c include/minpdf.h | build
	$(CC) -std=c11 -O1 -g -fsanitize=address,undefined \
		-DMINPDF_FUZZ_SERIALIZER -DMINPDF_FUZZ_STANDALONE $< -o $@

fuzz-smoke: build/fuzz-parsers-smoke build/fuzz-serializer-smoke
	./build/fuzz-parsers-smoke
	./build/fuzz-serializer-smoke

build/conformance-native: tests/conformance.c include/minpdf.h | build
	$(CC) $(CFLAGS) $< -o $@

build/conformance-wasm.js: tests/conformance.c include/minpdf.h | build
	emcc $< -O2 -s ENVIRONMENT=node -s FILESYSTEM=0 \
		-s EXIT_RUNTIME=1 -o $@

conformance: build/conformance-native build/conformance-wasm.js
	./build/conformance-native > build/conformance-native.out
	node build/conformance-wasm.js > build/conformance-wasm.out
	cmp build/conformance-native.out build/conformance-wasm.out

clean:
	rm -rf build
