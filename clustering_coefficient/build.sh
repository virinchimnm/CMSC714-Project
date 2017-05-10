
echo "Buiding serial code"
g++ serial-cc-efficient.cpp -o ./serial_cc  -std=c++0x 

echo "Buiding MPI parallel code"
# mpic++ parallel_cc.cpp -o ./parallel_cc  -std=c++0x 
mpic++ mpi-clustering-coeff.cpp -o mpi-cc -std=c++0x 

mpic++ mpi-count-triangles.cpp -o mpi-triangles -std=c++0x 

echo "Buiding MPI/OMP parallel code"
mpic++ -fopenmp omp-clustering-coeff.cpp -o omp-cc -std=c++0x
