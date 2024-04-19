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

#include <limits.h>
#include <stdint.h>
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


void print_help(void)
{
  printf("Usage: samocon [choice]\n");
  printf("Choice = afec0, pwm0, pwm1, hall, commutate, i2c_eeprom, calibrate_adcs\n");
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

static int afec0_example(void)
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
              printf("%s: %ld\n", afec0_channel_to_name(sample[j].am_channel),
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
  return OK;
}

static int pwm0_example(void)
{
  struct pwm_info_s info;

  int fd = open("/dev/pwm0", O_RDONLY);
  if (fd < 0)
    {
      printf("Error opening pwm0!\n");
      return ERROR;
    }
  printf("PWM0 open ok!\n");
  for (int i = 0; i < 4; ++i)
    {
      info.channels[i].channel = (i+1);
      info.channels[i].duty = 10000*(i+1);
      info.channels[i].cpol = PWM_CPOL_HIGH;
      info.frequency = 20000;
    }
  int ret = ioctl(fd, PWMIOC_SETCHARACTERISTICS, (unsigned long)((uintptr_t) &info));
  if (ret < 0)
    {
      printf("Ioctl of pwm0 failed!\n");
    }
  ret = ioctl(fd, PWMIOC_START, (unsigned long)((uintptr_t) &info));
  if (ret < 0)
    {
      printf("Ioctl start of pwm0 failed!\n");
    }
  printf("Set duty ok!\n");
  while(true){}
      usleep(1000*1000);
      struct pwm_info_s got_info;
      ret = ioctl(fd, PWMIOC_GETCHARACTERISTICS, (unsigned long)((uintptr_t) &got_info));
      if (ret < 0)
        {
          printf("fail get ioctl\n");
        }
      printf("Got freq = %lu\n", got_info.frequency);
      for(int i = 0; i < 4; ++i)
        {
          printf("Ch%d duty = %lu\n", info.channels[i].channel, info.channels[i].duty);
        }
  return OK;
}

static int pwm0_simple_example(void)
{
  struct pwm_info_s info;
  int fd = open("/dev/pwm0", O_RDONLY);
  int fd0 = open("/dev/pwmb_inh0", O_RDWR);
  int fd1 = open("/dev/pwmb_inh1", O_RDWR);
  int fd2 = open("/dev/pwmb_inh2", O_RDWR);
  int fd3 = open("/dev/pwmb_inh3", O_RDWR);

  ioctl(fd0, GPIOC_WRITE, (unsigned long) 1);
  ioctl(fd1, GPIOC_WRITE, (unsigned long) 1);
  ioctl(fd2, GPIOC_WRITE, (unsigned long) 1);
  ioctl(fd3, GPIOC_WRITE, (unsigned long) 1);


  if (fd < 0)
    {
      printf("error opening pwm!\n");
      return ERROR;
    }
  for (int i = 0; i < 4; ++i)
    {
      info.channels[i].channel = (i+1);
      info.channels[i].duty = 32000;
      info.channels[i].cpol = PWM_CPOL_HIGH;
      info.channels[i].dcpol = PWM_DCPOL_LOW;
      info.frequency = 2000;
    }
  int ret = ioctl(fd, PWMIOC_SETCHARACTERISTICS, (unsigned long)((uintptr_t) &info));
  if (ret < 0)
    {
      printf("Ioctl of pwm0 failed!\n");
    }
  ret = ioctl(fd, PWMIOC_START, (unsigned long)((uintptr_t) &info));
  if (ret < 0)
    {
      printf("Ioctl start of pwm0 failed!\n");
    }
  printf("Set duty OK!\n"); 
  printf("Press anything to turn off the pwm!\n");
  getchar();
  ret = ioctl(fd, PWMIOC_STOP, (unsigned long)((uintptr_t) &info));
  if (ret < 0)
    {
      printf("Ioctl start of pwm0 failed!\n");
    }
  printf("Check the PWM! Press anything to turn off the app!\n");
  getchar();
  close(fd);
  return OK;
}

static int pwm1_example(void)
{
  struct pwm_info_s info;
  int fd = open("/dev/pwm1", O_RDONLY);
  (void)info;
  (void)fd;
  return OK;
}

struct hall_device_s
{
  int fd;
  const char *devpath;
  int value;
};

static struct hall_device_s hall_devices_a[3], hall_devices_b[3];

static int hall_example(void)
{
  hall_devices_a[0].devpath = "/dev/halla_in0";
  hall_devices_a[1].devpath = "/dev/halla_in1";
  hall_devices_a[2].devpath = "/dev/halla_in2";

  hall_devices_b[0].devpath = "/dev/hallb_in0";
  hall_devices_b[1].devpath = "/dev/hallb_in1";
  hall_devices_b[2].devpath = "/dev/hallb_in2";
  /* Firstly open all halls */

  for (int i = 0; i < 3; ++i)
    {
      hall_devices_a[i].fd = open(hall_devices_a[i].devpath, O_RDONLY);
      if (hall_devices_a[i].fd < 0)
        {
          printf("Error opening A hall n=%d\n", i);
          return ERROR;
        }
    }

  for (int i = 0; i < 3; ++i)
    {
      hall_devices_b[i].fd = open(hall_devices_b[i].devpath, O_RDONLY);
      if (hall_devices_b[i].fd < 0)
        {
          printf("Error opening B hall n=%d\n", i);
          return ERROR;
        }
    }

  for (;;)
    {
      bool invalue;
      for (int i = 0; i < 3; ++i)
      {
        int ret = ioctl(hall_devices_a[i].fd, GPIOC_READ, (unsigned long)((uintptr_t)&invalue));
        if (ret < 0)
          {
            printf("ERR!\n");
            continue;
          }
        printf("HallA %d: %s\n", i, invalue ? "ON" : "OFF");
      }
      putchar('\n');
      for (int i = 0; i < 3; ++i)
      {
        int ret = ioctl(hall_devices_b[i].fd, GPIOC_READ, (unsigned long)((uintptr_t)&invalue));
        if (ret < 0)
          {
            printf("ERR!\n");
            continue;
          }
        printf("HallB %d: %s\n", i, invalue ? "ON" : "OFF");
      }
      putchar('\n');
      usleep(1000*100);
    }
  return OK;
}

static int hall_2_section(bool h0, bool h1, bool h2)
{
  int n = 0;
  n |= h0 ? (1 << 0) : 0;
  n |= h1 ? (1 << 1) : 0;
  n |= h2 ? (1 << 2) : 0;
  switch (n)
    {
    case 1:
      return 1;
    case 2: 
      return 3;
    case 3:
      return 2;
    case 4:
      return 5;
    case 5:
      return 0;
    case 6:
      return 4;
    default:
      return -1;
    }
  return -1;
}

#define DUTY_ON  (ub16_t) 15000
#define DUTY_OFF (ub16_t) 0

/*
 *
 * A->C
 * B->C
 * B->A
 * C->A
 * C->B
 * A->B
 */
static const char *commutate_bldc(
  int section,
  struct pwm_info_s *pwm,
  int fd_pwm,
  int fd_inh0,
  int fd_inh1,
  int fd_inh2,
  int fd_inh3,
  int fd_hall0,
  int fd_hall1,
  int fd_hall2)
{
  if (section < 0 || section >= 6)
    {
      printf("Wrong commutate input!\n");
      return NULL;
    }
  if (pwm == NULL || fd_inh0 < 0 || fd_inh1 < 0 || fd_inh2 < 0 || fd_inh3 < 0)
    {
      printf("Wrong file descriptor(s)!\n");
      return NULL;
    }
  
  const char *coils;
  int ret;
  unsigned long inh0, inh1, inh2;
  switch (section)
    {
    case 0:
      pwm->channels[0].duty = DUTY_ON;
      pwm->channels[1].duty = DUTY_OFF;
      pwm->channels[2].duty = DUTY_OFF;
      inh0 = 1;
      inh1 = 0;
      inh2 = 1;
      coils = "A->C";
      break;
    case 1:
      pwm->channels[0].duty = DUTY_OFF;
      pwm->channels[1].duty = DUTY_ON;
      pwm->channels[2].duty = DUTY_OFF;
      inh0 = 0;
      inh1 = 1;
      inh2 = 1;
      coils = "B->C";
      break;
    case 2:
      pwm->channels[0].duty = DUTY_OFF;
      pwm->channels[1].duty = DUTY_ON;
      pwm->channels[2].duty = DUTY_OFF;
      inh0 = 1;
      inh1 = 1;
      inh2 = 0;
      coils = "B->A";
      break;
    case 3:
      pwm->channels[0].duty = DUTY_OFF;
      pwm->channels[1].duty = DUTY_OFF;
      pwm->channels[2].duty = DUTY_ON;
      inh0 = 1;
      inh1 = 0;
      inh2 = 1;
      coils = "C->A";
      break;
    case 4:
      pwm->channels[0].duty = DUTY_OFF;
      pwm->channels[1].duty = DUTY_OFF;
      pwm->channels[2].duty = DUTY_ON;
      inh0 = 0;
      inh1 = 1;
      inh2 = 1;
      coils = "C->B";
      break;
    case 5:
      pwm->channels[0].duty = DUTY_ON;
      pwm->channels[1].duty = DUTY_OFF;
      pwm->channels[2].duty = DUTY_OFF;
      inh0 = 1;
      inh1 = 1;
      inh2 = 0;
      coils = "A->B";
      break;
    default:
      break;
    }
    ret = ioctl(fd_pwm, PWMIOC_SETCHARACTERISTICS,
                (unsigned long)((uintptr_t) pwm));
    if (ret < 0)
      {
        printf("Fail setting pwm in commutation!\n");
        return NULL;
      }
    ret = ioctl(fd_inh0, GPIOC_WRITE, (unsigned long) inh0);
    if (ret < 0)
      {
        printf("Fail setting inh%d in commutation!\n", 0);
      }
    ret = ioctl(fd_inh1, GPIOC_WRITE, (unsigned long) inh1);
    if (ret < 0)
      {
        printf("Fail setting inh%d in commutation!\n", 1);
      }
    ret = ioctl(fd_inh2, GPIOC_WRITE, (unsigned long) inh2);
    if (ret < 0)
      {
        printf("Fail setting inh%d in commutation!\n", 2);
      }
    //printf("Commutating section %d\n", section);
    return coils;
}

static int commutate_b_example(void)
{
  struct pwm_info_s info;
  // first, open all needed peripherals
  printf("Opening peripheral pwm0!\n");
  int fd_pwm = open("/dev/pwm0", O_RDONLY);
  if (fd_pwm < 0)
    {
      printf("Error opening pwm0!\n");
      return fd_pwm;
    }
  for (int i = 0; i < 4; ++i)
    {
      // Set all characteristics of PWM.
      info.channels[i].channel = (i+1);
      info.channels[i].duty = 20000;
      info.channels[i].cpol = PWM_CPOL_HIGH;
      info.frequency = 16000;
    }
  info.channels[3].duty = 0;
  int pwmret = ioctl(fd_pwm, PWMIOC_SETCHARACTERISTICS, (unsigned long)((uintptr_t) &info));
  if (pwmret < 0)
    {
      printf("Ioctl of pwm0 failed!\n");
      return pwmret;
    }

  printf("Success pwm0!\n");
  printf("Opening hall inputs!\n");
  int fd_h0 = open("/dev/hallb_in0", O_RDONLY);
  if (fd_h0 < 0)
    {
      printf("Error opening %d\n", 0);
      return fd_h0;
    }
  int fd_h1 = open("/dev/hallb_in1", O_RDONLY);
  if (fd_h1 < 0)
    {
      printf("Error opening %d\n", 1);
      return fd_h1;
    }
  int fd_h2 = open("/dev/hallb_in2", O_RDONLY);
  if (fd_h2 < 0)
    {
      printf("Error opening %d\n", 2);
      return fd_h2;
    }
  printf("Success hall!\n");
  printf("Opening peripheral pwm inhibits!\n");
  int fd_inh0 = open("/dev/pwmb_inh0", O_RDWR);
  if (fd_inh0 < 0)
    {
      printf("Error opening inh %d\n", 0);
      return fd_inh0;
    }
  int fd_inh1 = open("/dev/pwmb_inh1", O_RDWR);
  if (fd_inh1 < 0)
    {
      printf("Error opening inh %d\n", 1);
      return fd_inh1;
    }
  int fd_inh2 = open("/dev/pwmb_inh2", O_RDWR);
  if (fd_inh2 < 0)
    {
      printf("Error opening inh %d\n", 2);
      return fd_inh2;
    }
  int fd_inh3 = open("/dev/pwmb_inh3", O_RDWR);
  if (fd_inh3 < 0)
    {
      printf("Error opening inh %d\n", 3);
      return fd_inh3;
    }
  printf("Success inh outputs!\n");
  
  // turn of the half bridges
  int inhret;
  inhret = ioctl(fd_inh0, GPIOC_WRITE, (unsigned long) 0);
  if (inhret < 0)
    {
      return inhret;
    }
  ioctl(fd_inh1, GPIOC_WRITE, (unsigned long) 0);
  if (inhret < 0)
    {
      return inhret;
    }
  ioctl(fd_inh2, GPIOC_WRITE, (unsigned long) 0);
  if (inhret < 0)
    {
      return inhret;
    }
  ioctl(fd_inh3, GPIOC_WRITE, (unsigned long) 0);
  if (inhret < 0)
    {
      return inhret;
    }
  printf("Turning off halfbridges ok!\n");

  // start pwm
  pwmret = ioctl(fd_pwm, PWMIOC_START, (unsigned long)((uintptr_t) &info));
  if (pwmret < 0)
    {
      return pwmret;
    }
  printf("Starting pwm ok!\n");

  /* create timed periodic delay */
  struct timespec sample_time;
  long sample_time_nsec = 1000*100; // ms
  clock_gettime(CLOCK_MONOTONIC, &sample_time);
  do
    {
      sample_time.tv_nsec += sample_time_nsec;
      if (sample_time.tv_nsec > 1000*1000*1000) {
        sample_time.tv_nsec -= 1000*1000*1000;
        sample_time.tv_sec += 1;
      }
      clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &sample_time, NULL);

      int val = 0;
      bool h0, h1, h2;
      int ret = ioctl(fd_h0, GPIOC_READ, (unsigned long)(uintptr_t)(&h0));
      if (ret < 0) 
        {
          return ERROR;
        }
      ret = ioctl(fd_h1, GPIOC_READ, (unsigned long)(uintptr_t)(&h1));
      if (ret < 0) 
        {
          return ERROR;
        }
      ret = ioctl(fd_h2, GPIOC_READ, (unsigned long)(uintptr_t)(&h2));
      if (ret < 0) 
        {
          return ERROR;
        }
      //int section = hall_2_section(h0, h1, h2);
      //printf("Cur section: %d - %d %d %d\n", section, h0, h1, h2);
      //usleep(1000*100);

      /*
      for (int i = 0; i < 6; ++i)
        {
          const char *comm =
            commutate_bldc(i, &info, fd_pwm, fd_inh0, fd_inh1, fd_inh2, fd_inh3,
                         fd_h0, fd_h1, fd_h2);
          usleep(1000*1000);
          if (comm == NULL)
            {
              continue;
            }
          bool read_h0, read_h1, read_h2;
          ioctl(fd_h0, GPIOC_READ, (unsigned long)((uintptr_t) &read_h0));
          ioctl(fd_h1, GPIOC_READ, (unsigned long)((uintptr_t) &read_h1));
          ioctl(fd_h2, GPIOC_READ, (unsigned long)((uintptr_t) &read_h2));
          printf("%s: %d %d %d\n", comm, read_h0, read_h1, read_h2);
        }
        */
      int section = hall_2_section(h0, h1, h2);
      commutate_bldc(section, &info, fd_pwm, fd_inh0, fd_inh1, fd_inh2, fd_inh3,
                    fd_h0, fd_h1, fd_h2);
      val += 1;
      if (val == 10000) {
        printf("!\n");
        fflush(stdout);
      }
    } while(1);
  return OK; 
}

