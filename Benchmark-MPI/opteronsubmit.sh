#!/bin/bash
sbatch -p opterons --constraint=blade -n 64 -t 10 -D /gpfs/small/PPCC/home/PPCCzone/project/Benchmark-MPI -o /gpfs/small/PPCC/home/PPCCzone/project/Benchmark-MPI/opteronout.txt ./opteronjob.sh
