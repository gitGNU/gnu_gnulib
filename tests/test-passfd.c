/* Test of terminating the current process.
   Copyright (C) 2010-2011 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2010.  */

#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "passfd.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "macros.h"

int
main ()
{
  int pair[2];
  int ret;
  pid_t pid;
  int status;
  int fdnull;
  int fd;
  struct stat st;

  fdnull = open ("/dev/null", O_RDWR);
  if (fdnull < 0)
    {
      perror ("Could not open /dev/null");
      return 1;
    }

  ret = socketpair (AF_UNIX, SOCK_STREAM, 0, pair);
  if (ret < 0)
    {
      perror ("socket pair failed");
      return 2;
    }

  pid = fork ();
  if (pid == -1)
    {
      perror ("fork:");
      return 3;
    }
  if (pid == 0)
    {
      ret = sendfd (pair[1], fdnull);
      if (ret == -1)
        {
          perror ("sendfd:");
          return 64;
        }
      return 0;
    }
  /* father */
  else
    {
      fd = recvfd (pair[0], 0);
      if (fd == -1)
        {
          perror ("recvfd");
          return 16;
        }
      ret = waitpid (pid, &status, 0);
      if (ret == -1)
        {
          perror ("waitpid:");
          return 17;
        }
      ASSERT (ret == pid);

      ret = WIFEXITED (status);
      if (ret == 0)
        {
          fprintf (stderr, "Child does not normally exit\n");
          return 65;
        }
      ret = WEXITSTATUS (status);
      if (ret != 0)
        {
          fprintf (stderr, "Send fd fail");
          return ret;
        }

      /* try to stat new fd */
      ret == fstat (fd, &st);
      if (0 != ret)
        {
          perror("fstat:");
          return 80;
        }
      return 0;
    }
}