#ifndef    WORKER_H
#define    WORKER_H

#include "options.h"
#include "geometry.h"
#include "stats.h"
#include "../include/kernel.h"
#include "../include/image.h"


void rendering_block (options_t        *options,
	       image_geometry_t *geometry,
	       block_t      block,
	       render_stats_t   *stats, block_t three_blocks[3], unsigned char *image);

void drawAllImage(options_t        *options,
	       image_geometry_t *geometry,
	       block_t      block,
	       render_stats_t   *stats, unsigned char *image
	       );


#endif