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
 * @file tpoint_out.h
 * Output of temporal points in WKT, EWKT and MF-JSON format
 */

#ifndef __TPOINT_OUT_H__
#define __TPOINT_OUT_H__

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <catalog/pg_type.h>
/* MobilityDB */
#include "general/temporal.h"

/*****************************************************************************/

extern char *ewkt_out(Oid typid, Datum value);

extern char *tpoint_as_text(const Temporal *temp);
extern char *tpoint_as_ewkt(const Temporal *temp);
extern char **geoarr_as_text(const Datum *geoarr, int count, bool extended);
extern char **tpointarr_as_text(const Temporal **temparr, int count,
  bool extended);
extern char *tpointinst_as_mfjson(const TInstant *inst, int precision,
  const STBOX *bbox, char *srs);
extern char *tpointinstset_as_mfjson(const TInstantSet *ti, int precision,
  const STBOX *bbox, char *srs);
extern char *tpointseq_as_mfjson(const TSequence *seq, int precision,
  const STBOX *bbox, char *srs);
extern char *tpointseqset_as_mfjson(const TSequenceSet *ts, int precision,
  const STBOX *bbox, char *srs);
extern char *tpoint_as_mfjson(const Temporal *temp, int precision,
  int has_bbox, char *srs);
extern uint8_t *tpoint_to_wkb(const Temporal *temp, uint8_t variant,
  size_t *size_out);
extern char *tpoint_as_hexewkb(const Temporal *temp, uint8_t variant,
  size_t *size);

/*****************************************************************************/

#endif
