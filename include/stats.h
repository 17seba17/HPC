#ifndef STATS_H
#define STATS_H

#include "geometry.h"
#include "options.h"
#include <stdint.h>

#include <time.h>
#define FNV_OFFSET_BASIS (1469598103934665603ULL)
#define FNV_PRIME (1099511628211ULL)

typedef struct {
  unsigned int slot[4];
} checksum_t;

typedef struct {
  checksum_t my_checksum;
  uint64_t total_iterations;
  uint64_t inside_pixels;
  struct timespec start_t;
  struct timespec end_t;
  uint64_t time_waiting;
  uint64_t time_working;
  uint16_t tiles_processed;
} render_stats_t;

void aggregateStats(render_stats_t *master_stats, render_stats_t worker_stats);

void zero_stats(render_stats_t *stats);

checksum_t my_checksum_update(checksum_t checksum, unsigned int value1,
                              unsigned int value2, unsigned int value3);

void print_stats(const image_geometry_t *geometry, const options_t *options,
                 const render_stats_t *stats);

void updateTimeWaiting(render_stats_t *stats);

void updateTimeWorking(render_stats_t *stats);

#endif