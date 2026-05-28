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
 * @brief Public MEOS API for the temporal H3 index type (th3index).
 *
 * Implementations live in meos/src/h3/. The PG V1 wrappers in
 * mobilitydb/src/h3/ call these symbols.
 */

#ifndef __MEOS_H3_H__
#define __MEOS_H3_H__

#include <stdbool.h>
#include <stdint.h>
#include <h3api.h>
#include <meos.h>
#include <meos_geo.h>

/*****************************************************************************
 * Static h3index SQL type — analogue of meos_cbuffer.h's
 * static-cbuffer section.
 *****************************************************************************/

extern H3Index h3index_in(const char *str);
extern char *h3index_out(H3Index cell);

/*****************************************************************************
 * Type inheritance (analogue of meos_cbuffer.h's tcbuffer section)
 *****************************************************************************/

/* Input */
extern Temporal *th3index_in(const char *str);
extern TInstant *th3indexinst_in(const char *str);
extern TSequence *th3indexseq_in(const char *str, interpType interp);
extern TSequenceSet *th3indexseqset_in(const char *str);

/* Constructors */
extern Temporal *th3index_make(H3Index value, TimestampTz t);
extern TInstant *th3indexinst_make(H3Index value, TimestampTz t);
extern TSequence *th3indexseq_make(const H3Index *values,
  const TimestampTz *times, int count, bool lower_inc, bool upper_inc);
extern TSequenceSet *th3indexseqset_make(const TSequence **sequences, int count);

/* Accessors */
extern H3Index th3index_start_value(const Temporal *temp);
extern H3Index th3index_end_value(const Temporal *temp);
extern bool th3index_value_n(const Temporal *temp, int n, H3Index *result);
extern H3Index *th3index_values(const Temporal *temp, int *count);
extern bool th3index_value_at_timestamptz(const Temporal *temp, TimestampTz t,
  bool strict, H3Index *result);

/* MEOS-level conversions to/from tbigint */
extern Temporal *tbigint_to_th3index(const Temporal *temp);
extern Temporal *th3index_to_tbigint(const Temporal *temp);

/*****************************************************************************
 * Ever/always comparison operators
 *****************************************************************************/

extern int ever_eq_h3index_th3index(H3Index cell, const Temporal *temp);
extern int ever_eq_th3index_h3index(const Temporal *temp, H3Index cell);
extern int ever_ne_h3index_th3index(H3Index cell, const Temporal *temp);
extern int ever_ne_th3index_h3index(const Temporal *temp, H3Index cell);
extern int always_eq_h3index_th3index(H3Index cell, const Temporal *temp);
extern int always_eq_th3index_h3index(const Temporal *temp, H3Index cell);
extern int always_ne_h3index_th3index(H3Index cell, const Temporal *temp);
extern int always_ne_th3index_h3index(const Temporal *temp, H3Index cell);
extern int ever_eq_th3index_th3index(const Temporal *temp1,
  const Temporal *temp2);
extern int ever_ne_th3index_th3index(const Temporal *temp1,
  const Temporal *temp2);
extern int always_eq_th3index_th3index(const Temporal *temp1,
  const Temporal *temp2);
extern int always_ne_th3index_th3index(const Temporal *temp1,
  const Temporal *temp2);

/*****************************************************************************
 * Temporal comparison operators
 *****************************************************************************/

extern Temporal *teq_h3index_th3index(H3Index cell, const Temporal *temp);
extern Temporal *teq_th3index_h3index(const Temporal *temp, H3Index cell);
extern Temporal *teq_th3index_th3index(const Temporal *temp1,
  const Temporal *temp2);
extern Temporal *tne_h3index_th3index(H3Index cell, const Temporal *temp);
extern Temporal *tne_th3index_h3index(const Temporal *temp, H3Index cell);
extern Temporal *tne_th3index_th3index(const Temporal *temp1,
  const Temporal *temp2);

/*****************************************************************************
 * Inspection
 *****************************************************************************/

extern Temporal *th3index_get_resolution(const Temporal *temp);
extern Temporal *th3index_get_base_cell_number(const Temporal *temp);
extern Temporal *th3index_is_valid_cell(const Temporal *temp);
extern Temporal *th3index_is_res_class_iii(const Temporal *temp);
extern Temporal *th3index_is_pentagon(const Temporal *temp);

/*****************************************************************************
 * Hierarchy
 *****************************************************************************/

extern Temporal *th3index_cell_to_parent(const Temporal *temp, int32 resolution);
extern Temporal *th3index_cell_to_parent_next(const Temporal *temp);
extern Temporal *th3index_cell_to_center_child(const Temporal *temp, int32 resolution);
extern Temporal *th3index_cell_to_center_child_next(const Temporal *temp);
extern Temporal *th3index_cell_to_child_pos(const Temporal *temp, int32 parent_res);
extern Temporal *th3index_child_pos_to_cell(const Temporal *child_pos,
  const Temporal *parent, int32 child_res);

/*****************************************************************************
 * Lat/Lng conversion
 *****************************************************************************/

extern Temporal *tgeogpoint_to_th3index(const Temporal *temp, int32 resolution);
extern Temporal *tgeompoint_to_th3index(const Temporal *temp, int32 resolution);
extern Temporal *th3index_to_tgeogpoint(const Temporal *temp);
extern Temporal *th3index_to_tgeompoint(const Temporal *temp);
extern Temporal *th3index_cell_to_boundary(const Temporal *temp);

/* Static geometry → H3 cell / cell set.  See meos/src/h3/h3_geo.c. */
extern H3Index h3_gs_point_to_cell(const GSERIALIZED *point, int32 resolution);
extern Set    *geo_to_h3index_set(const GSERIALIZED *gs,    int32 resolution);
extern int     ever_eq_h3indexset_th3index(const Set *cells,
                                                  const Temporal *th3idx);

/*****************************************************************************
 * Directed edges
 *****************************************************************************/

extern Temporal *th3index_are_neighbor_cells(const Temporal *origin,
  const Temporal *dest);
extern Temporal *th3index_cells_to_directed_edge(const Temporal *origin,
  const Temporal *dest);
extern Temporal *th3index_is_valid_directed_edge(const Temporal *edge);
extern Temporal *th3index_get_directed_edge_origin(const Temporal *edge);
extern Temporal *th3index_get_directed_edge_destination(const Temporal *edge);
extern Temporal *th3index_directed_edge_to_boundary(const Temporal *edge);

/*****************************************************************************
 * Vertices
 *****************************************************************************/

extern Temporal *th3index_cell_to_vertex(const Temporal *temp, int32 vertex_num);
extern Temporal *th3index_vertex_to_latlng(const Temporal *temp);
extern Temporal *th3index_is_valid_vertex(const Temporal *temp);

/*****************************************************************************
 * Grid traversal
 *****************************************************************************/

extern Temporal *th3index_grid_distance(const Temporal *origin,
  const Temporal *dest);
extern Temporal *th3index_cell_to_local_ij(const Temporal *origin,
  const Temporal *cell);
extern Temporal *th3index_local_ij_to_cell(const Temporal *origin,
  const Temporal *coord);

/*****************************************************************************
 * Metrics
 *****************************************************************************/

extern Temporal *th3index_cell_area(const Temporal *temp, const char *unit);
extern Temporal *th3index_edge_length(const Temporal *temp, const char *unit);
extern Temporal *tgeogpoint_great_circle_distance(const Temporal *a,
  const Temporal *b, const char *unit);

#endif /* __MEOS_H3_H__ */
