#!/bin/bash
#SBATCH --job-name=schedule_mandelbrot
#SBATCH --partition=dcgp_usr_prod
#SBATCH --account=uts26_tornator_0
#SBATCH --nodes=1
#SBATCH --ntasks=4
#SBATCH --cpus-per-task=16
#SBATCH --mem=100G
#SBATCH --gres=tmpfs:10g
#SBATCH --time=00:10:00
#SBATCH --output=output/schedule_%j.out

mkdir -p output/


find . -type f \( -name "*.c" -o -name "*.h" \) -exec sed -i 's/dynamic/runtime/g' {} +


DETECTED_MARCH=$(gcc -Q --help=target -march=native | grep -E '^\s*-march=' | awk '{print $2}')
if [ -z "$DETECTED_MARCH" ]; then
    DETECTED_MARCH="x86-64-v4"
fi

make clean
make MARCH=$DETECTED_MARCH mandel_brute_omp mandel_brute_hyb mandel_mariani_omp mandel_mariani_hyb


export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
export OMP_PLACES=cores
export OMP_PROC_BIND=close

CSV_FILE="output/verify_schedule.csv"

echo "exec,schedule,res,factor,brute,run_id,time" > $CSV_FILE


EXECUTABLES=("mandel_mariani_hybrid")
SCHEDULES=("static" "dynamic" "dynamic,2" "dynamic,4" "dynamic,8" "guided")
RESOLUTIONS=(4096 8192)

REPETITIONS=5

FACTOR=16
BRUTE=512


for EXEC in "${EXECUTABLES[@]}"; do
    if [ ! -f "./build/$EXEC" ]; then
        echo "Salto ./build/$EXEC (non trovato)"
        continue
    fi

    for RES in "${RESOLUTIONS[@]}"; do
        for SCHED in "${SCHEDULES[@]}"; do
            export OMP_SCHEDULE="$SCHED"
            
            for RUN in $(seq 1 $REPETITIONS); do
                
                echo "Run $RUN/$REPETITIONS | Exec: $EXEC | Sched: $SCHED | Res: $RES"
                
                START=$(date +%s.%N)
                
                if [[ "$EXEC" == *"omp"* ]]; then
                    ./build/$EXEC --ppu $RES > /dev/null
                else
                    mpirun ./build/$EXEC --ppu $RES --dx-factor $FACTOR --dy-factor $FACTOR --brute $BRUTE > /dev/null
                fi
                
                END=$(date +%s.%N)
                TIME_RUN=$(echo "$END - $START" | bc)
                
                echo "$EXEC,\"$SCHED\",$RES,$FACTOR,$BRUTE,$RUN,$TIME_RUN" >> $CSV_FILE
                
            done
        done
    done
done


find . -type f \( -name "*.c" -o -name "*.h" \) -exec sed -i 's/runtime/dynamic/g' {} +

echo "end"
