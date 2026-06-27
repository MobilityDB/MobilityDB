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
 * @brief Convenience casts for `th3index`.
 *
 * `h3-pg` exposes plain-SQL `h3index :: point`, `h3index :: geometry`
 * and `h3index :: geography`. The temporal counterparts reuse the
 * lifted `h3_cell_to_latlng` conversion defined in
 * `282_th3index_latlng.in.sql` — these casts are pure sugar for the
 * explicit call.
 *
 * Casts are EXPLICIT (not IMPLICIT nor ASSIGNMENT): typing a cell as
 * a point should not happen by accident. Matches h3-pg's own cast
 * direction.
 *
 * The `th3index :: tbigint` and `tbigint :: th3index` casts are
 * already declared in `270_th3index.in.sql` as ASSIGNMENT and are
 * not re-declared here.
 */

/******************************************************************************
 * th3index :: tgeogpoint
 ******************************************************************************/

CREATE CAST (th3index AS tgeogpoint) WITH FUNCTION h3_cell_to_latlng(th3index);

/******************************************************************************
 * th3index :: tgeompoint
 ******************************************************************************/

CREATE CAST (th3index AS tgeompoint)
  WITH FUNCTION h3_cell_to_latlng_tgeompoint(th3index);

/******************************************************************************/
