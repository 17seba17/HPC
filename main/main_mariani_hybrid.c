#include "../include/constants.h"
#include "../include/geometry.h"
#include "../include/image.h"
#include "../include/io.h"
#include "../include/master.h"
#include "../include/options.h"
#include "../include/stats.h"
#include "../include/worker.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
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

  if (validate_options(&options) != 0 ||
      make_geometry(&options, &geometry) != 0) {
    return EXIT_FAILURE;
  }

  zero_stats(&stats);

  size_t max_buffer_size = sizeof(block_t) * 3 + (options.brute * 3);

  // MASTER
  if (rank == 0) {

    // defining all the arrays
    block_t *work_queue = NULL;
    int *idle_workers = NULL;
    unsigned char *image = NULL;
    uint8_t *recv_buffer = NULL; // single buffer from a single worker

    uint8_t **recv_buffers = malloc(
        size * sizeof(uint8_t *)); // array of buffers from different workers
    MPI_Request *requests =
        malloc(size * sizeof(MPI_Request)); // requests of the workers

    if (size < 2) {
      printf("You need to allocate at least two tasks!\n");
      master_terminate_with_error(work_queue, idle_workers, image, recv_buffer);
    }

    //  non-blocking reception for all worker processes
    for (int worker = 1; worker < size; ++worker) {
      recv_buffers[worker] = malloc(max_buffer_size);
      MPI_Irecv(recv_buffers[worker], max_buffer_size, MPI_BYTE, worker,
                MPI_ANY_TAG, MPI_COMM_WORLD, &requests[worker]);
    }

    // queue of blocks
    work_queue = malloc(MAX_BLOCKS_IN_QUEUE * sizeof(block_t));

    if (work_queue == NULL) {
      perror("Error during work_queue allocation!\n");
      master_terminate_with_error(work_queue, idle_workers, image, recv_buffer);
    }

    size_t image_bytes = (size_t)geometry.width * (size_t)geometry.height * 3u;

    unsigned int queue_size = 0;

    // workers in idle waiting for a new block
    idle_workers = malloc(size * sizeof(int));
    if (idle_workers == NULL) {
      perror("Error during idlw_workers allocation!\n");
      master_terminate_with_error(work_queue, NULL, image, recv_buffer);
    }

    gettingBlocks(&options, &geometry, work_queue, idle_workers, image,
                  recv_buffer, &queue_size);
    unsigned int num_idle = 0;

    unsigned int in_flight = 0;
    // distributing the blocks
    for (int worker = 1; worker < size; ++worker) {
      if (queue_size > 0) {
        block_t block = work_queue[--queue_size];
        MPI_Send(&block, sizeof(block_t), MPI_BYTE, worker,
                 SENDING_BLOCK_TO_WORKER, MPI_COMM_WORLD);
        in_flight++;
      } else {

        idle_workers[num_idle++] = worker;
      }
    }
    // creating a black image
    image = calloc(1, image_bytes);

    if (image == NULL) {
      perror("Error during image allocation!\n");
      master_terminate_with_error(work_queue, idle_workers, image, recv_buffer);
    }

    recv_buffer = malloc(max_buffer_size);
    while (queue_size > 0 ||
           in_flight > 0) { // the queue must not be empty and there is no block
                            // in_flight  (a worker is analysing a block)

      int completed_idx;
      MPI_Status status;
      // checking every request from each processor
      MPI_Waitany(size - 1, requests + 1, &completed_idx, &status);
      int worker_rank = completed_idx + 1;
      int tag = status.MPI_TAG;
      uint8_t *worker_buf = recv_buffers[worker_rank];
      int incoming_bytes = 0;
      MPI_Get_count(&status, MPI_BYTE, &incoming_bytes);

      if (tag == ASKING_BLOCK_TO_MASTER) {
        in_flight--;

        if (queue_size > 0) {
          // sending block
          block_t block = work_queue[--queue_size];
          MPI_Send(&block, sizeof(block_t), MPI_BYTE, status.MPI_SOURCE,
                   SENDING_BLOCK_TO_WORKER, MPI_COMM_WORLD);
          in_flight++;
        } else {
          // no block to send, the worker must wait
          idle_workers[num_idle++] = status.MPI_SOURCE;
        }
      } else if (tag == SENDING_THREE_BLOCKS_TO_MASTER) {
        // receiving three blocks
        block_t NE, SE, SW;
        memcpy(&NE, worker_buf, sizeof(block_t));
        memcpy(&SE, worker_buf + sizeof(block_t), sizeof(block_t));
        memcpy(&SW, worker_buf + 2 * sizeof(block_t), sizeof(block_t));
        work_queue[queue_size++] = NE;
        work_queue[queue_size++] = SE;
        work_queue[queue_size++] = SW;

        while (num_idle > 0 && queue_size > 0) {
          // sending blocks to idle workers since I receive 3 new tiles
          int worker_to_wake = idle_workers[--num_idle];
          block_t block = work_queue[--queue_size];

          MPI_Send(&block, sizeof(block_t), MPI_BYTE, worker_to_wake,
                   SENDING_BLOCK_TO_WORKER, MPI_COMM_WORLD);
          in_flight++;
        }

      } else if (tag == SENDING_IMAGE_TO_MASTER) {

        /// sending image to master

        block_t received_block;
        memcpy(&received_block, worker_buf, sizeof(block_t));

        unsigned int block_width =
            received_block.end_x - received_block.start_x;
        unsigned int block_height =
            received_block.end_y - received_block.start_y;

        uint8_t *pixel_data_start = worker_buf + sizeof(block_t);

        for (unsigned int i = 0; i < block_height; i++) {
          size_t dest_offset =
              ((size_t)(received_block.start_y + i) * geometry.width +
               received_block.start_x) *
              3u;
          size_t src_offset = (size_t)i * block_width * 3u;

          memcpy(&image[dest_offset], &pixel_data_start[src_offset],
                 block_width * 3u);
        }

        in_flight--;

        if (queue_size > 0) {
          // sending immediately a new block
          block_t block = work_queue[--queue_size];
          MPI_Send(&block, sizeof(block_t), MPI_BYTE, status.MPI_SOURCE,
                   SENDING_BLOCK_TO_WORKER, MPI_COMM_WORLD);
          in_flight++;
        } else {
          // setting a worker to idle
          idle_workers[num_idle++] = status.MPI_SOURCE;
        }
      }

      // receiving buffers
      MPI_Irecv(recv_buffers[worker_rank], max_buffer_size, MPI_BYTE,
                worker_rank, MPI_ANY_TAG, MPI_COMM_WORLD,
                &requests[worker_rank]);
    }

    // free memory
    for (int worker = 1; worker < size; ++worker) {
      MPI_Cancel(&requests[worker]);
      MPI_Request_free(&requests[worker]);
      free(recv_buffers[worker]);
    }
    free(recv_buffers);
    free(requests);
    // stopping workers and asking for stats
    for (int worker = 1; worker < size; ++worker) {
      MPI_Send(NULL, 0, MPI_BYTE, worker, STOPPING_PROCESS, MPI_COMM_WORLD);
    }

    // receiving stats
    for (int worker = 1; worker < size; ++worker) {
      render_stats_t worker_stats;
      MPI_Recv(&worker_stats, sizeof(render_stats_t), MPI_BYTE, worker,
               SENDING_STATS, MPI_COMM_WORLD, &status);

      aggregateStats(&stats, worker_stats);
    }
    // create ppm
    if (write_ppm_image(image, geometry.width, geometry.height,
                        options.output_path) != 0) {
      perror("Error during image saving");
      free(image);
      return EXIT_FAILURE;
    }
    // printing stats
    print_stats(&geometry, &options, &stats);
    // free memory
    free(recv_buffer);
    free(image);
    free(work_queue);
    free(idle_workers);
  }

  else {
    // WORKER
    clock_gettime(CLOCK_MONOTONIC, &stats.start_t);
    uint8_t worked = 0;
    block_t three_block_to_send[3];
    unsigned char *image_to_send;
    // biggest tile size
    image_to_send = malloc(max_buffer_size);

    while (1) {

      MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      int received_tag = status.MPI_TAG;

      if (received_tag == STOPPING_PROCESS) {
        // stopping
        if (worked == 1) {
          clock_gettime(CLOCK_MONOTONIC, &stats.end_t);
          updateTimeWaiting(&stats);
        }
        printf("Master asked to terminate! I am rank %u !\n", rank);
        // sending stats
        print_stats(&geometry, &options, &stats);

        MPI_Send(&stats, sizeof(stats), MPI_BYTE, 0, SENDING_STATS,
                 MPI_COMM_WORLD);
        free(image_to_send);
        MPI_Finalize();
        return EXIT_SUCCESS;
      }

      else {

        block_t block_to_analyse;
        // asking for a new block to analyse
        MPI_Recv(&block_to_analyse, sizeof(block_t), MPI_BYTE, 0,
                 SENDING_BLOCK_TO_WORKER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (worked == 1) {
          clock_gettime(CLOCK_MONOTONIC, &stats.end_t);
          updateTimeWaiting(&stats);
        }

        // analysing block (in this process the worker divide the tile in four
        // and send to the master & sending the tile processed to the master)
        clock_gettime(CLOCK_MONOTONIC, &stats.start_t);
        rendering_block(&options, &geometry, block_to_analyse, &stats,
                        three_block_to_send, image_to_send);
        worked = 1;
        clock_gettime(CLOCK_MONOTONIC, &stats.end_t);
        updateTimeWorking(&stats);

        clock_gettime(CLOCK_MONOTONIC, &stats.start_t);
      }
    }
  }

  MPI_Finalize();
  return EXIT_SUCCESS;
}
