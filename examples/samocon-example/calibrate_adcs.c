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

#include "adcs.h"


static int calibrate_measure_adc(int adc_fd, struct adc_msg_s *dest,
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