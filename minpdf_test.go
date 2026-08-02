package minpdf

import (
	"bytes"
	"encoding/base64"
	"errors"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func newDocument(t *testing.T) *Document {
	t.Helper()
	doc, err := New(A4)
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(doc.Close)
	return doc
}

func pdfBytes(t *testing.T, doc *Document) []byte {
	t.Helper()
	b, err := doc.Bytes()
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.HasPrefix(b, []byte("%PDF-1.4")) || !bytes.HasSuffix(b, []byte("%%EOF\n")) {
		t.Fatal("invalid PDF framing")
	}
	return b
}

func TestRGBAndPageSizes(t *testing.T) {
	if got := RGB(255, 128, 0); got.R != 1 || got.G < .50 || got.G > .51 || got.B != 0 {
		t.Fatalf("RGB conversion = %#v", got)
	}
	if A4 != (Size{595, 842}) || Letter != (Size{612, 792}) {
		t.Fatal("unexpected predefined page sizes")
	}
}

func TestDrawingMetadataAndPages(t *testing.T) {
	doc := newDocument(t)
	blue, black := RGB(52, 152, 219), Black
	checks := []error{
		doc.SetMetadata(Metadata{Title: "A (title)", Author: "Ada", Creator: "minpdf"}),
		doc.Text("Hello (Go)", 50, 50, &TextOptions{Size: 20, Color: black, Weight: WeightBold}),
		doc.Rect(50, 80, 200, 50, &RectOptions{Fill: &blue, Stroke: &black, LineWidth: 2, Radius: 4}),
		doc.Line(10, 150, 100, 150, &LineOptions{Color: blue, Width: 2, Dash: []float32{3, 2}}),
		doc.Circle(150, 150, 20, &RectOptions{Stroke: &black}),
		doc.Link("Website", "https://example.com?a=1&b=2", 20, 200, &LinkOptions{Color: blue, Underline: true, Size: 11}),
		doc.Page(Letter),
		doc.Text("Second page", 20, 20, nil),
	}
	for _, err := range checks {
		if err != nil {
			t.Fatal(err)
		}
	}
	b := pdfBytes(t, doc)
	for _, want := range []string{"/Count 2", "/Helvetica-Bold", "/Title(A \\(title\\))", "/URI(https://example.com?a=1&b=2)", "[3.00 2.00 ] 0 d"} {
		if !bytes.Contains(b, []byte(want)) {
			t.Errorf("PDF does not contain %q", want)
		}
	}
}

func TestTableAndRGBAImage(t *testing.T) {
	doc := newDocument(t)
	png, err := base64.StdEncoding.DecodeString("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAFgwJ/lqL8WQAAAABJRU5ErkJggg==")
	if err != nil {
		t.Fatal(err)
	}
	if err = doc.Table([][]string{{"Ada", "Engineer"}, {"Linus"}}, 20, 20, TableOptions{Columns: []TableColumn{{Header: "Name"}, {Header: "Role", Align: AlignRight}}}); err != nil {
		t.Fatal(err)
	}
	if err = doc.Image(png, 20, 150, &ImageOptions{Width: 10}); err != nil {
		t.Fatal(err)
	}
	b := pdfBytes(t, doc)
	for _, want := range []string{"(Name)", "(Engineer)", "/Subtype/Image", "/SMask"} {
		if !bytes.Contains(b, []byte(want)) {
			t.Errorf("PDF does not contain %q", want)
		}
	}
}

func TestOutputHelpers(t *testing.T) {
	doc := newDocument(t)
	if err := doc.Text("output", 10, 10, nil); err != nil {
		t.Fatal(err)
	}
	want := pdfBytes(t, doc)
	var out bytes.Buffer
	n, err := doc.WriteTo(&out)
	if err != nil || n != int64(len(want)) || !bytes.Equal(out.Bytes(), want) {
		t.Fatalf("WriteTo: n=%d err=%v", n, err)
	}
	path := filepath.Join(t.TempDir(), "document.pdf")
	if err = doc.Save(path); err != nil {
		t.Fatal(err)
	}
	got, err := os.ReadFile(path)
	if err != nil || !bytes.Equal(got, want) {
		t.Fatalf("Save mismatch: %v", err)
	}
}

func TestErrorsAndCloseIsIdempotent(t *testing.T) {
	var nilDoc *Document
	if err := nilDoc.Text("x", 0, 0, nil); err == nil || !strings.Contains(err.Error(), "closed") {
		t.Fatalf("nil document error = %v", err)
	}
	doc := newDocument(t)
	if err := doc.Page(Size{}); err == nil || !strings.Contains(err.Error(), "invalid page size") {
		t.Fatalf("invalid page error = %v", err)
	}
	if err := doc.Image(nil, 0, 0, nil); err == nil || !strings.Contains(err.Error(), "empty image") {
		t.Fatalf("empty image error = %v", err)
	}
	if err := doc.RegisterFont("font", nil); err == nil || !strings.Contains(err.Error(), "empty font") {
		t.Fatalf("empty font error = %v", err)
	}
	if err := doc.Table(nil, 0, 0, TableOptions{}); err == nil || !strings.Contains(err.Error(), "requires columns") {
		t.Fatalf("table error = %v", err)
	}
	doc.Close()
	doc.Close()
	if _, err := doc.Bytes(); err == nil || !strings.Contains(err.Error(), "closed") {
		t.Fatalf("closed document error = %v", err)
	}
}

type failingWriter struct{}

func (failingWriter) Write([]byte) (int, error) { return 0, errors.New("write failed") }

func TestWriteToPropagatesWriterError(t *testing.T) {
	doc := newDocument(t)
	n, err := doc.WriteTo(failingWriter{})
	if n != 0 || err == nil || err.Error() != "write failed" {
		t.Fatalf("WriteTo: n=%d err=%v", n, err)
	}
}

func BenchmarkDocument(b *testing.B) {
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		doc, _ := New(A4)
		for row := 0; row < 100; row++ {
			_ = doc.Text("Benchmark row", 40, float32(30+row*7), nil)
		}
		_, _ = doc.Bytes()
		doc.Close()
	}
}

func FuzzImageBoundary(f *testing.F) {
	png, _ := base64.StdEncoding.DecodeString("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAFgwJ/lqL8WQAAAABJRU5ErkJggg==")
	f.Add(png)
	f.Add([]byte("not an image"))
	f.Fuzz(func(t *testing.T, data []byte) {
		doc := newDocument(t)
		err := doc.Image(data, 0, 0, nil)
		if err == nil {
			pdfBytes(t, doc)
		}
	})
}

func FuzzFontBoundary(f *testing.F) {
	f.Add("font", []byte("not a font"))
	f.Add("", []byte{})
	f.Fuzz(func(t *testing.T, name string, data []byte) {
		doc := newDocument(t)
		if len(name) > 128 || len(data) > 1<<20 {
			return
		}
		err := doc.RegisterFont(name, data)
		if err == nil {
			pdfBytes(t, doc)
		}
	})
}

func FuzzTextBoundary(f *testing.F) {
	f.Add("plain text", float32(10), float32(20))
	f.Add("café", float32(0), float32(0))
	f.Fuzz(func(t *testing.T, value string, x, y float32) {
		doc := newDocument(t)
		if len(value) > 4096 {
			return
		}
		err := doc.Text(value, x, y, nil)
		if err == nil {
			pdfBytes(t, doc)
		}
	})
}
