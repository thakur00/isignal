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

#ifndef ISRRAN_DFT_H
#define ISRRAN_DFT_H

#include "isrran/config.h"
#include <stdbool.h>

/**********************************************************************************************
 *  File:         dft.h
 *
 *  Description:  Generic DFT module.
 *                Supports one-dimensional complex and real transforms. Options are set
 *                using the dft_plan_set_x functions.
 *
 *                Options (default is false):
 *
 *                mirror - Rearranges negative and positive frequency bins. Swaps after
 *                         transform for FORWARD, swaps before transform for BACKWARD.
 *                db     - Provides output in dB (10*log10(x)).
 *                norm   - Normalizes output (by sqrt(len) for complex, len for real).
 *                dc     - Handles insertion and removal of null DC carrier internally.
 *
 *  Reference:
 *********************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { ISRRAN_DFT_COMPLEX, ISRRAN_REAL } isrran_dft_mode_t;

typedef enum { ISRRAN_DFT_FORWARD, ISRRAN_DFT_BACKWARD } isrran_dft_dir_t;

typedef struct ISRRAN_API {
  int               init_size; // DFT length used in the first initialization
  int               size;      // DFT length
  void*             in;        // Input buffer
  void*             out;       // Output buffer
  void*             p;         // DFT plan
  bool              is_guru;
  bool              forward; // Forward transform?
  bool              mirror;  // Shift negative and positive frequencies?
  bool              db;      // Provide output in dB?
  bool              norm;    // Normalize output?
  bool              dc;      // Handle insertion/removal of null DC carrier internally?
  isrran_dft_dir_t  dir;     // Forward/Backward
  isrran_dft_mode_t mode;    // Complex/Real
} isrran_dft_plan_t;

ISRRAN_API int isrran_dft_plan(isrran_dft_plan_t* plan, int dft_points, isrran_dft_dir_t dir, isrran_dft_mode_t type);

ISRRAN_API int isrran_dft_plan_c(isrran_dft_plan_t* plan, int dft_points, isrran_dft_dir_t dir);

ISRRAN_API int isrran_dft_plan_guru_c(isrran_dft_plan_t* plan,
                                      int                dft_points,
                                      isrran_dft_dir_t   dir,
                                      cf_t*              in_buffer,
                                      cf_t*              out_buffer,
                                      int                istride,
                                      int                ostride,
                                      int                how_many,
                                      int                idist,
                                      int                odist);

ISRRAN_API int isrran_dft_plan_r(isrran_dft_plan_t* plan, int dft_points, isrran_dft_dir_t dir);

ISRRAN_API int isrran_dft_replan(isrran_dft_plan_t* plan, const int new_dft_points);

ISRRAN_API int isrran_dft_replan_guru_c(isrran_dft_plan_t* plan,
                                        const int          new_dft_points,
                                        cf_t*              in_buffer,
                                        cf_t*              out_buffer,
                                        int                istride,
                                        int                ostride,
                                        int                how_many,
                                        int                idist,
                                        int                odist);

ISRRAN_API int isrran_dft_replan_c(isrran_dft_plan_t* plan, int new_dft_points);

ISRRAN_API int isrran_dft_replan_r(isrran_dft_plan_t* plan, int new_dft_points);

ISRRAN_API void isrran_dft_plan_free(isrran_dft_plan_t* plan);

/* Set options */

ISRRAN_API void isrran_dft_plan_set_mirror(isrran_dft_plan_t* plan, bool val);

ISRRAN_API void isrran_dft_plan_set_db(isrran_dft_plan_t* plan, bool val);

ISRRAN_API void isrran_dft_plan_set_norm(isrran_dft_plan_t* plan, bool val);

ISRRAN_API void isrran_dft_plan_set_dc(isrran_dft_plan_t* plan, bool val);

/* Compute DFT */

ISRRAN_API void isrran_dft_run(isrran_dft_plan_t* plan, const void* in, void* out);

ISRRAN_API void isrran_dft_run_c_zerocopy(isrran_dft_plan_t* plan, const cf_t* in, cf_t* out);

ISRRAN_API void isrran_dft_run_c(isrran_dft_plan_t* plan, const cf_t* in, cf_t* out);

ISRRAN_API void isrran_dft_run_guru_c(isrran_dft_plan_t* plan);

ISRRAN_API void isrran_dft_run_r(isrran_dft_plan_t* plan, const float* in, float* out);

#ifdef __cplusplus
}
#endif

#endif // ISRRAN_DFT_H
