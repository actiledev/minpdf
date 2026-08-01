# Output, errors, and ownership

## In-memory output

All bindings produce the complete PDF in memory:

- C: `minpdf_build` returns `minpdf_buffer`; free it with `minpdf_buffer_free`.
- Go: `Bytes` returns a Go-owned copied `[]byte`.
- Python: `bytes` returns an independent Python `bytes` object.
- JavaScript: `bytes` returns a copied `Uint8Array` outside WASM memory.

Returned language-owned data remains valid after closing the PDF.

## File output

- C `minpdf_save` opens and writes the path.
- Go `Save` writes with mode `0644`.
- Python `save` uses `Path.write_bytes`.
- JavaScript `save` imports `node:fs/promises` and is for Node/Bun.
- Browsers use `download`, `blob`, or `objectURL`.

Parent directories are not created. Existing files are replaced
according to the host language's normal file-writing behavior.

## Error model

C returns a status and stores a short message on the PDF instance. Go converts the
status/message into `error`. Python and JavaScript raise exceptions.

Common failures include:

- non-positive page dimensions
- malformed or unsupported images/fonts
- invalid UTF-8 passed through the C API
- a custom font without a requested glyph
- duplicate or unregistered font names
- empty Go image/font slices
- allocation failure
- file open or write failure
- using a closed Go PDF instance

Treat any failed operation as incomplete. Inspect or destroy the PDF instance
afterward, but avoid continuing output generation after a content error.

## Deterministic output

minpdf does not add a current timestamp. With identical operation
order, inputs, and explicit metadata, the C engine emits deterministic bytes.
Set `creation_date`/`creationDate` when the PDF requires a timestamp.

## Security considerations

PDF text, metadata, and URLs are escaped by the serializer. Image and font
parsers perform bounds checks, but applications processing untrusted files
should enforce maximum input sizes, image dimensions, PDF page counts, and
output sizes. Do not expose arbitrary server filesystem paths through `save`.
