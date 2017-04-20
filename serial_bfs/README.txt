cd bin
g++ ../src/main.cpp ../src/graph.h -o out -std=c++11



SAMPLE OUTPUT:
## WITHOUT ARGUMENT - ROOT NODE SET TO 0, NO SAVING OF OUTPUT FILE
./out < ../src/input.txt

parsing input file
..... Done!
n_v = 25 n_e 40
skipping :4 partition info lines
First Edge Info Line:
0 0 2 1 5
Last Edge Info Line:
24 3 2 23 19
.....Graph Initialzed!! 
Graph with 25 created
L[0]: 0 
L[1]: 1 5 
L[2]: 2 6 10 
L[3]: 7 3 11 15 
L[4]: 8 12 4 16 20 
L[5]: 9 13 17 21 
L[6]: 14 18 22 
L[7]: 19 23 
L[8]: 24 

## CHANGE THE ROOT NODE USING -r argument
./out -r 0 < ../src/input.txt 
root set to 0
parsing input file
..... Done!
n_v = 25 n_e 40
skipping :4 partition info lines
First Edge Info Line:
0 0 2 1 5
Last Edge Info Line:
24 3 2 23 19
.....Graph Initialzed!! 
Graph with 25 created
L[0]: 0 
L[1]: 1 5 
L[2]: 2 6 10 
L[3]: 7 3 11 15 
L[4]: 8 12 4 16 20 
L[5]: 9 13 17 21 
L[6]: 14 18 22 
L[7]: 19 23 
L[8]: 24 


## SAVE OUTPUT TO A FILE USING -o argument, giving the relative path
./out -r 1 -o ../out.txt < ../src/input.txt root set to 1
output file path ../out.txt
parsing input file
..... Done!
n_v = 25 n_e 40
skipping :4 partition info lines
First Edge Info Line:
0 0 2 1 5
Last Edge Info Line:
24 3 2 23 19
.....Graph Initialzed!! 
Graph with 25 created
L[0]: 1 
L[1]: 0 2 6 
L[2]: 5 7 3 11 
L[3]: 10 8 12 4 16 
L[4]: 15 9 13 17 21 
L[5]: 20 14 18 22 
L[6]: 19 23 
L[7]: 24 
Output Save to ../out.txt
