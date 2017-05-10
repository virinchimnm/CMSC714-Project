#!/bin/bash
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=8
#SBATCH -t 00:02:00
#SBATCH --mail-user=somay@umd.edu
#SBATCH --mail-type=END
#SBATCH --output=omp_mpi_output_2_8.txt
#SBATCH -p debug
#SBATCH --exclusive

module unload intel
module load openmpi/gnu

OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
export OMP_NUM_THREADS

echo "Running OMP code"
mpirun  ./omp-cc -i /lustre/cmsc714-1buw/datasets/amazon-2.txt

echo "Running MPI code"
mpirun  ./mpi-cc -i /lustre/cmsc714-1buw/datasets/amazon-2.txt