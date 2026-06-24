#!/bin/bash
#SBATCH --job-name=mandel_dense
#SBATCH --partition=GENOA
#SBATCH --output=output/mandel_%j.out
#SBATCH --nodes=1
#SBATCH --ntasks=64
#SBATCH --cpus-per-task=1
#SBATCH --account=dssc
#SBATCH --mem=30G
#SBATCH --time=00:15:00

mkdir -p output/verify/

DETECTED_MARCH=$(gcc -Q --help=target -march=native | grep -E '^\s*-march=' | awk '{print $2}')
if [ -z "$DETECTED_MARCH" ]; then
    DETECTED_MARCH="x86-64-v4"
fi

make clean
make MARCH=$DETECTED_MARCH  mandel_mariani_mpi

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
export OMP_PLACES=cores
export OMP_PROC_BIND=close

CSV_FILE="output/verify_factors.csv"
echo "factor,risoluzione,tempo_di_esecuzione" > $CSV_FILE


# RESOLUTIONS=(8192)
# FACTOR_VALUES=({32..128})

# RESOLUTIONS=(2048 4096 8192)
# FACTOR_VALUES=(1 4 8 16 32 48 64 96 128 192 256 384 512 768 1024 1280 1536 1792 2048)

RESOLUTIONS=(8192)
FACTOR_VALUES=(1 2 3 4 5 8 9 16 17 32 33 64 65 128 129 256 257 512 513 1024 1025 2048 2049 4096 4097)

for FACTOR in "${FACTOR_VALUES[@]}"; do
    for PPU in "${RESOLUTIONS[@]}"; do
        
        WIDTH=$((4 * PPU))
        HEIGHT=$((4 * PPU))
        
        echo "Running PPU=$PPU (${WIDTH}x${HEIGHT}), FACTOR=$FACTOR..."
        
       
        # MPI
        START_MPI=$(date +%s.%N)
        OUTPUT_MPI=$(time mpirun ./build/mandel_mariani_mpi --ppu $PPU --dx-factor $FACTOR --dy-factor $FACTOR   2>&1)
        END_MPI=$(date +%s.%N)
        TIME_MPI=$(echo "$END_MPI - $START_MPI" | bc)
        

        echo "$FACTOR,${WIDTH}x${HEIGHT},$TIME_MPI" >> $CSV_FILE
        
    done
done

echo "Dati salvati in $CSV_FILE"
