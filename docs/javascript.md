# JavaScript and TypeScript guide

## Initialization

The C engine is loaded as WebAssembly, so PDF instance creation is asynchronous:

```ts
import { PDF, SIZES, rgb } from "minpdf";

const doc = await PDF.create(SIZES.A4);
try {
  doc.text("Hello", 50, 50, { size: 24, weight: "bold" });
  await doc.save("hello.pdf");
} finally {
  doc.close();
}
```

The engine module is initialized once and reused. Drawing and serialization are
synchronous after `PDF.create` resolves.

## Types

```ts
type Size = { width: number; height: number };
type Color = [number, number, number];
type Align = "left" | "center" | "right";
type Weight = "normal" | "bold" | "italic" | "bolditalic";
```

Presets are `SIZES.A3`, `SIZES.A4`, `SIZES.A5`, and `SIZES.LETTER`.

## Text and shapes

```ts
doc.text("Centered heading", 297.5, 60, {
  size: 22,
  color: rgb(44, 62, 80),
  align: "center",
  weight: "bold",
  maxWidth: 400,
});

doc.rect(50, 100, 200, 70, {
  fill: rgb(52, 152, 219),
  stroke: rgb(41, 128, 185),
  lineWidth: 1,
  radius: 10,
});
doc.circle(100, 240, 35, { fill: rgb(155, 89, 182) });
doc.line(50, 300, 300, 300, {
  color: rgb(231, 76, 60),
  width: 2,
  dash: [6, 3],
});
```

## Fonts, images, and links

```ts
const font = new Uint8Array(await readFile("NotoSans-Regular.ttf"));
doc.registerFont("body", font);
doc.text("Héllo 世界", 50, 340, { font: "body", size: 14 });

const logo = new Uint8Array(await readFile("logo.png"));
doc.image(logo, 50, 390, { width: 120, height: 60 });

doc.link("Project website", "https://example.com", 50, 480, {
  color: rgb(0, 102, 204),
  underline: true,
  size: 12,
});
```

Node examples can import `readFile` from `node:fs/promises`. In browsers, use
`fetch(...).arrayBuffer()` or a file input and wrap the result in `Uint8Array`.

## Tables

```ts
doc.table(
  [
    ["Design", "$500"],
    ["Development", "$1,200"],
  ],
  50,
  540,
  {
    columns: [
      { header: "Item", width: 240 },
      { header: "Amount", width: 100, align: "right" },
    ],
    fontSize: 10,
    padding: 8,
  },
);
```

## Output

```ts
const data: Uint8Array = doc.bytes();
await doc.save("report.pdf"); // Node.js or Bun only
```

`bytes` returns a copied `Uint8Array`, independent of WASM memory. Call
`close` when finished. Do not call methods after closing the PDF.

For browser-only output methods, see [Browser deployment](browser.md).
