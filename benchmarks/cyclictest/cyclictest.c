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

#include <stdio.h>
#include <getopt.h>
#include <assert.h>
#include <errno.h>
#include <nuttx/spinlock.h>
#include <time.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static struct option optargs[] =
{
  {"clock", optional_argument, 0, 'c'},
  {"distance", optional_argument, 0, 'd'},
  {"histogram", optional_argument, 0, 'h'},
  {"interval", optional_argument, 0, 'i'},
  {"loops", optional_argument, 0, 'l'},
  {"threads", optional_argument, 0, 't'}
};

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, char *argv[])
{
  printf("Hello cyclictest!\n");
  return OK;
}
