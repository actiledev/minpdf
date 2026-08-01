// Package minpdf creates PDF documents through the shared minpdf C11 file.
package minpdf

/*
#cgo CFLAGS: -std=c11 -O2 -I${SRCDIR}/include
#include <stdlib.h>
#include "minpdf.h"
*/
import "C"

import (
	"errors"
	"io"
	"os"
	"runtime"
	"unsafe"
)

type Size struct{ Width, Height float32 }
type Color struct{ R, G, B float32 }
type Align int
type Weight int

const (
	AlignLeft Align = iota
	AlignCenter
	AlignRight
)
const (
	WeightNormal Weight = iota
	WeightBold
	WeightItalic
	WeightBoldItalic
)

var (
	A3     = Size{842, 1191}
	A4     = Size{595, 842}
	A5     = Size{420, 595}
	Letter = Size{612, 792}
	Black  = RGB(0, 0, 0)
)

func RGB(r, g, b uint8) Color { return Color{float32(r) / 255, float32(g) / 255, float32(b) / 255} }

type TextOptions struct {
	Size     float32
	Color    Color
	Align    Align
	Weight   Weight
	MaxWidth float32
	Font     string
}
type RectOptions struct {
	Fill, Stroke      *Color
	LineWidth, Radius float32
}
type LineOptions struct {
	Color Color
	Width float32
	Dash  []float32
}
type ImageOptions struct{ Width, Height float32 }
type LinkOptions struct {
	Color     Color
	Underline bool
	Size      float32
}
type TableColumn struct {
	Header string
	Width  float32
	Align  Align
}
type TableOptions struct {
	Columns                                    []TableColumn
	HeaderBackground, HeaderColor, BorderColor Color
	FontSize, Padding                          float32
}
type Metadata struct{ Title, Author, Subject, Keywords, Creator, CreationDate string }

type Document struct {
	ptr    *C.minpdf_document
	closed bool
}

func cSize(s Size) C.minpdf_size {
	return C.minpdf_size{width: C.float(s.Width), height: C.float(s.Height)}
}
func cColor(c Color) C.minpdf_color {
	return C.minpdf_color{r: C.float(c.R), g: C.float(c.G), b: C.float(c.B)}
}

