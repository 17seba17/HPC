#ifndef IMAGE_H
#define IMAGE_H

#include "geometry.h"
#include "options.h"

typedef struct {
  unsigned int start_x;
  unsigned int start_y;
  unsigned int end_x;
  unsigned int end_y;
  unsigned int count_up;
  unsigned int count_down;
  unsigned int count_left;
  unsigned int count_right;
} block_t;

unsigned int checkVerticalStripe(options_t *options, image_geometry_t *geometry,
                                 const unsigned int col,
                                 const unsigned int y_begin,
                                 const unsigned int y_end);

unsigned int checkHorizontalStripe(options_t *options,
                                   image_geometry_t *geometry,
                                   const unsigned int col,
                                   const unsigned int y_begin,
                                   const unsigned int y_end);

void colour_from_iteration(unsigned int iteration, unsigned int kmax,
                           unsigned char *rgb);

#endif