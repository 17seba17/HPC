#include "../include/stats.h"
#include <stdio.h>

uint64_t checksum_update_uint (uint64_t     checksum,
		      unsigned int value)
{
  unsigned int byte_id;

  for (byte_id = 0; byte_id < 4; ++byte_id)
    {
      checksum ^= (uint64_t) ((value >> (8u * byte_id)) & 0xffu);
      checksum *= FNV_PRIME;
    }

  return checksum;
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
  printf ("iteration_checksum       %016llx\n", (unsigned long long) stats->checksum);
  printf ("my_checksum              %08x%08x%08x%08x\n", stats->my_checksum.slot[0],
                                                       stats->my_checksum.slot[1],
                                                       stats->my_checksum.slot[2],
                                                       stats->my_checksum.slot[3]);

  printf ("inside_pixels            %llu\n", (unsigned long long) stats->inside_pixels);
  printf ("inside_fraction          %.17g\n", inside_fraction);
  printf ("average_iterations       %.17g\n", average_iterations);
  printf ("total_iterations         %llu\n", (unsigned long long) stats->total_iterations);
}