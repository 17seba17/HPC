#!/bin/bash
#SBATCH --job-name=parameter_mandelbrot
#SBATCH --partition=GENOA
#SBATCH --output=output/parameter_%j.out
#SBATCH --nodes=1
#SBATCH --ntasks=32
#SBATCH --cpus-per-task=1
#SBATCH --account=dssc
#SBATCH --mem=50G
#SBATCH --time=00:40:00

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
echo "factor,res,time,brute" > $CSV_FILE


RESOLUTION=8192
FACTOR_VALUES=(4 5 8 9 16 17 32 33 64 65 128 129 256 257 512 513)
BRUTE_VALUES=(16384 32768 65536 131072)


for BRUTE in "${BRUTE_VALUES[@]}"; do
for FACTOR in "${FACTOR_VALUES[@]}"; do
        
        WIDTH=$((4 * RESOLUTION))
        HEIGHT=$((4 * RESOLUTION))
        
        echo "Running BRUTE=$BRUTE, FACTOR=$FACTOR..."
        
       
        # MPI
        START_MPI=$(date +%s.%N)
        OUTPUT_MPI=$(mpirun ./build/mandel_mariani_mpi --ppu $RESOLUTION --dx-factor $FACTOR --dy-factor $FACTOR --brute $BRUTE)
        END_MPI=$(date +%s.%N)
        TIME_MPI=$(echo "$END_MPI - $START_MPI" | bc)
        

        echo "$FACTOR,${WIDTH}x${HEIGHT},$TIME_MPI,$BRUTE" >> $CSV_FILE
        
    done
done

echo "end"
