#ifndef KERNEL_H
#define KERNEL_H

unsigned int mandelbrot_escape(double cr, double ci, unsigned int kmax);
unsigned int long_mandelbrot_escape(long double cr, long double ci,
                                    unsigned int kmax);

#endif