#ifndef MASTER_H
#define MASTER_H
#define MAX_BLOCKS_IN_QUEUE 500000
#include "options.h"
#include "geometry.h"
#include "stats.h"
#include "image.h"





void gettingBlocks (options_t        *options,
	      image_geometry_t *geometry, block_t *array,unsigned int *array_size, unsigned char *image);

           void master_terminate_with_error(block_t *work_queue, unsigned char *image, uint8_t *recv_buffer);
#endif