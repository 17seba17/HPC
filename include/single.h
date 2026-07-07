#ifndef SINGLE_H
#define SINGLE_H

#include "image.h"

#include "geometry.h"
#include "options.h"
#include "stats.h"

void render_image(options_t *options, image_geometry_t *geometry,
                  unsigned char *image, render_stats_t *stats);

void render_block(options_t *options, image_geometry_t *geometry, block_t block,
                  unsigned char *image, render_stats_t *stats);

void render_block_brute_force(options_t *options, image_geometry_t *geometry,
                              block_t block, unsigned char *image,
                              render_stats_t *stats);

#endif