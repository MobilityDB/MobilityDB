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
 * @mainpage MEOS and MobilityDB Developer's Documentation
 *
 * \htmlonly
 * <div class="row">
 *   <div class="column">
 *     <img src="meos-logo.png" alt="MEOS logo" width="10%"/>
 *   </div>
 *   <div class="column">
 *     <img src="mobilitydb-logo.png" alt="MobilityDB logo" width="10%"/>
 *   </div>
 * </div>
 * \endhtmlonly
 *
 * @image latex meos-logo.png width=10%
 * @image latex mobilitydb-logo.png width=10%
 *
 * This document defines the developer's documentation of both the MEOS library
 * and MobilityDB. The documentation is divided into modules, where the
 * top-level modules are as follows:
 *
 * - MEOS library
 *   - MEOS API
 *   - Internal functions
 * - MobilityDB API
 *
 * \tableofcontents
 *
 * Please refer to the Modules tab for a detailed account of these modules.
 */

/*****************************************************************************
 * Sections of the external MEOS API
 * These sections follow the ones of the MobilityDB documentation although
 * some subsections are merged into a single one
 *****************************************************************************/

/**
 * @defgroup meos MEOS library
 * @brief Mobility Engine Open Source (MEOS) library
 *
 * @defgroup meos_api External API
 * @ingroup meos
 * @brief External API of the Mobility Engine Open Source (MEOS) library
 *
 * @defgroup meos_internal Internal API
 * @ingroup meos
 * @brief Internal API of the Mobility Engine Open Source (MEOS) library
 */

/*****************************************************************************/

/**
 * @defgroup meos_base_types Functions for base and time types
 * @ingroup meos_api
 * @brief Functions for base and time types
 *
 *   @defgroup meos_base_bool Functions for the boolean type
 *   @ingroup meos_base_types
 *   @brief Functions for the boolean type
 *
 *   @defgroup meos_base_int Functions for the integer types
 *   @ingroup meos_base_types
 *   @brief Functions for the integer types
 *
 *   @defgroup meos_base_float Functions for the float types
 *   @ingroup meos_base_types
 *   @brief Functions for the float types
 *
 *   @defgroup meos_base_text Functions for the text type
 *   @ingroup meos_base_types
 *   @brief Functions for the text types
 *
 *   @defgroup meos_base_numeric Functions for the numeric type
 *   @ingroup meos_base_types
 *   @brief Functions for the numeric type
 *
 *   @defgroup meos_base_date Functions for the date type
 *   @ingroup meos_base_types
 *   @brief Functions for the date type
 *
 *   @defgroup meos_base_time Functions for the time type
 *   @ingroup meos_base_types
 *   @brief Functions for the time type
 *
 *   @defgroup meos_base_timestamp Functions for the timestamp types
 *   @ingroup meos_base_types
 *   @brief Functions for the timestamp types
 *
 *   @defgroup meos_base_interval Functions for the interval type
 *   @ingroup meos_base_types
 *   @brief Functions for the interval types
 *
 *   @defgroup meos_base_json Functions for the JSON types
 *   @ingroup meos_base_types
 *   @brief Functions for the JSON types
 *
 * @defgroup meos_setspan Functions for set and span types
 * @ingroup meos_api
 * @brief Functions for set and span types
 *
 * @defgroup meos_box Functions for temporal boxes
 * @ingroup meos_api
 * @brief Functions for temporal boxes
 *
 * @defgroup meos_temporal Functions for temporal types
 * @ingroup meos_api
 * @brief Functions for temporal types
 *
 * @defgroup meos_geo Functions for temporal geometries
 * @ingroup meos_api
 * @brief Functions for temporal geometries
 *
 * @defgroup meos_cbuffer Functions for temporal circular buffers
 * @ingroup meos_api
 * @brief Functions for temporal temporal circular buffers
 *
 * @defgroup meos_npoint Functions for temporal network points
 * @ingroup meos_api
 * @brief Functions for temporal network points
 *
 * @defgroup meos_pose Functions for temporal poses
 * @ingroup meos_api
 * @brief Functions for temporal poses
 *
 * @defgroup meos_rgeo Functions for temporal rigid geometries
 * @ingroup meos_api
 * @brief Functions for temporal rigid geometries
 *
 * @defgroup meos_misc Miscellaneous functions
 * @ingroup meos_api
 * @brief Miscellaneous functions
 */

