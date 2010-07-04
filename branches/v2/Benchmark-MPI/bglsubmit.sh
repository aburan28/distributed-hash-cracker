sbatch -p smallmem --nodes 64 -t 10 -o ./$SLURM_JOB_ID ./bgljob.sh
