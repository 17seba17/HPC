#include "../../include/worker.h"
#include "../../include/kernel.h"
#include "../../include/constants.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <stdlib.h>
#include <omp.h>

#pragma omp declare reduction(checksum_add : checksum_t : \
    omp_out.slot[0] += omp_in.slot[0], \
    omp_out.slot[1] += omp_in.slot[1], \
    omp_out.slot[2] += omp_in.slot[2], \
    omp_out.slot[3] += omp_in.slot[3]) \
    initializer(omp_priv = {{0, 0, 0, 0}})


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



    


// local variables to update

  checksum_t local_my_checksum;
  uint64_t     local_total_iterations;
  uint64_t     local_inside_pixels;

 
  memset(&local_my_checksum, 0, sizeof(local_my_checksum.slot));
     local_total_iterations = stats->total_iterations;
     local_inside_pixels = stats->inside_pixels;
// costants
          size_t width=block.end_x - block.start_x;
     double dx=geometry->dx;
     double dy=geometry->dy;
    unsigned int kmax=options->kmax;
     double ymax=options->ymax;
     double xmin=options->xmin;

unsigned int row;
  #pragma omp parallel for \
              default(none) \
              shared(options, geometry, block, image,width,dx,dy,kmax,ymax,xmin) \
              private(row) \
              reduction(+:local_total_iterations, local_inside_pixels) \
              reduction(checksum_add:local_my_checksum) \
              schedule(dynamic)


 for (row = block.start_y; row < block.end_y; ++row) { 
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

  stats->my_checksum.slot[0] += local_my_checksum.slot[0];
  stats->my_checksum.slot[1] += local_my_checksum.slot[1];
  stats->my_checksum.slot[2] += local_my_checksum.slot[2];
  stats->my_checksum.slot[3] += local_my_checksum.slot[3];
  stats->total_iterations = local_total_iterations;
  stats->inside_pixels = local_inside_pixels;

 sending_image(image,block);

 return;





}

