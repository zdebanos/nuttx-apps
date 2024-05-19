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
#include <unistd.h>


int i2c_eeprom_example(void)
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
