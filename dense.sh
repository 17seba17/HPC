#!/bin/bash
#SBATCH --job-name=mandel_dense
#SBATCH --partition=GENOA
#SBATCH --output=output/mandel_%j.out
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=64
#SBATCH --account=dssc
#SBATCH --mem=20G
#SBATCH --time=00:30:00

mkdir -p output/verify/

DETECTED_MARCH=$(gcc -Q --help=target -march=native | grep -E '^\s*-march=' | awk '{print $2}')
if [ -z "$DETECTED_MARCH" ]; then
    DETECTED_MARCH="x86-64-v4"
fi

make clean
make MARCH=$DETECTED_MARCH mandel_brute_omp mandel_mariani_omp

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
export OMP_PLACES=cores
export OMP_PROC_BIND=close

CSV_FILE="output/verify_results.csv"
echo "kmax,risoluzione,numero di pixel interni,metodo usato,tempo di esecuzione" > $CSV_FILE

RESOLUTIONS=(1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192)
KMAX_VALUES=(1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192)

for KMAX in "${KMAX_VALUES[@]}"; do
    for PPU in "${RESOLUTIONS[@]}"; do
        
        WIDTH=$((4 * PPU))
        HEIGHT=$((4 * PPU))
        
        echo "Running PPU=$PPU (${WIDTH}x${HEIGHT}), KMAX=$KMAX..."
        
        # BRUTE FORCE
        START_BRUTE=$(date +%s.%N)
        OUTPUT_BRUTE=$(time ./build/mandel_brute_omp --ppu $PPU --kmax $KMAX 2>&1)
        END_BRUTE=$(date +%s.%N)
        TIME_BRUTE=$(echo "$END_BRUTE - $START_BRUTE" | bc)
        PIXELS_BRUTE=$(echo "$OUTPUT_BRUTE" | grep "inside_pixels" | awk '{print $2}')
       
        # MARIANI SILVER
        START_MARIANI=$(date +%s.%N)
        OUTPUT_MARIANI=$(time ./build/mandel_mariani_omp --ppu $PPU --kmax $KMAX 2>&1)
        END_MARIANI=$(date +%s.%N)
        TIME_MARIANI=$(echo "$END_MARIANI - $START_MARIANI" | bc)
        PIXELS_MARIANI=$(echo "$OUTPUT_MARIANI" | grep "inside_pixels" | awk '{print $2}')
        

	echo "$KMAX,${WIDTH}x${HEIGHT},$PIXELS_BRUTE,brute_omp,$TIME_BRUTE" >> $CSV_FILE        
        echo "$KMAX,${WIDTH}x${HEIGHT},$PIXELS_MARIANI,mariani_omp,$TIME_MARIANI" >> $CSV_FILE
        
    done
done

echo "Esperimenti completati! Dati salvati in $CSV_FILE"
