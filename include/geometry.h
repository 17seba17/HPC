#ifndef GEOMETRY_H
#define GEOMETRY_H
#include "options.h"
typedef struct {
  unsigned int width;
  unsigned int height;
  double dx;
  double dy;
} image_geometry_t;

int make_geometry(options_t *options, image_geometry_t *geometry);

#endif