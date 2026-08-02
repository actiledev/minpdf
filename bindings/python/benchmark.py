"""Emit a reproducible end-to-end Python benchmark as JSON."""

import json
import platform
import statistics
import time
import tracemalloc

from minpdf import PDF


ITERATIONS = 500
SAMPLES = 7


def document() -> int:
    pdf = PDF()
    for row in range(100):
        pdf.text("Benchmark row", 40, 30 + row * 7)
    return len(pdf.bytes())


def run() -> tuple[float, int, int]:
    output_bytes = 0
    started = time.perf_counter()
    for _ in range(ITERATIONS):
        output_bytes += document()
    return time.perf_counter() - started, output_bytes, tracemalloc.get_traced_memory()[1]


document()
tracemalloc.start()
samples = [run() for _ in range(SAMPLES)]
tracemalloc.stop()
times = [sample[0] for sample in samples]
print(json.dumps({
    "schema_version": 1,
    "runtime": platform.python_version(),
    "benchmark": "text",
    "iterations": ITERATIONS,
    "median_seconds": statistics.median(times),
    "python_peak_bytes": max(sample[2] for sample in samples),
    "output_bytes": samples[0][1],
}, sort_keys=True))
