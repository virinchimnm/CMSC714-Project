
echo "Buiding serial code"
g++ serial_cc.cpp -o ./serial_cc  -std=c++0x 

echo "Buiding parallel code"
mpic++ parallel_cc.cpp -o ./parallel_cc  -std=c++0x 