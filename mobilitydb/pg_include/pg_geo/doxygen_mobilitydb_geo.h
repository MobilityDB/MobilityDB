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
 * @brief Sections for the documentation of the MobilityDB API: Geometries
 */

/*****************************************************************************
 * Definition of the modules of the MobilityDB API
 * These modules follow the sections of the MobilityDB documentation although
 * some subsections are merged into a single submodule
 *****************************************************************************/

/**
 * @defgroup mobilitydb_geo_set Functions for geometry sets
 * @ingroup mobilitydb_geo
 * @brief Functions for geometry sets
 *
 *   @defgroup mobilitydb_geo_set_inout Input/output functions
 *   @ingroup mobilitydb_geo_set
 *   @brief Input/output functions for geometry sets
 *
 *   @defgroup mobilitydb_geo_set_conversion Conversion functions
 *   @ingroup mobilitydb_geo_set
 *   @brief Conversion functions for geometry sets
 *
 *   @defgroup mobilitydb_geo_set_transf Transformation functions
 *   @ingroup mobilitydb_geo_set
 *   @brief Transformation functions for geometry sets
 *
 *   @defgroup mobilitydb_geo_set_srid Spatial reference system functions
 *   @ingroup mobilitydb_geo_set
 *   @brief Spatial reference system functions for geometry sets
 *
 * @defgroup mobilitydb_geo_box Functions for spatiotemporal boxes
 * @ingroup mobilitydb_geo
 * @brief Functions for geometry sets
 *
 * @defgroup mobilitydb_geo_inout Input and output functions
 * @ingroup mobilitydb_geo
 * @brief Input and output functions for temporal geometries
 *
 * @defgroup mobilitydb_geo_constructor Constructor functions
 * @ingroup mobilitydb_geo
 * @brief Constructor functions for temporal geometries
 *
 * @defgroup mobilitydb_geo_conversion Conversion functions
 * @ingroup mobilitydb_geo
 * @brief Conversion functions for temporal geometries
 *
 * @defgroup mobilitydb_geo_accessor Accessor functions
 * @ingroup mobilitydb_geo
 * @brief Accessor functions for temporal geometries
 *
 * @defgroup mobilitydb_geo_transf Transformation functions
 * @ingroup mobilitydb_geo
 * @brief Transformation functions for temporal geometries
 *
 * @defgroup mobilitydb_geo_srid Spatial reference system functions
 * @ingroup mobilitydb_geo
 * @brief Spatial reference system functions for temporal geometries
 *
 * @defgroup mobilitydb_geo_restrict Restriction functions
 * @ingroup mobilitydb_geo
 * @brief Restriction functions for temporal geometries
 *
 * @defgroup mobilitydb_geo_modif Modification functions
 * @ingroup mobilitydb_geo
 * @brief Modification functions for temporal geometries
 *
 * @defgroup mobilitydb_geo_bbox Bounding box functions
 * @ingroup mobilitydb_geo
 * @brief Bounding box functions for temporal geometries
 *
 *   @defgroup mobilitydb_geo_bbox_topo Topological functions
 *   @ingroup mobilitydb_geo_bbox
 *   @brief Topological functions for temporal geometries
 *
 *   @defgroup mobilitydb_geo_bbox_pos Position functions
 *   @ingroup mobilitydb_geo_bbox
 *   @brief Position functions for temporal geometries
 *
 * @defgroup mobilitydb_geo_dist Distance functions
 * @ingroup mobilitydb_geo
 * @brief Distance functions for temporal geometries
 *
 * @defgroup mobilitydb_geo_comp Comparison functions
 * @ingroup mobilitydb_geo
 * @brief Comparison functions for temporal geometries
 *
 *   @defgroup mobilitydb_geo_comp_ever Ever and always comparison functions
 *   @ingroup mobilitydb_geo_comp
 *   @brief Ever and always comparison functions for temporal geometries
 *
 *   @defgroup mobilitydb_geo_comp_temp Temporal comparison functions
 *   @ingroup mobilitydb_geo_comp
 *   @brief Comparison functions for temporal geometries
 *
 * @defgroup mobilitydb_geo_rel Spatial relationship functions
 * @ingroup mobilitydb_geo
 * @brief Spatial relationship functions for temporal geometries
 *
 *   @defgroup mobilitydb_geo_rel_ever Ever and always spatial relationship functions
 *   @ingroup mobilitydb_geo_rel
 *   @brief Ever and always spatial relationship functions for temporal geometries
 *
 *   @defgroup mobilitydb_geo_rel_temp Spatiotemporal relationship functions
 *   @ingroup mobilitydb_geo_rel
 *   @brief Spatiotemporal relationship functions for temporal geometries
 *
 * @defgroup mobilitydb_geo_agg Aggregate functions
 * @ingroup mobilitydb_geo
 * @brief Aggregate functions for temporal geometries
 *
 * @defgroup mobilitydb_geo_tile Tile functions
 * @ingroup mobilitydb_geo
 * @brief Tile functions for temporal geometries
 */

/*****************************************************************************/

/**
 * @defgroup mobilitydb_geo_box_inout Input and output functions
 * @ingroup mobilitydb_geo_box
 * @brief Input and output functions for spatiotemporal boxes
 *
 * @defgroup mobilitydb_geo_box_constructor Constructor functions
 * @ingroup mobilitydb_geo_box
 * @brief Constructor functions for spatiotemporal boxes
 *
 * @defgroup mobilitydb_geo_box_conversion Conversion functions
 * @ingroup mobilitydb_geo_box
 * @brief Conversion functions for spatiotemporal boxes
 *
 * @defgroup mobilitydb_geo_box_accessor Accessor functions
 * @ingroup mobilitydb_geo_box
 * @brief Accessor functions for spatiotemporal boxes
 *
 * @defgroup mobilitydb_geo_box_transf Transformation functions
 * @ingroup mobilitydb_geo_box
 * @brief Transformation functions for spatiotemporal boxes
 *
 * @defgroup mobilitydb_geo_box_srid Spatial reference system functions
 * @ingroup mobilitydb_geo_box
 * @brief Spatial reference system functions for spatiotemporal boxes
 *
 * @defgroup mobilitydb_geo_box_bbox Bounding box functions
 * @ingroup mobilitydb_geo_box
 * @brief Bounding box functions for spatiotemporal boxes
 *
 * @defgroup mobilitydb_geo_box_topo Topological functions
 * @ingroup mobilitydb_geo_box_bbox
 * @brief Topological functions for spatiotemporal boxes
 *
 * @defgroup mobilitydb_geo_box_pos Position functions
 * @ingroup mobilitydb_geo_box_bbox
 * @brief Position functions for spatiotemporal boxes
 *
 * @defgroup mobilitydb_geo_box_set Set functions
 * @ingroup mobilitydb_geo_box
 * @brief Set functions for spatiotemporal boxes
 *
 * @defgroup mobilitydb_geo_box_comp Comparison functions
 * @ingroup mobilitydb_geo_box
 * @brief Comparison functions for spatiotemporal boxes
 */

/*****************************************************************************/
