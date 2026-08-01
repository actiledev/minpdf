#define PY_SSIZE_T_CLEAN
#include "minpdf.h"
#include <Python.h>

static const char *CAPSULE = "minpdf.document";
static minpdf_document *doc_arg(PyObject *o) {
  return (minpdf_document *)PyCapsule_GetPointer(o, CAPSULE);
}
static void doc_free(PyObject *o) {
  minpdf_document *d = doc_arg(o);
  if (d)
    minpdf_destroy(d);
  PyErr_Clear();
}
static PyObject *failure(minpdf_document *d, minpdf_status s) {
  if (s == MINPDF_OK)
    Py_RETURN_NONE;
  PyErr_SetString(PyExc_RuntimeError, minpdf_error(d));
  return NULL;
}

static PyObject *py_create(PyObject *self, PyObject *args) {
  float w, h;
  minpdf_document *d;
  (void)self;
  if (!PyArg_ParseTuple(args, "ff", &w, &h))
    return NULL;
  d = minpdf_create((minpdf_size){w, h});
  if (!d)
    return PyErr_NoMemory();
  return PyCapsule_New(d, CAPSULE, doc_free);
}
static PyObject *py_page(PyObject *self, PyObject *args) {
  PyObject *o;
  float w, h;
  minpdf_document *d;
  (void)self;
  if (!PyArg_ParseTuple(args, "Off", &o, &w, &h) || (d = doc_arg(o)) == NULL)
    return NULL;
  return failure(d, minpdf_add_page(d, (minpdf_size){w, h}));
}
static PyObject *py_text(PyObject *self, PyObject *args) {
  PyObject *o;
  const char *t, *font;
  float x, y, size, r, g, b, maxw;
  int align, weight;
  minpdf_document *d;
  minpdf_text_options z = minpdf_text_defaults();
  (void)self;
  if (!PyArg_ParseTuple(args, "Osfffffffiis", &o, &t, &x, &y, &size, &r, &g, &b,
                        &maxw, &align, &weight, &font) ||
      (d = doc_arg(o)) == NULL)
    return NULL;
  z.size = size;
  z.color = (minpdf_color){r, g, b};
  z.max_width = maxw;
  z.align = (minpdf_align)align;
  z.weight = (minpdf_weight)weight;
  z.font = *font ? font : NULL;
  return failure(d, minpdf_text(d, t, x, y, &z));
}
static PyObject *py_rect(PyObject *self, PyObject *args) {
  PyObject *o;
  float x, y, w, h, fr, fg, fb, sr, sg, sb, lw, rad;
  int hf, hs;
  minpdf_document *d;
  minpdf_rect_options z = minpdf_rect_defaults();
  (void)self;
  if (!PyArg_ParseTuple(args, "Offffffffffffii", &o, &x, &y, &w, &h, &fr, &fg,
                        &fb, &sr, &sg, &sb, &lw, &rad, &hf, &hs) ||
      (d = doc_arg(o)) == NULL)
    return NULL;
  z.fill = (minpdf_color){fr, fg, fb};
  z.stroke = (minpdf_color){sr, sg, sb};
  z.line_width = lw;
  z.radius = rad;
  z.has_fill = hf;
  z.has_stroke = hs;
  return failure(d, minpdf_rect(d, x, y, w, h, &z));
}
static PyObject *py_line(PyObject *self, PyObject *args) {
  PyObject *o, *dash_arg, *dash_seq, *result;
  float x1, y1, x2, y2, r, g, b, w;
  float *dash = NULL;
  Py_ssize_t i, dash_count;
  minpdf_document *d;
  minpdf_line_options z = minpdf_line_defaults();
  (void)self;
  if (!PyArg_ParseTuple(args, "OffffffffO", &o, &x1, &y1, &x2, &y2, &r, &g,
                        &b, &w, &dash_arg) ||
      (d = doc_arg(o)) == NULL)
    return NULL;
  dash_seq = PySequence_Fast(dash_arg, "dash must be a sequence of numbers");
  if (!dash_seq)
    return NULL;
  dash_count = PySequence_Size(dash_seq);
  if (dash_count > 0) {
    dash = PyMem_New(float, dash_count);
    if (!dash) {
      Py_DECREF(dash_seq);
      return PyErr_NoMemory();
    }
    for (i = 0; i < dash_count; ++i) {
      PyObject *item = PySequence_GetItem(dash_seq, i);
      double value;
      if (!item) {
        PyMem_Free(dash);
        Py_DECREF(dash_seq);
        return NULL;
      }
      value = PyFloat_AsDouble(item);
      Py_DECREF(item);
      if (PyErr_Occurred()) {
        PyMem_Free(dash);
        Py_DECREF(dash_seq);
        return NULL;
      }
      dash[i] = (float)value;
    }
  }
  z.color = (minpdf_color){r, g, b};
  z.width = w;
  z.dash = dash;
  z.dash_count = (size_t)dash_count;
  result = failure(d, minpdf_line(d, x1, y1, x2, y2, &z));
  PyMem_Free(dash);
  Py_DECREF(dash_seq);
  return result;
}
static PyObject *py_circle(PyObject *self, PyObject *args) {
  PyObject *o;
  float x, y, rr, fr, fg, fb, sr, sg, sb, lw;
  int hf, hs;
  minpdf_document *d;
  minpdf_rect_options z = minpdf_rect_defaults();
  (void)self;
  if (!PyArg_ParseTuple(args, "Offffffffffii", &o, &x, &y, &rr, &fr, &fg, &fb,
                        &sr, &sg, &sb, &lw, &hf, &hs) ||
      (d = doc_arg(o)) == NULL)
    return NULL;
  z.fill = (minpdf_color){fr, fg, fb};
  z.stroke = (minpdf_color){sr, sg, sb};
  z.line_width = lw;
  z.has_fill = hf;
  z.has_stroke = hs;
  return failure(d, minpdf_circle(d, x, y, rr, &z));
}
static PyObject *py_image(PyObject *self, PyObject *args) {
  PyObject *o, *data;
  char *buf;
  Py_ssize_t len;
  float x, y, w, h;
  minpdf_document *d;
  minpdf_image_options z;
  (void)self;
  if (!PyArg_ParseTuple(args, "OOffff", &o, &data, &x, &y, &w, &h) ||
      (d = doc_arg(o)) == NULL)
    return NULL;
  if (PyBytes_AsStringAndSize(data, &buf, &len) < 0)
    return NULL;
  z.width = w;
  z.height = h;
  return failure(d, minpdf_image(d, buf, (size_t)len, x, y, &z));
}
static PyObject *py_font(PyObject *self, PyObject *args) {
  PyObject *o, *data;
  char *buf;
  Py_ssize_t len;
  const char *name;
  minpdf_document *d;
  (void)self;
  if (!PyArg_ParseTuple(args, "OsO", &o, &name, &data) ||
      (d = doc_arg(o)) == NULL)
    return NULL;
  if (PyBytes_AsStringAndSize(data, &buf, &len) < 0)
    return NULL;
  return failure(d, minpdf_register_font(d, name, buf, (size_t)len));
}
static PyObject *py_link(PyObject *self, PyObject *args) {
  PyObject *o;
  const char *label, *url;
  float x, y, r, g, b, size;
  int underline;
  minpdf_document *d;
  minpdf_link_options z;
  (void)self;
  if (!PyArg_ParseTuple(args, "Ossffffffi", &o, &label, &url, &x, &y, &r, &g,
                        &b, &size, &underline) ||
      (d = doc_arg(o)) == NULL)
    return NULL;
  z.color = (minpdf_color){r, g, b};
  z.size = size;
  z.underline = underline;
  return failure(d, minpdf_link(d, label, url, x, y, &z));
}
static PyObject *py_metadata(PyObject *self, PyObject *args) {
  PyObject *o;
  const char *a, *b, *c, *e, *f, *g;
  minpdf_document *d;
  (void)self;
  if (!PyArg_ParseTuple(args, "Ossssss", &o, &a, &b, &c, &e, &f, &g) ||
      (d = doc_arg(o)) == NULL)
    return NULL;
  minpdf_metadata m = {*a ? a : NULL, *b ? b : NULL, *c ? c : NULL,
                       *e ? e : NULL, *f ? f : NULL, *g ? g : NULL};
  return failure(d, minpdf_set_metadata(d, &m));
}
static PyObject *py_build(PyObject *self, PyObject *args) {
  PyObject *o;
  minpdf_document *d;
  minpdf_buffer b = {0};
  PyObject *r;
  (void)self;
  if (!PyArg_ParseTuple(args, "O", &o) || (d = doc_arg(o)) == NULL)
    return NULL;
  if (minpdf_build(d, &b) != MINPDF_OK) {
    PyErr_SetString(PyExc_RuntimeError, minpdf_error(d));
    return NULL;
  }
  r = PyBytes_FromStringAndSize((const char *)b.data, (Py_ssize_t)b.length);
  minpdf_buffer_free(&b);
  return r;
}
static PyMethodDef methods[] = {{"create", py_create, METH_VARARGS, NULL},
                                {"page", py_page, METH_VARARGS, NULL},
                                {"text", py_text, METH_VARARGS, NULL},
                                {"rect", py_rect, METH_VARARGS, NULL},
                                {"line", py_line, METH_VARARGS, NULL},
                                {"circle", py_circle, METH_VARARGS, NULL},
                                {"image", py_image, METH_VARARGS, NULL},
                                {"register_font", py_font, METH_VARARGS, NULL},
                                {"link", py_link, METH_VARARGS, NULL},
                                {"metadata", py_metadata, METH_VARARGS, NULL},
                                {"build", py_build, METH_VARARGS, NULL},
                                {NULL, NULL, 0, NULL}};
static struct PyModuleDef module = {PyModuleDef_HEAD_INIT, "_minpdf", NULL, -1,
                                    methods};
PyMODINIT_FUNC PyInit__minpdf(void) { return PyModule_Create(&module); }
