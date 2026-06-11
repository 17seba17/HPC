CC       ?= gcc
CFLAGS   ?= -std=c11 -O2 -Wall -Wextra -pedantic
LDFLAGS  ?=
LDLIBS   ?= -lm -lz

TARGET = main
SRC    = main.c options.c geometry.c kernel.c stats.c image.c io.c
HEADERS= options.h geometry.h kernel.h image.h io.h
OBJ = $(SRC:.c=.o)
DIR    = build/

.PHONY: create all make_dir
vpath %.c src
vpath %.h include

all: create

create: $(TARGET)

$(TARGET): $(addprefix $(DIR), $(OBJ))
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	@echo "$(TARGET) has NO errrors !!!"

$(DIR)%.o: %.c  $(HEADERS)| make_dir
	@echo "generated $@"
	$(CC) $(CFLAGS) -c $< -o $@

make_dir:
	@mkdir -p $(DIR)

smoke: clean
	@echo "BENCHMARK"
	@mkdir -p $(DIR) img/
	@for opt in "-O0" "-O1" "-O2" "-O3" "-O3 -march=native"; do \
		echo "flag: $$opt"; \
		$(CC) -std=c11 $$opt -Wall -Wextra -pedantic $(addprefix src/, $(SRC:main.c=)) main.c -o $(TARGET)_temp $(LDLIBS) $(LDFLAGS); \
		python3 benchmark.py ./$(TARGET)_temp; \
		rm -f $(TARGET)_temp; \
	done
clean:
	rm -rf $(DIR) $(TARGET) img/mandelbrot.pgm