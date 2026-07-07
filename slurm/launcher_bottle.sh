#!/bin/bash


mkdir -p output/
CSV_FILE="output/bottle.csv"
echo "exec,kmax,ppu,factor,brute,ranks,run_id,rank_id,inside_pixels,inside_fraction,average_iterations,total_iterations,time_waiting,time_working,tiles_processed" > $CSV_FILE

P_VALUES=(2 4 8 16 32)

for P in "${P_VALUES[@]}"; do
    JOB_CSV="output/mandel_bottleneck_P${P}.csv"
    rm -f $JOB_CSV
    
    echo "Job Slurm  P=$P..."
    sbatch --ntasks=$P \
           --job-name=bottle_P${P} \
           --export=ALL,P=$P,JOB_CSV=$JOB_CSV \
           slurm/bottle.sh
done
echo "done"
