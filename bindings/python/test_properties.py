import os

from hypothesis import HealthCheck, given, settings, strategies as st

from minpdf import PDF, TextOptions


FUZZ_EXAMPLES = int(os.environ.get("MINPDF_FUZZ_CASES", "100"))
COMMON_SETTINGS = settings(
    max_examples=FUZZ_EXAMPLES,
    deadline=None,
    suppress_health_check=[HealthCheck.too_slow],
)


@COMMON_SETTINGS
@given(st.binary(max_size=65536))
def test_image_boundary_never_crashes(data: bytes) -> None:
    pdf = PDF()
    try:
        pdf.image(data, 0, 0)
    except RuntimeError:
        return
    output = pdf.bytes()
    assert output.startswith(b"%PDF-1.4")
    assert output.endswith(b"%%EOF\n")


@COMMON_SETTINGS
@given(
    st.text(max_size=256),
    st.floats(-10000, 10000, allow_nan=False, allow_infinity=False),
    st.floats(-10000, 10000, allow_nan=False, allow_infinity=False),
)
def test_text_boundary_never_crashes(value: str, x: float, y: float) -> None:
    pdf = PDF()
    try:
        pdf.text(value, x, y, TextOptions(max_width=100))
    except (RuntimeError, UnicodeEncodeError, ValueError):
        return
    output = pdf.bytes()
    assert b"xref" in output


@COMMON_SETTINGS
@given(st.text(max_size=128), st.binary(max_size=65536))
def test_font_boundary_never_crashes(name: str, data: bytes) -> None:
    pdf = PDF()
    try:
        pdf.register_font(name, data)
    except (RuntimeError, UnicodeEncodeError, ValueError):
        return
    assert pdf.bytes().startswith(b"%PDF-1.4")
