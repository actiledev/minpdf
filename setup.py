from setuptools import Extension, setup

setup(
  options={"bdist_wheel": {"py_limited_api": "cp39"}},
  ext_modules=[Extension(
    "_minpdf",
    ["bindings/python/src/_minpdf.c", "core.c"],
    include_dirs=["include"],
    define_macros=[("Py_LIMITED_API", "0x03090000")],
    py_limited_api=True,
)])
