#!/bin/sh
squeue
echo
tail -n 5 slurm-175.out
echo
cat slurm-175.out | grep hit | wc -l
echo
cat slurm-175.out | grep hit | tail -n 5
