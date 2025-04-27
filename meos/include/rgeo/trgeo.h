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
/* MEOS */
#include "general/temporal.h"

/** Symbolic constants for the temporal instant geometry constuctor */
#define WITH_GEOM       true
#define NO_GEOM         false

/*****************************************************************************
 * Miscellaneous functions defined in trgeo.c
 *****************************************************************************/

extern bool ensure_has_geom(int16 flags);
extern bool ensure_valid_trgeo_geo(const Temporal *temp,
  const GSERIALIZED *gs);
extern bool ensure_valid_trgeo_stbox(const Temporal *temp,
  const STBox *box);
extern bool ensure_valid_trgeo_trgeo(const Temporal *temp1,
  const Temporal *temp2);
extern bool ensure_valid_trgeo_tpoint(const Temporal *temp1,
  const Temporal *temp2);
extern const GSERIALIZED *trgeo_geom_p(const Temporal *temp);

/* Input/output functions */

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

extern bool trgeo_value_at_timestamptz(const Temporal *temp, TimestampTz t,
  bool strict, Datum *result);

/* Transformation functions */


/*****************************************************************************/

#endif /* __TRGEO_H__ */
