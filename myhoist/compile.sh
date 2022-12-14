#Create bin directory
mkdir -p bin &

#Create log directory
mkdir -p log &

#Compile inspection 
gcc src/inspection_console.c -lncurses -lm -o bin/inspection &

#Compile command 
gcc src/command_console.c -lncurses -o bin/command &

#Compile master 
gcc src/master.c -o bin/master &

#Compile motors 
gcc src/mx.c -o bin/mx &

gcc src/mz.c -o bin/mz 


