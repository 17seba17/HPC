#ifndef MASTER_H
#define MASTER_H
#define MAX_BLOCKS_IN_QUEUE 5000000
#include "options.h"
#include "geometry.h"
#include "stats.h"
#include "image.h"





void gettingBlocks(options_t        *options,image_geometry_t        *geometry,
	     block_t *work_queue, int *idle_workers, unsigned char *image, uint8_t *recv_buffer,
                 unsigned int *arraysize

        );

void master_terminate_with_error(block_t *work_queue, int *idle_workers, unsigned char *image, uint8_t *recv_buffer);
#endif
