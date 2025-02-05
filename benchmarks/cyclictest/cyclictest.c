/****************************************************************************
 * apps/benchmarks/cyclictest/cyclictest.c
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

#include <sched.h>
#include <limits.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include <fcntl.h>

#include <nuttx/timers/timer.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

#ifdef CONFIG_SYSTEMTICK_HOOK
extern sem_t g_waitsem;
#endif

enum meas_method_e
{
  M_GETTIME = 0,
  M_TIMER_API,
  M_COUNT
};

enum wait_method_e
{
  W_NANOSLEEP = 0,
  W_SYSTICKHOOK,
  W_TIMERHOOK,
  W_COUNT
};

struct cyclictest_config_s
{
  int clock;
  int distance;
  int duration;
  int histogram;
  unsigned long interval;
  unsigned long loops;
  int threads;
  int policy;
  int prio;
  char *timer_dev;
  enum meas_method_e meas_method;
  enum wait_method_e wait_method;
};

struct thread_param_s
{
  int prio;
  int policy;
  unsigned long interval;
  unsigned long max_cycles;
  struct thread_stats_s *stats;
  int clock;
};

struct thread_stats_s
{
  long *hist_array;
  long hist_overflow;
  long min;
  long max;
  double avg;
  pthread_t id;
};

static bool running;
static struct cyclictest_config_s config;
static int timerfd;
static struct pollfd polltimer[1];

static const struct option optargs[] =
{
  {"clock", optional_argument, 0, 'c'},
  {"distance", optional_argument, 0, 'd'},
  {"duration", optional_argument, 0, 'D'},
  {"histogram", optional_argument, 0, 'h'},
  {"interval", optional_argument, 0, 'i'},
  {"loops", optional_argument, 0, 'l'},
  {"threads", optional_argument, 0, 't'},
  {"timer-device", optional_argument, 0, 'T'},
  {"policy", optional_argument, 0, 'y'},
  {"wait-method", optional_argument, 0, 'w'}
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void print_help(void)
{
  puts(
    "The Cyclictest Benchmark Utility\n"
    "Usage:\n"
    "  -c --clock [CLOCK]: selects the clock: 0 selects CLOCK_REALTIME, "
    "1 selects CLOCK_MONOTONIC. Applies only when -n=0 or -m=0.\n"
    "  -d --distance [US]: The distance of thread intervals. Default is 500us.\n"
    "  -D --duration [TIME]: Test duration length in seconds. Default is 0 (endless).\n"
    "  -h --histogram [US]: Output the histogram data to stdout. US is the maximum"
    "     value to be printed.\n"
    "  -i --interval [US]: The thread interval. Default is 1000us.\n"
    "  -l --loops [N]: The number of measurement loops. Default is 0 (endless).\n"
    "  -m --measurement [METHOD]: Set the time measurement method:\n"
    "     0 selects clock_gettime, 1 uses the NuttX timer API.\n"
    "     WARNING:\n"
    "       If METHOD 1 is selected, you need to specify a timer device (e.g. /dev/timer0) in -T.\n"
    "  -n --nanosleep [METHOD]: Set the waiting method: 0 selects clock_nanosleep,\n"
    "     1 waits for a semaphore (sem_t g_semwait) posted by a board_timerhook function,\n"
    "     2 waits for the POLLIN flag on a timer device.\n"
    "     Default is 0."
    "     WARNING:\n"
    "       Choosing 1 or 2 works only with one thread, "
    "       the -t value is therefore set to 1.\n"
    "       The METHOD 1 only works when compiled with CONFIG_SYSTEMTICK_HOOK."
    "       If METHOD 2 is selected, you need to specify a timer device (e.g. /dev/timer0) in -T.\n"
    "  -t --threads [N]: The number of test threads to be created. Default is 1.\n"
    "  -T --timer-device [DEV]: The measuring timer device.\n"
    "     Must be specified when -n=2 or -m=1.\n"
    "  -y --policy [NAME]: Set the scheduler policy, where NAME is fifo or rr.\n"
  );
}

static long arg_decimal(char *arg)
{
  long ret;
  char *endptr;
  ret = strtol(arg, &endptr, 10);
  if (endptr == arg)
    {
      return -1;
    }
  return ret; 
}

static bool parse_args(int argc, char * const argv[])
{
  int longindex;
  int opt;
  long decimal;
  while ((opt = getopt_long(argc, argv, "c:d:D:h:i:l:m:n:t:T:", optargs, &longindex)) != -1) {
    switch (opt) {
      case 'c':
        decimal = arg_decimal(optarg);
        if (decimal < 0)
          {
            return false;
          }
        else if (decimal == CLOCK_MONOTONIC || decimal == CLOCK_REALTIME)
          {
            config.clock = decimal;
          }
        break;
      case 'd':
        decimal = arg_decimal(optarg);
        if (decimal >= 0)
          {
            config.distance = decimal;
          }
        else
          {
            return false;
          }
        break;
      case 'D':
        decimal = arg_decimal(optarg);
        if (decimal >= 0)
          {
            config.duration = decimal;
          }
        else
          {
            return false;
          }
        break;
      case 'h':
        decimal = arg_decimal(optarg);
        if (decimal > 0)
          {
            config.histogram = decimal;
          }
        else
          {
            return false;
          }
        break;
      case 'i':
        decimal = arg_decimal(optarg);
        if (decimal >= 0)
          {
            config.interval = decimal;
          }
        else
          {
            return false;
          }
        break;
      case 'l':
        decimal = arg_decimal(optarg);
        if (decimal >= 0)
          {
            config.loops = decimal;
          }  
        else
          {
            return false;
          }
        break;
      case 'm':
        decimal = arg_decimal(optarg);
        if (decimal >= 0)
          {
            config.meas_method = decimal;
          }
        else
          {
            return false;
          }
        break;
      case 'n':
        decimal = arg_decimal(optarg);
        if (decimal >= 0)
          {
            config.wait_method = decimal;
          }
        else
          {
            return false;
          }
        break;
      case 't':
        decimal = arg_decimal(optarg);
        if (decimal > 0)
          {
            config.threads = decimal;
          }
        else
          {
            return false;
          }
        break;
      case 'T':
        config.timer_dev = optarg;
        break;
      case 'y':
        if (strcmp(optarg, "other") == 0)
          {
            config.policy = SCHED_OTHER;
          }
        else if (strcmp(optarg, "normal") == 0)
          {
            config.policy = SCHED_NORMAL;
          }
        else if (strcmp(optarg, "batch") == 0)
          {
            config.policy = SCHED_BATCH;
          }
        else if (strcmp(optarg, "idle") == 0)
          {
            config.policy = SCHED_IDLE;
          }
        else if (strcmp(optarg, "fifo") == 0)
          {
            config.policy = SCHED_FIFO;
          }
        else if (strcmp(optarg, "rr") == 0)
          {
            config.policy = SCHED_RR;
          }
        else
          {
            return false;
          }
        break;
      case '?':
        return false;
      default:
        break;
    }
  }
  if (optind < argc)
    {
      return false;
    }
  return true;
}

static bool check_args_logic(void)
{
  if (config.wait_method == W_SYSTICKHOOK)
    {
#ifndef CONFIG_SYSTEMTICK_HOOK
      fprintf(stderr, "NuttX is not compiled with CONFIG_SYSTEMTICK_HOOK!\n");
      return false;
#endif
    }

  if (config.wait_method == W_TIMERHOOK || config.meas_method == M_TIMER_API)
    {
      if (config.timer_dev == NULL)
        {
          fprintf(stderr, "Specify timer device!\n");
          return false;
        }
    }
  
  if (config.wait_method == W_SYSTICKHOOK || config.wait_method == W_TIMERHOOK)
    {
      config.threads = 1;
    }
  
  /* If the priority was not loaded, default to number of threads. */

  if (config.prio == 0)
    {
      config.prio = config.threads;
    }
  
  if (config.wait_method >= W_COUNT)
    {
      return false;     
    }
  
  if (config.meas_method >= M_COUNT)
    {
      return false;
    } 
  
  return true;  
}