/*****************************************************************************/

/**
 * @defgroup meos_setspan_inout Input and output functions
 * @ingroup meos_setspan
 * @brief Input and output functions for set and span types
 *
 * @defgroup meos_setspan_constructor Constructor functions
 * @ingroup meos_setspan
 * @brief Constructor functions for set and span types
 *
 * @defgroup meos_setspan_conversion Conversion functions
 * @ingroup meos_setspan
 * @brief Conversion functions for set and span types
 *
 * @defgroup meos_setspan_accessor Accessor functions
 * @ingroup meos_setspan
 * @brief Accessor functions for set and span types
 *
 * @defgroup meos_setspan_transf Transformation functions
 * @ingroup meos_setspan
 * @brief Transformation functions for set and span types
 *
 * @defgroup meos_setspan_srid Spatial reference system functions
 * @ingroup meos_setspan
 * @brief Spatial reference system functions for set and span types
 *
 * @defgroup meos_setspan_comp Comparison functions
 * @ingroup meos_setspan
 * @brief Comparison functions for set and span types
 *
 * @defgroup meos_setspan_bbox Bounding box functions
 * @ingroup meos_setspan
 * @brief Bounding box functions for set and span types
 *
 *   @defgroup meos_setspan_bbox_split Split functions
 *   @ingroup meos_setspan_bbox
 *   @brief Split functions for set and span types
 *
 *   @defgroup meos_setspan_topo Topological functions
 *   @ingroup meos_setspan_bbox
 *   @brief Topological functions for set and span types
 *
 *   @defgroup meos_setspan_pos Position functions
 *   @ingroup meos_setspan_bbox
 *   @brief Position functions for set and span types
 *
 * @defgroup meos_setspan_set Set functions
 * @ingroup meos_setspan
 * @brief Set functions for set and span types
 *
 * @defgroup meos_setspan_dist Distance functions
 * @ingroup meos_setspan
 * @brief Distance functions for set and span types
 *
 * @defgroup meos_setspan_agg Aggregate functions
 * @ingroup meos_setspan
 * @brief Aggregate functions for set and span types
 *
 * @defgroup meos_setspan_bin Bin functions
 * @ingroup meos_setspan
 * @brief Bin functions for set and span types
 */

/*****************************************************************************/

/**
 * @defgroup meos_box_inout Input and output functions
 * @ingroup meos_box
 * @brief Input and output functions for temporal boxes
 *
 * @defgroup meos_box_constructor Constructor functions
 * @ingroup meos_box
 * @brief Constructor functions for temporal boxes
 *
 * @defgroup meos_box_conversion Conversion functions
 * @ingroup meos_box
 * @brief Conversion functions for temporal boxes
 *
 * @defgroup meos_box_accessor Accessor functions
 * @ingroup meos_box
 * @brief Accessor functions for temporal boxes
 *
 * @defgroup meos_box_transf Transformation functions
 * @ingroup meos_box
 * @brief Transformation functions for temporal boxes
 *
 * @defgroup meos_box_srid Spatial reference system functions
 * @ingroup meos_box
 * @brief Spatial reference system functions for temporal boxes
 *
 * @defgroup meos_box_set Set functions
 * @ingroup meos_box
 * @brief Set functions for temporal boxes
 *
 * @defgroup meos_box_bbox Bounding box functions
 * @ingroup meos_box
 * @brief Bounding box functions for temporal boxes
 *
 *   @defgroup meos_box_bbox_topo Topological functions
 *   @ingroup meos_box_bbox
 *   @brief Topological functions for temporal boxes
 *
 *   @defgroup meos_box_bbox_pos Position functions
 *   @ingroup meos_box_bbox
 *   @brief Position functions for temporal boxes
 *
 * @defgroup meos_box_comp Comparison functions
 * @ingroup meos_box
 * @brief Comparison functions for temporal boxes
 */

