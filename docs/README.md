# minpdf documentation

minpdf is a PDF generation library powered by one C11 file. The C/C++, Go,
Python, JavaScript, and browser APIs share the same page model, coordinate
system, drawing behavior, image decoder, font parser, and PDF serializer.

## Start here

- [Installation](installation.md): requirements and setup for every language
- [Core concepts](concepts.md): pages, coordinates, colors, units, and resources
- [Shared API](api.md): operation mapping and default behavior
- [Complete example](example.md): the same invoice in each language

## Language guides

- [C and C++](c.md)
- [Go](go.md)
- [Python](python.md)
- [JavaScript and TypeScript](javascript.md)
- [Browser deployment](browser.md)

## Feature guides

- [Text and fonts](text-and-fonts.md)
- [Images](images.md)
- [Tables](tables.md)
- [Output, errors, and ownership](output-and-errors.md)
- [Performance and development](development.md)

## Supported in version 0.1.0

- PDF 1.4 generation
- A3, A4, A5, Letter, and arbitrary page sizes
- Built-in Helvetica normal, bold, italic, and bold-italic
- Embedded TrueType fonts with UTF-8 mapping and searchable Unicode output
- Text alignment and word wrapping
- Filled or stroked rectangles, rounded rectangles, circles, and lines
- JPEG and non-interlaced 8-bit PNG images, including transparency
- URL annotations, metadata, simple tables, and multiple pages

minpdf generates new PDFs. Reading, editing, merging, encrypting, signing, or
rendering existing PDF files is not supported.
