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
 * @file
 * @brief General functions for temporal rigid geometries
 */

#ifndef __TRGEO_H__
#define __TRGEO_H__

/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_type.h>
/* MEOS */
#include "general/temporal.h"

/** Symbolic constants for the temporal instant geometry constuctor */
#define WITH_GEOM       true
#define NO_GEOM         false

/*****************************************************************************
 * Miscellaneous functions defined in trgeometry.c
 *****************************************************************************/

extern bool ensure_has_geom(int16 flags);
extern Datum trgeo_geom_p(const Temporal *temp);
extern Datum trgeo_geom(const Temporal *temp);

/* Input/output functions */

extern char *trgeo_out(const Temporal *temp);
extern char *trgeo_wkt_out(const Temporal *temp, int maxdd, bool extended);

/* Constructor functions */

extern TInstant *geo_tposeinst_to_trgeo(const GSERIALIZED *gs,
  const TInstant *inst);
extern TSequence *geo_tposeseq_to_trgeo(const GSERIALIZED *gs,
  const TSequence *seq);
extern TSequenceSet *geo_tposeseqset_to_trgeo(const GSERIALIZED *gs,
  const TSequenceSet *ss);

/* Conversion functions */


/* Accessor functions */

extern Datum trgeo_start_value(const Temporal *temp);
extern Datum trgeo_end_value(const Temporal *temp);
extern bool trgeo_value_n(const Temporal *temp, int n, Datum *result);
extern bool trgeo_value_at_timestamptz(const Temporal *temp, TimestampTz t,
  bool strict, Datum *result);

/* Transformation functions */

extern Temporal *trgeo_round(const Temporal *temp, int maxdd);
extern TInstant *trgeo_to_tinstant(const Temporal *temp);
extern Temporal *trgeo_set_interp(const Temporal *temp, interpType interp);

/*****************************************************************************/

#endif /* __TRGEO_H__ */
