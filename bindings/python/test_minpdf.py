import base64
import tempfile
import unittest
from pathlib import Path

from minpdf import (
    A3,
    A4,
    A5,
    LETTER,
    ImageOptions,
    PDF,
    RectOptions,
    TableColumn,
    TextOptions,
    rgb,
)


PNG_RGBA = base64.b64decode(
    "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAFgwJ/lqL8WQAAAABJRU5ErkJggg=="
)


class MinPDFTests(unittest.TestCase):
    def assert_pdf(self, data: bytes) -> None:
        self.assertTrue(data.startswith(b"%PDF-1.4"))
        self.assertTrue(data.endswith(b"%%EOF\n"))
        self.assertIn(b"xref", data)

    def test_constants_and_rgb(self) -> None:
        self.assertEqual(A3, (842.0, 1191.0))
        self.assertEqual(A4, (595.0, 842.0))
        self.assertEqual(A5, (420.0, 595.0))
        self.assertEqual(LETTER, (612.0, 792.0))
        self.assertEqual(rgb(255, 0, 255), (1.0, 0.0, 1.0))
        self.assertAlmostEqual(rgb(0, 128, 0)[1], 128 / 255)

    def test_drawing_metadata_links_and_pages(self) -> None:
        data = (
            PDF(A4)
            .metadata(title="A (title)", author="Ada", creator="minpdf")
            .text(
                "Hello (Python)",
                50,
                50,
                TextOptions(size=20, color=rgb(10, 20, 30), weight="bold"),
            )
            .rect(
                50,
                80,
                200,
                50,
                RectOptions(
                    fill=rgb(52, 152, 219),
                    stroke=rgb(0, 0, 0),
                    line_width=2,
                    radius=4,
                ),
            )
            .line(10, 150, 100, 150, rgb(255, 0, 0), 2, [3, 2])
            .circle(150, 150, 20, RectOptions(stroke=rgb(0, 0, 0)))
            .link("Website", "https://example.com?a=1&b=2", 20, 200)
            .page(LETTER)
            .text("Second page", 20, 20)
            .bytes()
        )
        self.assert_pdf(data)
        for expected in (
            b"/Count 2",
            b"/Helvetica-Bold",
            b"/Title(A \\(title\\))",
            b"/URI(https://example.com?a=1&b=2)",
            b"[3.00 2.00 ] 0 d",
        ):
            self.assertIn(expected, data)

    def test_table_handles_short_rows_and_alignment(self) -> None:
        data = PDF().table(
            [["Ada", "Engineer"], ["Linus"]],
            20,
            20,
            [TableColumn("Name"), TableColumn("Role", align="right")],
        ).bytes()
        self.assert_pdf(data)
        for expected in (b"(Name)", b"(Role)", b"(Ada)", b"(Engineer)", b"(Linus)"):
            self.assertIn(expected, data)

    def test_rgba_png_and_explicit_dimensions(self) -> None:
        data = PDF().image(PNG_RGBA, 10, 10, ImageOptions(width=20)).bytes()
        self.assert_pdf(data)
        self.assertIn(b"/Subtype/Image", data)
        self.assertIn(b"/SMask", data)

    def test_save_matches_bytes(self) -> None:
        pdf = PDF().text("saved", 10, 10)
        expected = pdf.bytes()
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "document.pdf"
            self.assertIsNone(pdf.save(path))
            self.assertEqual(path.read_bytes(), expected)

    def test_context_manager_releases_document(self) -> None:
        with PDF() as pdf:
            self.assert_pdf(pdf.text("inside", 10, 10).bytes())
        with self.assertRaises((TypeError, ValueError)):
            pdf.bytes()

    def test_invalid_arguments_report_errors(self) -> None:
        with self.assertRaisesRegex(RuntimeError, "invalid page size"):
            PDF().page((0, 100))
        with self.assertRaisesRegex(RuntimeError, "invalid rectangle"):
            PDF().rect(0, 0, -1, 1)
        with self.assertRaisesRegex(RuntimeError, "invalid circle"):
            PDF().circle(0, 0, -1)
        with self.assertRaisesRegex(RuntimeError, "invalid image"):
            PDF().image(b"not an image", 0, 0)
        with self.assertRaisesRegex(RuntimeError, "font is not registered"):
            PDF().text("x", 0, 0, TextOptions(font="missing"))
        with self.assertRaisesRegex(RuntimeError, "non-ASCII text"):
            PDF().text("café", 0, 0)


if __name__ == "__main__":
    unittest.main()