static int i2c_eeprom_example(void)
{
  int ret;
  char buf[100];
  int fd = open("/dev/eeprom0", O_RDWR);
  if (fd < 0)
    {
      printf("Failed!\n");
      return ERROR;
    }
  ret = lseek(fd, 0, SEEK_SET);
  if (ret < 0)
    {
      printf("Lseek of the eeprom failed!\n");
      return ERROR;
    }
  printf("Lseek successful!\n");
  int readsize = read(fd, buf, 10);
  if (readsize != 10)
    {
      printf("Read only %d bytes, return\n", readsize); 
      return ERROR;
    }
  for (int i = 0; i < readsize; ++i) {
    printf("%d ", (int) buf[i]);
  }
  printf("Try write\n");
  memset((void*) buf, 50, 10);
  ret = lseek(fd, 0, SEEK_SET);
  if (ret < 0)
    {
      printf("Lseek of the eeprom failed!\n");
      return ERROR;
    }
  ret = write(fd, buf, 10);
  if (ret != 10)
    {
      printf("Writing %d bytes only!\n", ret);
      return ERROR;
    }
  printf("Write succesful!\n");
  return OK;
}

int calibrate_measure_adc(int adc_fd, struct adc_msg_s *dest,
                          const ssize_t channels)
{
  int32_t buf[8];
  memset(buf, 0, sizeof(buf));
  size_t readsize = channels * sizeof(struct adc_msg_s);
  int ret;
  int measured = 0;
  ret = ioctl(adc_fd, ANIOC_RESET_FIFO, 0);
  while (measured < 1)
    {
      ret = ioctl(adc_fd, ANIOC_TRIGGER, 0);
      if (ret < 0)
        {
          printf("ioctl fail\n");
        }
      int nbytes = read(adc_fd, dest, readsize);
      if (nbytes == readsize)
        {
          measured++;
          for (int i = 0; i < channels; ++i)
            {
              buf[i] += dest->am_data;
            }
        }
      else
        {
          ioctl(adc_fd, ANIOC_RESET_FIFO, 0);
        }
    }
  for (int i = 0; i < 8; ++i)
    {
      //buf[i] >>= 4;
      dest->am_data = buf[i];
    }
  return OK;
}

