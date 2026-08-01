# Core concepts

[Docs](README.md) · [Installation](installation.md) · [API](api.md) · [Output](output-and-errors.md)

## Coordinate system

All languages use the same coordinate system:

- `(0, 0)` is the top-left corner of the current page.
- `x` grows to the right.
- `y` grows downward.
- Widths, heights, coordinates, line widths, and font sizes are PDF points.
- 72 points equal one inch.

Useful conversions:

```text
points = inches × 72
points = millimetres × 72 / 25.4
```

## Pages

A newly initialized PDF instance contains one page. Add a page only when moving
to the next page. Each page can have a different size.

minpdf does not create pages when text or a table reaches the bottom.
Applications decide page breaks and repeat headers where necessary.

## Colors

The engine stores RGB components as floating-point values from 0 through 1.
Every binding provides an RGB helper accepting 8-bit values:

```text
C:          minpdf_rgb(52, 152, 219)
Go:         minpdf.RGB(52, 152, 219)
Python:     rgb(52, 152, 219)
JavaScript: rgb(52, 152, 219)
```

## Fluent and explicit-error APIs

JavaScript and Python drawing methods return the PDF instance for chaining and
raise exceptions on failure. C returns `minpdf_status`. Go returns `error` and
therefore favors one checked operation per statement.

## Resource ownership

Font and image data are copied when registered or added. Callers may release or
reuse input buffers after the operation returns. The PDF generator retains all
internal resources until closed or destroyed.

The serializer builds the complete PDF in memory. Peak memory includes the page state and final output.
