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

#ifndef ISRRAN_CONFIG_H
#define ISRRAN_CONFIG_H

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
#define ISRRAN_IMPORT __declspec(dllimport)
#define ISRRAN_EXPORT __declspec(dllexport)
#define ISRRAN_LOCAL
#else
#if __GNUC__ >= 4
#define ISRRAN_IMPORT __attribute__((visibility("default")))
#define ISRRAN_EXPORT __attribute__((visibility("default")))
#else
#define ISRRAN_IMPORT
#define ISRRAN_EXPORT
#define ISRRAN_LOCAL
#endif
#endif

// Define ISRRAN_API
// ISRRAN_API is used for the public API symbols.
#ifdef ISRRAN_DLL_EXPORTS // defined if we are building the ISRRAN DLL (instead of using it)
#define ISRRAN_API ISRRAN_EXPORT
#else
#define ISRRAN_API ISRRAN_IMPORT
#endif

// Useful macros for templates
#define CONCAT(a, b) a##b
#define CONCAT2(a, b) CONCAT(a, b)

#define STRING2(x) #x
#define STRING(x) STRING2(x)

// Common error codes
#define ISRRAN_SUCCESS 0
#define ISRRAN_ERROR -1
#define ISRRAN_ERROR_INVALID_INPUTS -2
#define ISRRAN_ERROR_TIMEOUT -3
#define ISRRAN_ERROR_INVALID_COMMAND -4
#define ISRRAN_ERROR_OUT_OF_BOUNDS -5
#define ISRRAN_ERROR_CANT_START -6
#define ISRRAN_ERROR_ALREADY_STARTED -7
#define ISRRAN_ERROR_RX_EOF -8

// cf_t definition
typedef _Complex float cf_t;

#ifdef ENABLE_C16
typedef _Complex short int c16_t;
#endif /* ENABLE_C16 */

#endif // ISRRAN_CONFIG_H
