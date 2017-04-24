#!/bin/tcsh
#SBATCH -t 00:02:30
#SBATCH --exclusive
#SBATCH --mem-per-cpu=2048
#SBATCH --mail-user=chiragm@umd.edu
#SBATCH --mail-type=END
#SBATCH --output=out
#SBATCH -p debug
#SBATCH --export=OMP_NUM_THREADS=16

module load openmpi/gnu
./run.sh
