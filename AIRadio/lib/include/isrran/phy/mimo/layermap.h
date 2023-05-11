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

/******************************************************************************
 *  File:         layermap.h
 *
 *  Description:  MIMO layer mapping and demapping.
 *                Single antenna, tx diversity and spatial multiplexing are
 *                supported.
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 6.3.3
 *****************************************************************************/

#ifndef ISRRAN_LAYERMAP_H
#define ISRRAN_LAYERMAP_H

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"

/* Generates the vector of layer-mapped symbols "x" based on the vector of data symbols "d"
 */
ISRRAN_API int isrran_layermap_single(cf_t* d, cf_t* x, int nof_symbols);

ISRRAN_API int isrran_layermap_diversity(cf_t* d, cf_t* x[ISRRAN_MAX_LAYERS], int nof_layers, int nof_symbols);

ISRRAN_API int isrran_layermap_multiplex(cf_t* d[ISRRAN_MAX_CODEWORDS],
                                         cf_t* x[ISRRAN_MAX_LAYERS],
                                         int   nof_cw,
                                         int   nof_layers,
                                         int   nof_symbols[ISRRAN_MAX_CODEWORDS]);

ISRRAN_API int isrran_layermap_type(cf_t*              d[ISRRAN_MAX_CODEWORDS],
                                    cf_t*              x[ISRRAN_MAX_LAYERS],
                                    int                nof_cw,
                                    int                nof_layers,
                                    int                nof_symbols[ISRRAN_MAX_CODEWORDS],
                                    isrran_tx_scheme_t type);

/* Generates the vector of data symbols "d" based on the vector of layer-mapped symbols "x"
 */
ISRRAN_API int isrran_layerdemap_single(cf_t* x, cf_t* d, int nof_symbols);

ISRRAN_API int isrran_layerdemap_diversity(cf_t* x[ISRRAN_MAX_LAYERS], cf_t* d, int nof_layers, int nof_layer_symbols);

ISRRAN_API int isrran_layerdemap_multiplex(cf_t* x[ISRRAN_MAX_LAYERS],
                                           cf_t* d[ISRRAN_MAX_CODEWORDS],
                                           int   nof_layers,
                                           int   nof_cw,
                                           int   nof_layer_symbols,
                                           int   nof_symbols[ISRRAN_MAX_CODEWORDS]);

ISRRAN_API int isrran_layerdemap_type(cf_t*              x[ISRRAN_MAX_LAYERS],
                                      cf_t*              d[ISRRAN_MAX_CODEWORDS],
                                      int                nof_layers,
                                      int                nof_cw,
                                      int                nof_layer_symbols,
                                      int                nof_symbols[ISRRAN_MAX_CODEWORDS],
                                      isrran_tx_scheme_t type);

ISRRAN_API int isrran_layermap_nr(cf_t** d, int nof_cw, cf_t** x, int nof_layers, uint32_t nof_re);

ISRRAN_API int isrran_layerdemap_nr(cf_t** d, int nof_cw, cf_t** x, int nof_layers, uint32_t nof_re);

#endif // ISRRAN_LAYERMAP_H
