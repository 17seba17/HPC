#include "../include/image.h"
#include "../include/stats.h"
#include "../include/single.h"
#include "../include/kernel.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>



void render_image (options_t        *options,
	      image_geometry_t *geometry,
	      unsigned char    *image,
	      render_stats_t   *stats)
{
  unsigned int block_x;
  unsigned int block_y;
  block_t block;
  memset(&block, 0, sizeof(block));


  for(block_x=0; block_x < options->dx_factor;++block_x){
  for(block_y=0; block_y < options->dy_factor;++block_y){
    block.start_x = (unsigned int) (((uint64_t) block_x * geometry->width)
                                  / options->dx_factor);
    block.end_x = (unsigned int) (((uint64_t) (block_x + 1u) * geometry->width)
                                  / options->dx_factor);
    block.start_y = (unsigned int) (((uint64_t) block_y * geometry->height)
                                  / options->dy_factor);
    block.end_y = (unsigned int) (((uint64_t) (block_y + 1u) * geometry->height)
                                  / options->dy_factor);
               render_block_brute_force(options, geometry, block, image, stats);

  }
  }
}


void render_block_brute_force (options_t        *options,
	       image_geometry_t *geometry,
	       block_t      block,
	       unsigned char    *image,
	       render_stats_t   *stats
	       )
{

  unsigned int row;
  unsigned int col;
  unsigned int iteration;

  checksum_t local_my_checksum;
  uint64_t     local_total_iterations;
  uint64_t     local_inside_pixels;

  double       cr;
  double       ci;
  size_t       pixel_offset;

     local_my_checksum = stats->my_checksum;
     local_total_iterations = stats->total_iterations;
     local_inside_pixels = stats->inside_pixels;

  for (row = block.start_y; row < block.end_y; ++row)
    {
      ci = options->ymax - ((double) row + 0.5) * geometry->dy;

      for (col = block.start_x; col < block.end_x; ++col)
        {
          cr = options->xmin + ((double) col + 0.5) * geometry->dx;
          iteration = mandelbrot_escape (cr, ci, options->kmax);
          
          pixel_offset = ((size_t) row * (size_t) geometry->width
                          + (size_t) col) * 3u;
          colour_from_iteration (iteration, options->kmax, &image[pixel_offset]);

        local_my_checksum = my_checksum_update(local_my_checksum, row, col, iteration);          local_total_iterations += (uint64_t) iteration;

          if (iteration >= options->kmax)
            local_inside_pixels += 1u;
        }
    }

  stats->my_checksum = local_my_checksum;
  stats->total_iterations = local_total_iterations;
  stats->inside_pixels = local_inside_pixels;
}




