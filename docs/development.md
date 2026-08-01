# Performance and development

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
```

`make test` checks C11 output and C++ linkage. The Go and Python tests exercise
their native bindings. The JavaScript build type-checks the wrapper and compiles
the C engine/bridge through Emscripten.

For a sanitizer build:

```sh
cc -std=c11 -O1 -g -fsanitize=address,undefined \
  tests/test_core.c -o /tmp/minpdf-test
/tmp/minpdf-test
```

## Benchmarks

```sh
make benchmark
go test -bench BenchmarkDocument -benchmem ./...
```

Compare warmed medians on the same hardware. Track both elapsed time and peak
memory for small, multipage, table-heavy, image-heavy, and Unicode PDFs.

## Release artifacts

A complete release consists of:

- source archive containing `include/minpdf.h` and `core.c`
- npm package containing generated JavaScript, declarations, and `.wasm`
- CPython 3.9+ abi3 wheels for supported platforms plus an sdist
- tagged Go module at `github.com/actiledev/minpdf`

The intended first-class matrix is Linux x64/arm64, macOS x64/arm64, Windows
x64, and evergreen browsers.

## Publishing a release

Update the version in `pyproject.toml`, `bindings/javascript/package.json`, and
`bindings/javascript/package-lock.json`. Push a matching tag such as `v0.1.0`.
The release workflow rejects tags that do not match all three files.

The workflow builds the npm package, a Python source distribution, and CPython
3.9 ABI3 wheels for Linux x64/arm64, macOS x64/arm64, and Windows x64. GitHub
environments named `npm` and `pypi` control the publication jobs. Configure both
environments with required reviewers before creating a release tag.

PyPI can create `minpdf` through a pending trusted publisher. Configure its
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
