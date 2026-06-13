#include "../include/consumer.h"
#include "../include/kernel.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <stdlib.h>

static void sending_image(unsigned char *local_image, block_t block) {

  
    uint8_t function = 0xf0; // sending photo (f0to)

    unsigned int block_width = block.end_x - block.start_x;
    unsigned int block_height = block.end_y - block.start_y;
    size_t pixel_bytes = (size_t)block_width * block_height * 3u;

    size_t size_total = sizeof(uint8_t) + sizeof(block_t) + pixel_bytes;
    uint8_t *send_buffer = malloc(size_total);

    size_t offset = 0;
    memcpy(send_buffer + offset, &function, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    
    memcpy(send_buffer + offset, &block, sizeof(block_t));
    offset += sizeof(block_t);
    
    memcpy(send_buffer + offset, local_image, pixel_bytes);

    MPI_Send(send_buffer, size_total, MPI_BYTE, 0, 0, MPI_COMM_WORLD);

    free(send_buffer);
    free(local_image);
    return;
}


static void colour_from_iteration (unsigned int    iteration,
		       unsigned int    kmax,
		       unsigned char  *rgb
		       );

 void drawBlackImage(options_t        *options,  render_stats_t   *stats, image_geometry_t *geometry, block_t block){


    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    size_t image_bytes = (size_t)(block.end_x-block.start_x) * (size_t)(block.end_y-block.start_y)  * 3u;
    unsigned char *image = malloc(image_bytes);


    if (image == NULL) {
        printf("Errore nell'allocazione immagine nel rank %i", rank);
        return;
        
    }


for (unsigned int r = block.start_y ; r < block.end_y ; ++r) {
    unsigned int local_r = r - block.start_y;
    for (unsigned int c_idx = block.start_x ; c_idx < block.end_x ; ++c_idx) {
        unsigned int local_c = c_idx - block.start_x;
        
        size_t pixel_offset = ((size_t) local_r * (size_t) (block.end_x - block.start_x) + (size_t) local_c) * 3u;
        
        colour_from_iteration(options->kmax, options->kmax, &image[pixel_offset]);
        
        // qua devo commentare my_checksum_update
        
        // stats->my_checksum = my_checksum_update(stats->my_checksum, r, 
        // c_idx, options->kmax);
    

stats->checksum = checksum_update_uint (stats->checksum, options->kmax);

        stats->total_iterations += (uint64_t)options->kmax;
        stats->inside_pixels += 1u;

    }
}

// sending_image(image, block);
    
        return;  

}



 void drawAllImage (options_t        *options,
	       image_geometry_t *geometry,
	       block_t      block,
	       render_stats_t   *stats
	       )
{///i have to allocate memory for the image


int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    size_t image_bytes = (size_t)(block.end_x-block.start_x) * (size_t)(block.end_y-block.start_y)  * 3u;
    unsigned char *image = malloc(image_bytes);


    if (image == NULL) {
        printf("Errore nell'allocazione immagine nel rank %i", rank);
        return;
        
    }
    




  for (unsigned int row = block.start_y; row < block.end_y; ++row)
    {
     double ci = options->ymax - ((double) row + 0.5) * geometry->dy;
         unsigned int local_r = row - block.start_y; 
      for (unsigned int col = block.start_x; col < block.end_x; ++col)
        {
          double cr = options->xmin + ((double) col + 0.5) * geometry->dx;
          unsigned int iteration = mandelbrot_escape (cr, ci, options->kmax);
                  unsigned int local_c = col - block.start_x; 
        size_t pixel_offset = ((size_t) (local_r) * (size_t) (block.end_x - block.start_x) + (size_t) local_c) * 3u;

          colour_from_iteration (iteration, options->kmax, &image[pixel_offset]);

        stats->my_checksum = my_checksum_update(stats->my_checksum, row, col, iteration);          
       stats->checksum = checksum_update_uint (stats->checksum, iteration);
       
        stats->total_iterations += (uint64_t) iteration;

          if (iteration >= options->kmax)
            stats->inside_pixels += 1u;

        }
    }
 sending_image(image,block);

 return;
}


void rendering_block (options_t        *options,
	       image_geometry_t *geometry,
	       block_t      block,
	       render_stats_t   *stats, unsigned int in_flight
	       ){


          
if((block.end_x-block.start_x)*(block.end_y-block.start_y)<1024){
   drawAllImage(options,geometry,block, stats);

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



if(upper_left+upper_right+down_right+down_left+left_up+left_down+right_up+right_down==2*height+2*width){ // se nero


    drawBlackImage(options,stats,geometry,block);
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

uint8_t function = 0x3b; //sending three blocks


size_t size_total = sizeof(unsigned int) + (3 * sizeof(block_t));
uint8_t *send_buffer = malloc(size_total);

size_t offset = 0;
memcpy(send_buffer + offset, &function, sizeof(uint8_t));
offset += sizeof(uint8_t);
memcpy(send_buffer + offset, &NE, sizeof(block_t));
offset += sizeof(block_t);
memcpy(send_buffer + offset, &SE, sizeof(block_t));
offset += sizeof(block_t);
memcpy(send_buffer + offset, &SW, sizeof(block_t));

MPI_Send(send_buffer,size_total,MPI_BYTE,0,0,MPI_COMM_WORLD);
free(send_buffer);
  rendering_block(options,geometry,NW,stats, in_flight);
return;

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




