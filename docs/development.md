# Performance and development

[Docs](README.md) · [Contributing](../CONTRIBUTING.md) · [Roadmap](roadmap.md) · [Security](../SECURITY.md)

## Performance model

The engine stores compact page commands and resources, then serializes once.
Buffers grow geometrically to avoid repeated full copies. JPEG data passes
through unchanged; transparent PNG and custom-font processing require more work.

For best results:

- Reuse one registered font name rather than registering duplicates.
- Prefer JPEG for large photographic images.
- Resize oversized images before adding them.
- Build once after all pages are complete.
- Close or destroy PDF instances when finished.
- Avoid creating many tiny PDF instances when one multipage PDF works.

## Tests

```sh
make test
GOCACHE=/tmp/minpdf-go-cache go test ./...

python -m pip install -e .
python -m pytest bindings/python

cd bindings/javascript
npm install
npm run build
npm test
```

`make test` checks C11 output and C++ linkage. The Go and Python tests exercise
their native bindings. The JavaScript build type-checks the wrapper and compiles
the C engine/bridge through Emscripten.

Run short sanitizer fuzzing and native/Wasm conformance checks with:

```sh
make fuzz-smoke
make conformance
```

The C fuzzers cover image and font parsers plus bounded serializer operation
sequences. Go fuzz targets cross cgo; Python Hypothesis and JavaScript fast-check
tests cover their extension and Wasm boundaries. Pull requests run bounded
smoke cases. `.github/workflows/quality.yml` runs deeper cases daily at 03:00
UTC and can be dispatched manually. Failed reproducers are retained as workflow
artifacts.

For a sanitizer build:

```sh
cc -std=c11 -O1 -g -fsanitize=address,undefined \
  tests/test_core.c -o /tmp/minpdf-test
/tmp/minpdf-test
```

## Benchmarks

```sh
make benchmark
./build/benchmark --json
go test -bench BenchmarkDocument -benchmem ./...
python bindings/python/benchmark.py

cd bindings/javascript
node benchmark/benchmark.mjs
```

Compare warmed medians on the same hardware. Track both elapsed time and peak
memory for small, multipage, table-heavy, image-heavy, and Unicode PDFs.
The C benchmark uses allocator overrides to report deterministic peak live
engine memory. `tests/benchmark-baseline.json` records output and memory budgets;
`tests/benchmark_budget.py` permits 10 percent peak-memory growth and reports,
but does not fail on, timing deltas. Python and JavaScript VM memory is reported
without gating because hosted-runner and garbage-collector noise is not
deterministic. Nightly results are retained for 90 days.

Update the baseline only for an intentional engine change. Run the complete
test and conformance suites, capture a release build with
`./build/benchmark --json`, review every delta, and commit the replacement
baseline with the change.

## Release artifacts

A complete release consists of:

- source archive containing `include/minpdf.h` and `core.c`
- npm package containing generated JavaScript, declarations, and `.wasm`
- GitHub npm package published as `@actiledev/minpdf` and linked to this repository
- CPython 3.9+ abi3 wheels for supported platforms plus an sdist
- tagged Go module at `github.com/actiledev/minpdf`

The intended first-class matrix is Linux x64/arm64, macOS x64/arm64, Windows
x64, and evergreen browsers.

Public API stability and coordinated versioning rules are defined in
[Compatibility and change management](compatibility.md).

## Publishing a release

Update the version in `pyproject.toml`, `bindings/javascript/package.json`, and
`bindings/javascript/package-lock.json`. Commit and push those changes, then tag
that same commit with a matching tag such as `v0.1.0`. The release workflow
rejects tags that do not match all three files.

The workflow builds the npm package, a Python source distribution, and CPython
3.9 ABI3 wheels for Linux x64/arm64, macOS x64/arm64, and Windows x64. It
publishes JavaScript releases to npm and GitHub Packages. The GitHub package uses
the workflow repository's `GITHUB_TOKEN`; its `packages: write` permission and
the package's `repository` metadata associate it with `actiledev/minpdf`. GitHub
environments named `npm` and `pypi` control the other publication jobs. Configure
both environments with required reviewers before creating a release tag.

PyPI can create `minpdf-core` through a pending trusted publisher. Configure its
GitHub owner as `actiledev`, its repository as `minpdf`, its workflow as
`release.yml`, and its environment as `pypi`.

npm requires the package to exist before it accepts a trusted publisher. For
the first release, enable 2FA on the owning npm account and run:

```sh
cd bindings/javascript
npm ci
npm run build
npm test
npm pack --dry-run
npm publish
```

Then configure the npm trusted publisher with owner `actiledev`, repository
`minpdf`, workflow `release.yml`, environment `npm`, and permission to run
`npm publish`. The workflow detects the manually published version and does not
upload it again when you push its tag.
