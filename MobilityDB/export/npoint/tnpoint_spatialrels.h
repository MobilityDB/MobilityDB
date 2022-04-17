/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @file tnpoint_spatialrels.h
 * Spatial relationships for temporal network points.
 */

#ifndef __TNPOINT_SPATIALRELS_H__
#define __TNPOINT_SPATIALRELS_H__

/* PostgreSQL */
#include <postgres.h>

/*****************************************************************************/

extern Datum Contains_geo_tnpoint(PG_FUNCTION_ARGS);

extern Datum Disjoint_geo_tnpoint(PG_FUNCTION_ARGS);
extern Datum Disjoint_npoint_tnpoint(PG_FUNCTION_ARGS);
extern Datum Disjoint_tnpoint_geo(PG_FUNCTION_ARGS);
extern Datum Disjoint_tnpoint_npoint(PG_FUNCTION_ARGS);

extern Datum Intersects_geo_tnpoint(PG_FUNCTION_ARGS);
extern Datum Intersects_npoint_tnpoint(PG_FUNCTION_ARGS);
extern Datum Intersects_tnpoint_geo(PG_FUNCTION_ARGS);
extern Datum Intersects_tnpoint_npoint(PG_FUNCTION_ARGS);

extern Datum Touches_geo_tnpoint(PG_FUNCTION_ARGS);
extern Datum Touches_npoint_tnpoint(PG_FUNCTION_ARGS);
extern Datum Touches_tnpoint_geo(PG_FUNCTION_ARGS);
extern Datum Touches_tnpoint_npoint(PG_FUNCTION_ARGS);

extern Datum Dwithin_geo_tnpoint(PG_FUNCTION_ARGS);
extern Datum Dwithin_npoint_tnpoint(PG_FUNCTION_ARGS);
extern Datum Dwithin_tnpoint_geo(PG_FUNCTION_ARGS);
extern Datum Dwithin_tnpoint_npoint(PG_FUNCTION_ARGS);
extern Datum Dwithin_tnpoint_tnpoint(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif /* __TNPOINT_SPATIALRELS_H__ */
