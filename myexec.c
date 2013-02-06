#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>

pid_t myexec (const char *command,bool wait) {
  pid_t pid;
  char *dummy;
  char **com_p;
  size_t i;
  int status;

  pid = fork ();
  if (pid == 0) {
    /* This is the child process*/
    
    /* do some string operations to match syntax of execvp 
       we do not care about freeing allocated memory,
       because we do execvp anyway
    */
    dummy=malloc(strlen(command));
    strcpy(dummy,command);
  
    com_p=malloc(sizeof(char*));

    com_p[0]=strtok(dummy," ");
    i=1;
    while (com_p[i-1]!=NULL) {
      com_p=realloc(com_p,sizeof(char*)*(i+1));
      com_p[i]=strtok(NULL," ");
      i++;
    }

    /*  now execute the command */
    execvp(com_p[0], com_p);
    _exit (EXIT_FAILURE);
  } else if (pid < 0)
    /* The fork failed.  Report failure.  */
    return -1;
  else
    /* This is the parent process */
    if (wait) {
      waitpid(pid,&status, 0);
      return WEXITSTATUS(status);
    } else {
      return pid;
    }
}
