# Python guide

[Docs](README.md) · [Installation](installation.md) · [API](api.md) · [Python example](../examples/python)

## Install

The package is named `minpdf-core` and imports as `minpdf`. Its first PyPI release is pending. Install it from the repository for now:

```sh
python -m pip install minpdf-core
```

Python 3.9 or newer and a C compiler are required.

The full example is in [`examples/python`](../examples/python). Run it from the repository root:

```sh
python3 examples/python/showcase.py
```

## Basic PDF creation

```python
from minpdf import PDF, A4, TextOptions, RectOptions, rgb

with PDF(A4) as doc:
    doc.metadata(title="Example report", author="Actile")
    doc.text(
        "Example report", 50, 55,
        TextOptions(size=24, weight="bold"),
    )
    doc.rect(
        50, 85, 495, 80,
        RectOptions(fill=rgb(240, 244, 248), radius=8),
    )
    doc.save("report.pdf")
```

Drawing methods return the same `PDF` object and may be chained. Native errors
are raised as `RuntimeError`; invalid Python option strings may raise `KeyError`.

## Page sizes and pages

Presets are `A3`, `A4`, `A5`, and `LETTER`. A custom size is a `(width, height)`
tuple:

```python
doc = PDF((500, 700))
doc.page(A5).text("A5 page", 40, 40)
```

Remember that the constructor already created the first page.

## Text

```python
doc.text("Centered", 297.5, 100, TextOptions(
    size=18,
    color=rgb(44, 62, 80),
    align="center",
    weight="bold",
    max_width=300,
))
```

Valid alignments are `left`, `center`, and `right`. Valid built-in weights are
`normal`, `bold`, `italic`, and `bolditalic`.

## Custom fonts and Unicode

```python
font_data = open("NotoSans-Regular.ttf", "rb").read()
doc.register_font("body", font_data)
doc.text("Héllo 世界", 50, 150, TextOptions(font="body", size=14))
```

Font and image arguments must be `bytes`, not arbitrary buffer
objects. The engine copies the data.

## Shapes and lines

```python
blue = rgb(52, 152, 219)
doc.rect(50, 200, 200, 70, RectOptions(fill=blue, radius=10))
doc.circle(100, 340, 35, RectOptions(fill=rgb(155, 89, 182)))
doc.line(50, 400, 300, 400, color=rgb(231, 76, 60), width=2, dash=[6, 3])
```

## Images and links

```python
from minpdf import ImageOptions

doc.image(open("logo.png", "rb").read(), 50, 430,
          ImageOptions(width=120, height=60))
doc.link("Project website", "https://example.com", 50, 520,
         color=rgb(0, 102, 204), underline=True, size=12)
```

## Tables

```python
from minpdf import TableColumn

doc.table(
    [["Design", "$500"], ["Development", "$1,200"]],
    50, 570,
    [
        TableColumn("Item", width=240),
        TableColumn("Amount", width=100, align="right"),
    ],
    font_size=10,
    padding=8,
)
```

## Output and lifetime

```python
data: bytes = doc.bytes()
doc.save("report.pdf")       # accepts str or pathlib.Path
```

The context manager drops its native PDF reference on exit. Do not retain
and use the instance after leaving the `with` block. Output returned by `bytes`
is an independent Python `bytes` object.
