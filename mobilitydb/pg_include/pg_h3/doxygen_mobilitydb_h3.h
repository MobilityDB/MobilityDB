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
 * @brief Sections for the documentation of the MobilityDB API: temporal
 * H3 cell indices.
 *
 * Mirrors `doxygen_mobilitydb_cbuffer.h`. Each PG V1 wrapper in
 * `mobilitydb/src/h3/*.c` is attached to one of these subgroups via
 * an `@ingroup mobilitydb_h3_<section>` line in its Doxygen header.
 */

/*****************************************************************************
 * Modules of the MobilityDB API for the th3index type
 *****************************************************************************/

/**
 * @defgroup mobilitydb_h3_base Functions for static H3 cell indices
 * @ingroup mobilitydb_h3
 * @brief Functions for static H3 cell indices
 *
 * @defgroup mobilitydb_h3_inout Input and output functions
 * @ingroup mobilitydb_h3
 * @brief Input and output functions for temporal H3 cell indices
 *
 * @defgroup mobilitydb_h3_constructor Constructor functions
 * @ingroup mobilitydb_h3
 * @brief Constructor functions for temporal H3 cell indices
 *
 * @defgroup mobilitydb_h3_conversion Conversion functions
 * @ingroup mobilitydb_h3
 * @brief Conversion functions for temporal H3 cell indices
 *
 * @defgroup mobilitydb_h3_accessor Accessor functions
 * @ingroup mobilitydb_h3
 * @brief Accessor functions for temporal H3 cell indices
 *
 * @defgroup mobilitydb_h3_inspection Index-inspection functions
 * @ingroup mobilitydb_h3
 * @brief Per-instant inspection of the underlying H3 cell payload
 *
 * @defgroup mobilitydb_h3_hierarchy Hierarchy functions
 * @ingroup mobilitydb_h3
 * @brief Per-instant parent / center-child / child-position helpers
 *
 * @defgroup mobilitydb_h3_latlng Lat/Lng-conversion functions
 * @ingroup mobilitydb_h3
 * @brief Per-instant centroid / boundary / latlng-to-cell helpers
 *
 * @defgroup mobilitydb_h3_edges Directed-edge functions
 * @ingroup mobilitydb_h3
 * @brief Per-instant directed-edge construction and accessors
 *
 * @defgroup mobilitydb_h3_vertex Vertex functions
 * @ingroup mobilitydb_h3
 * @brief Per-instant H3-vertex construction and accessors
 *
 * @defgroup mobilitydb_h3_traversal Grid-traversal functions
 * @ingroup mobilitydb_h3
 * @brief Per-instant grid distance and local-IJ helpers
 *
 * @defgroup mobilitydb_h3_set Set-returning functions
 * @ingroup mobilitydb_h3
 * @brief Static set-returning helpers (h3_grid_disk, h3_cell_to_children,
 * h3_compact_cells, …) surfaced via `mobilitydb/src/h3/h3index_sets.c`
 *
 * @defgroup mobilitydb_h3_metrics Metric functions
 * @ingroup mobilitydb_h3
 * @brief Per-instant area / edge-length / great-circle distance helpers
 *
 * @defgroup mobilitydb_h3_comp Comparison functions
 * @ingroup mobilitydb_h3
 * @brief Comparison functions for temporal H3 cell indices
 *
 *   @defgroup mobilitydb_h3_comp_ever Ever and always comparison functions
 *   @ingroup mobilitydb_h3_comp
 *   @brief Ever / always comparison helpers for temporal H3 cells
 *
 *   @defgroup mobilitydb_h3_comp_temp Temporal comparison functions
 *   @ingroup mobilitydb_h3_comp
 *   @brief Temporal comparison helpers (returning a temporal boolean)
 *
 * @defgroup mobilitydb_h3_bbox Bounding-box operators
 * @ingroup mobilitydb_h3
 * @brief Bounding-box / positional / topology operators on temporal H3
 * cells
 *
 * @defgroup mobilitydb_h3_index Index access methods
 * @ingroup mobilitydb_h3
 * @brief GiST and SP-GiST operator classes for temporal H3 cells
 */

/*****************************************************************************/

/**
 * @defgroup mobilitydb_h3_base_inout Input and output functions
 * @ingroup mobilitydb_h3_base
 * @brief Input and output functions for static H3 cell indices
 *
 * @defgroup mobilitydb_h3_base_accessor Accessor functions
 * @ingroup mobilitydb_h3_base
 * @brief Accessor functions for static H3 cell indices
 *
 * @defgroup mobilitydb_h3_base_comp Comparison functions
 * @ingroup mobilitydb_h3_base
 * @brief Comparison functions for static H3 cell indices
 */

/*****************************************************************************/
