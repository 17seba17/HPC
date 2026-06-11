#ifndef IMAGE_H
#define IMAGE_H


#include "options.h"
#include "geometry.h"
#include "stats.h"
typedef struct
{
  unsigned int          start_x;
  unsigned int          start_y;
  unsigned int          end_x;
  unsigned int          end_y;
  unsigned int                  status_up;
    unsigned int                  status_right;
  unsigned int                  status_down;
    unsigned int                  status_left;
} block_t;
void render_image (options_t        *options,
	      image_geometry_t *geometry,
	      unsigned char    *image,
	      render_stats_t   *stats);

void render_stripe (options_t        *options,
	       image_geometry_t *geometry,
	       unsigned int      row_begin,
	       unsigned int      row_end,
	       unsigned char    *image,
	       render_stats_t   *stats
	       );
void render_block (options_t        *options,
	       image_geometry_t *geometry,
	       block_t      block,
	       unsigned char    *image,
	       render_stats_t   *stats
	       );

void render_block_force_brute (options_t        *options,
	       image_geometry_t *geometry,
	       block_t      block,
	       unsigned char    *image,
	       render_stats_t   *stats
	       );


 unsigned int checkVerticalStripe(options_t *options, image_geometry_t *geometry, const unsigned int col, const unsigned int y_begin, const unsigned int y_end, unsigned char    *image);
 unsigned int checkHorizontalStripe(options_t *options, image_geometry_t *geometry, const unsigned int col, const unsigned int y_begin, const unsigned int y_end, unsigned char    *image);

#endif