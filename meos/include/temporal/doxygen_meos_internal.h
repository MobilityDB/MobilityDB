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
 * @brief MEOS Developer's Documentation: Internal API
 */

/*****************************************************************************
 * Sections of the MEOS internal API
 *****************************************************************************/

/**
 * @defgroup meos_internal_setspan Functions for set and span types
 * @ingroup meos_internal
 * @brief Functions for set and span types
 *
 * @defgroup meos_internal_box Functions for temporal boxes
 * @ingroup meos_internal
 * @brief Functions for temporal boxes
 *
 * @defgroup meos_internal_temporal Functions for temporal types
 * @ingroup meos_internal
 * @brief Functions for temporal types
 *
 * @defgroup meos_internal_geo Functions for temporal geometries
 * @ingroup meos_internal
 * @brief Functions for temporal geometries
 *
 * @defgroup meos_internal_cbuffer Functions for temporal circular buffers
 * @ingroup meos_internal
 * @brief Functions for temporal circular buffers
 *
 * @defgroup meos_internal_npoint Functions for temporal network points
 * @ingroup meos_internal
 * @brief Functions for temporal network points
 *
 * @defgroup meos_internal_pose Functions for temporal poses
 * @ingroup meos_internal
 * @brief Functions for temporal poses
 *
 * @defgroup meos_internal_rgeo Functions for temporal rigid geometries
 * @ingroup meos_internal
 * @brief Functions for temporal rigid geometries
 */

/*****************************************************************************/

/**
 * @defgroup meos_internal_setspan_inout Input and output functions
 * @ingroup meos_internal_setspan
 * @brief Input and output functions for set and span types
 *
 * @defgroup meos_internal_setspan_constructor Constructor functions
 * @ingroup meos_internal_setspan
 * @brief Constructor functions for set and span types
 *
 * @defgroup meos_internal_setspan_conversion Conversion functions
 * @ingroup meos_internal_setspan
 * @brief Conversion functions for set and span types
 *
 * @defgroup meos_internal_setspan_accessor Accessor functions
 * @ingroup meos_internal_setspan
 * @brief Accessor functions for set and span types
 *
 * @defgroup meos_internal_setspan_transf Transformation functions
 * @ingroup meos_internal_setspan
 * @brief Transformation functions for set and span types
 *
 * @defgroup meos_internal_setspan_bbox Bounding box functions
 * @ingroup meos_internal_setspan
 * @brief Bounding box functions for set and span types
 *
 * @defgroup meos_internal_setspan_topo Topological functions
 * @ingroup meos_internal_setspan_bbox
 * @brief Topological functions for set and span types
 *
 * @defgroup meos_internal_setspan_pos Position functions
 * @ingroup meos_internal_setspan_bbox
 * @brief Position functions for set and span types
 *
 * @defgroup meos_internal_setspan_set Set functions
 * @ingroup meos_internal_setspan
 * @brief Set functions for set and span types
 *
 * @defgroup meos_internal_setspan_dist Distance functions
 * @ingroup meos_internal_setspan
 * @brief Distance functions for set and span types
 *
 * @defgroup meos_internal_setspan_agg Aggregate functions
 * @ingroup meos_internal_setspan
 * @brief Aggregate functions for set and span types
 */

/*****************************************************************************/

/**
 * @defgroup meos_internal_box_constructor Constructor functions
 * @ingroup meos_internal_box
 * @brief Constructor functions for box types
 *
 * @defgroup meos_internal_box_conversion Conversion functions
 * @ingroup meos_internal_box
 * @brief Conversion functions for box types
 *
 * @defgroup meos_internal_box_transf Transformation functions
 * @ingroup meos_internal_box
 * @brief Transformation functions for box types
 *
 * @defgroup meos_internal_box_set Set functions
 * @ingroup meos_internal_box
 * @brief Set functions for box types
  */

/*****************************************************************************/

/**
 * @defgroup meos_internal_temporal_inout Input and output functions
 * @ingroup meos_internal_temporal
 * @brief Input and output functions for temporal types
 *
 * @defgroup meos_internal_temporal_constructor Constructor functions
 * @ingroup meos_internal_temporal
 * @brief Constructor functions for temporal types
 *
 * @defgroup meos_internal_temporal_conversion Conversion functions
 * @ingroup meos_internal_temporal
 * @brief Conversion functions for temporal types
 *
 * @defgroup meos_internal_temporal_accessor Accessor functions
 * @ingroup meos_internal_temporal
 * @brief Accessor functions for temporal types
 *
 * @defgroup meos_internal_temporal_transf Transformation functions
 * @ingroup meos_internal_temporal
 * @brief Transformation functions for temporal types
 *
 * @defgroup meos_internal_temporal_modif Modification functions
 * @ingroup meos_internal_temporal
 * @brief Modification functions for temporal types
 *
 * @defgroup meos_internal_temporal_restrict Restriction functions
 * @ingroup meos_internal_temporal
 * @brief Restriction functions for temporal types
 *
 * @defgroup meos_internal_temporal_comp Comparison functions
 * @ingroup meos_internal_temporal
 * @brief Comparison functions for temporal types
 *
 * @defgroup meos_internal_temporal_bbox Bounding box functions
 * @ingroup meos_internal_temporal
 * @brief Bounding box functions for temporal types
 *
 * @defgroup meos_internal_temporal_math Mathematical functions
 * @ingroup meos_internal_temporal
 * @brief Mathematical functions for temporal types
 *
 * @defgroup meos_internal_temporal_dist Distance functions
 * @ingroup meos_internal_temporal
 * @brief Distance functions for temporal types
 *
 * @defgroup meos_internal_temporal_comp_trad Traditional comparison functions
 * @ingroup meos_internal_temporal_comp
 * @brief Tranditional comparison functions for temporal types
 *
 * @defgroup meos_internal_temporal_comp_ever Ever/always comparison functions
 * @ingroup meos_internal_temporal_comp
 * @brief Ever and always comparison functions for temporal types
 *
 * @defgroup meos_internal_temporal_comp_temp Temporal comparison functions
 * @ingroup meos_internal_temporal_comp
 * @brief Temporal comparison functions for temporal types
 *
 * @defgroup meos_internal_temporal_spatial Spatial functions
 * @ingroup meos_internal_temporal
 * @brief Spatial functions for temporal geos
 *
 * @defgroup meos_internal_temporal_spatial_accessor Spatial accessor functions
 * @ingroup meos_internal_temporal_spatial
 * @brief Spatial accessor functions for temporal geos
 *
 * @defgroup meos_internal_temporal_spatial_transf Spatial transformation functions
 * @ingroup meos_internal_temporal_spatial
 * @brief Spatial transformation functions for temporal geos
 *
 * @defgroup meos_internal_temporal_agg Aggregate functions
 * @ingroup meos_internal_temporal
 * @brief Aggregate functions for temporal types
 */

/*****************************************************************************/
