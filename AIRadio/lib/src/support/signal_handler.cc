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

#include "isrran/support/signal_handler.h"
#include "isrran/support/emergency_handlers.h"
#include <atomic>
#include <csignal>
#include <cstdio>
#include <unistd.h>

#ifndef ISRRAN_TERM_TIMEOUT_S
#define ISRRAN_TERM_TIMEOUT_S (5)
#endif

/// Handler called after the user interrupts the program.
static std::atomic<isrran_signal_hanlder> user_handler;

static void isrran_signal_handler(int signal)
{
  switch (signal) {
    case SIGALRM:
      fprintf(stderr, "Couldn't stop after %ds. Forcing exit.\n", ISRRAN_TERM_TIMEOUT_S);
      execute_emergency_cleanup_handlers();
      raise(SIGKILL);
    default:
      // all other registered signals try to stop the app gracefully
      // Call the user handler if present and remove it so that further signals are treated by the default handler.
      if (auto handler = user_handler.exchange(nullptr)) {
        handler();
      } else {
        return;
      }
      fprintf(stdout, "Stopping ..\n");
      alarm(ISRRAN_TERM_TIMEOUT_S);
      break;
  }
}

void isrran_register_signal_handler(isrran_signal_hanlder handler)
{
  user_handler.store(handler);

  signal(SIGINT, isrran_signal_handler);
  signal(SIGTERM, isrran_signal_handler);
  signal(SIGHUP, isrran_signal_handler);
  signal(SIGALRM, isrran_signal_handler);
}
