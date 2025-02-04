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
#include <nuttx/timers/pwm.h>
#include <arch/board/board.h>
#include <nuttx/ioexpander/gpio.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "samocon_include.h"

void print_help(void)
{
  printf("Usage: samocon [choice]\n");
  printf("Choice = afec0, rstest, canbus, pwm0, pwm1, hall, commutate, i2c_eeprom, calibrate_adcs\n");
}

int main(int argc, FAR char *argv[])
{
  if (argc < 2)
    {
      print_help();
      return 0;
    }
  if (strcmp(argv[1], "afec0") == 0)
    {
      return afec0_example();
    }
  if (strcmp(argv[1], "rstest") == 0) 
    {
      return rstest_example();
    }
  if (strcmp(argv[1], "canbus") == 0)
    {
      return canbus_example();
    }
  else if (strcmp(argv[1], "pwm0") == 0)
    {
      return pwm0_example();
    }
  else if (strcmp(argv[1], "pwm1") == 0)
    {
      return pwm1_example();
    }
  else if (strcmp(argv[1], "hall") == 0)
    {
      return hall_example();
    }
  else if (strcmp(argv[1], "commutate") == 0)
    {
      return commutate_b_example();
    }  
  else if (strcmp(argv[1], "i2c_eeprom") == 0)
    {
      return i2c_eeprom_example();
    }
  else if (strcmp(argv[1], "timerhook") == 0)
    {
      return timerhook_example();
    }
  else if (strcmp(argv[1], "calibrate_adcs") == 0)
    {
      if (argc != 3) 
        {
          printf("Usage: samocon calibrate_adcs [0/1]\n");
          return 1;
        }
      if (argv[2][0] == '0')
        {
          return calibrate_adcs(0);
        }
      else if (argv[2][0] == '1')
        {
          return calibrate_adcs(1);
        }
      else
        {
          printf("Wrong adc channel!\n");
          return 1;
        }
    }
  else if (strcmp(argv[1], "ctrl_smoke_test") == 0)
    {
      pthread_t thrd;
      signal(SIGINT,rt_task_endme);
      signal(SIGKILL,rt_task_endme);
      printf("Thread rt_task created!\n");
      pthread_create(&thrd,NULL,rt_task,NULL);
      pthread_join(thrd,NULL);
    }
  return 0;
}