/*****************************************************************************/

/**
 * @defgroup meos_temporal_inout Input and output functions
 * @ingroup meos_temporal
 * @brief Input and output functions for temporal types
 *
 * @defgroup meos_temporal_constructor Constructor functions
 * @ingroup meos_temporal
 * @brief Constructor functions for temporal types
 *
 * @defgroup meos_temporal_conversion Conversion functions
 * @ingroup meos_temporal
 * @brief Conversion functions for temporal types
 *
 * @defgroup meos_temporal_accessor Accessor functions
 * @ingroup meos_temporal
 * @brief Accessor functions for temporal types
 *
 * @defgroup meos_temporal_transf Transformation functions
 * @ingroup meos_temporal
 * @brief Transformation functions for temporal types
 *
 * @defgroup meos_temporal_modif Modification functions
 * @ingroup meos_temporal
 * @brief Modification functions for temporal types
 *
 * @defgroup meos_temporal_restrict Restriction functions
 * @ingroup meos_temporal
 * @brief Restriction functions for temporal types
 *
 * @defgroup meos_temporal_comp Comparison functions
 * @ingroup meos_temporal
 * @brief Comparison functions for temporal types
 *
 * @defgroup meos_temporal_comp_trad Traditional comparison functions
 * @ingroup meos_temporal_comp
 * @brief Traditional comparison functions for temporal types
 *
 * @defgroup meos_temporal_comp_ever Ever and always comparison functions
 * @ingroup meos_temporal_comp
 * @brief Ever and always comparison functions for temporal types
 *
 * @defgroup meos_temporal_comp_temp Temporal comparison functions
 * @ingroup meos_temporal_comp
 * @brief Temporal comparison functions for temporal types
 *
 * @defgroup meos_temporal_bbox Bounding box functions
 * @ingroup meos_temporal
 * @brief Bounding box functions for temporal types
 *
 *   @defgroup meos_temporal_bbox_split Split functions
 *   @ingroup meos_temporal_bbox
 *   @brief Split functions for temporal types
 *
 *   @defgroup meos_temporal_bbox_topo Topological functions
 *   @ingroup meos_temporal_bbox
 *   @brief Topological functions for temporal types
 *
 *   @defgroup meos_temporal_bbox_pos Position functions
 *   @ingroup meos_temporal_bbox
 *   @brief Position functions for temporal types
 *
 * @defgroup meos_temporal_bool Boolean functions
 * @ingroup meos_temporal
 * @brief Boolean functions for temporal types
 *
 * @defgroup meos_temporal_math Mathematical functions
 * @ingroup meos_temporal
 * @brief Mathematical functions for temporal types
 *
 * @defgroup meos_temporal_text Text functions
 * @ingroup meos_temporal
 * @brief Text functions for temporal types
 *
 * @defgroup meos_temporal_dist Distance functions
 * @ingroup meos_temporal
 * @brief Distance functions for temporal types
 *
 * @defgroup meos_temporal_agg Aggregate functions
 * @ingroup meos_temporal
 * @brief Aggregate functions for temporal types
 *
 * @defgroup meos_temporal_analytics Analytics functions
 * @ingroup meos_temporal
 * @brief Analytics functions for temporal types
 *
 * @defgroup meos_temporal_analytics_simplify Simplification functions
 * @ingroup meos_temporal_analytics
 * @brief Simplification functions for temporal types
 *
 * @defgroup meos_temporal_analytics_reduction Reduction functions
 * @ingroup meos_temporal_analytics
 * @brief Reduction functions for temporal types
 *
 * @defgroup meos_temporal_analytics_similarity Similarity functions
 * @ingroup meos_temporal_analytics
 * @brief Similarity functions for temporal types
 *
 * @defgroup meos_temporal_analytics_tile Tile functions
 * @ingroup meos_temporal_analytics
 * @brief Tile functions for temporal types
 */

/*****************************************************************************/
