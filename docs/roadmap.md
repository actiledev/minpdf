# Roadmap

[Docs](README.md) · [API](api.md) · [Development](development.md) · [Contributing](../CONTRIBUTING.md)

This roadmap lists minpdf's current features and planned work. It does not set
delivery dates. Priorities may change as maintainers learn from users and
contributors.

## Version 0.1.0 baseline

minpdf 0.1.0 generates new PDFs with these features:

- [x] Shared C11 PDF-generation engine
- [x] Pages, core drawing primitives, and simple tables
- [x] JPEG and PNG images
- [x] Built-in fonts and embedded TrueType fonts
- [x] Deterministic bytes when calls, inputs, and metadata stay fixed
- [x] C and C++, Go, Python, JavaScript, and TypeScript APIs
- [x] Native platforms and browser support through WebAssembly

## 1. Release readiness

- [x] Test supported platforms and language toolchains in continuous integration
- [x] Produce repeatable releases for each maintained binding
- [x] Fuzz parsers, serializers, and cross-language boundaries
- [x] Track performance and memory use with reproducible benchmarks
- [x] Compare behavior across native and WebAssembly targets
- [x] Define PDF API compatibility guarantees and change management

## 2. PDF layout features

- [ ] Add automatic pagination for text, tables, and composed layouts
- [ ] Support table wrapping, flexible row heights, and more styling controls
- [ ] Add repeating headers and footers
- [ ] Add reusable components and templates
- [ ] Support general vector paths
- [ ] Add gradients and clipping controls
- [ ] Support more image formats, conversion options, and placement controls

## 3. Text and fonts

Complete this work in dependency order:

- [ ] Subset embedded fonts
- [ ] Add automatic font fallback
- [ ] Apply kerning
- [ ] Support OpenType shaping
- [ ] Support bidirectional text
- [ ] Complete complex-script support

## 4. PDF capabilities

- [ ] Add PDF outlines, bookmarks, and internal links
- [ ] Embed file attachments
- [ ] Add interactive form fields
- [ ] Support passwords and PDF permissions
- [ ] Support digital signatures
- [ ] Produce tagged PDF output
- [ ] Stream output without retaining the complete PDF in memory

## 5. Language ecosystem

New bindings must follow the shared API instead of creating separate PDF
implementations.

- [ ] Add a Rust binding
- [ ] Add a Java binding
- [ ] Add a Kotlin binding
- [ ] Add a C# binding
- [ ] Consider PHP, Swift, and Zig bindings based on user interest
- [ ] Consider other bindings based on user interest

## 6. Long-term exploration

The maintainers may research these features. They may remain outside minpdf if
they would compromise its compact architecture.

- [ ] Import existing PDFs
- [ ] Inspect PDF structure and content
- [ ] Merge existing PDFs
- [ ] Edit existing PDFs

## Contributing to the roadmap

Proposals should identify user needs, dependencies, and behavior across
languages. Features in the shared engine must behave the same across maintained
bindings. Contributions should include regression tests for the engine and
affected bindings, plus updates to public API documentation and examples.
