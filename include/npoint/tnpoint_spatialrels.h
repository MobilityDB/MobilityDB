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
#include <catalog/pg_type.h>
#include <fmgr.h>
/* MobilityDB */
#include "general/temporal.h"
#include "npoint/tnpoint.h"

/*****************************************************************************/

extern Datum spatialrel_tnpoint_geo(const Temporal *temp, Datum geom,
  Datum (*func)(Datum, Datum), bool invert);
extern Datum spatialrel_tnpoint_npoint(const Temporal *temp, const Npoint *np,
  Datum (*func)(Datum, Datum), bool invert);
extern int spatialrel_tnpoint_tnpoint(const Temporal *temp1,
  const Temporal *temp2, Datum (*func)(Datum, Datum));

extern Datum spatialrel3_tnpoint_geom(const Temporal *temp, Datum geom,
  Datum param, Datum (*func)(Datum, Datum, Datum), bool invert);
extern Datum spatialrel3_tnpoint_npoint(const Temporal *temp, const Npoint *np,
  Datum param, Datum (*func)(Datum, Datum, Datum), bool invert);

extern int dwithin_tnpoint_tnpoint(const Temporal *temp1,
  const Temporal *temp2, Datum param);

/*****************************************************************************/

#endif /* __TNPOINT_SPATIALRELS_H__ */
