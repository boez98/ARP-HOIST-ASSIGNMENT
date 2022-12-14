#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>


#define SIZE  80


// Constants to store the minimum and maximum x position
const float x_min = 0.0;
const float x_max = 40.0;

//the amount of movement made
float movement_distance = 0.4; 

// position and velocity
float x_pos = 0.0;
int vx = 0;

// change position and counter for Inactivity
int pos_changed = 0;
int counter = 0;


int log_file;
int fd_vx, fdx_pos;

char buffer[SIZE];

// Signal handlers
void reset_handler(int sig);
void stop_handler(int sig);



// Stop signal handler
void stop_handler(int sig)
{
    if (sig == SIGUSR1)
    {
        

        // Log file signal received
        sprintf(buffer, "STOP x received \n");
        write(log_file, buffer, SIZE);

        // Stop the motor
        vx = 0;

        // Set position change to 1 because we don't want Inactivity RESET on EMERGENCY STOP
        pos_changed = 1;
        
    }
}



// Reset signal handler
void reset_handler(int sig)
{
    if (sig == SIGUSR2)
    {
        // Log file signal received
        sprintf(buffer, "RESET x received \n");
        write(log_file, buffer, SIZE);
        
        // Setting velocity to -5
        vx = -5;    
   
    }
}




int main(int argc, char const *argv[])
{
    // Open the log file
    log_file = open("log/log.txt", O_WRONLY | O_APPEND | O_CREAT, 0666);
    
    // path FIFO
    char *vx_fifo = "/tmp/vx_fifo";
    char *x_pos_fifo = "/tmp/x_pos_fifo";
    
    // Create FIFO
    mkfifo(vx_fifo, 0666);
    mkfifo(x_pos_fifo, 0666);

    // Open the FIFO
    fd_vx = open(vx_fifo, O_RDWR);
    fdx_pos = open(x_pos_fifo, O_WRONLY);

    
    // Signal
    signal(SIGUSR1, stop_handler);
    signal(SIGUSR2, reset_handler);
    
    
    // Random error
    srand((unsigned int)time(NULL));
    float a = movement_distance * 0.05; // 5% error
    float random_error = ((float)rand()/(float)(RAND_MAX))*a;
    
    
    // Loop is set to true
    while (1)
    {
    
        // Set the file descriptors 
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(fd_vx, &readfds);

        // Set the timeout
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 300000;

        // Wait for the file descriptor to be ready
        int ready = select(fd_vx + 1, &readfds, NULL, NULL, &timeout);

        
        if (ready > 0)
        {
            // reset position
            pos_changed = 0;
        
            // Read the velocity increment
            char buf[2];
            read(fd_vx, buf, 2); 

            int vx_increment = atoi(buf);

            
	    // Stop motor
            if (vx_increment == 0 && vx != 0)
            {
                vx = 0;

                // Log button pressed
                sprintf(buffer, "Motor x: stop speed \n");
                write(log_file, buffer, SIZE);
            }


	    // Increasing motor
	    else if (vx_increment == 1 && x_pos < x_max)
            {
                vx_increment = 1;
		 
                // Log button pressed
                sprintf(buffer, "Motor x: increase speed \n");
                write(log_file, buffer, SIZE);
                
            }

	    // Decreasing motor
            else if (vx_increment == 2 && x_pos > x_min)
            {
            	
                vx_increment = -1;

                // Log button pressed
                sprintf(buffer, "Motor x: decrease speed \n");
                write(log_file, buffer, SIZE);
            }

	// Final vx
	vx = vx + vx_increment;
            
	 
        }
        

  // Update the position
        float pos_increment = vx * (movement_distance+random_error);
        float new_x_pos = x_pos + pos_increment;

        // Check if the position is out of bounds
        if (new_x_pos < x_min)
        {
            new_x_pos = x_min;

            // Stop the motor
            vx = 0;
        }
        else if (new_x_pos > x_max)
        {
            new_x_pos = x_max;

            // Stop the motor
            vx = 0;
        }

        // Check if the position has changed
        if (new_x_pos != x_pos)
        {
            // Update the position
            x_pos = new_x_pos;
            
            // Send position x
            char x_pos_str[50];
            sprintf(x_pos_str, "%f", x_pos);

            write(fdx_pos, x_pos_str, strlen(x_pos_str));
            
            // Wrize on log file x position  
            sprintf(x_pos_str, "Position x: %f\n", x_pos);
            write(log_file, x_pos_str, strlen(x_pos_str));
            
           pos_changed = 1;
        }

        // If position doesn't change for 10 sec, reset motor x
        if (!pos_changed || x_pos==x_max)
        {
            counter= counter+1;
	    
	    if (counter == 50) {
            
                 vx = -5;
                 
                 // reset counter
                 counter =0;
                 
                // Log Inactivity
                sprintf(buffer, "Inactivity motor x: Resetting \n");
                write(log_file, buffer, SIZE);
        }
        
      }  
    }

    // Close the FIFOs
    close(fd_vx);
    close(fdx_pos);

    // Close the log file
    close(log_file);

    exit(0);
}
