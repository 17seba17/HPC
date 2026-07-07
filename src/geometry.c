#include "../include/geometry.h"
#include <limits.h>
#include <math.h>
#include <stdio.h>

int make_geometry(options_t *options, image_geometry_t *geometry) {
  double x_extent;
  double y_extent;
  double width_double;
  double height_double;

  x_extent = options->xmax - options->xmin;
  y_extent = options->ymax - options->ymin;
  width_double = floor(x_extent * (double)options->ppu + 0.5);
  height_double = floor(y_extent * (double)options->ppu + 0.5);

  if (width_double < 1.0 || height_double < 1.0 ||
      width_double > (double)UINT_MAX || height_double > (double)UINT_MAX) {
    fprintf(stderr, "Image dimensions are outside the supported range\n");
    return -1;
  }

  geometry->width = (unsigned int)width_double;
  geometry->height = (unsigned int)height_double;
  geometry->dx = x_extent / (double)geometry->width;
  geometry->dy = y_extent / (double)geometry->height;

  return 0;
}