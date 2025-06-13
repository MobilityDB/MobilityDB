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
 * @brief Functions for temporal geos.
 */

#ifndef __TGEO_H__
#define __TGEO_H__

/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include "temporal/meos_catalog.h"

/*****************************************************************************/

/* General functions */

extern void mobilitydb_init(void);

/* Conversion functions */

extern STBox *geo_stbox(const GSERIALIZED *gs);
extern GSERIALIZED *stbox_geo(const STBox *box);

/* Temporal comparisons */

extern Temporal *tcomp_geo_tgeo(const GSERIALIZED *gs,
  const Temporal *temp, Datum (*func)(Datum, Datum, meosType));
extern Temporal *tcomp_tgeo_geo(const Temporal *temp,
  const GSERIALIZED *gs, Datum (*func)(Datum, Datum, meosType));

/*****************************************************************************/

#endif
