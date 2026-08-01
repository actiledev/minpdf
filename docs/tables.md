# Tables

Tables are a small convenience layout feature for headers and fixed-height text
rows. They are not a spreadsheet or general layout engine.

## Layout

- The first row is a styled header built from column definitions.
- Body rows alternate white and light-gray backgrounds.
- Row height is `font size + 2 × padding`.
- A border is drawn around the whole table.
- Cell text uses each column's left, center, or right alignment.
- Missing cells are treated as empty strings.

If a column width is zero/omitted, the binding estimates a width from its header
and body strings. Explicit widths give more predictable cross-language output.

## Example structure

```text
columns = [
  { header: "Item",   width: 240, align: left  },
  { header: "Amount", width: 100, align: right },
]
rows = [
  ["Design", "$500"],
  ["Development", "$1,200"],
]
```

See the language guides for the exact struct/class syntax.

## Limitations

- Cells are one line; there is no cell wrapping or clipping.
- Tables do not split across pages.
- There are no row spans, column spans, nested content, or per-cell styles.
- Row height is fixed and does not inspect font ascent/descent.
- Table text uses built-in fonts in the current convenience APIs.

For pagination, determine how many rows fit, draw that slice, add a page, and
draw the next slice. Each table call repeats the header.

```text
row height = font size + 2 × padding
available rows = floor((page height - start y - bottom margin - row height) / row height)
```
