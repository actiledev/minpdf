# Go guide

## Requirements

```sh
go get github.com/actiledev/minpdf
```

The package requires cgo and a C compiler. Build with `CGO_ENABLED=1`.

## Creating and closing a PDF

```go
package main

import (
    "log"
    "github.com/actiledev/minpdf"
)

func main() {
    doc, err := minpdf.New(minpdf.A4)
    if err != nil { log.Fatal(err) }
    defer doc.Close()

    if err := doc.Text("Hello from Go", 50, 50, &minpdf.TextOptions{
        Size: 24,
        Color: minpdf.Black,
        Weight: minpdf.WeightBold,
    }); err != nil { log.Fatal(err) }

    if err := doc.Save("hello.pdf"); err != nil { log.Fatal(err) }
}
```

Call `Close`. A finalizer exists as a safety net. Methods called
after `Close` return `minpdf: document is closed`.

## Drawing

```go
blue := minpdf.RGB(52, 152, 219)
border := minpdf.RGB(41, 128, 185)

err = doc.Rect(50, 90, 250, 80, &minpdf.RectOptions{
    Fill: &blue, Stroke: &border, LineWidth: 1, Radius: 10,
})
if err != nil { return err }

err = doc.Circle(100, 230, 35, &minpdf.RectOptions{Fill: &blue})
if err != nil { return err }

err = doc.Line(50, 290, 300, 290, &minpdf.LineOptions{
    Color: border, Width: 2, Dash: []float32{6, 3},
})
```

When an options pointer is `nil`, engine defaults are used. When providing an
options struct, set `Color` where needed; its zero value is black.

## Pages and metadata

```go
if err := doc.SetMetadata(minpdf.Metadata{
    Title: "Quarterly report",
    Author: "Actile",
}); err != nil { return err }

if err := doc.Page(minpdf.Letter); err != nil { return err }
if err := doc.Text("Second page", 50, 50, nil); err != nil { return err }
```

## Fonts and images

```go
fontData, err := os.ReadFile("NotoSans-Regular.ttf")
if err != nil { return err }
if err := doc.RegisterFont("body", fontData); err != nil { return err }
if err := doc.Text("Héllo 世界", 50, 100, &minpdf.TextOptions{
    Size: 14, Color: minpdf.Black, Font: "body", MaxWidth: 400,
}); err != nil { return err }

imageData, err := os.ReadFile("logo.png")
if err != nil { return err }
if err := doc.Image(imageData, 50, 160, &minpdf.ImageOptions{
    Width: 120, Height: 60,
}); err != nil { return err }
```

The engine copies both byte slices during the call.

## Tables

```go
err = doc.Table([][]string{
    {"Design", "$500"},
    {"Development", "$1,200"},
}, 50, 300, minpdf.TableOptions{
    Columns: []minpdf.TableColumn{
        {Header: "Item", Width: 240, Align: minpdf.AlignLeft},
        {Header: "Amount", Width: 100, Align: minpdf.AlignRight},
    },
    HeaderBackground: minpdf.RGB(240, 240, 240),
    HeaderColor: minpdf.Black,
    BorderColor: minpdf.RGB(204, 204, 204),
    FontSize: 10,
    Padding: 8,
})
```

## Output

```go
data, err := doc.Bytes()       // Go-owned []byte
n, err := doc.WriteTo(writer)  // implements an io.Writer workflow
err = doc.Save("report.pdf")   // mode 0644
```

`Bytes` copies native output into Go memory. The returned slice remains valid
after closing the PDF.
