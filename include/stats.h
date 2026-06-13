#ifndef STATS_H
#define STATS_H



#include <stdint.h>
#include "options.h"
#include "geometry.h"

#define FNV_OFFSET_BASIS   (1469598103934665603ULL)
#define FNV_PRIME          (1099511628211ULL)

typedef struct
{
    unsigned int slot[4];
}checksum_t;

typedef struct
{ uint64_t          checksum;
  checksum_t        my_checksum;
  uint64_t          total_iterations;
  uint64_t          inside_pixels;
} render_stats_t;


void aggregateStats(render_stats_t *master_stats, render_stats_t consumer_stats);

void zero_stats(render_stats_t *stats);
uint64_t checksum_update_uint (uint64_t     checksum,
                      unsigned int value);

checksum_t my_checksum_update (checksum_t     checksum,
                      unsigned int value1, unsigned int value2, unsigned int value3);

void
print_stats (const image_geometry_t *geometry,
             const options_t        *options,
             const render_stats_t   *stats);
#endif