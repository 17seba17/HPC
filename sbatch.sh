#!/bin/bash
#SBATCH --job-name=mandel_numa
#SBATCH --partition=GENOA
#SBATCH --output=output/mandel_%j.out
#SBATCH --nodes=1
#SBATCH --ntasks=4
#SBATCH --cpus-per-task=4
#SBATCH --account=dssc
#SBATCH --mem=5G
#SBATCH --time=00:15:00
EXECUTABLE="mandel_mariani_mpi"
mkdir -p output/
DETECTED_MARCH=$(gcc -Q --help=target -march=native | grep -E '^\s*-march=' | awk '{print $2}')

if [ -z "$DETECTED_MARCH" ]; then
    DETECTED_MARCH="x86-64-v4"
fi


make clean
make MARCH=$DETECTED_MARCH  $EXECUTABLE


export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

export OMP_PLACES=cores
export OMP_PROC_BIND=close
export I_MPI_PIN=1
export I_MPI_PIN_DOMAIN=numa

export OMP_PLACES=cores
export OMP_PROC_BIND=close

time mpirun ./build/$EXECUTABLE --kmax 1000 --ppu 1000
