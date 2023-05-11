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

/**
 * @file signal_handler.h
 * @brief Common signal handling methods for all isrRAN applications.
 */

#ifndef ISRRAN_SIGNAL_HANDLER_H
#define ISRRAN_SIGNAL_HANDLER_H

using isrran_signal_hanlder = void (*)();

/// Registers the specified function to be called when the user interrupts the program execution (eg: via Ctrl+C).
/// Passing a null function pointer disables the current installed handler.
void isrran_register_signal_handler(isrran_signal_hanlder handler);

#endif // ISRRAN_SIGNAL_HANDLER_H