static inline void tsnorm(struct timespec *ts)
{
	while (ts->tv_nsec >= NSEC_PER_SEC)
    {
      ts->tv_nsec -= NSEC_PER_SEC;
      ts->tv_sec++;
    }
}

static inline int64_t timediff_us(struct timespec t1, struct timespec t2)
{
  int64_t ret;
  ret = 1000000 * (int64_t) ((int) t1.tv_sec - (int) t2.tv_sec);
  ret += (int64_t) ((int) t1.tv_nsec - (int) t2.tv_nsec);
  return ret;
}

static void *testthread(void *arg)
{
  int ret;
  int64_t diff = 0;
  struct thread_param_s *param = (struct thread_param_s*)arg;
  struct thread_stats_s *stats = param->stats;
  struct timespec now, next, interval;
  struct sched_param schedp;

  stats->min = LONG_MAX;
  interval.tv_sec = param->interval / 1000000;
  interval.tv_nsec = (param->interval % 1000000) * 1000;

  /* Set priority and policy */

  schedp.sched_priority = param->prio;
  if ((ret = sched_setscheduler(0, param->policy, &schedp)) < 0)
    {
      goto threadend;
    }

  switch (config.meas_method)
    {
      case M_GETTIME:
        clock_gettime(param->clock, &now);
        next = now;
        next.tv_sec += interval.tv_sec;
        next.tv_nsec += interval.tv_nsec;
        tsnorm(&next);
        break;
      case M_TIMER_API:
        break;
      default:
        break;
    }

  while (running)
    {
      switch (config.wait_method)
        {
          case W_NANOSLEEP:
            if ((ret = clock_nanosleep(param->clock, TIMER_ABSTIME, &next, NULL)) < 0)
              {
                goto threadend;
              }
            break;
#ifdef CONFIG_SYSTEMTICK_HOOK
          case W_SYSTICKHOOK:
            if ((ret = sem_wait(&g_waitsem)) < 0)
              {
                goto threadend;
              }
            break;
#endif
          case W_TIMERHOOK:
            if ((ret = poll(polltimer, 1, -1)) < 0)
              {
                goto threadend;
              }
            break;
          default:
            break;
        }


      switch (config.meas_method)
        {
          case M_GETTIME:
            if ((ret = clock_gettime(param->clock, &now)) < 0)
              {
                goto threadend;
              }
            diff = timediff_us(now, next);
            break;
          case M_TIMER_API:
            break;
          default:
            break;
        }

      
      if (diff < stats->min)
        {
          stats->min = diff;
        }
      if (diff > stats->max)
        {
          stats->max = diff;
        }
      stats->avg += (double) diff;

      if (config.histogram)
        {
          if (diff < config.histogram)
            {
              stats->hist_array[diff] += 1;
            }
          else
            {
              stats->hist_overflow += 1;
            }
        }  

      if (config.wait_method == W_NANOSLEEP)
        {
          next.tv_sec += interval.tv_sec;
          next.tv_nsec += interval.tv_nsec;
          tsnorm(&next);
        }
    }

  return NULL;

threadend:
  return NULL;
}

