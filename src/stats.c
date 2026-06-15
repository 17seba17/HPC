#include "../include/stats.h"
#include <stdio.h>
#include <string.h>

void zero_stats(render_stats_t *stats){

  


  memset(stats->my_checksum.slot, 0, sizeof(stats->my_checksum.slot));

  stats->total_iterations = 0u;
  stats->inside_pixels = 0u;
  stats->time_waiting =0u;
  stats->time_working =0u;
}

checksum_t my_checksum_update (checksum_t     checksum,
                      unsigned int value1, unsigned int value2, unsigned int value3){
unsigned int idx = (value1 + value3) % 4;
unsigned int val = value2+value3;

checksum.slot[idx]+=val;

  return checksum;


}

/*
  Stampa a schermo le statistiche finali del rendering.
*/
void
print_stats (const image_geometry_t *geometry,
             const options_t        *options,
             const render_stats_t   *stats)
{
  uint64_t pixel_count;
  double   inside_fraction;
  double   average_iterations;

  pixel_count        = (uint64_t) geometry->width * (uint64_t) geometry->height;
  inside_fraction    = (double) stats->inside_pixels / (double) pixel_count;
  average_iterations = (double) stats->total_iterations / (double) pixel_count;

  printf ("output_file              %s\n", options->output_path);
  printf ("width                    %u\n", geometry->width);
  printf ("height                   %u\n", geometry->height);
  printf ("kmax                     %u\n", options->kmax);
  printf ("my_checksum              %08x%08x%08x%08x\n", stats->my_checksum.slot[0],
                                                       stats->my_checksum.slot[1],
                                                       stats->my_checksum.slot[2],
                                                       stats->my_checksum.slot[3]);

  printf ("inside_pixels            %llu\n", (unsigned long long) stats->inside_pixels);
  printf ("inside_fraction          %.17g\n", inside_fraction);
  printf ("average_iterations       %.17g\n", average_iterations);
  printf ("total_iterations         %llu\n", (unsigned long long) stats->total_iterations);
  printf("time waiting              %.6f s\n", (double)stats->time_waiting / 1000000.0);
  printf("time working              %.6f s\n", (double)stats->time_working / 1000000.0);
} 


void aggregateStats(render_stats_t *master_stats, render_stats_t worker_stats){



for(unsigned int i=0;i<4;i++){
master_stats->my_checksum.slot[i]+=worker_stats.my_checksum.slot[i];}
master_stats->total_iterations+=worker_stats.total_iterations;
master_stats->inside_pixels+=worker_stats.inside_pixels;
master_stats->time_waiting+=worker_stats.time_waiting;
master_stats->time_working+=worker_stats.time_working;
}





static uint64_t get_delta_us(struct timespec start, struct timespec end) {
    uint64_t start_us = (uint64_t)start.tv_sec * 1000000ULL + (start.tv_nsec / 1000ULL);
    uint64_t end_us   = (uint64_t)end.tv_sec   * 1000000ULL + (end.tv_nsec   / 1000ULL);
    
    if (end_us < start_us) return 0;
    return end_us - start_us;
}

void updateTimeWaiting(render_stats_t *stats) {
    stats->time_waiting += get_delta_us(stats->start_t, stats->end_t);
}

void updateTimeWorking(render_stats_t *stats) {
    stats->time_working += get_delta_us(stats->start_t, stats->end_t);
} uint64_t          time_waiting;
  uint64_t          time_working;