import { performance } from "node:perf_hooks";
import process from "node:process";

import { PDF } from "../dist/index.js";

const iterations = 500;
const samples = 7;

async function document() {
  const pdf = await PDF.create();
  try {
    for (let row = 0; row < 100; row++) {
      pdf.text("Benchmark row", 40, 30 + row * 7);
    }
    return pdf.bytes().length;
  } finally {
    pdf.close();
  }
}

async function run() {
  let outputBytes = 0;
  let peakHeapBytes = process.memoryUsage().heapUsed;
  const started = performance.now();
  for (let index = 0; index < iterations; index++) {
    outputBytes += await document();
    peakHeapBytes = Math.max(peakHeapBytes, process.memoryUsage().heapUsed);
  }
  return { seconds: (performance.now() - started) / 1000, outputBytes, peakHeapBytes };
}

await document();
const results = [];
for (let index = 0; index < samples; index++) results.push(await run());
const times = results.map((result) => result.seconds).sort((a, b) => a - b);
process.stdout.write(`${JSON.stringify({
  schema_version: 1,
  runtime: process.version,
  benchmark: "text",
  iterations,
  median_seconds: times[Math.floor(times.length / 2)],
  javascript_peak_heap_bytes: Math.max(...results.map((result) => result.peakHeapBytes)),
  output_bytes: results[0].outputBytes,
})}\n`);
