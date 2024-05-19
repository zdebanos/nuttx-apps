#include <stdint.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <nuttx/config.h>
#include <nuttx/analog/adc.h>
#include <nuttx/analog/ioctl.h>
#include <nuttx/timers/pwm.h>
#include <arch/board/board.h>
#include <nuttx/ioexpander/gpio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include "adcs.h"

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

int afec0_example(void)
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
