#include "../include/minpdf.h"

int main() {
  minpdf_document *doc = minpdf_create(MINPDF_A4);
  minpdf_destroy(doc);
  return 0;
}
