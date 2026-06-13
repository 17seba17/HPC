#include "../include/image.h"
#include "../include/kernel.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static void colour_from_iteration (unsigned int    iteration,
		       unsigned int    kmax,
		       unsigned char  *rgb
		       );

void render_image (options_t        *options,
	      image_geometry_t *geometry,
	      unsigned char    *image,
	      render_stats_t   *stats)
{
  unsigned int stripe;
  unsigned int row_begin;
  unsigned int row_end;
  stats->checksum = FNV_OFFSET_BASIS;
  stats->total_iterations = 0u;
  stats->inside_pixels = 0u;

  

  for (stripe = 0; stripe < options->dy_factor; ++stripe)
    {
      row_begin = (unsigned int) (((uint64_t) stripe * geometry->height)
                                  / options->dy_factor);
      row_end = (unsigned int) (((uint64_t) (stripe + 1u) * geometry->height)
                                / options->dy_factor);

      if (row_begin < row_end)
        render_stripe (options, geometry, row_begin, row_end, image, stats);
    }
  }



void render_stripe (options_t        *options,
	       image_geometry_t *geometry,
	       unsigned int      row_begin,
	       unsigned int      row_end,
	       unsigned char    *image,
	       render_stats_t   *stats
	       )
{
  unsigned int row;
  unsigned int col;
  unsigned int iteration;
  uint64_t     local_checksum;
  uint64_t     local_total_iterations;
  uint64_t     local_inside_pixels;
  double       cr;
  double       ci;
  size_t       pixel_offset;

  local_checksum = stats->checksum;
  local_total_iterations = stats->total_iterations;
  local_inside_pixels = stats->inside_pixels;

  for (row = row_begin; row < row_end; ++row)
    {
      // Row zero is the top of the image, hence the minus sign from ymax.
      ci = options->ymax - ((double) row + 0.5) * geometry->dy;

      for (col = 0; col < geometry->width; ++col)
        {
          cr = options->xmin + ((double) col + 0.5) * geometry->dx;
          iteration = mandelbrot_escape (cr, ci, options->kmax);

          pixel_offset = ((size_t) row * (size_t) geometry->width
                          + (size_t) col) * 3u;
          colour_from_iteration (iteration, options->kmax, &image[pixel_offset]);
          if(iteration<DEFAULT_KMAX){
          local_checksum = checksum_update_uint (local_checksum, iteration);}
          local_total_iterations += (uint64_t) iteration;
          stats->my_checksum = my_checksum_update(stats->my_checksum, row, col, iteration);
          if (iteration >= options->kmax)
            local_inside_pixels += 1u;
        }
    }

  stats->checksum = local_checksum;
  stats->total_iterations = local_total_iterations;
  stats->inside_pixels = local_inside_pixels;
}


static void colour_from_iteration (unsigned int    iteration,
		       unsigned int    kmax,
		       unsigned char  *rgb
		       )
{
  double t;
  double one_minus_t;

  if (iteration >= kmax)
    {
      rgb[0] = 0u;
      rgb[1] = 0u;
      rgb[2] = 0u;
      return;
    }

  t = (double) iteration / (double) kmax;
  one_minus_t = 1.0 - t;

  rgb[0] = (unsigned char) (255.0 * 9.0 * one_minus_t * t * t * t);
  rgb[1] = (unsigned char) (255.0 * 15.0 * one_minus_t * one_minus_t * t * t);
  rgb[2] = (unsigned char) (255.0 * 8.5 * one_minus_t * one_minus_t * one_minus_t * t);
}




