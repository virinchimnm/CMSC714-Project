
echo "Buiding serial code"
# g++ serial_cc.cpp -o ./serial_cc  -std=c++0x 

echo "Buiding parallel code"
# mpic++ parallel_cc.cpp -o ./parallel_cc  -std=c++0x 
mpic++ mpi-clustering-coeff.cpp -o mpi-cc -std=c++0x 

