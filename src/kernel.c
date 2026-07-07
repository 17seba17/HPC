#include "../include/kernel.h"
#include <math.h>

unsigned int mandelbrot_escape(double cr, double ci, unsigned int kmax) {
  double zr;
  double zi;
  double zr_next;
  double zr2;
  double zi2;
  unsigned int k;

  zr = 0.0;
  zi = 0.0;
  k = 0;

  while (k < kmax) {
    zr2 = zr * zr;
    zi2 = zi * zi;

    if (zr2 + zi2 > 4.0)
      break;

    zr_next = zr2 - zi2 + cr;
    zi = 2.0 * zr * zi + ci;
    zr = zr_next;
    k += 1;
  }

  return k;
}