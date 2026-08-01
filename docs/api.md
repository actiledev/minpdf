# Shared API reference

[Docs](README.md) · [Installation](installation.md) · [Languages](README.md#language-guides) · [Features](README.md#feature-guides)

All bindings expose the same PDF operations. Names follow each language's usual style.

## Operation mapping

| Operation     | C                      | Go             | Python          | JavaScript         |
| ------------- | ---------------------- | -------------- | --------------- | ------------------ |
| Create        | `minpdf_create`        | `New`          | `PDF`           | `await PDF.create` |
| Add page      | `minpdf_add_page`      | `Page`         | `page`          | `page`             |
| Metadata      | `minpdf_set_metadata`  | `SetMetadata`  | `metadata`      | `metadata`         |
| Register font | `minpdf_register_font` | `RegisterFont` | `register_font` | `registerFont`     |
| Text          | `minpdf_text`          | `Text`         | `text`          | `text`             |
| Rectangle     | `minpdf_rect`          | `Rect`         | `rect`          | `rect`             |
| Line          | `minpdf_line`          | `Line`         | `line`          | `line`             |
| Circle        | `minpdf_circle`        | `Circle`       | `circle`        | `circle`           |
| Image         | `minpdf_image`         | `Image`        | `image`         | `image`            |
| Link          | `minpdf_link`          | `Link`         | `link`          | `link`             |
| Table         | `minpdf_table`         | `Table`        | `table`         | `table`            |
| Memory output | `minpdf_build`         | `Bytes`        | `bytes`         | `bytes`            |
| File output   | `minpdf_save`          | `Save`         | `save`          | `save`             |
| Release       | `minpdf_destroy`       | `Close`        | GC/context exit | `close`            |

## PDF lifecycle

Creating a PDF instance initializes its first page. Drawing operations target
the active page. Calling `page`, `Page`, or `minpdf_add_page` appends a new page
and makes it active.

PDF instances retain drawing commands and copied resource data until released.
Building output does not close the generator; applications can add content and
build again.

## Page sizes

| Preset | Width | Height |
| ------ | ----: | -----: |
| A3     |   842 |   1191 |
| A4     |   595 |    842 |
| A5     |   420 |    595 |
| Letter |   612 |    792 |

Sizes are in PDF points. Pass width and height for custom sizes. Both values
must exceed zero.

## Text options

| Option    | Default | Description                                   |
| --------- | ------- | --------------------------------------------- |
| size      | 12      | Font size in points                           |
| color     | black   | Normalized RGB color                          |
| align     | left    | Treat `x` as left edge, center, or right edge |
| weight    | normal  | Built-in font weight/style                    |
| max width | 0       | Word-wrap when greater than zero              |
| font      | empty   | Registered TrueType font name                 |

The `weight` setting selects built-in Helvetica variants. It does not synthesize
bold or italic variants for a custom font.

## Shape options

Rectangles and circles accept optional fill and stroke colors. If neither is
specified, the path is invisible. `lineWidth`/`line_width` defaults to 1.
Rectangle radius defaults to 0 and is clamped to half the smallest dimension.

Lines accept color, width, and an optional dash array in every API.

## Image options

Width and height default to the image's pixel dimensions interpreted as points.
If only one dimension is supplied, the other remains its original dimension;
calculate the matching dimension yourself to preserve aspect ratio.

## Metadata fields

Supported fields are title, author, subject, keywords, creator, and creation
date. For a PDF-formatted date, use a value such as `D:20260801143000+07'00'`.
Metadata strings and input resource bytes are copied into the PDF engine.

## Thread safety

Separate threads may operate on distinct PDF instances. Do not mutate a single
PDF instance concurrently. The engine holds no global mutable state.
