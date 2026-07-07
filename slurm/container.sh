#!/bin/bash
#SBATCH --nodes=1
#SBATCH --time=00:10:00

module load singularity
module load openMPI/4.1.6 

if [ "$TYPE" = "omp" ] || [ "$TYPE" = "hybrid" ]; then
    export OMP_NUM_THREADS=$CPUS
    export OMP_PLACES=cores
    export OMP_PROC_BIND=close
else
    export OMP_NUM_THREADS=1
fi

for RUN in {1..5}; do
    START_TIME=$(date +%s.%N)
    
    if [ "$TYPE" = "omp" ]; then
        singularity exec "$CONTAINER" ./build/$EXEC --ppu $PPU --kmax $KMAX --dx-factor $DX --dy-factor $DY --brute $BRUTE --output /dev/null
    else
        srun -n $TASKS singularity exec "$CONTAINER" ./build/$EXEC --ppu $PPU --kmax $KMAX --dx-factor $DX --dy-factor $DY --brute $BRUTE --output /dev/null
    fi
    
    END_TIME=$(date +%s.%N)
    T_REAL=$(awk -v s="$START_TIME" -v e="$END_TIME" 'BEGIN {print e - s}')
    
    echo "$T_REAL,$KMAX,$PPU,$RES,$BRUTE,$CONF" >> "$CSV"
done
echo "end"
