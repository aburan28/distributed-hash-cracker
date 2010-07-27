#!/bin/sh
squeue
echo
tail -n 5 slurm-117.out
echo
cat slurm-117.out | grep hit | wc -l
echo
cat slurm-117.out | grep hit | tail -n 5
