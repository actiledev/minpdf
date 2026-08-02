"""Check deterministic C benchmark memory/output budgets."""

import json
import sys
from pathlib import Path


if len(sys.argv) != 3:
    raise SystemExit("usage: benchmark_budget.py RESULTS BASELINE")

results = json.loads(Path(sys.argv[1]).read_text())
baseline = json.loads(Path(sys.argv[2]).read_text())
expected = {item["name"]: item for item in baseline["benchmarks"]}
failures: list[str] = []

for item in results["benchmarks"]:
    reference = expected.get(item["name"])
    if reference is None:
        failures.append(f"missing baseline for {item['name']}")
        continue
    memory_limit = int(reference["peak_live_bytes"] * 1.10) + 64
    if item["peak_live_bytes"] > memory_limit:
        failures.append(
            f"{item['name']} peak memory {item['peak_live_bytes']} exceeds {memory_limit}"
        )
    if item["output_bytes"] != reference["output_bytes"]:
        failures.append(
            f"{item['name']} output bytes changed from {reference['output_bytes']} "
            f"to {item['output_bytes']}"
        )
    timing_delta = (item["median_seconds"] / reference["median_seconds"] - 1) * 100
    print(
        f"{item['name']}: {item['median_seconds']:.6f}s "
        f"({timing_delta:+.1f}%), {item['peak_live_bytes']} peak bytes"
    )

if failures:
    raise SystemExit("\n".join(failures))
