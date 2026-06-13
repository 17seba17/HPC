#include "../include/image.h"
#include "../include/kernel.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>





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







void render_image (options_t        *options,
	      image_geometry_t *geometry,
	      unsigned char    *image,
	      render_stats_t   *stats)
{
  unsigned int block_x;
  unsigned int block_y;
  block_t block;
  memset(&block, 0, sizeof(block));

  stats->checksum = FNV_OFFSET_BASIS;

  memset(stats->my_checksum.slot, 0, sizeof(stats->my_checksum.slot));

  stats->total_iterations = 0u;
  stats->inside_pixels = 0u;

  


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
                 render_block(options, geometry, block, image, stats);

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
  uint64_t local_checksum;
  checksum_t local_my_checksum;
  uint64_t     local_total_iterations;
  uint64_t     local_inside_pixels;
  double       cr;
  double       ci;
  size_t       pixel_offset;

     local_checksum = stats->checksum;
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

        local_checksum = checksum_update_uint(local_checksum, iteration);
        local_my_checksum = my_checksum_update(local_my_checksum, row, col, iteration);



        local_total_iterations += (uint64_t) iteration;

          if (iteration >= options->kmax)
            local_inside_pixels += 1u;
        }
    }

  stats->checksum = local_checksum;
  stats->my_checksum = local_my_checksum;
  stats->total_iterations = local_total_iterations;
  stats->inside_pixels = local_inside_pixels;
}


void render_block (options_t        *options,
	       image_geometry_t *geometry,
	       block_t      block,
	       unsigned char    *image,
	       render_stats_t   *stats
	       ){


unsigned int start_x = block.start_x;

unsigned int width = block.end_x-block.start_x;
unsigned int height = block.end_y-block.start_y;

if(width*height<1024){
   render_block_brute_force(options,geometry,block, image, stats);
   return;
}




unsigned int mid_x = (block.start_x + block.end_x) / 2;
unsigned int mid_y = (block.start_y + block.end_y) / 2;


unsigned l1=width/2;
unsigned h1=height/2;

unsigned int upper_left = block.count_up > l1 ? l1 : block.count_up;
unsigned int upper_right = block.count_up > l1 ?  block.count_up -l1 : 0;
unsigned int down_left = block.count_down > l1 ? l1 : block.count_down;
unsigned int down_right = block.count_down > l1 ?  block.count_down -l1 : 0;

unsigned int left_up = block.count_left > h1 ? h1 : block.count_left;
unsigned int left_down = block.count_left > h1 ?  block.count_left-h1 : 0;
unsigned int right_up = block.count_right > h1 ? h1 : block.count_right;
unsigned int right_down = block.count_right > h1 ?  block.count_right -h1: 0;

unsigned int row_col=block.start_y; // single variable for indicating or abscissa or ordinate

if(upper_left == 0){
  upper_left = checkHorizontalStripe(options, geometry, row_col, block.start_x, mid_x);
}
if(upper_right == 0){
  upper_right =  checkHorizontalStripe(options, geometry, row_col, mid_x, block.end_x);
}

row_col=block.end_y-1;

if(down_left == 0){
  down_left = checkHorizontalStripe(options, geometry, row_col, block.start_x, mid_x);
}

if(down_right == 0){
down_right = checkHorizontalStripe(options, geometry, row_col, mid_x, block.end_x);
}




row_col=block.start_x;

if(left_up == 0){
  left_up = checkVerticalStripe(options, geometry, row_col, block.start_y, mid_y);
}
if(left_down == 0){
  left_down =  checkVerticalStripe(options, geometry, row_col, mid_y, block.end_y);
}

row_col=block.end_x-1;

if(right_up == 0){
  right_up = checkVerticalStripe(options, geometry, row_col, block.start_y, mid_y);
}

if(right_down == 0){
    right_down = checkVerticalStripe(options, geometry, row_col, mid_y, block.end_y); 
}




if(upper_left+upper_right+down_right+down_left+left_up+left_down+right_up+right_down==2*height+2*width){ // if black

uint64_t local_checksum;
uint64_t local_total_iterations;
uint64_t local_inside_pixels;

local_checksum = stats->checksum;
local_total_iterations = stats->total_iterations;
local_inside_pixels = stats->inside_pixels;

const uint64_t kmax = options->kmax;
const size_t width = geometry->width;
    for (unsigned int r = block.start_y ; r < block.end_y ; ++r) {
        for (unsigned int c_idx = block.start_x ; c_idx < block.end_x ; ++c_idx) {
            size_t pixel_offset = ((size_t) r * width + (size_t) c_idx) * 3u;
            colour_from_iteration(kmax, kmax, &image[pixel_offset]);

            local_checksum = checksum_update_uint (local_checksum, kmax);
           
            local_total_iterations += kmax;
           local_inside_pixels += 1u;
        }
    }

    stats->checksum =local_checksum;
    stats->total_iterations =local_total_iterations;
    stats->inside_pixels =local_inside_pixels;

    return;
}





block_t NW={block.start_x,
            block.start_y,
            mid_x,
            mid_y,
            upper_left,0,left_up,0};

block_t NE={mid_x,
            block.start_y,
            block.end_x,
            mid_y,
            upper_right,0,0,right_up};

block_t SE={mid_x,
            mid_y,
            block.end_x,
            block.end_y,
            0,down_right,0,right_down};

block_t SW={block.start_x,
            mid_y,
             mid_x,
            block.end_y,
            0,down_left,left_down,0};



render_block(options,geometry,NW,image,stats);
render_block(options,geometry,SE,image,stats);
render_block(options,geometry,SW,image,stats);
render_block(options,geometry,NE,image,stats);

}
unsigned int checkHorizontalStripe(options_t *options, image_geometry_t *geometry, const     unsigned int row, const unsigned int x_begin, const unsigned int x_end){
  
    
  unsigned int counter=0;

    const double ci = options->ymax - ((double) (row) + 0.5) * geometry->dy;
    


//inizio del for
for (unsigned int col=x_begin;  col <x_end; ++col){
    double cr = options->xmin + ((double) col + 0.5) * geometry->dx;
    unsigned int iteration = mandelbrot_escape (cr, ci, options->kmax);

    if(iteration!=options->kmax){
        return counter;
    }

    counter++;
 
}
    return counter;
}

 unsigned int checkVerticalStripe(options_t *options, image_geometry_t *geometry, const unsigned 
    int col, const unsigned int y_begin, const unsigned int y_end){


      unsigned int counter=0;
        const double cr = options->xmin +((double)col + 0.5)* geometry->dx;
        



// inizio del for  
for (unsigned int row=y_begin; row < y_end; ++row){
    double ci = options-> ymax- ((double) row + 0.5) * geometry->dy;
    unsigned int iteration = mandelbrot_escape (cr, ci, options->kmax);
    
    // se sono diversi allora non ha senso continuare
    if(iteration!=options->kmax){
        return counter;
    }
  counter++;
}
    return counter;

}


