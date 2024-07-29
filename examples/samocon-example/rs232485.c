#include <stdint.h>
#include <stdlib.h>
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
#include <termios.h>

static int rstest_rs485(int fd)
{
  int write_success = write(fd, "Nazdar\r\n", 8);
  if (write_success < 0)
    {
      printf("Testing write failed!\n");
      return EXIT_FAILURE;
    }

  usleep(100000);

  char recbuf[20];
  int read_success = read(fd, recbuf, 8);
  if (read_success < 0)
    {
      printf("Testing read failed!\n");
      return EXIT_FAILURE;
    }
  printf("Read %d bytes\n", read_success);
  for (int i = 0; i < read_success; ++i)
    {
      putchar(recbuf[i]);
    }
  putchar('\n');
  return EXIT_SUCCESS;
}

static int rstest_rs232(int fd)
{
  printf("testing rs232\n");
  struct termios ts;
  int ret = tcgetattr(fd, &ts);
  if (ret < 0)
    {
      printf("Can't get termios struct!\n");
      return EXIT_FAILURE;
    }

  cfmakeraw(&ts);
  ts.c_cflag |= CRTSCTS;
  ret = tcsetattr(fd, TCSANOW, &ts);
  if (ret < 0)
    {
      printf("Can't set attributes!\n");
      return EXIT_FAILURE;
    }
  
  
  int write_success = write(fd, "Nazdar\r\n", 8);
  if (write_success < 0)
    {
      printf("Testing write failed!\n");
      return EXIT_FAILURE;
    }

  /*
  usleep(100000);

  char recbuf[20];
  int read_success = read(fd, recbuf, 8);
  if (read_success < 0)
    {
      printf("Testing read failed!\n");
      return EXIT_FAILURE;
    }
  printf("Read %d bytes\n", read_success);
  for (int i = 0; i < read_success; ++i)
    {
      putchar(recbuf[i]);
    }
  putchar('\n');
  */
  
  return EXIT_SUCCESS;
}


int rstest_example(void)
{
  int fd = open("/dev/ttyS1", O_RDWR);
  if (fd < 0)
    {
      printf("Failed to open the USART2 peripheral!\n");
      return EXIT_FAILURE;
    }    
  const char *mode;
#ifdef CONFIG_SAMV7_USART2_RS485MODE
  mode = "RS485";
#else
  mode = "RS232"; 
#endif

    char unused;
    UNUSED(unused);

    printf("Current RS mode: %s\n", mode);
    printf("Press enter to transmit!\n");
    unused = getchar();
    if (strcmp(mode, "RS485") == 0)
      {
        return rstest_rs485(fd);   
      }  
    else if (strcmp(mode, "RS232") == 0)
      {
        return rstest_rs232(fd);
      }


    return EXIT_SUCCESS;
}