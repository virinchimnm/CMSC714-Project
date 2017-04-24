WORKDIR=.

echo "Setting up directories .. "
if [ ! -d "$WORKDIR/output" ]; then
  echo "Creating output dir"
  mkdir "$WORKDIR/output"
fi

root=2
ip_f_name="youtube-4.txt"
op_f_prefix="you4"
#inp="input/"$ip_f_name
inp=/lustre/cmsc714-1buw/datasets/com-youtube.ungraph-4.txt

opser="output/"$op_f_prefix"ser.txt"
opmpi="output/"$op_f_prefix"mpi.txt" 
opomp="output/"$op_f_prefix"omp.txt"

echo "Running Serial BFS :"
./bin/serial.out -r $root -o "$opser" < $inp 

echo "Running MPI BFS :"
mpirun -n 4 bin/mpibfs.out -i "$inp" -o "$opmpi"

echo "Comparing outputs"
./compare.sh $opser $opmpi

echo "Running MPI BFS :"
mpirun -n 4 bin/ompbfs.out -i "$inp" -o "$opomp"

echo "Comparing outputs"
./compare.sh $opser $opomp