struct adc_lookup_s
{
  uint8_t phase;   /* the number */
  bool high;       /* true if hss, false otherwise */
};

int32_t adc0_channel_lookup(struct adc_msg_s msgs[], 
                             size_t channels,
                             struct adc_lookup_s look)
{
  uint8_t channel = 0;
  switch (look.phase)
    {
    case 0:
      if (look.high)
        {
          channel = 10;
        }
      else
        {
          channel = 9;
        }
      break;
    case 1:
      if (look.high)
        {
          channel = 0;
        }
      else
        {
          channel = 3;
        }
      break;
    case 2:
      if (look.high)
        {
          channel = 8;          
        }
      else
        {
          channel = 7;
        }
      break;
    case 3:
      if (look.high)
        {
          channel = 4;
        }
      else
        {
          channel = 6;
        }
      break;
    default:
      break;
    }
  int i;
  for (i = 0; i < channels; ++i)
    {
      if (msgs[i].am_channel != channel)
        {
          continue;
        }
      return msgs[i].am_data;
    }
  return -INT_MAX;
}

int32_t adc1_channel_lookup(struct adc_msg_s msgs[], 
                             size_t channels,
                             struct adc_lookup_s look)
{
  uint8_t channel = 0;
  switch (look.phase)
    {
    case 0:
      if (look.high)
        {
          channel = 10;
        }
      else
        {
          channel = 9;
        }
      break;
    case 1:
      if (look.high)
        {
          channel = 0;
        }
      else
        {
          channel = 1;
        }
      break;
    case 2:
      if (look.high)
        {
          channel = 2;
        }
      else
        {
          channel = 5;
        }
      break;
    case 3:
      if (look.high)
        {
          channel = 6;          
        }
      else
        {
          channel = 4;
        }
      break;
    default:
      break;
    }
  int i;
  for (i = 0; i < channels; ++i)
    {
      if (msgs[i].am_channel != channel)
        {
          continue;
        }
      return msgs[i].am_data;
    }
  return -INT_MAX;
}

