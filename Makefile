CC       = mpicc
CFLAGS   ?= -std=c11 -O2 -Wall -Wextra -pedantic
LDFLAGS  ?=
LDLIBS   ?= -lm -lz
DIR      = build/


#Sequential Brute Force
BIN_BRUTE_SEQ   = mandel_brute_seq
SRC_BRUTE_SEQ   = main_brute_seq.c image.c options.c geometry.c kernel.c stats.c single/image_brute_seq.c io.c
OBJ_BRUTE_SEQ   = $(addprefix $(DIR), $(notdir $(SRC_BRUTE_SEQ:.c=.o)))

# Brute Force OpenMP
BIN_BRUTE_OMP   = mandel_brute_omp
SRC_BRUTE_OMP   = main_brute_omp.c image.c options.c geometry.c kernel.c stats.c single/image_brute_omp.c io.c
OBJ_BRUTE_OMP   = $(addprefix $(DIR), $(notdir $(SRC_BRUTE_OMP:.c=.o)))

# Hybrid Brute Force (MPI + OMP)
BIN_BRUTE_HYB   = mandel_brute_hybrid
SRC_BRUTE_HYB   = main_brute_hybrid.c image.c options.c geometry.c kernel.c stats.c io.c mpi/master_brute_hybrid.c mpi/worker_brute_hybrid.c
OBJ_BRUTE_HYB   = $(addprefix $(DIR), $(notdir $(SRC_BRUTE_HYB:.c=.o)))

# Mariani-Silver Seq.
BIN_MARIANI_SEQ = mandel_mariani_seq
SRC_MARIANI_SEQ = main_mariani_seq.c image.c options.c geometry.c kernel.c stats.c single/image_mariani_seq.c io.c
OBJ_MARIANI_SEQ = $(addprefix $(DIR), $(notdir $(SRC_MARIANI_SEQ:.c=.o)))

#  Mariani-Silver OpenMP
BIN_MARIANI_OMP = mandel_mariani_omp
SRC_MARIANI_OMP = main_mariani_omp.c image.c options.c geometry.c kernel.c stats.c single/image_mariani_omp.c io.c
OBJ_MARIANI_OMP = $(addprefix $(DIR), $(notdir $(SRC_MARIANI_OMP:.c=.o)))

# 6. Mariani-Silver MPI Puro (Master-Worker)
BIN_MARIANI_MPI = mandel_mariani_mpi
SRC_MARIANI_MPI = main_mariani_mpi.c image.c options.c geometry.c kernel.c stats.c io.c mpi/master_mariani_mpi.c mpi/worker_mariani_mpi.c
OBJ_MARIANI_MPI = $(addprefix $(DIR), $(notdir $(SRC_MARIANI_MPI:.c=.o)))

#  Mariani-Silver Hybrid 
BIN_MARIANI_HYB = mandel_mariani_hybrid
SRC_MARIANI_HYB = main_mariani_hybrid.c image.c options.c geometry.c kernel.c stats.c io.c mpi/master_mariani_hybrid.c mpi/worker_mariani_hybrid.c
OBJ_MARIANI_HYB = $(addprefix $(DIR), $(notdir $(SRC_MARIANI_HYB:.c=.o)))

ALL_TARGETS = $(DIR)$(BIN_BRUTE_SEQ) \
              $(DIR)$(BIN_BRUTE_OMP) \
              $(DIR)$(BIN_BRUTE_HYB) \
              $(DIR)$(BIN_MARIANI_SEQ) \
              $(DIR)$(BIN_MARIANI_OMP) \
              $(DIR)$(BIN_MARIANI_MPI) \
              $(DIR)$(BIN_MARIANI_HYB)


.PHONY: all clean make_dir show_image smoke mandel_brute_seq mandel_mariani_seq mandel_mariani_omp mandel_brute_omp mandel_brute_hyb mandel_mariani_mpi

vpath %.c src src/single src/mpi main
vpath %.h include

# 3/7
mandel_brute_seq: $(DIR)$(BIN_BRUTE_SEQ)
mandel_brute_omp: $(DIR)$(BIN_BRUTE_OMP)
mandel_brute_hyb: $(DIR)$(BIN_BRUTE_HYB)
mandel_mariani_seq: $(DIR)$(BIN_MARIANI_SEQ)
mandel_mariani_omp: $(DIR)$(BIN_MARIANI_OMP)
mandel_mariani_mpi: $(DIR)$(BIN_MARIANI_MPI)

all: make_dir $(ALL_TARGETS)

make_dir:
	@mkdir -p $(DIR) img/


$(DIR)%.o: %.c | make_dir
	@echo "Compilazione oggetto: $<"
	$(CC) $(CFLAGS) -fopenmp -Iinclude -c $< -o $@


