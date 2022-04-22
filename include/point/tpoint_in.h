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
 * @file tpoint_in.h
 * Input of temporal points in WKT, EWKT and MF-JSON format
 */

#ifndef __TPOINT_IN_H__
#define __TPOINT_IN_H__

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* JSON-C */
#include <json-c/json.h>
/* MobilityDB */
#include "general/temporal.h"
#include "general/tempcache.h"

/*****************************************************************************/

extern TInstant *tpointinst_from_mfjson(json_object *mfjson, int srid,
  CachedType temptype);
extern TInstantSet *tpointinstset_from_mfjson(json_object *mfjson, int srid,
  CachedType temptype);
extern TSequence *tpointseq_from_mfjson(json_object *mfjson, int srid,
  CachedType temptype, bool linear);
extern TSequenceSet *tpointseqset_from_mfjson(json_object *mfjson, int srid,
  CachedType temptype, bool linear);
extern Temporal *tpoint_from_mfjson_ext(FunctionCallInfo fcinfo,
  text *mfjson_input, CachedType temptype);
extern Temporal *tpoint_from_ewkb(uint8_t *wkb, int size);
extern Temporal *tpoint_from_hexewkb(const char *hexwkb);
extern Temporal *tpoint_from_ewkt(const char *wkt, Oid temptypid);

/*****************************************************************************/

#endif
