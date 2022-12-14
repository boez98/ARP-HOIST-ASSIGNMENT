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


#define SIZE 80


//  minimum and maximum z position
const float z_min = 0.0;
const float z_max = 10.0;

//the amount of movement made
float movement_distance = 0.1;  

// position and velocity
float z_pos = 0.0;
int vz = 0;

// change position and counter for Inactivity
int pos_changed = 0;
int counter = 0;

char buffer[SIZE];

int log_file;
int fd_vz, fdz_pos;

// Signal handlers
void reset_handler(int sig);
void stop_handler(int sig);




// Stop signal handler
void stop_handler(int sig)
{
    if (sig == SIGUSR1)
    {
        
        // Log file signal received
        sprintf(buffer, "STOP z received \n");
        write(log_file, buffer, SIZE);
        
        // Stop the motor
        vz = 0;

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
        sprintf(buffer, "RESET z received \n");
        write(log_file, buffer, SIZE);
        

       // Setting velocity to -5
        vz = -5;

        
    }
}




int main(int argc, char const *argv[])
{
    // Open the log file
    log_file = open("log/log.txt", O_WRONLY | O_APPEND | O_CREAT, 0666);
    
    // FIFO path
    char *vz_fifo = "/tmp/vz_fifo";
    char *z_pos_fifo = "/tmp/z_pos_fifo";
    
    // Create FIFO
    mkfifo(vz_fifo, 0666);
    mkfifo(z_pos_fifo, 0666);

    // Open the FIFO
    fd_vz = open(vz_fifo, O_RDWR);
    fdz_pos = open(z_pos_fifo, O_WRONLY);

    // signals
    signal(SIGUSR1, stop_handler);
    signal(SIGUSR2, reset_handler);


    // Random error
    srand((unsigned int)time(NULL));
    float a = movement_distance * 0.05; // 5% error
    float random_error = ((float)rand()/(float)(RAND_MAX))*a;
    
    
    // Loop set to true
    while (1)
    {

        // Set the file descriptors 
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(fd_vz, &readfds);

        // Set the timeout
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 300000;

        // Wait for the file descriptor to be ready
        int ready = select(fd_vz + 1, &readfds, NULL, NULL, &timeout);

        
        if (ready > 0)
        {
            // reset position
            pos_changed = 0;
            
            // Read the velocity increment
            char buf[2];
            read(fd_vz, buf, 2);

            int vz_increment = atoi(buf);

            // Stop motor
            if (vz_increment == 0 && vz != 0)
            {
                vz = 0;

                // Log button pressed
                sprintf(buffer, "Motor z: stopped speed \n");
                write(log_file, buffer, SIZE);
            }

            // Increasing motor
	    else if (vz_increment == 1 && z_pos < z_max)
            {
                vz_increment = 1;
		 
                // Log button pressed
                sprintf(buffer, "Motor z: increase speed \n");
                write(log_file, buffer, SIZE);
            }

	    // Decreasing motor
            else if (vz_increment == 2 && z_pos > z_min)
            {
                vz_increment = -1;

                // Log button pressed
                sprintf(buffer, "Motor z: decrease speed \n");
                write(log_file, buffer, SIZE);
            }

	// Final vz
	vz = vz + vz_increment;
            
	 
        }

   // Update the position
        float pos_increment = vz * (movement_distance+random_error);
        float new_z_pos = z_pos + pos_increment;

        // Check if the position is out of bounds
        if (new_z_pos < z_min)
        {
            new_z_pos = z_min;

            // Stop the motor
            vz = 0;
        }
        else if (new_z_pos > z_max)
        {
            new_z_pos = z_max;

            // Stop the motor
            vz = 0;
        }

        // Check if the position has changed
        if (new_z_pos != z_pos)
        {
            // Update the position
            z_pos = new_z_pos;
            
            // Send position z 
            char z_pos_str[50];
            sprintf(z_pos_str, "%f", z_pos);
            
            write(fdz_pos, z_pos_str, strlen(z_pos_str));

  	    // Write z position
	    sprintf(z_pos_str, "Position z: %f \n", z_pos);
            write(log_file, z_pos_str, strlen(z_pos_str));
            
            pos_changed = 1;
        }

        // If position doesn't change for 10 sec, reset motor x
        if (!pos_changed || z_pos==z_max)
        {
            
	    counter= counter+1;
	    
	    if (counter == 50) {
            
                 vz = -5;
                 
                 // reset counter
                 counter = 0;
                 
                // Log Inactivity
                sprintf(buffer, "Inactivity motor z: Resetting\n");
                write(log_file, buffer, SIZE);
            }
        }
   
    }

    // Close the FIFO
    close(fd_vz);
    close(fdz_pos);

    
    
    // Close the log file
    close(log_file);

    

    exit(0);
}
