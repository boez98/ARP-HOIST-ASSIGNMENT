#include "./../include/command_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>


#define SIZE 80

char buffer[SIZE];
char v_buf[2];

int log_file;



int main(int argc, char const *argv[])
{
    // Open the log file
    log_file = open("log/log.txt", O_WRONLY | O_APPEND | O_CREAT, 0666);

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize User Interface
    init_console_ui();

    // path
    char *vx_fifo = "/tmp/vx_fifo";
    char *vz_fifo = "/tmp/vz_fifo";
    
    // create FIFO
    mkfifo(vx_fifo, 0666);
    mkfifo(vz_fifo, 0666);

    int fd_vx, fd_vz;

    // Open the FIFO
    fd_vx = open(vx_fifo, O_WRONLY);
    fd_vz = open(vz_fifo, O_WRONLY);


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

                // Vx++ button pressed
                if (check_button_pressed(vx_incr_btn, &event))
                {
                    // Log button pressed
                    sprintf(buffer, "Button: Vx++ pressed \n");
                    write(log_file, buffer, SIZE); 
                    
                    // Send vx to motor x
    		    sprintf(v_buf, "%d", 1);
    		    write(fd_vx, v_buf, strlen(v_buf));
    		    
                }

                // Vx-- button pressed
                else if (check_button_pressed(vx_decr_btn, &event))
                {
                    // Log button pressed
                    sprintf(buffer, "Button: Vx-- pressed \n");
                    write(log_file, buffer, SIZE); 

                    // Send vx to motor x
    		    sprintf(v_buf, "%d", 2);
    		    write(fd_vx, v_buf, strlen(v_buf));
                }

                // Vx stop button pressed
                else if (check_button_pressed(vx_stp_button, &event))
                {
                    // Log button pressed
                    sprintf(buffer, "Button: Vx stop pressed \n");
                    write(log_file, buffer, SIZE); 

                    // Send vx to motor x
    		    sprintf(v_buf, "%d", 0);
    		    write(fd_vx, v_buf, strlen(v_buf));
                }

                // Vz++ button pressed
                else if (check_button_pressed(vz_incr_btn, &event))
                {
                    // Log button pressed
                    sprintf(buffer, "Button: Vz++ pressed \n");
                    write(log_file, buffer, SIZE); 
                   
                    // Send vz to motor z
    		    sprintf(v_buf, "%d", 1);
    		    write(fd_vz, v_buf, strlen(v_buf));
                }

                // Vz-- button pressed
                else if (check_button_pressed(vz_decr_btn, &event))
                {
                    // Log button pressed
                    sprintf(buffer, "Button: Vz-- pressed \n");
                    write(log_file, buffer, SIZE); 
                    

                    // Send vz to motor z
    		    sprintf(v_buf, "%d", 2);
    		    write(fd_vz, v_buf, strlen(v_buf));
                    
                }

                // Vz stop button pressed
                else if (check_button_pressed(vz_stp_button, &event))
                {
                    // Log button pressed
                    sprintf(buffer, "Button: Vz stop pressed \n");
                    write(log_file, buffer, SIZE); 
                    
                    // Send vz to motor z
    		    sprintf(v_buf, "%d", 0);
    		    write(fd_vz, v_buf, strlen(v_buf));
                }
            }
        }
        refresh();
    }

    // Close the FIFOs
    close(fd_vx);
    close(fd_vz);

    // Terminate
    endwin();

    
    // Close the log file
    close(log_file);

    exit(0);
}
