/**
 * Copyright 2013-2022 iSignal Research Labs Pvt Ltd.
 *
 * This file is part of isrRAN.
 *
 * isrRAN is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * isrRAN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "isrran/common/crash_handler.h"

#if HAVE_BACKWARD
#include "isrran/common/backward.hpp"
using namespace backward;
void isrran_debug_handle_crash(int argc, char** argv)
{
  static backward::SignalHandling sh;
}
#else // HAVE_BACKWARD
#include "isrran/common/backtrace.h"
#include "isrran/version.h"

static int    bt_argc;
static char** bt_argv;

static void crash_handler(int sig)
{
  FILE*     f = stderr;
  time_t    lnTime;
  struct tm stTime;
  char      strdate[32];

  time(&lnTime);
  gmtime_r(&lnTime, &stTime);

  strftime(strdate, sizeof(strdate), "%d/%m/%Y %H:%M:%S", &stTime);

  fprintf(f, "--- command='");
  for (int i = 0; i < bt_argc; i++) {
    fprintf(f, "%s%s", (i == 0) ? "" : " ", bt_argv[i]);
  }
  fprintf(f, "' version=%s signal=%d date='%s' ---\n", ISRRAN_VERSION_STRING, sig, strdate);

  isrran_backtrace_print(f);

  fprintf(f, "isrRAN crashed. Please send this backtrace to the developers ...\n");

  fprintf(f, "---  exiting  ---\n");
  exit(1);
}

void isrran_debug_handle_crash(int argc, char** argv)
{
  bt_argc = argc;
  bt_argv = argv;

  signal(SIGSEGV, crash_handler);
  signal(SIGABRT, crash_handler);
  signal(SIGILL, crash_handler);
  signal(SIGFPE, crash_handler);
  signal(SIGPIPE, crash_handler);
}

#endif // HAVE_BACKWARD
