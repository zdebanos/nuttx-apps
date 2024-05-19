/* halls.c */

#include <stdint.h>
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
#include <unistd.h>

struct hall_device_s
{
  int fd;
  const char *devpath;
  int value;
};

int hall_example(void)
{
  struct hall_device_s hall_devices_a[3], hall_devices_b[3];
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
