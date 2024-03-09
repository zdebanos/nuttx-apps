/****************************************************************************
 * apps/examples/hello/hello_main.c
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

#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>

#include <nuttx/config.h>
#include <nuttx/analog/adc.h>
#include <nuttx/analog/ioctl.h>


void print_help(void)
{
  printf("Usage: samocon [choice]\n");
  printf("Choice = \"afec0\", pwm");
}

static const char *afec0_channel_to_name(uint8_t ch)
{
  switch (ch)
  {
    case 0:
      return "HSS-1";
      break;
    case 3:
      return "LSS-1";
      break;
    case 4:
      return "HSS-3";
      break;
    case 6:
      return "LSS-3";
      break;
    case 7:
      return "LSS-2";
      break;
    case 8:
      return "HSS-2";
      break;
    case 9:
      return "LSS-0";
      break;
    case 10:
      return "HSS-0";
      break;
    default:
      return "wtf";
      break;
  }
}

static void afec0_example(void)
{
  const ssize_t channels = 8;
  struct adc_msg_s sample[channels];
  int readsize = channels * sizeof(struct adc_msg_s);
  printf("Trying to open ADC drivers!!\n");

  int fd = open("/dev/adc0", O_RDONLY);
  if (fd < 0)
    {
      printf("Error opening adc0 peripheral, ret = %d\n", fd);
    } 
  else
    {
      printf("Opening adc0 succesful\n");
    }

  int ret;
  ret = ioctl(fd, ANIOC_RESET_FIFO, 0);
  for (;;)
    {
      ret = ioctl(fd, ANIOC_TRIGGER, 0);
      if (ret < 0)
        {
          printf("ioctl fail\n");
        }
      int nbytes = read(fd, sample, readsize);
      if (nbytes == readsize)
        {
          for (int j = 0; j < channels; ++j)
            {
              printf("%s: %d\n", afec0_channel_to_name(sample[j].am_channel),
                      sample[j].am_data);
            }
          putchar('\n');
        }
      else
        {
          ioctl(fd, ANIOC_RESET_FIFO, 0);
        }
      usleep(1000*100);
    }
}

static void pwm0_example(void)
{

}

int main(int argc, FAR char *argv[])
{
  if (argc != 2)
    {
      print_help();
      return 0;
    }
  if (strcmp(argv[1], "afec0") == 0)
    {
      afec0_example();
    }
  else if (strcmp(argv[1], "pwm") == 0)
    {
      pwm0_example();
    }
  return 0;
}
