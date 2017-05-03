echo "Diffing $1 $2"
sort $1 > temp1
sort $2 > temp2
vimdiff temp1 temp2