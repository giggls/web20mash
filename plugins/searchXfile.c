/*

searchXfile function(c) 2013 Sven Geggus

Unfortunately posix or even glibc do not provide something like this!

Try to find out if given string points to an executable binary
imitating the behaviour of execpc as closely as possible.

For this purpose, parts of the code are shamelessly stolen from glibc ;)

So this is also:
Copyright (C) 1991,92, 1995-99, 2002, 2004, 2005, 2007, 2009
Free Software Foundation, Inc.

This code is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with the GNU C Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA. 

*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int tryXaccess(char *file) {
  int res;
  res=access(file,X_OK);
  if(res < 0) {
    return 1;
  }
  // check if file is readable
  res=access(file,R_OK);
  if(res < 0)
    fprintf(stderr,
            "WARNING: file %s is unreadable you may not be able to execute it\n",
            file);
  return 0;
}

int searchXfile(char *file, char **executable) {
  if (strchr (file, '/') != NULL) {
    /* Check right away if executable is there, when it contains a slash.  */
    // check if file is executable
    if (0==tryXaccess(file)) {
      *executable=malloc(strlen(file)+1);
      strcpy(*executable,file);
    }
  } else {
    /* if there is no path delimiter is given search in PATH environment */
    size_t pathlen;
    size_t alloclen = 0;

    char *path = getenv ("PATH");
    if (path == NULL) {
      pathlen = confstr (_CS_PATH, (char *) NULL, 0);
      alloclen = pathlen + 1;
    } else {
      pathlen = strlen (path);
    }

    size_t len = strlen (file) + 1;
    alloclen += pathlen + len + 1;
    char *name;

    char *path_malloc = NULL;
    path_malloc = name = malloc(alloclen);
    if (name == NULL)
      return 1;
        
    if (path == NULL) {
      /* There is no `PATH' in the environment.
             The default search path is the current directory
             followed by the path `confstr' returns for `_CS_PATH'.  */
      path = name + pathlen + len + 1;
      path[0] = ':';
      (void) confstr (_CS_PATH, path + 1, pathlen);
    }

    /* Copy the file name at the top.  */
    name = (char *) memcpy (name + pathlen + 1, file, len);
    /* And add the slash.  */
    *--name = '/';

    char *p = path;
    do {
      char *startp;
      path = p;
      p = strchrnul(path, ':');
      if (p == path)
        /* Two adjacent colons, or a colon at the beginning or the end
           of `PATH' means to search the current directory.  */
        startp = name + 1;
      else
        startp = (char *) memcpy (name - (p - path), path, p - path);
      if (0==tryXaccess(startp)) {
        *executable=malloc(strlen(startp)+1);
        strcpy(*executable,startp);
        free(path_malloc);
        return 0;
      }
    } while (*p++ != '\0');
    free(path_malloc);
    return 1;
  }
  return 0;
}
