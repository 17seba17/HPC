#ifndef OPTIONS_H
#define OPTIONS_H

#define DEFAULT_XMIN       (-2.5)
#define DEFAULT_XMAX       (1.5)
#define DEFAULT_YMIN       (-2.0)
#define DEFAULT_YMAX       (2.0)
#define DEFAULT_PPU        (256u)
#define DEFAULT_DX_FACTOR  (8u)
#define DEFAULT_DY_FACTOR  (8u)
#define DEFAULT_KMAX       (1024u)
#define DEFAULT_OUTPUT     "img/mandelbrot.pgm"

typedef struct
{
  double            xmin;
  double            xmax;
  double            ymin;
  double            ymax;
  unsigned int      ppu;
  unsigned int      dx_factor;
  unsigned int      dy_factor;
  unsigned int      kmax;
  char             *output_path;
} options_t;

void print_usage (char *program_name);
void set_default_options (options_t    *options);
int parse_command_line (int        argc, char       **argv, options_t   *options);
int validate_options (options_t   *options);
#endif