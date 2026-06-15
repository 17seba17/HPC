#include "../include/options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <math.h>


static int
parse_unsigned_option ( int            argc,    // number of command-line tokens
			char         **argv,    // command-line token vector
			int           *i,       // index of the option currently being parsed
			unsigned int  *value    // destination for the parsed integer value
			);
static int
parse_double_option ( int      argc,    // number of command-line tokens
		      char   **argv,    // command-line token vector
		      int     *i,       // index of the option currently being parsed
		      double  *value    // destination for the parsed floating-point value
		      );
// print usage
void print_usage (char     *program_name)
{
  fprintf (stderr,
           "Usage: %s [options]\n"
           "\n"
           "Options:\n"
           "  --xmin VALUE        real coordinate of the upper-left corner     (%.17g)\n"
           "  --ymax VALUE        imaginary coordinate of the upper-left corner (%.17g)\n"
           "  --xmax VALUE        real coordinate of the bottom-right corner   (%.17g)\n"
           "  --ymin VALUE        imaginary coordinate of the bottom-right corner (%.17g)\n"
           "  --ppu VALUE         pixels per unit length on the real axis      (%u)\n"
           "  --dx-factor VALUE   accepted for interface compatibility         (%u)\n"
           "  --dy-factor VALUE   number of serial horizontal work stripes     (%u)\n"
           "  --kmax VALUE        maximum Mandelbrot iteration count           (%u)\n"
           "  --brute VALUE       the threshold for Mariani Silver algorithm   (%u)\n"
           "  --output FILE       output PNG file                              (%s)\n"
           "  --help              show this help message\n"
           "\n"
           "Example:\n"
           "  %s --ppu 512 --kmax 2048 --output mandelbrot.png\n",
           program_name,
           DEFAULT_XMIN, DEFAULT_YMAX, DEFAULT_XMAX, DEFAULT_YMIN,
           DEFAULT_PPU, DEFAULT_DX_FACTOR, DEFAULT_DY_FACTOR,
           DEFAULT_KMAX,DEFAULT_BRUTE, DEFAULT_OUTPUT, program_name);
}

//set default options
void set_default_options (options_t   *options)
{
  options->xmin      = DEFAULT_XMIN;
  options->xmax      = DEFAULT_XMAX;
  options->ymin      = DEFAULT_YMIN;
  options->ymax      = DEFAULT_YMAX;
  options->ppu       = DEFAULT_PPU;
  options->dx_factor = DEFAULT_DX_FACTOR;
  options->dy_factor = DEFAULT_DY_FACTOR;
  options->kmax      = DEFAULT_KMAX;
  options->brute      = DEFAULT_BRUTE;
  options->output_path = DEFAULT_OUTPUT;
}

// parse command line
int parse_command_line (int          argc, char       **argv, options_t   *options)
{
  int i;

  for (i = 1; i < argc; ++i)
    {
      if (strcmp (argv[i], "--help") == 0)
        {
          print_usage (argv[0]);
          return 1;
        }
      else if (strcmp (argv[i], "--xmin") == 0)
        {
          if (parse_double_option (argc, argv, &i, &options->xmin) != 0)
            return -1;
        }
      else if (strcmp (argv[i], "--xmax") == 0)
        {
          if (parse_double_option (argc, argv, &i, &options->xmax) != 0)
            return -1;
        }
      else if (strcmp (argv[i], "--ymin") == 0)
        {
          if (parse_double_option (argc, argv, &i, &options->ymin) != 0)
            return -1;
        }
      else if (strcmp (argv[i], "--ymax") == 0)
        {
          if (parse_double_option (argc, argv, &i, &options->ymax) != 0)
            return -1;
        }
      else if (strcmp (argv[i], "--ppu") == 0)
        {
          if (parse_unsigned_option (argc, argv, &i, &options->ppu) != 0)
            return -1;
        }
      else if (strcmp (argv[i], "--dx-factor") == 0)
        {
          if (parse_unsigned_option (argc, argv, &i, &options->dx_factor) != 0)
            return -1;
        }
      else if (strcmp (argv[i], "--dy-factor") == 0)
        {
          if (parse_unsigned_option (argc, argv, &i, &options->dy_factor) != 0)
            return -1;
        }
      else if (strcmp (argv[i], "--kmax") == 0)
        {
          if (parse_unsigned_option (argc, argv, &i, &options->kmax) != 0)
            return -1;
        }
      else if (strcmp (argv[i], "--brute") == 0)
        {
          if (parse_unsigned_option (argc, argv, &i, &options->brute) != 0)
            return -1;
        }
      else if (strcmp (argv[i], "--output") == 0)
        {
          if (i + 1 >= argc)
            {
              fprintf (stderr, "Missing value after --output\n");
              return -1;
            }

          options->output_path = argv[i + 1];
          i += 1;
        }
      else
        {
          fprintf (stderr, "Unknown option: %s\n", argv[i]);
          print_usage (argv[0]);
          return -1;
        }
    }

  return 0;
}

