#!/bin/bash
#SBATCH --partition=GENOA
#SBATCH --account=dssc
#SBATCH --nodes=1
#SBATCH --cpus-per-task=1
#SBATCH --time=00:20:00
#SBATCH --output=output/mandel_weak_P%j.out

TIMEOUT_LIMIT="3m"

EXEC="mandel_mariani_mpi"
REPETITIONS=5
FACTOR=16
BRUTE=1024
TEMP_OUT="temp_mandel_weak_P${P}_run.out"

echo "=== Esecuzione Job per Ranks=$P e PPU=$PPU ==="

for RUN in $(seq 1 $REPETITIONS); do
    echo "Execution Run $RUN/$REPETITIONS..."
    
    timeout $TIMEOUT_LIMIT mpirun -n $P ./build/$EXEC --ppu $PPU --dx-factor $FACTOR --dy-factor $FACTOR --brute $BRUTE > $TEMP_OUT
    EXIT_STATUS=$?
    
    if [ $EXIT_STATUS -eq 124 ]; then
        echo "WARNING: Run $RUN/$REPETITIONS has reached TIMEOUT ($TIMEOUT_LIMIT)!"
        echo "$EXEC,$PPU_BASE,$PPU,$P,$RUN,TIMEOUT_ERROR,0,0,0,0,0,0,0" >> $JOB_CSV
        continue
    fi
    
    awk -v exec="$EXEC" -v ppu_b="$PPU_BASE" -v ppu_r="$PPU" -v p="$P" -v run="$RUN" '
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
            print exec","ppu_b","ppu_r","p","run","rank_id","inside","inside_frac","avg_iter","tot_iter","t_wait","t_work","tiles
        }
        rank_id=""; inside=""; inside_frac=""; avg_iter=""; tot_iter=""; t_wait=""; t_work=""; tiles=""
    }
    ' $TEMP_OUT >> $JOB_CSV
    
done

echo "end"