int calibrate_adcs(int adc_n)
{
  int adc_fd, inh0_fd, inh1_fd, inh2_fd, inh3_fd, pwm_fd;
  const char *inh0_path, *inh1_path, *inh2_path, *inh3_path, *pwm_path;
  if (adc_n != 0 && adc_n != 1)
    {
      printf("Wrong ADC channel!\n");
      return ERROR;
    }
  /* Firstly, open AD converters*/
  char adc_dev[20];
  sprintf(adc_dev, "/dev/adc%d", adc_n);
  adc_fd = open(adc_dev, O_RDONLY);
  if (adc_fd < 0)
    {
      printf("Error opening %s!\n", adc_dev);
      return ERROR;
    }
  /* Open inhibits and PWMs */
  /* 0 resembles the left side in terms of ADC's,
      however the left PWM is the PWM1
  */
  if (adc_n == 0)
    {
      inh0_path = "/dev/pwma_inh0";
      inh1_path = "/dev/pwma_inh1";
      inh2_path = "/dev/pwma_inh2";
      inh3_path = "/dev/pwma_inh3";
      pwm_path  = "/dev/pwm1";
    }
  else if (adc_n == 1)
    {
      inh0_path = "/dev/pwmb_inh0";
      inh1_path = "/dev/pwmb_inh1";
      inh2_path = "/dev/pwmb_inh2";
      inh3_path = "/dev/pwmb_inh3";
      pwm_path  = "/dev/pwm0";
    }
  inh0_fd = open(inh0_path, O_RDWR);
  inh1_fd = open(inh1_path, O_RDWR);
  inh2_fd = open(inh2_path, O_RDWR);
  inh3_fd = open(inh3_path, O_RDWR);
  pwm_fd  = open(pwm_path, O_RDWR);
  if (inh0_fd < 0 || inh1_fd < 0 || inh2_fd < 0 || inh3_fd < 0 || pwm_fd < 0)
    {
      printf("ERROR! Inh0=%d, inh1=%d, inh2=%d, inh3=%d, pwm=%d",
             inh0_fd, inh1_fd, inh2_fd, inh3_fd, pwm_fd); 
      return ERROR;
    }
  /* Now everything is opened */
  ioctl(inh0_fd, GPIOC_WRITE, (unsigned long) 0);
  ioctl(inh1_fd, GPIOC_WRITE, (unsigned long) 0);
  ioctl(inh2_fd, GPIOC_WRITE, (unsigned long) 0);
  ioctl(inh3_fd, GPIOC_WRITE, (unsigned long) 0);

  struct pwm_info_s info;
  for (int i = 0; i < 4; ++i)
    {
      // Set all characteristics of PWM.
      info.channels[i].channel = (i+1);
      info.channels[i].duty = 0;
      info.channels[i].cpol = PWM_CPOL_HIGH;
      info.frequency = 1000;
    }
  info.channels[3].duty = 0;
  int pwmret = ioctl(pwm_fd, PWMIOC_SETCHARACTERISTICS, (unsigned long)((uintptr_t) &info));
  if (pwmret < 0)
    {
      printf("Set PWM ioctl failed!\n");
      return ERROR;
    }


  printf("%s will be calibrated.\n", adc_dev);
  int state = 0;
  bool running = true;
  const ssize_t channels = 8;
  struct adc_msg_s samples[channels];

  /* In the time of writing this stuff,
   * there are problems with the PWM1 switching.
   * Hence the differentiation of loops
   */
  if (adc_n == 0)
    {
    }
  else if (adc_n == 1)
    {

      while (running)
        {
          if (state == 4)
            {
              printf("End of measurement!\n");
              break;
            }
          switch (state)
            {
            case 0:
              {
                info.channels[0].duty = 65535;
                info.channels[1].duty = 0;
                ioctl(pwm_fd, PWMIOC_SETCHARACTERISTICS, (unsigned long)((uintptr_t) &info));
                ioctl(pwm_fd, PWMIOC_START, (unsigned long)((uintptr_t) &info));
                ioctl(inh0_fd, GPIOC_WRITE, (unsigned long) 1);
                ioctl(inh1_fd, GPIOC_WRITE, (unsigned long) 1);
                ioctl(inh2_fd, GPIOC_WRITE, (unsigned long) 0);
                ioctl(inh3_fd, GPIOC_WRITE, (unsigned long) 0);
                printf("Writing H0, L1\n");
                break;
              }
            case 1:
              {
                info.channels[0].duty = 0;
                info.channels[1].duty = 65535;
                ioctl(pwm_fd, PWMIOC_SETCHARACTERISTICS, (unsigned long)((uintptr_t) &info));
                ioctl(pwm_fd, PWMIOC_START, (unsigned long)((uintptr_t) &info));
                ioctl(inh0_fd, GPIOC_WRITE, (unsigned long) 1);
                ioctl(inh1_fd, GPIOC_WRITE, (unsigned long) 1);
                ioctl(inh2_fd, GPIOC_WRITE, (unsigned long) 0);
                ioctl(inh3_fd, GPIOC_WRITE, (unsigned long) 0);
                printf("Writing L0, H1\n");
                break;
              }
            case 2:
              {
                info.channels[2].duty = 65535;
                info.channels[3].duty = 0;
                ioctl(pwm_fd, PWMIOC_SETCHARACTERISTICS, (unsigned long)((uintptr_t) &info));
                ioctl(pwm_fd, PWMIOC_START, (unsigned long)((uintptr_t) &info));
                ioctl(inh0_fd, GPIOC_WRITE, (unsigned long) 0);
                ioctl(inh1_fd, GPIOC_WRITE, (unsigned long) 0);
                ioctl(inh2_fd, GPIOC_WRITE, (unsigned long) 1);
                ioctl(inh3_fd, GPIOC_WRITE, (unsigned long) 1);
                printf("Writing H2, L3\n");
                break;
              }
            case 3:
              {
                info.channels[2].duty = 0;
                info.channels[3].duty = 65535;
                ioctl(pwm_fd, PWMIOC_SETCHARACTERISTICS, (unsigned long)((uintptr_t) &info));
                ioctl(pwm_fd, PWMIOC_START, (unsigned long)((uintptr_t) &info));
                ioctl(inh0_fd, GPIOC_WRITE, (unsigned long) 0);
                ioctl(inh1_fd, GPIOC_WRITE, (unsigned long) 0);
                ioctl(inh2_fd, GPIOC_WRITE, (unsigned long) 1);
                ioctl(inh3_fd, GPIOC_WRITE, (unsigned long) 1);
                printf("Writing L2, H3\n");
                break;
              }
            }
          usleep(1000*10);
          calibrate_measure_adc(adc_fd, samples, channels);
          switch (state)
            {
            case 0:
              {
                struct adc_lookup_s l1 = {0, true}, l2 = {1, false};
                int32_t val1 = adc1_channel_lookup(samples, 8, l1);
                int32_t val2 = adc1_channel_lookup(samples, 8, l2);
                printf("H0 = %ld\nL1 = %ld\n", val1, val2); 
                break;
              }
            case 1:
              {
                struct adc_lookup_s l1 = {0, false}, l2 = {1, true};
                int32_t val1 = adc1_channel_lookup(samples, 8, l1);
                int32_t val2 = adc1_channel_lookup(samples, 8, l2);
                printf("L0 = %ld\nH1 = %ld\n", val1, val2); 
                break;
              }
            case 2:
              {
                struct adc_lookup_s l1 = {2, true}, l2 = {3, false};
                int32_t val1 = adc1_channel_lookup(samples, 8, l1);
                int32_t val2 = adc1_channel_lookup(samples, 8, l2);
                printf("H2 = %ld\nL3 = %ld\n", val1, val2); 
                break;
              }
            case 3:
              {
                struct adc_lookup_s l1 = {2, false}, l2 = {3, true};
                int32_t val1 = adc1_channel_lookup(samples, 8, l1);
                int32_t val2 = adc1_channel_lookup(samples, 8, l2);
                printf("L2 = %ld\nH3 = %ld\n", val1, val2); 
                break;
              }
            default:
              {
                break;
              }
            }
            printf("Press p to switch phases. Enter anything to measure once more.\n");
            char choice = getchar();
            if (choice == 'p')
              {
                state++;
              }
        }
    }
  ioctl(pwm_fd, PWMIOC_STOP, (unsigned long)((uintptr_t) &info));
  return OK;
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
  else if (strcmp(argv[1], "pwm0") == 0)
    {
      return pwm0_example();
    }
  else if (strcmp(argv[1], "pwm0_simple") == 0)
    {
      return pwm0_simple_example();
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
  return 0;
}
