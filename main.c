#include "include/options.h"
#include "include/geometry.h"
#include "include/stats.h"
#include "include/image.h"
#include "include/io.h"

#include <stdlib.h>
#include <stdio.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

int main(int argc, char **argv) {
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

    // Allocazione buffer
    size_t image_bytes = (size_t)geometry.width * (size_t)geometry.height * 3u;
    unsigned char *image = malloc(image_bytes);
    if (image == NULL) {
        return EXIT_FAILURE;
    }

    // Logica di rendering (qui verrà inserito il parallelismo MPI/OpenMP)
    render_image(&options, &geometry, image, &stats);

    // Salvataggio dei risultati
 if (write_ppm_image(image, geometry.width, geometry.height, options.output_path) != 0) {
        perror("Errore durante il salvataggio dell'immagine"); 
        free(image);
        return EXIT_FAILURE;
    }
    // Stampa statistiche
    print_stats(&geometry, &options, &stats);

    free(image);
    return EXIT_SUCCESS;
}
