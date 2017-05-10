
echo "Buiding serial code"
g++ serialSomay.cpp -o ./serial_cc  -std=c++0x 

echo "Buiding parallel code"
# mpic++ parallel_cc.cpp -o ./parallel_cc  -std=c++0x 
mpic++ mpi-clustering-coeff.cpp -o mpi-cc -std=c++0x 

mpic++ mpi-count-triangles.cpp -o mpi-triangles -std=c++0x 

mpic++ -fopenmp omp-clustering-coeff.cpp -o omp-cc -std=c++0x
