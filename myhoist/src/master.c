#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


#define SIZE 80


// Variables to store the PID
pid_t pid_cmd;
pid_t pid_mx;
pid_t pid_mz;
pid_t pid_insp;

int log_file;
int status;

char buffer[SIZE];


// Function to fork and create a child process
int spawn(const char *program, char *arg_list[])
{

  pid_t child_pid = fork();

  // If fork() returns a negative value, the fork failed.
  if (child_pid < 0)
  {
    return -1;
  }

  // If fork() returns a positive value, we are in the parent process.
  else if (child_pid != 0)
  {
    return child_pid;
  }

  // If fork() returns 0, we are in the child process.
  else
  {
    if (execvp(program, arg_list) == 0)
      ;

    // If execvp() returns, it must have failed.
    return -1;
  }
}



// Function to kill all the child processes
void kill_all()
{
  kill(pid_cmd, SIGKILL);
  kill(pid_mx, SIGKILL);
  kill(pid_mz, SIGKILL);
  kill(pid_insp, SIGKILL);
 
}



int main()
{

  // Open the log file and remove the old one
  remove("log/log.txt");
  log_file = open("log/log.txt", O_WRONLY | O_APPEND | O_CREAT, 0666);
  
  
  // Log master process started 
  sprintf(buffer, "Hoist started \n");
  write(log_file, buffer, SIZE);
 
  
  // path command, motor x & z
  char *arg_list_command[] = {"/usr/bin/konsole", "-e", "./bin/command", NULL};
  char *arg_list_mx[] = {"./bin/mx", NULL}; 
  char *arg_list_mz[] = {"./bin/mz",NULL};
  
  // Spawn command, motor x & z
  pid_cmd = spawn("/usr/bin/konsole", arg_list_command); 
  pid_mx = spawn("./bin/mx", arg_list_mx);
  pid_mz = spawn("./bin/mz", arg_list_mz);
 
  
  // Convert pid to string
  char pid_mx_str[10];
  sprintf(pid_mx_str, "%d", pid_mx);
  char pid_mz_str[10];
  sprintf(pid_mz_str, "%d", pid_mz);
 
   
  // inspection process + pid motor x&z
  char *arg_list_inspection[] = {"/usr/bin/konsole", "-e", "./bin/inspection", pid_mx_str, pid_mz_str, NULL};
  pid_insp = spawn("/usr/bin/konsole", arg_list_inspection);
  
  
  
 
// Check if the child processes are still alive. If one of them is terminated, it will kill the others

  while (1)
  {
    
    // Check if a child process terminated unexpectedly and kill all the child processes. -1 means any child
    if (waitpid(-1, &status, WNOHANG) > 0)
    {
      
      kill_all();
      
      sprintf(buffer, "Child terminated unexpectedly \n");
      write(log_file, buffer, SIZE);
      
      return -1;
      
    }
    
    // If error, kill all the child processes
    else if (waitpid(-1, &status, WNOHANG) == -1)
    {
      
      kill_all();
      
      sprintf(buffer, "error in any child \n");
      write(log_file, buffer, strlen(buffer));
      
      return -1;
    }

}
  
  
  // Log the end of the program
  sprintf(buffer, "Hoist terminated \n");
  write(log_file, buffer, SIZE);

  
  // Close the log file
  close(log_file);

  return 0;
}
