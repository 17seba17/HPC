#ifndef IO_H
#define IO_H

int write_ppm_image( const void * restrict image,
			    const int             xsize,
			    const int             ysize,
			    const char * restrict image_name);
#endif