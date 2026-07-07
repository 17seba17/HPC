#!/bin/bash
OUTPUT_FILE="container.csv"
if [ ! -f "$OUTPUT_FILE" ]; then
    echo "t_real,kmax,ppu,risolution,brute,conf_cpu" > "$OUTPUT_FILE"
fi

PPU=16384
KMAX=16384
BRUTE=16384
DX=33
DY=33
RES="65536x65536"
CONTAINER="mandelbrot.sif"

sbatch --nodes=1 --ntasks=1 --cpus-per-task=16 --partition=GENOA --account=dssc --mem=32G --job-name=m_omp --export=ALL,EXEC=mandel_mariani_omp,TYPE=omp,TASKS=1,CPUS=16,PPU=$PPU,KMAX=$KMAX,BRUTE=$BRUTE,DX=$DX,DY=$DY,RES=$RES,CONF="16_omp",CSV=$OUTPUT_FILE,CONTAINER=$CONTAINER slurm/container.sh

sbatch --nodes=1 --ntasks=16 --cpus-per-task=1 --partition=GENOA --account=dssc --mem=32G --job-name=m_mpi --export=ALL,EXEC=mandel_mariani_mpi,TYPE=mpi,TASKS=16,CPUS=1,PPU=$PPU,KMAX=$KMAX,BRUTE=$BRUTE,DX=$DX,DY=$DY,RES=$RES,CONF="16_mpi",CSV=$OUTPUT_FILE,CONTAINER=$CONTAINER slurm/container.sh

sbatch --nodes=1 --ntasks=4 --cpus-per-task=4 --partition=GENOA --account=dssc --mem=32G --job-name=m_hyb --export=ALL,EXEC=mandel_mariani_hybrid,TYPE=hybrid,TASKS=4,CPUS=4,PPU=$PPU,KMAX=$KMAX,BRUTE=$BRUTE,DX=$DX,DY=$DY,RES=$RES,CONF="4_mpi_4_omp",CSV=$OUTPUT_FILE,CONTAINER=$CONTAINER slurm/container.sh


