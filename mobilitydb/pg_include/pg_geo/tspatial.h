/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @brief Spatial functions for temporal points.
 */

#ifndef __PG_TSPATIAL_H__
#define __PG_TSPATIAL_H__

/* PostgreSQL */
#include <postgres.h>
#include <utils/array.h>
#include <fmgr.h>

/*****************************************************************************/

/* Fetch from and store in the cache the fcinfo of the external function */
extern FunctionCallInfo fetch_fcinfo(void);
extern void store_fcinfo(FunctionCallInfo fcinfo);

extern Temporal *tspatial_valid_typmod(Temporal *temp, int32_t typmod);
extern uint32 tspatial_typmod_in(ArrayType *arr, int is_point, int is_geodetic);
extern Datum Spatialarr_as_text_ext(FunctionCallInfo fcinfo, bool extended);

extern Datum EA_spatialrel_geo_tspatial(FunctionCallInfo fcinfo,
  int (*func)(const GSERIALIZED *, const Temporal *, bool), bool ever);
extern Datum EA_spatialrel_tspatial_geo(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const GSERIALIZED *, bool), bool ever);
extern Datum EA_spatialrel_tspatial_tspatial(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const Temporal *, bool), bool ever);

extern Datum Tinterrel_geo_tspatial(FunctionCallInfo fcinfo, bool tinter);
extern Datum Tinterrel_tspatial_geo(FunctionCallInfo fcinfo, bool tinter);

extern Datum Tspatialrel_geo_tspatial(FunctionCallInfo fcinfo,
  Temporal * (*func)(const GSERIALIZED *, const Temporal *, bool, bool));
extern Datum Tspatialrel_tspatial_geo(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Temporal *, const GSERIALIZED *, bool, bool));
extern Datum Tspatialrel_tspatial_tspatial(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Temporal *, const Temporal *, bool, bool));

extern Datum EA_dwithin_tspatial_geo(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const GSERIALIZED *, double dist, bool),
  bool ever);
extern Datum EA_dwithin_geo_tspatial(FunctionCallInfo fcinfo,
  int (*func)(const GSERIALIZED *, const Temporal *, double dist, bool),
  bool ever);

/*****************************************************************************/

#endif /* __PG_TSPATIAL_H__ */
