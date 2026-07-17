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
 * @file
 * @brief Public MEOS API for temporal cell index types
 *
 * The temporal cell index (tcellindex) is the abstract temporal type shared by
 * the concrete cell index families (tquadbin, th3index): its accessors are
 * declared once here and overload on the concrete temporal type in the SQL and
 * binding layers.
 */

#ifndef __MEOS_CELLINDEX_H__
#define __MEOS_CELLINDEX_H__

#include <stdbool.h>
#include <stdint.h>
/* MEOS */
#include <meos.h>

/*****************************************************************************
 * Accessor functions for temporal cell indices
 *****************************************************************************/

extern Temporal *tcellindex_get_resolution(const Temporal *temp);
extern Temporal *tcellindex_is_valid_cell(const Temporal *temp);
extern Temporal *tcellindex_cell_to_parent(const Temporal *temp,
  int32 resolution);
extern Temporal *tcellindex_cell_to_point(const Temporal *temp);
extern Temporal *tcellindex_cell_to_boundary(const Temporal *temp);
extern Temporal *tcellindex_cell_area(const Temporal *temp);

/*****************************************************************************/

#endif /* __MEOS_CELLINDEX_H__ */
