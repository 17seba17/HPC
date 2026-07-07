#!/bin/bash
make clean
make MARCH=x86-64-v4 mandel_mariani_mpi

mkdir -p output/

P_VALUES=(2 4 8 16 32)
PPU_BASE=2048

JOB_IDS=()

for P in "${P_VALUES[@]}"; do
    PPU=$(awk -v base="$PPU_BASE" -v p="$P" 'BEGIN { print int(base * sqrt(p - 1) + 0.5) }')
    JOB_CSV="output/mandel_weak_P${P}.csv"
    rm -f $JOB_CSV    
    echo "Submitting Job (P=$P and PPU=$PPU)..."
    
    SUBMIT_MSG=$(sbatch --ntasks=$P \
                        --job-name=weak_P${P} \
                        --export=ALL,P=$P,PPU=$PPU,PPU_BASE=$PPU_BASE,JOB_CSV=$JOB_CSV \
                        slurm/weak_scaling.sh)
    
    JOB_ID=$(echo "$SUBMIT_MSG" | awk '{print $NF}')
    echo "Job ID: $JOB_ID"
    
    JOB_IDS+=("$JOB_ID")
done

DEP_STRING=$(printf ":%s" "${JOB_IDS[@]}")

echo "All the jobs were submitted"

sbatch --partition=GENOA \
       --account=dssc \
       --ntasks=1 \
       --time=00:02:00 \
       --job-name=merge_weak_results \
       --dependency=afterok$DEP_STRING \
       --wrap="echo 'eseguibile,ppu_base,ppu_reale,ranks,run_id,rank_id,inside_pixels,inside_fraction,average_iterations,total_iterations,time_waiting,time_working,tiles_processed' > output/mandel_mpi_weak_metrics.csv && cat output/mandel_weak_P*.csv >> output/mandel_mpi_weak_metrics.csv"

echo "end"
