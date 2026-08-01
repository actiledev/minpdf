# Browser deployment

[Docs](README.md) · [Installation](installation.md) · [JavaScript](javascript.md) · [Output](output-and-errors.md)

## Install

```sh
npm install minpdf
```

The JavaScript package runs in browsers, workers, Node.js, and Bun. It loads the C engine as WebAssembly.

The repository example is in [`examples/javascript`](../examples/javascript). It writes a file with Node.js, but uses the same drawing API as a browser. In a browser, use `download`, `blob`, or `objectURL` for output.

## Downloading a PDF

```ts
import { PDF, SIZES } from "minpdf";

const doc = await PDF.create(SIZES.A4);
try {
  doc.text("Created in a browser", 50, 50, { size: 20 });
  doc.download("browser.pdf");
} finally {
  doc.close();
}
```

`download` creates a temporary object URL, clicks an anchor, and schedules the
URL for revocation.

## Previewing or uploading

```ts
const blob = doc.blob();

const url = doc.objectURL();
iframe.src = url;
// When the preview is no longer used:
URL.revokeObjectURL(url);

await fetch("/upload", {
  method: "POST",
  headers: { "Content-Type": "application/pdf" },
  body: blob,
});
```

Unlike `download`, `objectURL` does not revoke the returned URL.

## Loading assets

```ts
const fontData = new Uint8Array(
  await fetch("/fonts/NotoSans-Regular.ttf").then((r) => r.arrayBuffer()),
);
doc.registerFont("body", fontData);

const imageData = new Uint8Array(
  await fetch("/images/logo.png").then((r) => r.arrayBuffer()),
);
doc.image(imageData, 50, 100, { width: 120, height: 60 });
```

Check `response.ok` in production before consuming a response body.

## Hosting and bundlers

The loader resolves its `.wasm` file relative to the JavaScript module. Deployments must copy that asset and serve it with the
`application/wasm` MIME type. Content Security Policy must permit loading the
module and WASM asset from their deployed origins.

For workers, instantiate and use a PDF generator within the worker and transfer
the result buffer back to the main thread:

```ts
const bytes = doc.bytes();
postMessage(bytes, [bytes.buffer]);
```

## Browser limitations

- `save(path)` imports Node's filesystem module and is not a browser API.
- Popup/download policies may require `download` to run from a user gesture.
- Large fonts and images occupy both JavaScript/WASM input memory and PDF engine
  memory while being copied.
