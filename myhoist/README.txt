# ARP-Hoist-Assignment - Group AA98
Andrea Bolla 4482930 
Anna Possamai 5621738

Base project structure for the first Advanced and Robot Programming (ARP) assignment.
The project provides the basic functionalities for the Command and Inspection processes, both of which are implemented through the ncurses library as simple GUIs. 
In particular, the repository is organized as follows:

- The src folder contains the source code for the Command, Inspection, motor x, motor z and Master processes;
- The include folder contains all the data structures and methods used within the ncurses framework to build the two GUIs. Unless you want to expand the graphical capabilities of the UIs (which requires understanding how ncurses works), you can ignore the content of this folder, as it already provides you with all the necessary functionalities;
- The README file with the instruction about the programm;
- The compile.sh to compile the code
- The run.sh to run the code

after compile there will be also:
- The bin folder is where the executable files are expected to be after compilation;
-the log folder containing the log.txt file in which is written informations about what the hoist is doing.


#Introduction
The first assignment consists in creating a simulation of a hoist in C language. 
The hoist can be controlled by a command console in which there are 6 buttons:

-Vx++ and Vx-- to increase and decrease the speed along the horizontal axis;
-Vz++ and Vz-- to increase and decrease the speed along the vertical axis;
-two STP buttons to stop the velocity along both axis;

and by a inspection console in which there are 2 buttons:

-S the emergency stop;
-R the reset button. 

The position of the hoist is shown by the inspection console.


# Structure
The program is composed of 5 processes:

1. master.c is the first process to be executed and it spawns all the other processes and check if they are still working. In case one of them terminates unexpectedly or there is an error the master process will kill all the processes.

2. command_console.c creates a console capable of reading the user inputs by clicking on the buttons, in order to move the hoist. After that, the program send commands to the two motors.

3. inspection_console.c creates a console with two buttons, the stop and reset buttons, they both send signals to the two motors to do an emergency stop or go back to the initial position. It also gets the position from the two motors and displays the hoist position and the coordinates in real time, using ncurses GUI. 

4. mx.c process manages the motor x that makes the hoist move in the horizontal way. It's controlled by the commands sent from the command and inspection console.
It computes the new x position with random error and send it to the ispection console. 
The programm can manage the stop, increasing and decreasing velocity, the RESET and EMERGENCY STOP signal. It can reset the x position if the motor is inactive for 10 seconds but not in the case of an EMERGENCY STOP.

5. mz.c does the same thing as mx.c but on the z motor that makes the hoist move vertical.


# konsole and ncurses installation
To install the ncurses library, simply open a terminal and type the following command:

	sudo apt-get install libncurses-dev

for the konsole type:

	sudo apt-get install konsole


# Compiling and running the code
The Command and Inspection processes depend on the ncurses library, which needs to be linked during the compilation step. Furthermore, the Inspection process also uses the mathematical library for some additional computation. 
In the folder are attached the compile and the running file. Firstly you have to compile the files by writing on the konsole:

	
	./compile.sh
	
	
After compiling, you can easily run the programm by typing:

	
	./run.sh


which will be responsible of running the program and spawning the two GUIs.


# Troubleshooting
Should you experience some weird behavior after launching the application (buttons not spawning inside the GUI or graphical assets misaligned) simply try to resize the terminal window, it should solve the bug.


# Problems and Improvements
The programm works but I noticed three problems:

1. The log file sometimes has a strange behavior, it can't read some information. I don't know if it's a code problem or my computer fault.

2. The inactivity reset doesn't respect the rules given by the teacher. 
My program restars each motor individually if it is inactive for 10 seconds. 
A possible improvement is to create another process with a countdown timer that restarts itself if it receive a signal from the other processes in the case of position change or a button is clicked. If it doesn't received signal and the time reach 0, it send a SIGUSR2 signal to the two motors, in this case the motor reset the hoist position. This was my idea but I wasn't able to do it.

3. The last problem is on the Reset button. My idea by resetting the hoist was to reach the initial coordinates (0,0) without the interferenze of the command console buttons, only the emergency stop has to work. In my programm during the reset you can modify the speed using the command console. I wasn't able to fix this problem