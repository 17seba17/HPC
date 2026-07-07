#!/bin/bash
#SBATCH --job-name=strong_scaling_mandelbrot
#SBATCH --partition=dcgp_usr_prod
#SBATCH --account=uts26_tornator_0
#SBATCH --nodes=1
#SBATCH --ntasks=32
#SBATCH --cpus-per-task=1
#SBATCH --time=00:15:00
#SBATCH --output=output/strong_%j.out

mkdir -p output/

DETECTED_MARCH=$(gcc -Q --help=target -march=native | grep -E '^\s*-march=' | awk '{print $2}')
if [ -z "$DETECTED_MARCH" ]; then
    DETECTED_MARCH="x86-64-v4"
fi

make clean
make MARCH=$DETECTED_MARCH mandel_mariani_mpi

CSV_FILE="output/strong.csv"


echo "exec,res,ranks,run_id,rank_id,inside_pixels,inside_fraction,average_iterations,total_iterations,time_waiting,time_working,tiles_processed" > $CSV_FILE


EXEC="mandel_mariani_mpi"
RES=8192
KMAX=(1024 2048 4096 8192 16384 32768 65536 131072)
P_VALUES=(2 4 8 16 32)
REPETITIONS=5

FACTOR=16
BRUTE=512
TEMP_OUT="temp_mandel_run.out"

if [ ! -f "./build/$EXEC" ]; then
    echo "Errore: ./build/$EXEC non trovato!"
    exit 1
fi

mpirun -n 4 ./build/$EXEC
exit 0

for K in "${KMAX[@]}"; do
    for P in "${P_VALUES[@]}"; do
        for RUN in $(seq 1 $REPETITIONS); do
            
            echo "Esecuzione Run $RUN/$REPETITIONS | Ranks: $P | Risoluzione: $RES"
            
            mpirun -n $P ./build/$EXEC --kmax $K --ppu $RES --dx-factor $FACTOR --dy-factor $FACTOR --brute $BRUTE > $TEMP_OUT
            
            awk -v exec="$EXEC" -v res="$RES" -v p="$P" -v run="$RUN" '
            BEGIN {
                rank_id=""; inside=""; inside_frac=""; avg_iter=""; tot_iter=""; t_wait=""; t_work=""; tiles=""
            }
            $1 == "rank" { rank_id=$2 }
            $1 == "inside_pixels" { inside=$2 }
            $1 == "inside_fraction" { inside_frac=$2 }
            $1 == "average_iterations" { avg_iter=$2 }
            $1 == "total_iterations" { tot_iter=$2 }
            $1 == "time" && $2 == "waiting" { t_wait=$3 }
            $1 == "time" && $2 == "working" { t_work=$3 }
            $1 == "tiles_processed" { 
                tiles=$2
                
                if (rank_id != "") {
                    print exec","res","p","run","rank_id","inside","inside_frac","avg_iter","tot_iter","t_wait","t_work","tiles
                }
                rank_id=""; inside=""; inside_frac=""; avg_iter=""; tot_iter=""; t_wait=""; t_work=""; tiles=""
            }
            ' $TEMP_OUT >> $CSV_FILE
            
        done
    done
done


rm -f $TEMP_OUT
echo "end"
