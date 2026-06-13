#include "../include/options.h"
#include "../include/geometry.h"
#include "../include/stats.h"
#include "../include/image.h"
#include "../include/io.h"
#include "../include/master.h"
#include "../include/consumer.h"

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


    const  uint8_t asking_new_block = 0xab; //asking block
    const  uint8_t sending_three_blocks = 0x3b; //sending the three blocks

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


    if(rank==0){
block_t *work_queue = NULL;
unsigned char *image = NULL;
uint8_t *recv_buffer = NULL;



if(size<2){
    printf("devi allocare almeno due nodi\n");
    master_terminate_with_error(work_queue,image, recv_buffer);
}



     work_queue = malloc(MAX_BLOCKS_IN_QUEUE * sizeof(block_t));

    if (work_queue == NULL) {
        perror("Errore nell'allocazione della coda di lavoro");
        master_terminate_with_error(work_queue,image,recv_buffer);
    }

    
    size_t image_bytes = (size_t)geometry.width * (size_t)geometry.height * 3u;


     unsigned int queue_size = 0;


     gettingBlocks(&options,&geometry,work_queue,&queue_size,image); // passaggio critico mentre alloco memoria



   
int *idle_workers = malloc(size * sizeof(int));
unsigned int num_idle = 0;




unsigned int in_flight = 0;

for (int worker = 1; worker < size; ++worker) {
    if (queue_size > 0) {
        block_t block = work_queue[--queue_size];
        MPI_Send(&block, sizeof(block_t), MPI_BYTE, worker, 0, MPI_COMM_WORLD);
        in_flight++;
    } else {

 idle_workers[num_idle++] = worker;

    }
}


image = calloc(1, image_bytes);

    if(image == NULL) {
        perror("Errore nell'allocazione immagine");
        master_terminate_with_error(work_queue,image,recv_buffer);
    }



while(queue_size > 0 || in_flight>0){
   
   MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);


   int incoming_bytes = 0;
   MPI_Get_count(&status, MPI_BYTE, &incoming_bytes);
   recv_buffer = malloc(incoming_bytes);


   MPI_Recv(recv_buffer, incoming_bytes, MPI_BYTE, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


   //getting instruction
   uint8_t function = recv_buffer[0];
   if(function==0xee){
    printf("error from consumer\n");
    master_terminate_with_error(work_queue,image,recv_buffer);
   }
   //asking block
   else if(function==0xab){ 

    in_flight--; 
unsigned int f;
    memcpy(&f, recv_buffer + 1, sizeof(unsigned int));
// in_flight-=f;

        if (queue_size > 0) {
        block_t block = work_queue[--queue_size];
        MPI_Send(&block, sizeof(block_t), MPI_BYTE, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
        in_flight++;
    } else {
                idle_workers[num_idle++] = status.MPI_SOURCE;
    }
   }
    //sending three blocks
   else  if(function==0x3b){

    
block_t NE, SE, SW;
    memcpy(&NE, recv_buffer + 1, sizeof(block_t));
    memcpy(&SE, recv_buffer + 1 + sizeof(block_t), sizeof(block_t));
    memcpy(&SW, recv_buffer + 1 + 2 * sizeof(block_t), sizeof(block_t));
    work_queue[queue_size++] = NE;
    work_queue[queue_size++] = SE;
    work_queue[queue_size++] = SW;


while (num_idle > 0 && queue_size > 0) {
        int worker_to_wake = idle_workers[--num_idle];
        block_t block = work_queue[--queue_size];
        
        MPI_Send(&block, sizeof(block_t), MPI_BYTE, worker_to_wake, 0, MPI_COMM_WORLD);
        in_flight++;
    }


   }
   else if (function == 0xf0) {
     

    block_t received_block;
    memcpy(&received_block, recv_buffer + sizeof(uint8_t), sizeof(block_t));

        unsigned int block_width = received_block.end_x - received_block.start_x;
        unsigned int block_height = received_block.end_y - received_block.start_y;

        uint8_t *pixel_data_start = recv_buffer + sizeof(uint8_t) + sizeof(block_t);

        for (unsigned int i = 0; i < block_height; i++) {
            size_t dest_offset = ((size_t)(received_block.start_y + i) * geometry.width + received_block.start_x) * 3u;
            size_t src_offset = (size_t)i * block_width * 3u;
            
            memcpy(&image[dest_offset], &pixel_data_start[src_offset], block_width * 3u);
        }
        
        //  in_flight--; 
    }



free(recv_buffer);



}
for (int worker = 1; worker < size; ++worker) {
            MPI_Send(NULL, 0, MPI_BYTE, worker, 2, MPI_COMM_WORLD);
}

for (int worker = 1; worker < size; ++worker) {
                render_stats_t consumer_stats;
MPI_Recv(&consumer_stats, sizeof(render_stats_t),MPI_BYTE, worker,42,MPI_COMM_WORLD, &status);

aggregateStats(&stats,consumer_stats);

            }


 if (write_ppm_image(image, geometry.width, geometry.height, options.output_path) != 0) {
        perror("Errore durante il salvataggio dell'immagine"); 
        free(image);
        return EXIT_FAILURE;
    }
    // Stampa statistiche
    print_stats(&geometry, &options, &stats);


free(image);
    
}

else{

    // unsigned int in_flight=0;
    while(1){

       MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
       int received_tag = status.MPI_TAG;

       if(received_tag==2){
        printf("Master asked to terminate!\n");
        MPI_Send(&stats, sizeof(stats),MPI_BYTE,0,42,MPI_COMM_WORLD);
        MPI_Finalize();
        return EXIT_SUCCESS;
       }

       else{

        


        block_t block_to_analyse;
        MPI_Recv(&block_to_analyse, sizeof(block_t), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        unsigned int in_flight=0;
        rendering_block(&options, &geometry,block_to_analyse,&stats,0);

    size_t size_total = sizeof(uint8_t) + sizeof(unsigned int);
    uint8_t *send_buffer = malloc(size_total);

    size_t offset = 0;
    uint8_t function = 0xab;
    memcpy(send_buffer + offset, &function, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    
    memcpy(send_buffer + offset, &in_flight, sizeof(unsigned int));

    
         MPI_Send(&function,sizeof(uint8_t),MPI_BYTE,0,1,MPI_COMM_WORLD);
       }


    }



}


 MPI_Finalize();
return EXIT_SUCCESS;
}
