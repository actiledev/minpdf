CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Wpedantic

.PHONY: all test benchmark clean
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

clean:
	rm -rf build
