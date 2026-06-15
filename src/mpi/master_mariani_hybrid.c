#include "../../include/master.h"
#include "../../include/constants.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


 void master_terminate_with_error(
    block_t *work_queue, int *idle_workers, unsigned char *image, uint8_t *recv_buffer // arrays
) {
printf("master terminating processes\n");
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);


    for (int worker = 1; worker < size; ++worker) {
        MPI_Send(NULL, 0, MPI_BYTE, worker, STOPPING_PROCESS, MPI_COMM_WORLD);
    }

    if (work_queue != NULL) {
        free(work_queue);
    }
    if (idle_workers != NULL) {
        free(idle_workers);
    }
    if (image != NULL) {
        free(image);
    }
    if (recv_buffer != NULL) {
        free(recv_buffer);
    }

    MPI_Finalize();

    exit(EXIT_FAILURE);
}


void gettingBlocks (options_t        *options,image_geometry_t        *geometry,
	     block_t *array, int *idle_workers, unsigned char *image, uint8_t *recv_buffer, //arrays
         unsigned int *arraysize
)
{






unsigned int queue_size = 0;

    for (unsigned int bx = 0; bx < options->dx_factor; ++bx) {
        for (unsigned int by = 0; by < options->dy_factor; ++by) {
            
            if (queue_size >= MAX_BLOCKS_IN_QUEUE) {
                fprintf(stderr, "Errore: Superato il limite MAX_BLOCKS durante l'inizializzazione!\n");
                master_terminate_with_error(array, idle_workers,image,recv_buffer);
                
            }

            block_t block = {0};
            block.start_x = (unsigned int)(((uint64_t)bx * geometry->width) / options->dx_factor);
            block.end_x   = (unsigned int)(((uint64_t)(bx + 1) * geometry->width) / options->dx_factor);
            block.start_y = (unsigned int)(((uint64_t)by * geometry->height) / options->dy_factor);
            block.end_y   = (unsigned int)(((uint64_t)(by + 1) * geometry->height) / options->dy_factor);

            


            array[queue_size] = block;
            queue_size++;
        }
    }
*arraysize=queue_size;


}