func New(size Size) (*Document, error) {
	p := C.minpdf_create(cSize(size))
	if p == nil {
		return nil, errors.New("minpdf: cannot create document")
	}
	d := &Document{ptr: p}
	runtime.SetFinalizer(d, (*Document).Close)
	return d, nil
}
func (d *Document) err(status C.minpdf_status) error {
	if status == C.MINPDF_OK {
		return nil
	}
	if d == nil || d.ptr == nil {
		return errors.New("minpdf: document is closed")
	}
	return errors.New("minpdf: " + C.GoString(C.minpdf_error(d.ptr)))
}
func (d *Document) valid() error {
	if d == nil || d.ptr == nil || d.closed {
		return errors.New("minpdf: document is closed")
	}
	return nil
}
func (d *Document) Close() {
	if d != nil && d.ptr != nil {
		C.minpdf_destroy(d.ptr)
		d.ptr = nil
		d.closed = true
		runtime.SetFinalizer(d, nil)
	}
}
func (d *Document) Page(size Size) error {
	if e := d.valid(); e != nil {
		return e
	}
	return d.err(C.minpdf_add_page(d.ptr, cSize(size)))
}
func (d *Document) Text(text string, x, y float32, options *TextOptions) error {
	if e := d.valid(); e != nil {
		return e
	}
	s := C.CString(text)
	defer C.free(unsafe.Pointer(s))
	co := C.minpdf_text_defaults()
	var font *C.char
	if options != nil {
		co.size = C.float(options.Size)
		if co.size <= 0 {
			co.size = 12
		}
		co.color = cColor(options.Color)
		co.align = C.minpdf_align(options.Align)
		co.weight = C.minpdf_weight(options.Weight)
		co.max_width = C.float(options.MaxWidth)
		if options.Font != "" {
			font = C.CString(options.Font)
			defer C.free(unsafe.Pointer(font))
			co.font = font
		}
	}
	return d.err(C.minpdf_text(d.ptr, s, C.float(x), C.float(y), &co))
}
func (d *Document) Rect(x, y, w, h float32, options *RectOptions) error {
	if e := d.valid(); e != nil {
		return e
	}
	o := C.minpdf_rect_defaults()
	if options != nil {
		o.line_width = C.float(options.LineWidth)
		o.radius = C.float(options.Radius)
		if options.Fill != nil {
			o.fill = cColor(*options.Fill)
			o.has_fill = 1
		}
		if options.Stroke != nil {
			o.stroke = cColor(*options.Stroke)
			o.has_stroke = 1
		}
	}
	return d.err(C.minpdf_rect(d.ptr, C.float(x), C.float(y), C.float(w), C.float(h), &o))
}
func (d *Document) Line(x1, y1, x2, y2 float32, options *LineOptions) error {
	if e := d.valid(); e != nil {
		return e
	}
	o := C.minpdf_line_defaults()
	var dash unsafe.Pointer
	if options != nil {
		o.color = cColor(options.Color)
		o.width = C.float(options.Width)
		if len(options.Dash) > 0 {
			dash = C.CBytes(unsafe.Slice((*byte)(unsafe.Pointer(&options.Dash[0])), len(options.Dash)*4))
			if dash == nil {
				return errors.New("minpdf: cannot allocate dash pattern")
			}
			defer C.free(dash)
			o.dash = (*C.float)(dash)
			o.dash_count = C.size_t(len(options.Dash))
		}
	}
	return d.err(C.minpdf_line(d.ptr, C.float(x1), C.float(y1), C.float(x2), C.float(y2), &o))
}
func (d *Document) Circle(cx, cy, r float32, options *RectOptions) error {
	if e := d.valid(); e != nil {
		return e
	}
	o := C.minpdf_rect_defaults()
	if options != nil {
		o.line_width = C.float(options.LineWidth)
		if options.Fill != nil {
			o.fill = cColor(*options.Fill)
			o.has_fill = 1
		}
		if options.Stroke != nil {
			o.stroke = cColor(*options.Stroke)
			o.has_stroke = 1
		}
	}
	return d.err(C.minpdf_circle(d.ptr, C.float(cx), C.float(cy), C.float(r), &o))
}
func (d *Document) Image(data []byte, x, y float32, options *ImageOptions) error {
	if e := d.valid(); e != nil {
		return e
	}
	if len(data) == 0 {
		return errors.New("minpdf: empty image")
	}
	o := C.minpdf_image_defaults()
	if options != nil {
		o.width = C.float(options.Width)
		o.height = C.float(options.Height)
	}
	return d.err(C.minpdf_image(d.ptr, unsafe.Pointer(&data[0]), C.size_t(len(data)), C.float(x), C.float(y), &o))
}
func (d *Document) RegisterFont(name string, data []byte) error {
	if e := d.valid(); e != nil {
		return e
	}
	if len(data) == 0 {
		return errors.New("minpdf: empty font")
	}
	n := C.CString(name)
	defer C.free(unsafe.Pointer(n))
	return d.err(C.minpdf_register_font(d.ptr, n, unsafe.Pointer(&data[0]), C.size_t(len(data))))
}
func (d *Document) Link(label, url string, x, y float32, options *LinkOptions) error {
	if e := d.valid(); e != nil {
		return e
	}
	l := C.CString(label)
	u := C.CString(url)
	defer C.free(unsafe.Pointer(l))
	defer C.free(unsafe.Pointer(u))
	o := C.minpdf_link_defaults()
	if options != nil {
		o.color = cColor(options.Color)
		o.underline = 0
		if options.Underline {
			o.underline = 1
		}
		o.size = C.float(options.Size)
	}
	return d.err(C.minpdf_link(d.ptr, l, u, C.float(x), C.float(y), &o))
}
func (d *Document) SetMetadata(m Metadata) error {
	if e := d.valid(); e != nil {
		return e
	}
	values := []string{m.Title, m.Author, m.Subject, m.Keywords, m.Creator, m.CreationDate}
	cs := make([]*C.char, 6)
	for i, v := range values {
		if v != "" {
			cs[i] = C.CString(v)
			defer C.free(unsafe.Pointer(cs[i]))
		}
	}
	cm := C.minpdf_metadata{title: cs[0], author: cs[1], subject: cs[2], keywords: cs[3], creator: cs[4], creation_date: cs[5]}
	return d.err(C.minpdf_set_metadata(d.ptr, &cm))
}
func (d *Document) Bytes() ([]byte, error) {
	if e := d.valid(); e != nil {
		return nil, e
	}
	var b C.minpdf_buffer
	if e := d.err(C.minpdf_build(d.ptr, &b)); e != nil {
		return nil, e
	}
	defer C.minpdf_buffer_free(&b)
	return C.GoBytes(unsafe.Pointer(b.data), C.int(b.length)), nil
}
func (d *Document) WriteTo(w io.Writer) (int64, error) {
	b, e := d.Bytes()
	if e != nil {
		return 0, e
	}
	n, e := w.Write(b)
	return int64(n), e
}
func (d *Document) Save(path string) error {
	b, e := d.Bytes()
	if e != nil {
		return e
	}
	return os.WriteFile(path, b, 0644)
}
func (d *Document) Table(rows [][]string, x, y float32, o TableOptions) error {
	if len(o.Columns) == 0 {
		return errors.New("minpdf: table requires columns")
	}
	fs := o.FontSize
	if fs <= 0 {
		fs = 10
	}
	pad := o.Padding
	if pad <= 0 {
		pad = 8
	}
	widths := make([]float32, len(o.Columns))
	total := float32(0)
	for i, c := range o.Columns {
		w := c.Width
		if w <= 0 {
			w = float32(len([]rune(c.Header)))*fs*.52 + 2*pad
			for _, row := range rows {
				if i < len(row) {
					z := float32(len([]rune(row[i])))*fs*.52 + 2*pad
					if z > w {
						w = z
					}
				}
			}
		}
		widths[i] = w
		total += w
	}
	hbg := o.HeaderBackground
	if hbg == (Color{}) {
		hbg = RGB(240, 240, 240)
	}
	hc := o.HeaderColor
	bc := o.BorderColor
	if bc == (Color{}) {
		bc = RGB(204, 204, 204)
	}
	if err := d.Rect(x, y, total, fs+2*pad, &RectOptions{Fill: &hbg}); err != nil {
		return err
	}
	cx := x
	for i, c := range o.Columns {
		tx := cx + pad
		if c.Align == AlignCenter {
			tx = cx + widths[i]/2
		} else if c.Align == AlignRight {
			tx = cx + widths[i] - pad
		}
		if err := d.Text(c.Header, tx, y+pad+fs*.8, &TextOptions{Size: fs, Color: hc, Align: c.Align, Weight: WeightBold}); err != nil {
			return err
		}
		cx += widths[i]
	}
	cy := y + fs + 2*pad
	for ri, row := range rows {
		fill := RGB(255, 255, 255)
		if ri%2 == 1 {
			fill = RGB(249, 249, 249)
		}
		if err := d.Rect(x, cy, total, fs+2*pad, &RectOptions{Fill: &fill}); err != nil {
			return err
		}
		cx = x
		for i, c := range o.Columns {
			value := ""
			if i < len(row) {
				value = row[i]
			}
			tx := cx + pad
			if c.Align == AlignCenter {
				tx = cx + widths[i]/2
			} else if c.Align == AlignRight {
				tx = cx + widths[i] - pad
			}
			if err := d.Text(value, tx, cy+pad+fs*.8, &TextOptions{Size: fs, Color: Black, Align: c.Align}); err != nil {
				return err
			}
			cx += widths[i]
		}
		cy += fs + 2*pad
	}
	return d.Rect(x, y, total, float32(len(rows)+1)*(fs+2*pad), &RectOptions{Stroke: &bc, LineWidth: 1})
}