static inline void init_thread_param(struct thread_param_s *param, 
                                     unsigned long interval,
                                     unsigned long max_cycles,
                                     int policy,
                                     int prio,
                                     struct thread_stats_s *stats,
                                     int clock)
{
  param->interval = interval;
  param->max_cycles = max_cycles;
  param->policy = policy;
  param->prio = prio;
  param->stats = stats;
  param->clock = clock;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, char *argv[])
{
  int i;
  running = true;
  struct thread_param_s **params = NULL;
  struct thread_stats_s **stats = NULL;

  config.clock = CLOCK_MONOTONIC;
  config.distance = 500;
  config.duration = 0;
  config.histogram = 0;
  config.interval = 1000;
  config.loops = 0;
  config.threads = 1;
  config.prio = 0;
  config.policy = SCHED_FIFO;
  config.meas_method = M_GETTIME;
  config.wait_method = W_NANOSLEEP;
  config.timer_dev = NULL;

  if (!parse_args(argc, argv))
    {
      print_help();
      return ERROR;
    }
  
  if (!check_args_logic())
    {
      print_help();
      return ERROR;
    }

  /* Timer must be opened */

  if (config.wait_method == W_TIMERHOOK || config.meas_method == M_TIMER_API)
    {
      timerfd = open(config.timer_dev, O_RDWR);
      if (timerfd < 0)
        {
          perror("open timer");
          return ERROR;
        }
    }
  
  /* Configure the poll to wait for a timer notification */

  if (config.wait_method == W_TIMERHOOK)
    {
      polltimer[0].fd     = timerfd;
      polltimer[0].events = POLLIN;
    }

  params = calloc(config.threads, sizeof(struct thread_param_s*));
  if (params == NULL)
    {
      perror("params");
      goto main_error;
    }
  stats  = calloc(config.threads, sizeof(struct thread_stats_s*));
  if (stats == NULL)
    {
      perror("stats");
      goto main_error;
    }

  for (i = 0; i < config.threads; ++i)
    {
      params[i] = malloc(sizeof(struct thread_param_s));
      if (params == NULL)
        {
          perror("params[i]");
          goto main_error;
        }
      stats[i]  = malloc(sizeof(struct thread_stats_s));
      if (params == NULL)
        {
          perror("stats[i]");
          goto main_error;
        }
      stats[i]->hist_array = calloc(config.histogram, sizeof(long));
      if (stats[i]->hist_array == NULL)
        {
          perror("hist_array");
          goto main_error;
        }
      init_thread_param(params[i], config.interval, config.loops,
                 config.policy, config.prio, stats[i], config.clock);

      pthread_create(&stats[i]->id, NULL, testthread, &stats[i]);
      config.interval += config.distance;
      if (config.prio > 0)
        {
          config.prio--;
        }
    }

  
  while (running)
    {
      usleep(100*1000);
    }

  return OK;

main_error:
  if (stats != NULL)
    {
      for (i = 0; i < config.threads; ++i)
        {
          if (params[i] != NULL)
            {
              free(params[i]);
            }
          if (stats[i] != NULL)
            {
              if (stats[i]->hist_array != NULL)
                {
                  free(stats[i]->hist_array);
                }
              free(stats[i]);
            }
        }
    }
  free(stats);

  return ERROR;
}
