# Complete invoice example

Each language creates one A4 page, adds metadata, draws a heading and panel,
and writes a two-column table.

## JavaScript

```ts
import { PDF, SIZES, rgb } from "minpdf";

const doc = await PDF.create(SIZES.A4);
try {
  doc
    .metadata({ title: "Invoice #1001", author: "Acme" })
    .text("INVOICE", 50, 55, { size: 26, weight: "bold" })
    .rect(50, 85, 495, 70, { fill: rgb(245, 247, 250), radius: 8 })
    .text("Invoice #1001", 70, 115, { size: 14, weight: "bold" })
    .table(
      [
        ["Design", "$500"],
        ["Development", "$1,200"],
      ],
      50,
      190,
      {
        columns: [
          { header: "Item", width: 340 },
          { header: "Amount", width: 155, align: "right" },
        ],
      },
    );
  await doc.save("invoice.pdf");
} finally {
  doc.close();
}
```

## Python

```python
from minpdf import PDF, A4, TextOptions, RectOptions, TableColumn, rgb

with PDF(A4) as doc:
    (doc.metadata(title="Invoice #1001", author="Acme")
        .text("INVOICE", 50, 55, TextOptions(size=26, weight="bold"))
        .rect(50, 85, 495, 70, RectOptions(fill=rgb(245,247,250), radius=8))
        .text("Invoice #1001", 70, 115, TextOptions(size=14, weight="bold"))
        .table(
            [["Design", "$500"], ["Development", "$1,200"]],
            50, 190,
            [TableColumn("Item", 340),
             TableColumn("Amount", 155, "right")],
        ))
    doc.save("invoice.pdf")
```

## Go

```go
doc, err := minpdf.New(minpdf.A4)
if err != nil { return err }
defer doc.Close()
if err := doc.SetMetadata(minpdf.Metadata{Title:"Invoice #1001", Author:"Acme"}); err != nil { return err }
if err := doc.Text("INVOICE",50,55,&minpdf.TextOptions{Size:26,Color:minpdf.Black,Weight:minpdf.WeightBold}); err != nil { return err }
panel := minpdf.RGB(245,247,250)
if err := doc.Rect(50,85,495,70,&minpdf.RectOptions{Fill:&panel,Radius:8}); err != nil { return err }
if err := doc.Table([][]string{{"Design","$500"},{"Development","$1,200"}},50,190,minpdf.TableOptions{
    Columns: []minpdf.TableColumn{{Header:"Item",Width:340},{Header:"Amount",Width:155,Align:minpdf.AlignRight}},
    HeaderBackground:minpdf.RGB(240,240,240), HeaderColor:minpdf.Black,
    BorderColor:minpdf.RGB(204,204,204), FontSize:10, Padding:8,
}); err != nil { return err }
return doc.Save("invoice.pdf")
```

For the C table and error-checking form, see [C and C++](c.md).
