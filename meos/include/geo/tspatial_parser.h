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
 * @brief Functions for parsing temporal points.
 */

#ifndef __TSPATIAL_PARSER_H__
#define __TSPATIAL_PARSER_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include "temporal/meos_catalog.h"

/*****************************************************************************/

extern bool srid_parse(const char **str, int *srid);
extern bool spatial_parse_elem(const char **str, meosType temptype, char delim,
  int *temp_srid, Datum *result);
extern bool geo_parse(const char **str, meosType basetype, char delim, 
  int *srid, GSERIALIZED **result);
extern STBox *stbox_parse(const char **str);
extern Temporal *tpoint_parse(const char **str, meosType temptype);

extern bool tspatialinst_parse(const char **str, meosType temptype, bool end,
  int *temp_srid, TInstant **result);
extern TSequence *tspatialseq_disc_parse(const char **str, meosType temptype,
  int *temp_srid);
extern bool tspatialseq_cont_parse(const char **str, meosType temptype,
  interpType interp, bool end, int *temp_srid, TSequence **result);
extern TSequenceSet *tspatialseqset_parse(const char **str, meosType temptype,
  interpType interp, int *temp_srid);
extern Temporal *tspatial_parse(const char **str, meosType temptype);

/*****************************************************************************/

#endif /* __TSPATIAL_PARSER_H__ */
