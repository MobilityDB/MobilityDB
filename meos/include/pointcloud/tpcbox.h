/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Internal declarations for the TPCBox bounding-box type.
 */

#ifndef __TPCBOX_H__
#define __TPCBOX_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_pointcloud.h>

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

#define DatumGetTpcboxP(X)         ((TPCBox *) DatumGetPointer(X))
#define TpcboxPGetDatum(X)         PointerGetDatum(X)
#define PG_GETARG_TPCBOX_P(X)      DatumGetTpcboxP(PG_GETARG_DATUM(X))
#define PG_RETURN_TPCBOX_P(X)      return TpcboxPGetDatum(X)

/*****************************************************************************/

/* Validity functions */

extern bool ensure_valid_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);

/* Bounding box operators */

extern bool tpcbox_contains(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_contained(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_overlaps(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_same(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_adjacent(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_left(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_right(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_overleft(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_overright(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_below(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_above(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_overbelow(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_overabove(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_front(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_back(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_overfront(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_overback(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_before(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_after(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_overbefore(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_overafter(const TPCBox *box1, const TPCBox *box2);

/* Input/output functions */

extern TPCBox *tpcbox_parse(const char **str);

/*****************************************************************************/

#endif /* __TPCBOX_H__ */
