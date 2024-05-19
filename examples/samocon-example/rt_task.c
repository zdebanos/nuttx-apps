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


static double Tsamp = 0.001;
static volatile int end = 0;

static inline double calcdiff(struct timespec t1, struct timespec t2)
{
  double diff;
  diff = 1.0 * ((long) t1.tv_sec - (long) t2.tv_sec);
  diff += 1e-9*t1.tv_nsec - 1e-9*t2.tv_nsec;
  return (diff);
}

static inline void tsnorm(struct timespec *ts)
{
  while (ts->tv_nsec >= NSEC_PER_SEC) {
    ts->tv_nsec -= NSEC_PER_SEC;
    ts->tv_sec++;
  }
}

void rt_task_endme(int n)
{
  end = 1;
}

void *rt_task(void *p)
{
  struct timespec t_next, t_current, t_isr, T0;
  struct sched_param param;
  double T;

  param.sched_priority = 150;
  if(sched_setscheduler(0, SCHED_FIFO, &param)==-1){
    perror("sched_setscheduler failed");
    exit(-1);
  }


  t_isr.tv_sec =  0L;
  t_isr.tv_nsec = (long)(1e9*Tsamp);
  tsnorm(&t_isr);


  /* get current time */
  clock_gettime(CLOCK_MONOTONIC,&t_current);
  T0 = t_current;

  int loop_count = 0;
  int overrun_count;
  int accumulator = 0;
  
  while(1){
    /* periodic task */
    T = calcdiff(t_current,T0);

    /* Here do the periodic stuff */
    

    t_next.tv_sec = t_current.tv_sec + t_isr.tv_sec;
    t_next.tv_nsec = t_current.tv_nsec + t_isr.tv_nsec;
    tsnorm(&t_next);

    /* Check if Overrun */
    clock_gettime(CLOCK_MONOTONIC,&t_current);
    if (t_current.tv_sec > t_next.tv_sec ||
	      (t_current.tv_sec == t_next.tv_sec && t_current.tv_nsec > t_next.tv_nsec))
      {
        int usec = (t_current.tv_sec - t_next.tv_sec) * 1000000 + (t_current.tv_nsec -
                  t_next.tv_nsec)/1000;
        //fprintf(stderr, "Base rate overrun by %d us\n", usec);
        accumulator += usec;
        overrun_count++;
        t_next = t_current;
      }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t_next, NULL);
    t_current = t_next;

    loop_count++;
    if (loop_count == 1024)
      {
        accumulator /= overrun_count;

        printf("Avg.ovr: %d, ovr cnt: %d\n", accumulator, overrun_count);

        overrun_count = 0;
        loop_count = 0;
      }
  }
  pthread_exit(0);
}