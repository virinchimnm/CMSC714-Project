#!/bin/tcsh
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=1
#SBATCH -t 00:00:15
#SBATCH --mem-per-cpu=2048
#SBATCH --exclusive
#SBATCH --mail-user=somay@umd.edu
#SBATCH --mail-type=END
#SBATCH --output=output_2_16.txt
#SBATCH -p debug

module unload intel
module load openmpi/gnu

#echo "Hostnames - "
#/bin/hostname

./build++.sh

mpirun  bin/mpibfs.out -i /lustre/cmsc714-1buw/datasets/amazon-2.txt -o /lustre/cmsc714-1buw/datasets/output/mpi_2_16.txt

mpirun  bin/ompbfs.out -i /lustre/cmsc714-1buw/datasets/amazon-2.txt -o /lustre/cmsc714-1buw/datasets/output/openmp_2_16.txt