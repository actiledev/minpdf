# Text and fonts

[Docs](README.md) · [API](api.md) · [Images](images.md) · [Tables](tables.md)

## Built-in fonts

Without a custom font, minpdf uses PDF's standard Helvetica family:

- normal → Helvetica
- bold → Helvetica-Bold
- italic → Helvetica-Oblique
- bolditalic/bold-italic → Helvetica-BoldOblique

Built-in text is limited to ASCII in minpdf 0.1.0. For accented characters,
non-Latin scripts, or reliable Unicode extraction, register a TrueType font.

## Registering a TrueType font

Registration takes a unique name and complete font bytes. The engine parses
`cmap`, `head`, `hhea`, `hmtx`, and `maxp`, and copies the font data.

```text
C:          minpdf_register_font(doc, "body", data, length)
Go:         doc.RegisterFont("body", data)
Python:     doc.register_font("body", data)
JavaScript: doc.registerFont("body", data)
```

Select the same name in text options. A missing registration or missing glyph
is reported as an error; minpdf does not substitute another font.

## Unicode behavior

Input strings are UTF-8 in the C engine. Bindings convert their native strings
to UTF-8. Custom text is mapped through the font's Unicode cmap to glyph IDs,
rendered with an Identity-H Type0 font, and accompanied by a ToUnicode map so
copying/searching text works in PDF viewers.

Features omitted in version 0.1.0:

- Arabic/Indic or other complex-script shaping
- bidi reordering
- kerning or OpenType layout features
- automatic font fallback
- font subsetting
- synthetic bold or italic custom fonts

Use already-shaped text only when its code-point order and the selected font's
simple glyph mapping produce the intended visual output.

## Alignment

For left alignment, `x` is the left edge. For center alignment, `x` is the
horizontal center. For right alignment, `x` is the right edge. Custom-font
alignment uses actual glyph advance widths. Built-in Helvetica uses the
engine's compact width estimate.

## Word wrapping

Set `maxWidth`/`max_width` greater than zero to wrap at spaces. The line height
is `font size × 1.2`. Long individual words are not hyphenated or split.

Wrapping does not create a new page. Calculate remaining page height and add a
page before drawing when needed.

## Font file size

The full TrueType file is embedded each time a registered font is used in a
PDF. Register each font once and reuse its name. Because version 0.1.0 does
not subset fonts, large CJK fonts can produce large PDF files.
