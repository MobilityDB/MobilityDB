/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @file
 * @brief Multi-entry support of the temporal rigid geometries: the keys of a
 * value are the spatiotemporal boxes MEOS splits it into
 * @details The extraction methods of their multi-entry classes, as
 * tgeo_mest.c holds those of the temporal geometries; each calls the internal
 * twin of a splitter or a merge of the temporal rigid geometries, whose boxes
 * read the body of the value
 */

#include "pg_temporal/temporal_mest.h"

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_rgeo.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/temporal.h"
#include "rgeo/trgeo_boxops.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"

/*****************************************************************************
 * Extraction of the spatiotemporal boxes of a temporal rigid geometry
 *****************************************************************************/

/* GENERATED-BOXMEST-BEGIN trgeometry — tools/codegen/inherited/generate.py from templates/boxmest.c.tmpl; DO NOT EDIT BY HAND;
 * edit the template + manifest.d/boxsplit.yaml and re-run. */

PG_FUNCTION_INFO_V1(Trgeometry_mest_equisplit);
/**
 * @brief Multi-entry extract method keyed by a number of boxes
 * @details The keys of a temporal rigid geometry are a number of
 * spatiotemporal boxes of its consecutive instants or segments, the number being
 * the option of the class (#mest_count) and the split that of
 * #trgeometry_split_n_stboxes
 */
Datum
Trgeometry_mest_equisplit(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  STBox *boxes = trgeometry_split_n_stboxes_intl(temp, mest_count(fcinfo), nkeys);
  PG_RETURN_POINTER(mest_keys(boxes, sizeof(STBox), *nkeys));
}

PG_FUNCTION_INFO_V1(Trgeometry_mest_segsplit);
/**
 * @brief Multi-entry extract method keyed by the boxes of a number of segments
 * @details The keys of a temporal rigid geometry are the
 * spatiotemporal boxes of a number of its consecutive instants or segments each, the
 * number being the option of the class (#mest_count) and the split that of
 * #trgeometry_split_each_n_stboxes
 */
Datum
Trgeometry_mest_segsplit(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  STBox *boxes = trgeometry_split_each_n_stboxes_intl(temp, mest_count(fcinfo),
    nkeys);
  PG_RETURN_POINTER(mest_keys(boxes, sizeof(STBox), *nkeys));
}

PG_FUNCTION_INFO_V1(Trgeometry_mest_mergesplit);
/**
 * @brief Multi-entry extract method keyed by a number of merged boxes
 * @details The keys of a temporal rigid geometry are a number of
 * spatiotemporal boxes merged from the boxes of its instants or segments, the number
 * being the option of the class (#mest_count) and the merge that of
 * #trgeometry_merge_n_stboxes
 */
Datum
Trgeometry_mest_mergesplit(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  STBox *boxes = trgeometry_merge_n_stboxes_intl(temp, mest_count(fcinfo), nkeys);
  PG_RETURN_POINTER(mest_keys(boxes, sizeof(STBox), *nkeys));
}

PG_FUNCTION_INFO_V1(Trgeometry_mest_adaptsplit);
/**
 * @brief Multi-entry extract method keyed by boxes of a number of segments
 * @details The keys of a temporal rigid geometry are the
 * spatiotemporal boxes merged from the boxes of its instants or segments until a box
 * holds on average a number of them,
 * the number being the option of the class (#mest_count) and the merge that of
 * #trgeometry_merge_each_n_stboxes
 */
Datum
Trgeometry_mest_adaptsplit(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  STBox *boxes = trgeometry_merge_each_n_stboxes_intl(temp, mest_count(fcinfo),
    nkeys);
  PG_RETURN_POINTER(mest_keys(boxes, sizeof(STBox), *nkeys));
}

/* GENERATED-BOXMEST-END trgeometry */
