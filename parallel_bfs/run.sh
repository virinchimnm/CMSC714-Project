WORKDIR=.

echo "Setting up directories .. "
if [ ! -d "$WORKDIR/output" ]; then
  echo "Creating output dir"
  mkdir "$WORKDIR/output"
fi

root=0
ip_f_name="sample_input.txt"
op_f_prefix="s"
inp="input/"$ip_f_name

opser="output/"$op_f_prefix"ser.txt"
opmpi="output/"$op_f_prefix"mpi.txt" 

echo "Running Serial BFS :"
./bin/serial.out -r $root -o "$opser" < $inp 

echo "Running MPI BFS :"
mpirun -n 4 bin/mpibfs.out -i "$inp" -o "$opmpi"

echo "Comparing outputs"
./compare.sh $opser $opmpi

