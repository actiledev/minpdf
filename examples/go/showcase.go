package main

import (
	"os"

	minpdf "github.com/actiledev/minpdf"
)

func main() {
	png, _ := os.ReadFile("path/to/your/image.png") // Replace with PNG image path.
	pdf, _ := minpdf.New(minpdf.A4)
	defer pdf.Close()
	blue, dark, muted := minpdf.RGB(30, 64, 175), minpdf.RGB(15, 23, 42), minpdf.RGB(71, 85, 105)
	panel, border, circle := minpdf.RGB(239, 246, 255), minpdf.RGB(147, 197, 253), minpdf.RGB(37, 99, 235)
	pdf.SetMetadata(minpdf.Metadata{Title: "minpdf Go", Author: "Actile", Subject: "Subject here", Keywords: "minpdf,pdf,showcase", Creator: "minpdf", CreationDate: "D:20260101000000Z"})
	pdf.Text("minpdf Showcase", 48, 62, &minpdf.TextOptions{Size: 26, Color: blue, Weight: minpdf.WeightBold})
	pdf.Text("One layout, four language bindings", 48, 84, &minpdf.TextOptions{Size: 11, Color: muted})
	pdf.Rect(48, 110, 499, 100, &minpdf.RectOptions{Fill: &panel, Stroke: &border, LineWidth: 1, Radius: 10})
	pdf.Circle(100, 160, 28, &minpdf.RectOptions{Fill: &circle, Stroke: &blue, LineWidth: 1})
	pdf.Text("Shapes, fills, strokes and rounded corners", 148, 150, &minpdf.TextOptions{Size: 14, Color: minpdf.RGB(30, 41, 59), Weight: minpdf.WeightBold})
	pdf.Text("All coordinates and styles match across examples.", 148, 174, &minpdf.TextOptions{Size: 10, Color: muted, Weight: minpdf.WeightItalic})
	pdf.Line(48, 232, 547, 232, &minpdf.LineOptions{Color: minpdf.RGB(148, 163, 184), Width: 1})
	pdf.Text("Team", 48, 264, &minpdf.TextOptions{Size: 16, Color: dark, Weight: minpdf.WeightBold})
	pdf.Table([][]string{{"Ada Lovelace", "Engineer"}, {"Grace Hopper", "Admiral"}, {"Linus Torvalds", "Maintainer"}}, 48, 282, minpdf.TableOptions{Columns: []minpdf.TableColumn{{Header: "Name", Width: 220}, {Header: "Role", Width: 225, Align: minpdf.AlignRight}}})
	pdf.Text("Embedded RGBA image", 48, 430, &minpdf.TextOptions{Size: 16, Color: dark, Weight: minpdf.WeightBold})
	pdf.Image(png, 48, 450, &minpdf.ImageOptions{Width: 64, Height: 64})
	pdf.Text("The red square is a scaled 1x1 PNG with alpha.", 128, 486, &minpdf.TextOptions{Size: 11, Color: muted})
	pdf.Link("Visit minpdf documentation", "https://github.com/actiledev/minpdf", 48, 550, &minpdf.LinkOptions{Color: circle, Underline: true, Size: 12})
	pdf.Text("Centered text", 297.5, 610, &minpdf.TextOptions{Size: 12, Color: dark, Align: minpdf.AlignCenter, Weight: minpdf.WeightBold})
	pdf.Text("Right-aligned bold italic", 547, 635, &minpdf.TextOptions{Size: 12, Color: dark, Align: minpdf.AlignRight, Weight: minpdf.WeightBoldItalic})
	pdf.Page(minpdf.Letter)
	pdf.Text("Page 2", 48, 62, &minpdf.TextOptions{Size: 24, Color: blue, Weight: minpdf.WeightBold})
	pdf.Text("This Letter-sized page demonstrates mixed page sizes.", 48, 92, &minpdf.TextOptions{Size: 12, Color: minpdf.RGB(51, 65, 85)})
	pdf.Save("path/to/your/output.pdf") // Replace with output path.
}
