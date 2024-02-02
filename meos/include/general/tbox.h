/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief Functions for temporal bounding boxes.
 */

#ifndef __TBOX_H__
#define __TBOX_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>

/*****************************************************************************/

/* fmgr macros temporal types */

#define DatumGetTboxP(X)    ((TBox *) DatumGetPointer(X))
#define TboxPGetDatum(X)    PointerGetDatum(X)
#define PG_GETARG_TBOX_P(n) DatumGetTboxP(PG_GETARG_DATUM(n))
#define PG_RETURN_TBOX_P(x) return TboxPGetDatum(x)

/*****************************************************************************/

/* Parameter tests */

extern bool ensure_has_X_tbox(const TBox *box);
extern bool ensure_has_T_tbox(const TBox *box);
extern bool ensure_same_dimensionality_tbox(const TBox *box1, const TBox *box2);

/* Conversion */

extern void spanset_tbox_slice(Datum psdatum, TBox *box);

/*****************************************************************************/

#endif
