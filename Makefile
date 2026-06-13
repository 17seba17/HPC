CC       = mpicc
CFLAGS   ?= -std=c11 -O2 -Wall -Wextra -pedantic
LDFLAGS  ?=
LDLIBS   ?= -lm -lz
DIR      = build/

TARGET_BRUTE  = main_brute_force
SRC_BRUTE     = main_brute_seq.c options.c geometry.c kernel.c stats.c image_forza_bruta.c io.c
OBJ_BRUTE     = $(addprefix $(DIR), $(SRC_BRUTE:.c=.o))


TARGET_SINGLE = main_single_thread
SRC_SINGLE    = main_brute_seq.c options.c geometry.c kernel.c stats.c image_mariani_silver_single.c io.c
OBJ_SINGLE    = $(addprefix $(DIR), $(SRC_SINGLE:.c=.o))


TARGET_MPI    = main_mpi
SRC_MPI       = main_mpi.c options.c geometry.c kernel.c stats.c  io.c master.c consumer.c
OBJ_MPI       = $(addprefix $(DIR), $(SRC_MPI:.c=.o))

HEADERS = options.h geometry.h kernel.h image.h io.h

.PHONY: all make_dir clean open brute_force single_thread mpi smoke
vpath %.c src main
vpath %.h include


mpi:            $(DIR)$(TARGET_MPI) |make_dir

all:  brute_force single_thread mpi


brute_force:    $(DIR)$(TARGET_BRUTE)|make_dir
single_thread:  $(DIR)$(TARGET_SINGLE)|make_dir


$(DIR)$(TARGET_STRIPE): $(OBJ_STRIPE)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	@echo "Compilato: $@"

$(DIR)$(TARGET_BRUTE): $(OBJ_BRUTE)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	@echo "Compilato: $@"

$(DIR)$(TARGET_SINGLE): $(OBJ_SINGLE)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	@echo "Compilato: $@"

$(DIR)$(TARGET_MPI): $(OBJ_MPI)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	@echo "Compilato: $@"

$(DIR)%.o: %.c  $(HEADERS)| make_dir
	@echo "generated $@"
	$(CC) $(CFLAGS) -c $< -o $@

make_dir:
	@mkdir -p $(DIR)


smoke: clean
	@echo "BENCHMARK"
	@mkdir -p $(DIR) img/
	@for opt in "-O0" "-O1" "-O2" "-O3" "-O3 -march=native"; do \
		echo "Compilazione con flag: $$opt"; \
		$(CC) -std=c11 $$opt -Wall -Wextra -pedantic $(addprefix src/, $(SRC_SINGLE)) -o $(TARGET_SINGLE)_temp $(LDLIBS) $(LDFLAGS); \
		python3 benchmark.py ./$(TARGET_SINGLE)_temp; \
		rm -f $(TARGET_SINGLE)_temp; \
	done

clean:
	@rm -rf $(DIR) img/*

open:
	@find img/ -type f  -exec xdg-open {} \;