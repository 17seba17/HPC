#include "../include/image.h"
#include "../include/kernel.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>




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




void colour_from_iteration (unsigned int    iteration,
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





