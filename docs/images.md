# Images

## Supported formats

### JPEG

JPEG data is embedded using PDF's `DCTDecode`; pixels are not decoded
or recompressed. Baseline and progressive JPEG dimensions are read from their
frame markers.

### PNG

minpdf supports non-interlaced, 8-bit PNG files with these color types:

| Type | Meaning              |
| ---: | -------------------- |
|    0 | Grayscale            |
|    2 | RGB                  |
|    3 | Indexed/palette      |
|    4 | Grayscale with alpha |
|    6 | RGBA                 |

Palette and `tRNS` transparency are supported. Alpha is emitted as a PDF soft
mask. PNG filtering and zlib/DEFLATE decoding are implemented in the C engine.

Interlaced PNGs, 16-bit channels, and other image formats are rejected. Convert
WebP, GIF, SVG, TIFF, HEIC, or BMP to JPEG/PNG before calling minpdf.

## Dimensions

Image coordinates specify the top-left corner. With no options, one image pixel
becomes one PDF point. This is a geometric mapping, not a DPI-aware conversion.

To preserve aspect ratio, supply both dimensions or calculate the missing one:

```text
target height = source height × target width / source width
```

Supplying only width does not scale height, and vice versa.

## Memory behavior

Input bytes are copied into the PDF generator. JPEG remains compressed. Simple
grayscale/RGB PNG streams can remain compressed in the PDF; PNGs requiring
color conversion or transparency are decoded into RGB and optional alpha
buffers, which can use more memory than the source file.

Validate dimensions and file sizes before accepting untrusted uploads. The
decoder checks bounds and malformed streams, but applications should set
appropriate request and resource limits.
