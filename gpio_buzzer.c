/*

control buzzer connected to gpio of raspberry pi

(c) 2013 Sven Geggus <sven-web20mash@geggus.net>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY;without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.

*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/file.h>

#define GPIO "/sys/class/gpio/gpio18/value"

int gpiofd,run;

void signalHandler() {
  run=0;
}

void die_usage(char *prog) {
  fprintf(stderr,"usage: %s ?duration? ?interval(seconds)?\n",prog);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

  int duration;
  float interval;
  run=1;
  
  if ((argc >3) || (argc <1)) die_usage(argv[0]);    
  
  signal(SIGINT,signalHandler);
  signal(SIGINT,signalHandler);
  signal(SIGALRM,signalHandler);
    
  gpiofd = open(GPIO, O_RDWR);
  if (gpiofd < 0) {
    fprintf(stderr,"unable to open GPIO device >%s<\n in rw mode\n",GPIO);
    exit(EXIT_FAILURE);
  }
  // only on buzzer process per time should use the device
  flock(gpiofd,LOCK_EX);

  // set alarm clock if duration has been requested
  if (argc > 1) {
    if (1!=sscanf(argv[1],"%d",&duration)) die_usage(argv[0]);
    if (duration <0) die_usage(argv[0]);
    alarm(duration);
  }
  
  if (argc == 3) {
    if (1!=sscanf(argv[2],"%f",&interval)) die_usage(argv[0]);
    if (interval <0) die_usage(argv[0]);
  } else {
    interval=100;
  }
  
  while (run) {
    write(gpiofd,"1",1);
    usleep(1000*interval);
    write(gpiofd,"0",1);
    usleep(1000*interval);
  }
  write(gpiofd,"0",1);
  flock(gpiofd,LOCK_UN);
  close(gpiofd);
  
  exit(EXIT_SUCCESS); 
}
