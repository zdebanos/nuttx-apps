/****************************************************************************
 * apps/examples/nxboot-updater/updater_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <nuttx/mtd/mtd.h>
#include <nxboot.h>

#define SERVER_PORT 1234

static void updater(int connection)
{
  char buffer[256];
  uint32_t confirm = 0x534f584e;

  int fd = nxboot_open_update_partition();
  if (fd < 0)
    {
      fprintf(stderr, "Failed open update partition: %s\n", strerror(errno));
      return;
    }

  while (true)
    {
      memset(buffer, 0, sizeof buffer);
      ssize_t n = read(connection, buffer, sizeof buffer);
      if (n < 0)
        {
          fprintf(stderr, "Failed to read a socket: %s\n", strerror(errno));
          break;
        }

      if (n == 0)
        {
          printf("Finished...\n");
          break;
        }

      int i = write(fd, buffer, n);
      if (i != n)
        {
          fprintf(stderr, "Could not write update: %s\n", strerror(errno));
          break;
        }

      n = write(connection, &confirm, sizeof confirm);
      if (n != sizeof confirm)
        {
          fprintf(stderr, "Failed to write a socket: %s\n", strerror(errno));
          break;  
        }
    }

  close(fd);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * hello_main
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  int fd;
  int connection;
  socklen_t len; 
  struct sockaddr_in address = {0};

  fd = socket(AF_INET, SOCK_STREAM, 0); 
  if (fd == -1)
    { 
      fprintf(stderr, "Failed to create a socket: %s\n", strerror(errno));
      return -1;
    } 

  address.sin_family = AF_INET; 
  address.sin_addr.s_addr = htonl(INADDR_ANY); 
  address.sin_port = htons(SERVER_PORT); 

  if ((bind(fd, (const struct sockaddr *)&address, sizeof address)) != 0)
    { 
      fprintf(stderr, "Failed to bind a socket: %s\n", strerror(errno)); 
      close(fd);
      return -1;
    } 

  printf("Listen to socket...\n");

  if ((listen(fd, 1)) != 0)
    { 
      fprintf(stderr, "Failed to listen a socket: %s\n", strerror(errno));
      close(fd);
      return -1;
    } 

  len = sizeof address; 

  connection = accept(fd, (struct sockaddr *)&address, &len); 
  if (connection < 0) { 
      fprintf(stderr, "Failed to listen a socket: %s\n", strerror(errno));
      close(fd);
      return -1;
  } 

  updater(connection);

  close(fd); 
  return 0;
}
