# Contributing to minpdf

Thanks for contributing. Please follow the [Code of Conduct](CODE_OF_CONDUCT.md) and report vulnerabilities through the [security policy](SECURITY.md), not a public issue.

## Before starting

- Search the [issues](https://github.com/actiledev/minpdf/issues) first.
- Open an issue before proposing a large feature, public API change, or new binding.
- Keep pull requests focused and include tests and documentation for changed behavior.
- Put shared PDF behavior in `include/minpdf.h`; bindings should remain consistent with the shared engine.

## Setup and tests

Install only the tools needed for your change: a C11 compiler and `make`, Go 1.22+, Python 3.9+, or Node.js 18+ with Emscripten.

```sh
make test
GOCACHE=/tmp/minpdf-go-cache go test ./...

python -m pip install -e .
python -m pytest bindings/python/test_minpdf.py

cd bindings/javascript
npm install
npm run build
npm test
```

Shared-engine changes should run every relevant binding test. See [Performance and development](docs/development.md) for benchmarks, sanitizers, and release details.

## Pull requests

Explain the problem and solution, list affected languages, and describe the tests you ran. Call out public API, ownership, or generated-output changes. By submitting a contribution, you agree that it is licensed under the project's [MIT License](LICENSE).
