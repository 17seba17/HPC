#include "../include/image.h"
#include "../include/kernel.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
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
  unsigned int block_x;
  unsigned int block_y;
//   unsigned int row_begin;
//   unsigned int row_end;
  block_t block;
  block.status_down=1;
  block.status_left=1;
  block.status_right=1;
  block.status_up=1;
  stats->checksum = FNV_OFFSET_BASIS;
  stats->my_checksum.slot[0] =0;
  stats->my_checksum.slot[1] =0;
  stats->my_checksum.slot[2] =0;
  stats->my_checksum.slot[3] =0;

  stats->total_iterations = 0u;
  stats->inside_pixels = 0u;

  

//   for (stripe = 0; stripe < options->dy_factor; ++stripe)
    // {
    //   row_begin = (unsigned int) (((uint64_t) stripe * geometry->height)
    //                               / options->dy_factor);
    //   row_end = (unsigned int) (((uint64_t) (stripe + 1u) * geometry->height)
    //                             / options->dy_factor);

    //   if (row_begin < row_end)
    //     render_stripe (options, geometry, row_begin, row_end, image, stats);
    // }
  for(block_x=0; block_x < 1;++block_x){
//   for(block_x=0; block_x < options->dx_factor;++block_x){
//   for(block_y=0; block_y < options->dy_factor;++block_y){
  for(block_y=0; block_y < 1;++block_y){
    block.start_x = (unsigned int) (((uint64_t) block_x * geometry->width)
                                  / options->dx_factor);
    block.end_x = (unsigned int) (((uint64_t) (block_x + 1u) * geometry->width)
                                  / options->dx_factor);
    block.start_y = (unsigned int) (((uint64_t) block_y * geometry->height)
                                  / options->dy_factor);
    block.end_y = (unsigned int) (((uint64_t) (block_y + 1u) * geometry->height)
                                  / options->dy_factor);
                // render_block(options, geometry, block, image, stats);
                render_block_force_brute(options, geometry, block, image, stats);

  }
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
          local_checksum = checksum_update_uint (local_checksum, iteration);
          local_total_iterations += (uint64_t) iteration;

          if (iteration >= options->kmax)
            local_inside_pixels += 1u;
        }
    }

  stats->checksum = local_checksum;
  stats->total_iterations = local_total_iterations;
  stats->inside_pixels = local_inside_pixels;
}


void render_block_force_brute (options_t        *options,
	       image_geometry_t *geometry,
	       block_t      block,
	       unsigned char    *image,
	       render_stats_t   *stats
	       )
{

    unsigned int row;
  unsigned int col;
  unsigned int iteration;
//   uint64_t     local_checksum;
checksum_t local_checksum;
  uint64_t     local_total_iterations;
  uint64_t     local_inside_pixels;
  double       cr;
  double       ci;
  size_t       pixel_offset;

//   local_checksum = stats->checksum;
     local_checksum = stats->my_checksum;
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

 //   local_checksum = checksum_update_uint (local_checksum, iteration);
        local_checksum = my_checksum_update(local_checksum, row, col, iteration);          local_total_iterations += (uint64_t) iteration;

          if (iteration >= options->kmax)
            local_inside_pixels += 1u;
        }
    }

//   stats->checksum = local_checksum;
  stats->my_checksum = local_checksum;
  stats->total_iterations = local_total_iterations;
  stats->inside_pixels = local_inside_pixels;
}


