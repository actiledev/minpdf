"""Generate the same feature showcase as the C, Go, and JavaScript examples."""
import base64
import sys
from pathlib import Path

# Allow `python3 examples/python/showcase.py` from a source checkout. An
# installed minpdf package still takes precedence when one is available.
SOURCE_BINDING = Path(__file__).resolve().parents[2] / "bindings" / "python" / "src"
if str(SOURCE_BINDING) not in sys.path:
    sys.path.append(str(SOURCE_BINDING))

from minpdf import A4, LETTER, ImageOptions, PDF, RectOptions, TableColumn, TextOptions, rgb


PNG_RGBA = base64.b64decode(
    "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAFgwJ/lqL8WQAAAABJRU5ErkJggg=="
)


pdf = PDF(A4)
pdf.metadata(
    title="minpdf Cross-Language Showcase",
    author="Actile",
    subject="Identical feature showcase for every binding",
    keywords="minpdf,pdf,showcase",
    creator="minpdf",
    creation_date="D:20260101000000Z",
)
pdf.text("minpdf Showcase", 48, 62, TextOptions(26, rgb(30, 64, 175), weight="bold"))
pdf.text("One layout, four language bindings", 48, 84, TextOptions(11, rgb(71, 85, 105)))
pdf.rect(48, 110, 499, 100, RectOptions(rgb(239, 246, 255), rgb(147, 197, 253), 1, 10))
pdf.circle(100, 160, 28, RectOptions(rgb(37, 99, 235), rgb(30, 64, 175)))
pdf.text("Shapes, fills, strokes and rounded corners", 148, 150, TextOptions(14, rgb(30, 41, 59), weight="bold"))
pdf.text("All coordinates and styles match across examples.", 148, 174, TextOptions(10, rgb(71, 85, 105), weight="italic"))
pdf.line(48, 232, 547, 232, rgb(148, 163, 184), 1)
pdf.text("Team", 48, 264, TextOptions(16, rgb(15, 23, 42), weight="bold"))
pdf.table(
    [["Ada Lovelace", "Engineer"], ["Grace Hopper", "Admiral"], ["Linus Torvalds", "Maintainer"]],
    48,
    282,
    [TableColumn("Name", 220), TableColumn("Role", 225, "right")],
)
pdf.text("Embedded RGBA image", 48, 430, TextOptions(16, rgb(15, 23, 42), weight="bold"))
pdf.image(PNG_RGBA, 48, 450, ImageOptions(64, 64))
pdf.text("The red square is a scaled 1x1 PNG with alpha.", 128, 486, TextOptions(11, rgb(71, 85, 105)))
pdf.link("Visit minpdf documentation", "https://github.com/actiledev/minpdf", 48, 550, rgb(37, 99, 235), True, 12)
pdf.text("Centered text", 297.5, 610, TextOptions(12, rgb(15, 23, 42), "center", "bold"))
pdf.text("Right-aligned bold italic", 547, 635, TextOptions(12, rgb(15, 23, 42), "right", "bolditalic"))
pdf.page(LETTER)
pdf.text("Page 2", 48, 62, TextOptions(24, rgb(30, 64, 175), weight="bold"))
pdf.text("This Letter-sized page demonstrates mixed page sizes.", 48, 92, TextOptions(12, rgb(51, 65, 85)))
pdf.save("examples/results/minpdf-showcase-python.pdf")
print("wrote examples/results/minpdf-showcase-python.pdf")
