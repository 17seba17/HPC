#ifndef     CONSUMER_H
#define     CONSUMER_H

#include "options.h"
#include "geometry.h"
#include "stats.h"
#include "image.h"



void rendering_block (options_t        *options,
	       image_geometry_t *geometry,
	       block_t      block,
	       render_stats_t   *stats, unsigned int in_flight
	       );

void drawAllImage(options_t        *options,
	       image_geometry_t *geometry,
	       block_t      block,
	       render_stats_t   *stats
	       );


 unsigned int checkVerticalStripe(options_t *options, image_geometry_t *geometry, const unsigned int col, const unsigned int y_begin, const unsigned int y_end);
 unsigned int checkHorizontalStripe(options_t *options, image_geometry_t *geometry, const unsigned int col, const unsigned int y_begin, const unsigned int y_end);


#endif