void render_block (options_t        *options,
	       image_geometry_t *geometry,
	       block_t      block,
	       unsigned char    *image,
	       render_stats_t   *stats
	       ){

    // printf("x1:%i x2:%iy1:%i y2:%i\n",block.start_x,block.end_x,block.start_y,block.end_y);
if((block.end_x-block.start_x)*(block.end_y-block.start_y)<1024*1024){
    // printf("qua?");
   render_block_force_brute(options,geometry,block, image, stats);
   return;
}




unsigned int a,b,c,d,e,f,g,h;
if(block.status_up==1){
    unsigned int row=block.start_x;
a=checkHorizontalStripe(options, geometry, row, block.start_x, block.end_x/2,image);
b=checkHorizontalStripe(options, geometry, row, block.end_x/2+1, block.end_x-2,image);
}
else{
    a=1;
    b=1;
}


if(block.status_right==1){
    unsigned int col=block.end_y-1;

    c=checkVerticalStripe(options, geometry, col, block.start_y, block.end_y/2,image);
d=checkVerticalStripe(options, geometry, col, block.end_y/2+1, block.end_x-2,image);
}
else{
    c=1;
    d=1;
}

if(block.status_down==1){
        unsigned int row=block.end_x-1;
f=checkHorizontalStripe(options, geometry, row, block.start_x+1, block.end_x/2,image);
e=checkHorizontalStripe(options, geometry, row, block.end_x/2+1, block.end_x-1,image);
}
else{
    e=1;
    f=1;
}

if(block.status_left==1){
    unsigned int col=block.start_y;
h=checkVerticalStripe(options, geometry, col, block.start_y+1, block.end_y/2,image);
g=checkVerticalStripe(options, geometry, col, block.end_y/2+1, block.end_y-1,image);
}
else{
    g=1;
    h=1;
}
if(a==2 &&b==2&&c==2&&d==2&&e==2 &&f==2&&g==2&&h==2){
// già colorato i bordi
printf("ho colorato i bordi e ora faccio forza bruta\n");
    block.start_x+=1;
    block.end_x-=1;
        block.start_y+=1;
    block.end_y-=1;
render_block_force_brute(options,geometry,block, image, stats);
return;
}


block_t NW={block.start_x,
            block.start_y,
            (block.start_x+block.end_x)/2,
            (block.start_y+block.end_y)/2,
            a, 1,1,h };

block_t NE={(block.start_x+block.end_x)/2+1,
            block.start_y,
            block.end_x,
            (block.start_y+block.end_y)/2,
            b,c,1,1};

block_t SE={(block.start_x+block.end_x)/2+1,
            (block.start_y+block.end_y)/2+1,
            block.end_x,
            block.end_y,
            1,d,e,1 };

block_t SW={block.start_x,
        (block.start_y+block.end_y)/2+1,
           (block.start_x+block.end_x)/2,
            block.end_y,
            1,1,f,g };

    // printf("NE: x1:%i x2:%iy1:%i y2:%i\n",NE.start_x,NE.end_x,NE.start_y,NE.end_y);
    // printf("NW: x1:%i x2:%iy1:%i y2:%i\n",NW.start_x,NW.end_x,NW.start_y,NW.end_y);
    // printf("SW: x1:%i x2:%iy1:%i y2:%i\n",SW.start_x,SW.end_x,SW.start_y,SW.end_y);
    // printf("SE: x1:%i x2:%iy1:%i y2:%i\n",SE.start_x,SE.end_x,SE.start_y,SE.end_y);


// if(block.end_y-block.start_y<70 || block.end_x-block.start_x<70){
//    render_block_force_brute(options,geometry,block, image, stats);
//    return;
// }


render_block(options,geometry,NW,image,stats);
render_block(options,geometry,NE,image,stats);
render_block(options,geometry,SE,image,stats);
render_block(options,geometry,SW,image,stats);

}
 unsigned int checkHorizontalStripe(options_t *options, image_geometry_t *geometry, const unsigned int row, const unsigned int x_begin, const unsigned int x_end, unsigned char    *image){

const double ci = options->ymax - ((double) (row) + 0.5) * geometry->dy; //row = 0

unsigned int col=x_begin;
unsigned int first_pixel=options->xmin+((double) col + 0.5) * geometry->dx == DEFAULT_KMAX;


      for (col = x_begin+1; col <x_end; ++col){
    double cr = options->xmin + ((double) col + 0.5) * geometry->dx;
    int iteration = mandelbrot_escape (cr, ci, options->kmax);
    
    if((iteration==DEFAULT_KMAX)==first_pixel){
        return 1;
    }

    size_t pixel_offset = ((size_t) row * (size_t) geometry->width
                          + (size_t) col) * 3u;
          colour_from_iteration (iteration, options->kmax, &image[pixel_offset]);


}
if(first_pixel==0){
    return 0;
}
 return 2;
}

 unsigned int checkVerticalStripe(options_t *options, image_geometry_t *geometry, const unsigned 
    int col, const unsigned int y_begin, const unsigned int y_end, unsigned char    *image){
const double cr = options->xmax -((double)col + 0.5)* geometry->dx;
unsigned int row=y_begin;
unsigned int first_pixel=options->ymin+((double) row + 0.5) * geometry->dy == DEFAULT_KMAX;

  for (row = y_begin+1; row < y_end; ++row){
    double ci = options->ymin + ((double) row + 0.5) * geometry->dy;
    int iteration = mandelbrot_escape (cr, ci, options->kmax);
    
    if((iteration==DEFAULT_KMAX)==first_pixel){
        return 1;
    }

    size_t pixel_offset = ((size_t) row * (size_t) geometry->width
                          + (size_t) col) * 3u;
          colour_from_iteration (iteration, options->kmax, &image[pixel_offset]);

}
if(first_pixel==0){
    return 0;
}
 return 2;
}




static void colour_from_iteration (unsigned int    iteration,
		       unsigned int    kmax,
		       unsigned char  *rgb
		       )
{
    printf("i am colouring\n");
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




