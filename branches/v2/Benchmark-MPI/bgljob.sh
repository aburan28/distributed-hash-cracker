#!/bin/bash

#--name=$SLURM_JOB_ID
echo "Job starting..."
mpirun -np 64 -mode VN -cwd /gpfs/small/PPCC/home/PPCCzone/project/Benchmark-MPI /gpfs/small/PPCC/home/PPCCzone/project/Benchmark-MPI/benchmark-md5
