#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>

#include <nuttx/config.h>
#include <nuttx/timers/pwm.h>
#include <arch/board/board.h>
#include <nuttx/ioexpander/gpio.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>


#define DUTY_ON  (ub16_t) 15000
#define DUTY_OFF (ub16_t) 0

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

int commutate_b_example(void)
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
