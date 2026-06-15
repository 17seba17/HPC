#include "../include/options.h"
#include "../include/geometry.h"
#include "../include/stats.h"
#include "../include/image.h"
#include "../include/io.h"
#include "../include/master.h"
#include "../include/worker.h"
#include "../include/constants.h"

#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <string.h>



#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1


int main(int argc, char **argv) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;



    options_t options;
    image_geometry_t geometry;
    render_stats_t stats;

    set_default_options(&options);
    if (parse_command_line(argc, argv, &options) != 0) {
         return EXIT_FAILURE;
    }

    if (validate_options(&options) != 0 || make_geometry(&options, &geometry) != 0) {
        return EXIT_FAILURE;
    }



    zero_stats(&stats);

    unsigned int max_block_width = (geometry.width + options.dx_factor - 1) / options.dx_factor;

unsigned int max_block_height = (geometry.height + options.dy_factor - 1) / options.dy_factor;

size_t max_pixel_bytes = (size_t)max_block_width * max_block_height * 3u;

size_t max_buffer_size = sizeof(block_t) + max_pixel_bytes;

    if(rank==0){

//defining all the arrays
block_t *work_queue = NULL;
int *idle_workers = NULL;
unsigned char *image = NULL;
uint8_t *recv_buffer = NULL;



if(size<2){
    printf("devi allocare almeno due nodi\n");
    master_terminate_with_error(work_queue, idle_workers, image, recv_buffer);
}


// queue of blocks
work_queue = malloc(MAX_BLOCKS_IN_QUEUE * sizeof(block_t));

if (work_queue == NULL) {
        perror("Errore nell'allocazione della coda di lavoro\n");
        master_terminate_with_error(work_queue, idle_workers, image, recv_buffer);
    }
     unsigned int queue_size = 0;

    
    size_t image_bytes = (size_t)geometry.width * (size_t)geometry.height * 3u;




gettingBlocks(&options,&geometry,work_queue,idle_workers,image,recv_buffer,&queue_size);




unsigned int in_flight = 0;

for (int worker = 1; worker < size; ++worker) {
    if (queue_size > 0) {
        block_t block = work_queue[--queue_size];
        MPI_Send(&block, sizeof(block_t), MPI_BYTE, worker, SENDING_BLOCK_TO_WORKER, MPI_COMM_WORLD);
        in_flight++;
    } 
}


image = calloc(1, image_bytes);

    if(image == NULL) {
        perror("Errore nell'allocazione immagine\n");
        master_terminate_with_error(work_queue, idle_workers, image, recv_buffer);
    }


recv_buffer = malloc(max_buffer_size);

while(queue_size > 0 || in_flight>0){
   
   MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);


   int incoming_bytes = 0;
   MPI_Get_count(&status, MPI_BYTE, &incoming_bytes);


   MPI_Recv(recv_buffer, incoming_bytes, MPI_BYTE, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


   

   
    if(status.MPI_TAG == ASKING_BLOCK_TO_MASTER){ 
    in_flight--; 


        if (queue_size > 0) {
        block_t block = work_queue[--queue_size];
        MPI_Send(&block, sizeof(block_t), MPI_BYTE, status.MPI_SOURCE, SENDING_BLOCK_TO_WORKER, MPI_COMM_WORLD);
        in_flight++;
    } else {

    }
   }
  
   else if(status.MPI_TAG == SENDING_IMAGE_TO_MASTER){ 
   
/// sending image to master     

    block_t received_block;
    memcpy(&received_block, recv_buffer, sizeof(block_t));

        unsigned int block_width = received_block.end_x - received_block.start_x;
        unsigned int block_height = received_block.end_y - received_block.start_y;

        uint8_t *pixel_data_start = recv_buffer  + sizeof(block_t);

        for (unsigned int i = 0; i < block_height; i++) {
            size_t dest_offset = ((size_t)(received_block.start_y + i) * geometry.width + received_block.start_x) * 3u;
            size_t src_offset = (size_t)i * block_width * 3u;
            
            memcpy(&image[dest_offset], &pixel_data_start[src_offset], block_width * 3u);
        }
        
    }






}
for (int worker = 1; worker < size; ++worker) {
            MPI_Send(NULL, 0, MPI_BYTE, worker, STOPPING_PROCESS, MPI_COMM_WORLD);
}

for (int worker = 1; worker < size; ++worker) {
                render_stats_t worker_stats;
MPI_Recv(&worker_stats, sizeof(render_stats_t),MPI_BYTE, worker,SENDING_STATS,MPI_COMM_WORLD, &status);

aggregateStats(&stats,worker_stats);

            }


 if (write_ppm_image(image, geometry.width, geometry.height, options.output_path) != 0) {
        perror("Errore durante il salvataggio dell'immagine"); 
        free(image);
        return EXIT_FAILURE;
    }
    // Stampa statistiche
    print_stats(&geometry, &options, &stats);

free(recv_buffer);
free(image);
    free(work_queue);
free(idle_workers);
}

else{

clock_gettime(CLOCK_MONOTONIC, &stats.start_t);
uint8_t worked=0;

        unsigned char *image_to_send;

 image_to_send = malloc(max_buffer_size);

    while(1) {
        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int received_tag = status.MPI_TAG;

        if(received_tag == STOPPING_PROCESS) {
        printf("Master asked to terminate!\n");
         free(image_to_send);
              if(worked==1){
        clock_gettime(CLOCK_MONOTONIC, &stats.end_t);
            updateTimeWaiting(&stats);

    }


        MPI_Send(&stats, sizeof(stats),MPI_BYTE,0,SENDING_STATS,MPI_COMM_WORLD);
        MPI_Finalize();
        return EXIT_SUCCESS;
       }

       else{


        block_t block_to_analyse;
        MPI_Recv(&block_to_analyse, sizeof(block_t), MPI_BYTE, 0, SENDING_BLOCK_TO_WORKER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if(worked==1){
        clock_gettime(CLOCK_MONOTONIC, &stats.end_t);
            updateTimeWaiting(&stats);

    }

        clock_gettime(CLOCK_MONOTONIC, &stats.start_t);
        drawAllImage(&options, &geometry,block_to_analyse,&stats, image_to_send);
        worked=1;
        clock_gettime(CLOCK_MONOTONIC, &stats.end_t);
        updateTimeWorking(&stats);


        clock_gettime(CLOCK_MONOTONIC, &stats.start_t);
         MPI_Send(NULL,0,MPI_BYTE,0,ASKING_BLOCK_TO_MASTER,MPI_COMM_WORLD);

       }


    }



}


 MPI_Finalize();
return EXIT_SUCCESS;
}
