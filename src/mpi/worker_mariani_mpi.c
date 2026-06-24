#include "../../include/worker.h"
#include "../../include/kernel.h"
#include "../../include/constants.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <stdlib.h>

 void sending_image(unsigned char *local_image, block_t block) {

    unsigned int block_width = block.end_x - block.start_x;
    unsigned int block_height = block.end_y - block.start_y;
    size_t size_total = (size_t)block_width * block_height * 3u + sizeof(block_t);




    memcpy(local_image , &block, sizeof(block_t));
    

    MPI_Send(local_image, size_total, MPI_BYTE, 0, SENDING_IMAGE_TO_MASTER, MPI_COMM_WORLD);
    return;
}






 void drawAllImage (options_t        *options,
	       image_geometry_t *geometry,
	       block_t      block,
	       render_stats_t   *stats, unsigned char *image
	       )
{
  ///i have to allocate memory for the image


int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    
    if (image == NULL) {
        printf("Errore nell'allocazione immagine nel rank %i", rank);
        return;
        
    }
    

checksum_t local_my_checksum=stats->my_checksum;
uint64_t     local_total_iterations=stats->total_iterations;
uint64_t     local_inside_pixels= stats->inside_pixels;

     size_t width=block.end_x - block.start_x;
     double dx=geometry->dx;
     double dy=geometry->dy;
    unsigned int kmax=options->kmax;
     double ymax=options->ymax;
     double xmin=options->xmin;

  for (unsigned int row = block.start_y; row < block.end_y; ++row) { 
     double ci = ymax - ((double) row + 0.5) * dy;
     unsigned int local_r = row - block.start_y; 
      for (unsigned int col = block.start_x; col < block.end_x; ++col)
        {
          double cr = xmin + ((double) col + 0.5) * dx;
          unsigned int iteration = mandelbrot_escape (cr, ci,kmax);
                  unsigned int local_c = col - block.start_x; 
        size_t pixel_offset = ((size_t) (local_r) * width + (size_t) local_c) * 3u+sizeof(block_t);

          colour_from_iteration (iteration, options->kmax, &image[pixel_offset]);

        local_my_checksum = my_checksum_update(local_my_checksum, row, col, iteration);          
       
        local_total_iterations += (uint64_t) iteration;

          if (iteration >= kmax)
            local_inside_pixels += 1u;

        }
    }

       stats->my_checksum =local_my_checksum;
    stats->total_iterations =local_total_iterations;
    stats->inside_pixels =local_inside_pixels;




 sending_image(image,block);

 return;
}


void rendering_block (options_t        *options,
	       image_geometry_t *geometry,
	       block_t      block,
	       render_stats_t   *stats, block_t three_blocks[3], unsigned char *image
	       ){

stats->tiles_processed+=1;

          
if((block.end_x-block.start_x)*(block.end_y-block.start_y)<options->brute){
   drawAllImage(options,geometry,block, stats,image);

   return;
}


unsigned int mid_x = (block.start_x + block.end_x) / 2;
unsigned int mid_y = (block.start_y + block.end_y) / 2;

unsigned int width = block.end_x-block.start_x;
unsigned int height = block.end_y-block.start_y;

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

unsigned int row=block.start_y;

if(upper_left == 0){
  upper_left = checkHorizontalStripe(options, geometry, row, block.start_x, mid_x);
}
if(upper_right == 0){
  upper_right =  checkHorizontalStripe(options, geometry, row, mid_x, block.end_x);
}

row=block.end_y-1;

if(down_left == 0){
  down_left = checkHorizontalStripe(options, geometry, row, block.start_x, mid_x);
}

if(down_right == 0){
down_right = checkHorizontalStripe(options, geometry, row, mid_x, block.end_x);
}




unsigned int col=block.start_x;

if(left_up == 0){
  left_up = checkVerticalStripe(options, geometry, col, block.start_y, mid_y);
}
if(left_down == 0){
  left_down =  checkVerticalStripe(options, geometry, col, mid_y, block.end_y);
}

col=block.end_x-1;

if(right_up == 0){
  right_up = checkVerticalStripe(options, geometry, col, block.start_y, mid_y);
}

if(right_down == 0){
    right_down = checkVerticalStripe(options, geometry, col, mid_y, block.end_y); 
}



if(upper_left+upper_right+down_right+down_left+left_up+left_down+right_up+right_down==2*height+2*width){ // if black


uint64_t block_area = width * height;
stats->total_iterations += block_area * (uint64_t)options->kmax;
stats->inside_pixels += block_area;


    return;
}
else if(upper_left+upper_right+down_right+down_left+left_up+left_down+right_up+right_down==0){


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



memcpy(&three_blocks[0], &NE, sizeof(block_t));
memcpy(&three_blocks[1], &SE, sizeof(block_t));
memcpy(&three_blocks[2], &SW, sizeof(block_t));

MPI_Send(three_blocks,3*sizeof(block_t),MPI_BYTE,0,SENDING_THREE_BLOCKS_TO_MASTER,MPI_COMM_WORLD);

rendering_block(options,geometry,NW,stats, three_blocks, image);
return;

}