#include "./../include/inspection_utilities.h"
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>


#define SIZE 80


char buffer[SIZE];

int log_file;


int main(int argc, char const *argv[])
{
    // Mx and Mz process id
    pid_t pid_mx = atoi(argv[1]);
    pid_t pid_mz = atoi(argv[2]);

    // Open log file
    log_file = open("log/log.txt", O_WRONLY | O_APPEND | O_CREAT, 0666);

    // FIFO path
    char *x_pos_fifo = "/tmp/x_pos_fifo";
    char *z_pos_fifo = "/tmp/z_pos_fifo";

    // Create the FIFO
    mkfifo(x_pos_fifo, 0666);
    mkfifo(z_pos_fifo, 0666);

    int fdx_pos;
    int fdz_pos;
    
    // Open the FIFO
    fdx_pos = open(x_pos_fifo, O_RDWR);
    fdz_pos = open(z_pos_fifo, O_RDWR);


    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // End-effector coordinates
    float ee_x = 0.0;
    float ee_y = 0.0;

    // Initialize User Interface
    init_console_ui();

    // Infinite loop
    while (TRUE)
    {
        // Get mouse/resize commands in non-blocking mode...
        int cmd = getch();

        // If user resizes screen, re-draw UI
        if (cmd == KEY_RESIZE)
        {
            if (first_resize)
            {
                first_resize = FALSE;
            }
            else
            {
                reset_console_ui();
            }
        }
        // Else if mouse has been pressed
        else if (cmd == KEY_MOUSE)
        {

            // Check which button has been pressed...
            if (getmouse(&event) == OK)
            {

                // STOP button pressed
                if (check_button_pressed(stp_button, &event))
                {
                    // Send stop signal to mx and mz
                    kill(pid_mx, SIGUSR1);
                    kill(pid_mz, SIGUSR1);

                    // Log file button pressed
                    sprintf(buffer, "Button: EMERGENCY STOP \n");
                    write(log_file, buffer, SIZE); 
                }

                // RESET button pressed
                else if (check_button_pressed(rst_button, &event))
                {
                    // Send reset signal to mx and mz
                    kill(pid_mx, SIGUSR2); 
		    kill(pid_mz, SIGUSR2);

                    // Log file button pressed
                    sprintf(buffer, "Button: RESET \n");
                    write(log_file, buffer, SIZE); 
                }
            }
	}

	
        char x_pos_ee[50];
        char z_pos_ee[50];

        // Setting parameters 
        fd_set readfds;
        struct timeval timeout;
        FD_ZERO(&readfds);
        
        FD_SET(fdx_pos, &readfds);
        FD_SET(fdz_pos, &readfds);
        
	int max_fd = fmax(fdx_pos, fdz_pos) + 1;

        // Setting timeout 
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000;

        // Wait for the file descriptor to be ready
        int ready = select(max_fd, &readfds, NULL, NULL, &timeout);

        
        if (ready > 0)
        {
        	// If x position is ready change it
        	if (FD_ISSET(fdx_pos,&readfds))
        	{
        			
            		// Read and Store the position
 	    		read(fdx_pos, x_pos_ee, 20);
	    		sscanf(x_pos_ee, "%f", &ee_x);
	    	}
	    	
	    	// If z position is ready change it
	    	else if (FD_ISSET(fdz_pos,&readfds))
        	{
	    	
	    		// Read and Store the real position
	    		read(fdz_pos, z_pos_ee, 20);
	    		sscanf(z_pos_ee, "%f", &ee_y);
         	}
        }

        
        // Update UI
        update_console_ui(&ee_x, &ee_y);
    
}
    // Close the FIFO
    close(fdx_pos);
    close(fdz_pos);

    // Terminate
    endwin();

    // Close log file
    close(log_file);

    
    exit(0);
}