// validate options
int validate_options (options_t   *options)
{
  double x_extent;
  double y_extent;
  double width_estimate;
  double height_estimate;

  if (!(options->xmax > options->xmin))
    {
      fprintf (stderr, "Invalid view: --xmax must be larger than --xmin\n");
      return -1;
    }

  if (!(options->ymax > options->ymin))
    {
      fprintf (stderr, "Invalid view: --ymax must be larger than --ymin\n");
      return -1;
    }

  x_extent = options->xmax - options->xmin;
  y_extent = options->ymax - options->ymin;
  width_estimate = x_extent * (double) options->ppu;
  height_estimate = y_extent * (double) options->ppu;

  if (!isfinite (width_estimate) || !isfinite (height_estimate)
      || width_estimate < 1.0 || height_estimate < 1.0
      || width_estimate > (double) UINT_MAX
      || height_estimate > (double) UINT_MAX)
    {
      fprintf (stderr, "Invalid image dimensions implied by the view and --ppu\n");
      return -1;
    }

  return 0;
}


static int
parse_double_option ( int      argc,    // number of command-line tokens
		      char   **argv,    // command-line token vector
		      int     *i,       // index of the option currently being parsed
		      double  *value    // destination for the parsed floating-point value
		      )
{
  char   *endptr;
  double  parsed;

  if (*i + 1 >= argc)
    {
      fprintf (stderr, "Missing value after %s\n", argv[*i]);
      return -1;
    }

  errno = 0;
  endptr = NULL;
  parsed = strtod (argv[*i + 1], &endptr);

  if (errno != 0 || endptr == argv[*i + 1] || *endptr != '\0' || !isfinite (parsed))
    {
      fprintf (stderr, "Invalid floating-point value for %s: %s\n",
               argv[*i], argv[*i + 1]);
      return -1;
    }

  *value = parsed;
  *i += 1;

  return 0;
}

/*
  Parse a positive unsigned option of the form "--name value".
  It is used for ppu, factors, and kmax.
*/
static int
parse_unsigned_option ( int            argc,    // number of command-line tokens
			char         **argv,    // command-line token vector
			int           *i,       // index of the option currently being parsed
			unsigned int  *value    // destination for the parsed integer value
			)
{
  char          *endptr;
  unsigned long  parsed;

  if (*i + 1 >= argc)
    {
      fprintf (stderr, "Missing value after %s\n", argv[*i]);
      return -1;
    }

  errno = 0;
  endptr = NULL;
  parsed = strtoul (argv[*i + 1], &endptr, 10);

  if (errno != 0 || endptr == argv[*i + 1] || *endptr != '\0'
      || parsed == 0 || parsed > UINT_MAX)
    {
      fprintf (stderr, "Invalid positive integer value for %s: %s\n",
               argv[*i], argv[*i + 1]);
      return -1;
    }

  *value = (unsigned int) parsed;
  *i += 1;

  return 0;
}