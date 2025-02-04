#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <semaphore.h>
#include <unistd.h>
#include <poll.h>
#include <pthread.h>
#include <stdlib.h>

#include <nuttx/timers/timer.h>

#define PERIOD 101
#define LOOPS ((int)(20000000/PERIOD))

void print_hist(int hist[], int cnt)
{
  for (int i = 0; i < cnt; ++i)
    {
      if (hist[i] != 0)
        {
          printf("%d: %d\n", i, hist[i]);
        }
    }
}

int timerhook_example(void)
{
  int ret;
  uint32_t maxtimeout;
  struct timer_notify_s notify;
  struct timer_status_s timer_status;
  struct pollfd fds[1];
  struct sigevent notify_event;
  struct sched_param param;

  // initialize sigevent, SIGEV_NONE is sufficient
  memset(&notify_event, 0, sizeof(struct sigevent));
  notify_event.sigev_notify = SIGEV_NONE;
  
  // fill in the timer_notify_s struct
  notify.periodic = true;
  notify.pid      = getpid();
  notify.event    = notify_event;
  
  // boost prio
  param.sched_priority = 150;
  pthread_setschedparam(pthread_self(), SCHED_RR, &param);

  puts("Timerhook example");
  
  int fd = open("/dev/timer9", O_RDWR);
  if (fd < 0)
    {
      perror("open timer");
      return ERROR;
    }

  // prepare for poll
  fds[0].fd = fd;
  fds[0].events = POLLIN;
  
  ret = ioctl(fd, TCIOC_SETTIMEOUT, (unsigned long)PERIOD);
  if (ret < 0)
    {
      perror("set timeout timer");
      return ERROR;
    }

  ret = ioctl(fd, TCIOC_NOTIFICATION, (unsigned long)((uintptr_t) &notify));
  if (ret < 0)
    {
      perror("notify timer");
      return ERROR;
    }

  ret = ioctl(fd, TCIOC_START);
  if (ret < 0)
    {
      perror("start timer");
      return ERROR;
    }
  
  
  uint32_t maxlatency = 0, latency;
  int *hist = calloc(PERIOD, sizeof(int));
  if (hist == NULL)
    {
      perror("alloc");
      return ERROR;
    }
  
  for (int cnt = 0; cnt < LOOPS; ++cnt)
    {
      ret = poll(fds, 1, -1);
      if (ret < 0)
        {
          perror("poll");
          break;
        }
      ret = ioctl(fd, TCIOC_GETSTATUS, (unsigned long)((uintptr_t) &timer_status));
      if (ret < 0)
        {
          perror("getstaus");
          break;
        }
      latency = timer_status.timeout - timer_status.timeleft;
      if (latency > maxlatency) {
        maxlatency = latency;
      }
      if (latency < PERIOD && latency >= 0)
        hist[latency] += 1;
    }	
  
  print_hist(hist, PERIOD);
  free(hist);
  ioctl(fd, TCIOC_STOP);
  close(fd);
  printf("maxlatency = %lu\n", maxlatency);
  return OK;
}
