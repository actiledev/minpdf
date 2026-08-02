# Compatibility and change management

[Docs](README.md) · [API](api.md) · [Development](development.md) · [Changelog](../CHANGELOG.md)

minpdf treats the public APIs released in version 0.1 as stable. Applications
may upgrade through the 0.x series without source changes caused by removals,
renames, incompatible signatures, or changed ownership rules.

## Versioning

- Patch releases contain compatible fixes and security updates.
- Minor releases may add APIs and PDF capabilities without breaking existing calls.
- The first planned opportunity for an intentional breaking change is 1.0. Any
  such change requires a changelog entry and migration instructions.
- After 1.0, incompatible public changes require a new major version.

Urgent security or correctness fixes may override these rules when preserving
the old behavior would keep users unsafe or produce invalid documents. Release
notes must identify the exception and required migration.

Versions in the C header, Python distribution, npm package, lockfile, and Git
tag move together. The Go module uses that same repository tag.

## What is public

The compatibility promise covers exported C declarations and numeric status
values, exported Go identifiers, Python names listed in `minpdf.__all__`, and
JavaScript exports and TypeScript declarations. It also covers documented
ownership, lifecycle, error categories, units, defaults, and input formats.

New status values, options, and exported functions may be added. Exact error
message wording is diagnostic and may improve between releases. Internals,
build artifacts, test helpers, and undocumented details are not public APIs.

All maintained bindings continue to expose the same shared-engine operations.
Language-specific conveniences may differ when the difference is documented.

## PDF output

Identical calls, inputs, metadata, and minpdf version produce deterministic
bytes. Exact bytes are not promised across versions because valid serializer
improvements may change object layout or compression. Existing operations retain
their documented PDF semantics.

Deprecations remain functional and documented for at least one minor release.
They cannot be removed during 0.x. Compatibility-sensitive changes must update
the changelog, relevant guides, conformance fixture, and binding tests.