$(DIR)$(BIN_BRUTE_SEQ): $(OBJ_BRUTE_SEQ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	@echo "Generato: $@"

$(DIR)$(BIN_BRUTE_OMP): $(OBJ_BRUTE_OMP)
	$(CC) $(CFLAGS) -fopenmp $(LDFLAGS) -o $@ $^ $(LDLIBS)
	@echo "Generato: $@"

$(DIR)$(BIN_BRUTE_HYB): $(OBJ_BRUTE_HYB)
	$(CC) $(CFLAGS) -fopenmp $(LDFLAGS) -o $@ $^ $(LDLIBS)
	@echo "Generato: $@"

$(DIR)$(BIN_MARIANI_SEQ): $(OBJ_MARIANI_SEQ)
	$(CC) $(CFLAGS) -fopenmp $(LDFLAGS) -o $@ $^ $(LDLIBS)
	@echo "Generato: $@"

$(DIR)$(BIN_MARIANI_OMP): $(OBJ_MARIANI_OMP)
	$(CC) $(CFLAGS) -fopenmp $(LDFLAGS) -o $@ $^ $(LDLIBS)
	@echo "Generato: $@"

$(DIR)$(BIN_MARIANI_MPI): $(OBJ_MARIANI_MPI)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	@echo "Generato: $@"

$(DIR)$(BIN_MARIANI_HYB): $(OBJ_MARIANI_HYB)
	$(CC) $(CFLAGS) -fopenmp $(LDFLAGS) -o $@ $^ $(LDLIBS)
	@echo "Generato: $@"



smoke: all
	@echo "=========================================================================="
	@echo "                   ANALISI DIAGNOSTICA DEI BINARI                         "
	@echo "=========================================================================="
	@mkdir -p img/
	@for exe in $(ALL_TARGETS); do \
		name=$$(basename $$exe); \
		output_img="img/$$name.ppm"; \
		rm -f $$output_img; \
		\
		echo -n "Stato di [$$name]: "; \
		\
		# Esecuzione del binario e cattura dell'output di console \
		if echo $$name | grep -q "mpi\|hybrid"; then \
			console_out=$$(mpirun -np 2 ./$$exe --output $$output_img 2>&1); \
		else \
			console_out=$$(./$$exe --output $$output_img 2>&1); \
		fi; \
		status=$$?; \
		\
		# 1. Rilevamento crash di sistema (es. Segmentation Fault) \
		if [ $$status -eq 139 ]; then \
			echo "ERRORE DI SISTEMA (Segmentation Fault / Core Dumped)"; \
			continue; \
		elif [ $$status -ne 0 ]; then \
			echo "ERRORE DI ESECUZIONE (Codice di errore: $$status)"; \
			continue; \
		fi; \
		\
		# 2. Rilevamento blocco non implementato (Output 'TBD') \
		if echo "$$console_out" | grep -q -i "TBD"; then \
			echo "NON IMPLEMENTATO (Rilevata stringa di completamento 'TBD')"; \
			continue; \
		fi; \
		\
		# 3. Verifica della presenza fisica del file d'immagine sul disco \
		if [ ! -f $$output_img ]; then \
			echo "ESECUZIONE CORRETTA SENZA OUTPUT (Nessun file d'immagine generato)"; \
			continue; \
		fi; \
		\
		# 4. Estrazione delle metriche dall'output della console \
		width=$$(echo "$$console_out" | grep "^width" | awk '{print $$2}'); \
		height=$$(echo "$$console_out" | grep "^height" | awk '{print $$2}'); \
		inside=$$(echo "$$console_out" | grep "^inside_pixels" | awk '{print $$2}'); \
		my_checksum=$$(echo "$$console_out" | grep "^my_checksum" | awk '{print $$2}'); \
		\
		# Calcolo dei limiti teorici \
		total_pixels=$$((width * height)); \
		\
		# 5. Classificazione analitica in base alle statistiche estratte \
		if [ -z "$$inside" ] || [ -z "$$width" ] || [ -z "$$height" ]; then \
			echo "ERRORE DI RENDERING (Impossibile verificare le statistiche del calcolo)"; \
		elif [ "$$inside" -eq "$$total_pixels" ]; then \
			echo "ERRORE DI RENDERING (Immagine generata ma completamente nera)"; \
		elif [ "$$inside" -eq 0 ]; then \
			echo "ERRORE DI RENDERING (Immagine generata ma priva di pixel interni)"; \
		else \
			echo "COMPILATO E FUNZIONANTE (Immagine valida generata in $$output_img, inside: $$inside) checksum=$$my_checksum"; \
		fi; \
	done
	@echo "=========================================================================="

clean:
	@rm -rf $(DIR) img/*

show_image:
	@find img/ -type f -exec xdg-open {} \;