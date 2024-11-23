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
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <nuttx/spinlock.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct cyclictest_config_s
{
  int clock;
  int distance;
  int histogram;
  unsigned long interval;
  unsigned long loops;
  int threads;
  int policy;
  int prio;
};

struct thread_param_s
{
  int prio;
  int policy;
  unsigned long interval;
  unsigned long max_cycles;
  struct thread_stats_s *stats;
};

struct thread_stats_s
{
  long *hist_array;
  long min;
  long max;
  pthread_t id;
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static struct cyclictest_config_s config;

static const struct option optargs[] =
{
  {"clock", optional_argument, 0, 'c'},
  {"distance", optional_argument, 0, 'd'},
  {"histogram", optional_argument, 0, 'h'},
  {"interval", optional_argument, 0, 'i'},
  {"loops", optional_argument, 0, 'l'},
  {"threads", optional_argument, 0, 't'},
  {"policy", optional_argument, 0, 'y'}
};

static void print_help(void)
{
  puts(
    "The Cyclictest Benchmark Utility\n"
    "Usage:\n"
    "  -c --clock [CLOCK]: selects the clock: 0 selects CLOCK_REALTIME, "
    "1 selects CLOCK_MONOTONIC\n"
    "  -d --distance [US]: The distance of thread intervals. Default is 500us.\n"
    "  -h --histogram [US]: Output the histogram data to stdout. US is the maximum"
    "value to be printed.\n"
    "  -i --interval [US]: The thread interval. Default is 1000us.\n"
    "  -l --loops [N]: The number of measurement loops. Default is 0 (endless).\n"
    "  -t --threads [N]: The number of test threads to be created. Default is 1.\n"
    "  -y --policy [NAME]: Set the scheduler policy, where NAME is fifo or rr."
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
  while ((opt = getopt_long(argc, argv, "c:d:h:i:l:t:", optargs, &longindex)) != -1) {
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
      case 'h':
        printf("%d\n", config.histogram);
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

static void *testthread(void *arg)
{
  struct thread_param_s *param = (struct thread_param_s*)arg;
  struct timespec now, next; 

  return NULL;
}

static inline void init_thread_param(struct thread_param_s *param, 
                                     unsigned long interval,
                                     unsigned long max_cycles,
                                     int policy,
                                     int prio,
                                     struct thread_stats_s *stats)
{
  param->interval = interval;
  param->max_cycles = max_cycles;
  param->policy = policy;
  param->prio = prio;
  param->stats = stats;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, char *argv[])
{
  int i;
  bool running = true;
  struct thread_param_s **params = NULL;
  struct thread_stats_s **stats = NULL;

  config.clock = CLOCK_MONOTONIC;
  config.distance = 500;
  config.histogram = 0;
  config.interval = 1000;
  config.loops = 0;
  config.threads = 1;
  config.prio = 0;
  config.policy = SCHED_FIFO;

  if (!parse_args(argc, argv))
    {
      print_help();
      return ERROR;
    }

  /* If the priority was not loaded, default to number of threads. */
  if (config.prio == 0)
    {
      config.prio = config.threads;
    }

  params = malloc(config.threads * sizeof(struct thread_param_s*));
  if (params == NULL)
    {
      perror("params");
      goto main_error;
    }
  stats  = malloc(config.threads * sizeof(struct thread_stats_s*));
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
      init_thread_param(params[i], config.interval, config.loops, config.policy, config.prio, stats[i]);

      config.interval += config.distance;
      config.prio--;
    }

  
  while (running)
    {
      usleep(100*1000);
    }

  return OK;

main_error:


}
