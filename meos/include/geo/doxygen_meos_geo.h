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
 * @defgroup meos_geo_base Functions for static geometries
 * @ingroup meos_geo
 * @brief Functions for static geometries
 *
 * @defgroup meos_geo_set Functions for geometry sets
 * @ingroup meos_geo
 * @brief Functions for geometry sets
 *
 * @defgroup meos_geo_box Functions for spatiotemporal boxes
 * @ingroup meos_geo
 * @brief Functions for spatiotemporal boxes
 *
 * @defgroup meos_geo_inout Input and output functions
 * @ingroup meos_geo
 * @brief Input and output functions for temporal geometries
 *
 * @defgroup meos_geo_constructor Constructor functions
 * @ingroup meos_geo
 * @brief Constructor functions for temporal geometries
 *
 * @defgroup meos_geo_conversion Conversion functions
 * @ingroup meos_geo
 * @brief Conversion functions for temporal geometries
 *
 * @defgroup meos_geo_accessor Accessor functions
 * @ingroup meos_geo
 * @brief Accessor functions for temporal geometries
 *
 * @defgroup meos_geo_transf Transformation functions
 * @ingroup meos_geo
 * @brief Transformation functions for temporal geometries
 *
 * @defgroup meos_geo_restrict Restriction functions
 * @ingroup meos_geo
 * @brief Restriction functions for temporal geometries
 *
 * @defgroup meos_geo_comp Comparison functions
 * @ingroup meos_geo
 * @brief Comparison functions for temporal geometries
 *
 *   @defgroup meos_geo_comp_ever Ever and always comparison functions
 *   @ingroup meos_geo_comp
 *   @brief Ever and always comparison functions for temporal geometries
 *
 *   @defgroup meos_geo_comp_temp Temporal comparison functions
 *   @ingroup meos_geo_comp
 *   @brief Temporal comparison functions for temporal geometries
 *
 * @defgroup meos_geo_bbox Bounding box functions
 * @ingroup meos_geo
 * @brief Bounding box functions for temporal geometries
 *
 *   @defgroup meos_geo_bbox_topo Topological functions
 *   @ingroup meos_geo_bbox
 *   @brief Topological functions for temporal geometries
 *
 *   @defgroup meos_geo_bbox_pos Position functions
 *   @ingroup meos_geo_bbox
 *   @brief Position functions for temporal geometries
 *
 * @defgroup meos_geo_dist Distance functions
 * @ingroup meos_geo
 * @brief Distance functions for temporal geometries
 *
 * @defgroup meos_geo_srid Spatial reference system functions
 * @ingroup meos_geo
 * @brief Spatial reference system functions for temporal geometries
 *
 * @defgroup meos_geo_rel Spatial relationship functions
 * @ingroup meos_geo
 * @brief Spatial relationship functions for temporal geometries
 *
 *   @defgroup meos_geo_rel_ever Ever/always relationship functions
 *   @ingroup meos_geo_rel
 *   @brief Ever/always relationship functions for temporal geometries
 *
 *   @defgroup meos_geo_rel_temp Temporal relationship functions
 *   @ingroup meos_geo_rel
 *   @brief Temporal relationship functions for temporal geometries
 *
 * @defgroup meos_geo_agg Aggregate functions
 * @ingroup meos_geo
 * @brief Aggregate functions for temporal geometries
 *
 * @defgroup meos_geo_tile Tile functions
 * @ingroup meos_geo
 * @brief Tile functions for temporal geometries
 */

/*****************************************************************************/

/**
 * @defgroup meos_geo_base_inout Input and output functions
 * @ingroup meos_geo_base
 * @brief Input and output functions for static geometries
 *
 * @defgroup meos_geo_base_constructor Constructor functions
 * @ingroup meos_geo_base
 * @brief Constructor functions for static geometries
 *
 * @defgroup meos_geo_base_conversion Conversion functions
 * @ingroup meos_geo_base
 * @brief Conversion functions for static geometries
 *
 * @defgroup meos_geo_base_accessor Accessor functions
 * @ingroup meos_geo_base
 * @brief Accessor functions for static geometries
 *
 * @defgroup meos_geo_base_transf Transformation functions
 * @ingroup meos_geo_base
 * @brief Transformation functions for static geometries
 *
 * @defgroup meos_geo_base_srid Spatial reference system functions
 * @ingroup meos_geo_base
 * @brief Spatial reference system functions for temporal geos
 *
 * @defgroup meos_geo_base_spatial Spatial processing functions
 * @ingroup meos_geo_base
 * @brief Spatial processing functions for static geometries
 *
 * @defgroup meos_geo_base_rel Spatial relationship functions
 * @ingroup meos_geo_base
 * @brief Spatial relationship functions for temporal geos
 *
 * @defgroup meos_geo_base_bbox Bounding box functions
 * @ingroup meos_geo_base
 * @brief Bounding box functions for static geometries
 *
 * @defgroup meos_geo_base_comp Comparison functions
 * @ingroup meos_geo_base
 * @brief Comparison functions for static geometries
 */

/*****************************************************************************/

/**
 * @defgroup meos_geo_set_inout Input and output functions
 * @ingroup meos_geo_set
 * @brief Input and output functions for geometry sets
 *
 * @defgroup meos_geo_set_constructor Constructor functions
 * @ingroup meos_geo_set
 * @brief Constructor functions for geometry sets
 *
 * @defgroup meos_geo_set_conversion Conversion functions
 * @ingroup meos_geo_set
 * @brief Conversion functions for geometry sets
 *
 * @defgroup meos_geo_set_accessor Accessor functions
 * @ingroup meos_geo_set
 * @brief Accessor functions for geometry sets
 *
 * @defgroup meos_geo_set_setops Set operations
 * @ingroup meos_geo_set
 * @brief Set operations for geometry sets
 */

/*****************************************************************************/

/**
 * @defgroup meos_geo_box_inout Input and output functions
 * @ingroup meos_geo_box
 * @brief Input and output functions for spatiotemporal boxes
 *
 * @defgroup meos_geo_box_constructor Constructor functions
 * @ingroup meos_geo_box
 * @brief Constructor functions for spatiotemporal boxes
 *
 * @defgroup meos_geo_box_conversion Conversion functions
 * @ingroup meos_geo_box
 * @brief Conversion functions for spatiotemporal boxes
 *
 * @defgroup meos_geo_box_accessor Accessor functions
 * @ingroup meos_geo_box
 * @brief Accessor functions for spatiotemporal boxes
 *
 * @defgroup meos_geo_box_transf Transformation functions
 * @ingroup meos_geo_box
 * @brief Transformation functions for spatiotemporal boxes
 *
 * @defgroup meos_geo_box_srid Spatial reference system functions
 * @ingroup meos_geo_box
 * @brief Spatial reference system functions for spatiotemporal boxes
 *
 * @defgroup meos_geo_box_bbox Bounding box functions
 * @ingroup meos_geo_box
 * @brief Bounding box functions for spatiotemporal boxes
 *
 * @defgroup meos_geo_box_topo Topological functions
 * @ingroup meos_geo_box_bbox
 * @brief Topological functions for spatiotemporal boxes
 *
 * @defgroup meos_geo_box_pos Position functions
 * @ingroup meos_geo_box_bbox
 * @brief Position functions for spatiotemporal boxes
 *
 * @defgroup meos_geo_box_set Set functions
 * @ingroup meos_geo_box
 * @brief Set functions for spatiotemporal boxes
 *
 * @defgroup meos_geo_box_comp Comparison functions
 * @ingroup meos_geo_box
 * @brief Comparison functions for spatiotemporal boxes
 */

/*****************************************************************************/
