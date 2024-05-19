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


int pwm0_example(void)
{
  struct pwm_info_s info;
  int fd = open("/dev/pwm0", O_RDONLY);
  int fd0 = open("/dev/pwma_inh0", O_RDWR);
  int fd1 = open("/dev/pwma_inh1", O_RDWR);
  int fd2 = open("/dev/pwma_inh2", O_RDWR);
  int fd3 = open("/dev/pwma_inh3", O_RDWR);

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
      info.channels[i].duty = 10000*(i+1);
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

int pwm1_example(void)
{
  struct pwm_info_s info;
  int fd = open("/dev/pwm1", O_RDONLY);
  int fd0 = open("/dev/pwmb_inh0", O_RDWR);
  int fd1 = open("/dev/pwmb_inh1", O_RDWR);
  int fd2 = open("/dev/pwmb_inh2", O_RDWR);
  int fd3 = open("/dev/pwmb_inh3", O_RDWR);

  ioctl(fd0, GPIOC_WRITE, (unsigned long) 1);
  ioctl(fd1, GPIOC_WRITE, (unsigned long) 1);
  ioctl(fd2, GPIOC_WRITE, (unsigned long) 1);
  ioctl(fd3, GPIOC_WRITE, (unsigned long) 1);

  int ret;

  if (fd < 0)
    {
      printf("error opening pwm!\n");
      return ERROR;
    }
  for (int i = 0; i < 4; ++i)
    {
      info.channels[i].channel = (i+1);
      info.channels[i].duty = 0;
      info.channels[i].cpol = PWM_CPOL_HIGH;
      info.channels[i].dcpol = PWM_DCPOL_LOW;
      info.frequency = 20000;
    }

  info.channels[2].cpol = PWM_CPOL_LOW;
  info.channels[2].dcpol = PWM_DCPOL_HIGH;
  ret = ioctl(fd, PWMIOC_SETCHARACTERISTICS, (unsigned long)((uintptr_t) &info));
  ret = ioctl(fd, PWMIOC_START, (unsigned long)((uintptr_t) &info));
  if (ret < 0)
    {
      printf("Ioctl start of pwm1 failed!\n");
    }
  uint16_t cnt = 0;
  for (int j = 0; j < 6553; j++, cnt += 10)
  {
    info.channels[0].duty = cnt;
    info.channels[1].duty = cnt;
    info.channels[2].duty = cnt;
    info.channels[3].duty = cnt;
    ret = ioctl(fd, PWMIOC_SETCHARACTERISTICS, (unsigned long)((uintptr_t) &info));
    if (ret < 0)
      {
        printf("Ioctl of pwm1 failed!\n");
      }
    usleep(1000);
  }
  printf("Set duty OK!\n"); 
  printf("Press anything to turn off the pwm!\n");
  getchar();
  ret = ioctl(fd, PWMIOC_STOP, (unsigned long)((uintptr_t) &info));
  if (ret < 0)
    {
      printf("Ioctl start of pwm1 failed!\n");
    }
  printf("Check the PWM! Press anything to turn off the app!\n");
  getchar();
  close(fd);
  return OK;
}
