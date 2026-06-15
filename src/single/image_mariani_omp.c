#include "../../include/single.h"
#include "../../include/kernel.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>


#pragma omp declare reduction(checksum_add : checksum_t : \
    omp_out.slot[0] += omp_in.slot[0], \
    omp_out.slot[1] += omp_in.slot[1], \
    omp_out.slot[2] += omp_in.slot[2], \
    omp_out.slot[3] += omp_in.slot[3]) \
    initializer(omp_priv = {{0, 0, 0, 0}})



void render_image (options_t        *options,
                   image_geometry_t *geometry,
                   unsigned char    *image,
                   render_stats_t   *stats)
{

    uint64_t total_iterations = 0;
    uint64_t inside_pixels = 0;
    checksum_t my_checksum = {{0, 0, 0, 0}};

    #pragma omp parallel for collapse(2) schedule(dynamic, 1) \
        reduction(+:total_iterations, inside_pixels) \
        reduction(checksum_add:my_checksum)
    for(unsigned int block_x = 0; block_x < options->dx_factor; ++block_x) {
        for(unsigned int block_y = 0; block_y < options->dy_factor; ++block_y) {
            
            block_t block;
            memset(&block, 0, sizeof(block));
            
            block.start_x = (unsigned int) (((uint64_t) block_x * geometry->width) / options->dx_factor);
            block.end_x   = (unsigned int) (((uint64_t) (block_x + 1u) * geometry->width) / options->dx_factor);
            block.start_y = (unsigned int) (((uint64_t) block_y * geometry->height) / options->dy_factor);
            block.end_y   = (unsigned int) (((uint64_t) (block_y + 1u) * geometry->height) / options->dy_factor);
            
            render_stats_t local_stats;
            memset(local_stats.my_checksum.slot, 0, sizeof(local_stats.my_checksum.slot));
            local_stats.total_iterations = 0;
            local_stats.inside_pixels = 0;

            render_block(options, geometry, block, image, &local_stats);

            total_iterations += local_stats.total_iterations;
            inside_pixels    += local_stats.inside_pixels;
            

            for (int i = 0; i < 4; i++) {
                my_checksum.slot[i] += local_stats.my_checksum.slot[i];
            }
        }
    }

    stats->total_iterations = total_iterations;
    stats->inside_pixels    = inside_pixels;
    stats->my_checksum      = my_checksum;
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

        local_my_checksum = my_checksum_update(local_my_checksum, row, col, iteration);



        local_total_iterations += (uint64_t) iteration;

          if (iteration >= options->kmax)
            local_inside_pixels += 1u;
        }
    }

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

uint64_t block_area = width * height;
stats->total_iterations += block_area * (uint64_t)options->kmax;
stats->inside_pixels += block_area;

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