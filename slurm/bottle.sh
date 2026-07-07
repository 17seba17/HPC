#!/bin/bash
#SBATCH --partition=GENOA
#SBATCH --account=dssc
#SBATCH --nodes=1
#SBATCH --cpus-per-task=1
#SBATCH --time=00:15:00
#SBATCH --output=output/mandel_bottle_P%j.out

TIMEOUT_LIMIT="2m"
EXEC="mandel_mariani_mpi"
REPETITIONS=5

K_LOW=8192
RES=16384
FACTOR_HIGH=33
BRUTE_LOW=1024

TEMP_OUT="temp_mandel_bottle_P${P}_run.out"


for RUN in $(seq 1 $REPETITIONS); do
    echo "Exec Run $RUN/$REPETITIONS..."
    
    timeout $TIMEOUT_LIMIT mpirun -n $P ./build/$EXEC --kmax $K_LOW --ppu $RES --dx-factor $FACTOR_HIGH --dy-factor $FACTOR_HIGH --brute $BRUTE_LOW > $TEMP_OUT
    EXIT_STATUS=$?
    
    if [ $EXIT_STATUS -eq 124 ]; then
        echo "ATTENZIONE: Run $RUN/$REPETITIONS è andata in TIMEOUT dopo $TIMEOUT_LIMIT!"
        echo "$EXEC,$K_LOW,$RES,$FACTOR_HIGH,$BRUTE_LOW,$P,$RUN,TIMEOUT_ERROR,0,0,0,0,0,0,0" >> $JOB_CSV
        continue
    fi
    
    awk -v exec="$EXEC" -v k="$K_LOW" -v res="$RES" -v f="$FACTOR_HIGH" -v b="$BRUTE_LOW" -v p="$P" -v run="$RUN" '
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
            print exec","k","res","f","b","p","run","rank_id","inside","inside_frac","avg_iter","tot_iter","t_wait","t_work","tiles
        }
        rank_id=""; inside=""; inside_frac=""; avg_iter=""; tot_iter=""; t_wait=""; t_work=""; tiles=""
    }
    ' $TEMP_OUT >> $JOB_CSV
    
done
echo "end"
