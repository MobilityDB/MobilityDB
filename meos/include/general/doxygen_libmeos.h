/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Sections for the documentation of the MEOS library
 */

/*****************************************************************************
 * Sections of the external libMEOS API
 * These sections follow the ones of the MobilityDB documentation although
 * some subsections are merged into a single one
 *****************************************************************************/

/**
 * @defgroup libmeos MEOS library
 * @brief Mobility Engine Open Source (MEOS) library.
 *
 * @defgroup libmeos_api API of the MEOS library
 * @ingroup libmeos
 * @brief API of the Mobility Engine Open Source (MEOS) library.
 *
 * @defgroup libmeos_internal Internal functions of the MEOS library
 * @ingroup libmeos
 * @brief Internal functions of Mobility Engine Open Source (MEOS) library.
 */

/**
 * @defgroup libmeos_pg_types Functions for PostgreSQL types
 * @ingroup libmeos_api
 * @brief Functions for PostgreSQL types.

 * @defgroup libmeos_pgis_types Functions for PostGIS types
 * @ingroup libmeos_api
 * @brief Functions for PostGIS types.

 * @defgroup libmeos_setspan Functions for set and span types
 * @ingroup libmeos_api
 * @brief Functions for set and span types.
 *
 * @defgroup libmeos_setspan_inout Input/output functions
 * @ingroup libmeos_setspan
 * @brief Input/output functions for set and span types.
 *
 * @defgroup libmeos_setspan_constructor Constructor functions
 * @ingroup libmeos_setspan
 * @brief Constructor functions for set and span types.
 *
 * @defgroup libmeos_setspan_cast Cast functions
 * @ingroup libmeos_setspan
 * @brief Cast functions for set and span types.
 *
 * @defgroup libmeos_setspan_accessor Accessor functions
 * @ingroup libmeos_setspan
 * @brief Accessor functions for set and span types.
 *
 * @defgroup libmeos_setspan_transf Transformation functions
 * @ingroup libmeos_setspan
 * @brief Transformation functions for set and span types.
 *
 * @defgroup libmeos_setspan_bbox Bounding box functions
 * @ingroup libmeos_setspan
 * @brief Bounding box functions for set and span types.
 *
 * @defgroup libmeos_setspan_topo Topological functions
 * @ingroup libmeos_setspan_bbox
 * @brief Topological functions for set and span types.
 *
 * @defgroup libmeos_setspan_pos Position functions
 * @ingroup libmeos_setspan_bbox
 * @brief Position functions for set and span types.
 *
 * @defgroup libmeos_setspan_set Set functions
 * @ingroup libmeos_setspan
 * @brief Set functions for set and span types.
 *
 * @defgroup libmeos_setspan_dist Distance functions
 * @ingroup libmeos_setspan
 * @brief Distance functions for set and span types.
 *
 * @defgroup libmeos_setspan_agg Aggregate functions
 * @ingroup libmeos_setspan
 * @brief Aggregate functions for set and span types.
 *
 * @defgroup libmeos_setspan_comp Comparison functions
 * @ingroup libmeos_setspan
 * @brief Comparison functions for set and span types.
 */

/**
 * @defgroup libmeos_box Functions for box types
 * @ingroup libmeos_api
 * @brief Functions for box types.
 *
 * @defgroup libmeos_box_inout Input/output functions
 * @ingroup libmeos_box
 * @brief Input/output functions for box types.
 *
 * @defgroup libmeos_box_constructor Constructor functions
 * @ingroup libmeos_box
 * @brief Constructor functions for box types.
 *
 * @defgroup libmeos_box_cast Cast functions
 * @ingroup libmeos_box
 * @brief Cast functions for box types.
 *
 * @defgroup libmeos_box_accessor Accessor functions
 * @ingroup libmeos_box
 * @brief Accessor functions for box types.
 *
 * @defgroup libmeos_box_transf Transformation functions
 * @ingroup libmeos_box
 * @brief Transformation functions for box types.
 *
 * @defgroup libmeos_box_topo Topological functions
 * @ingroup libmeos_box
 * @brief Topological functions for box types.
 *
 * @defgroup libmeos_box_pos Position functions
 * @ingroup libmeos_box
 * @brief Position functions for box types.
 *
 * @defgroup libmeos_box_set Set functions
 * @ingroup libmeos_box
 * @brief Set functions for box types.
 *
 * @defgroup libmeos_box_comp Comparison functions
 * @ingroup libmeos_box
 * @brief Comparison functions for box types.
 */

/**
 * @defgroup libmeos_temporal Functions for temporal types
 * @ingroup libmeos_api
 * @brief Functions for temporal types.
 *
 * @defgroup libmeos_temporal_inout Input/output functions
 * @ingroup libmeos_temporal
 * @brief Input/output functions for temporal types.
 *
 * @defgroup libmeos_temporal_constructor Constructor functions
 * @ingroup libmeos_temporal
 * @brief Constructor functions for temporal types.
 *
 * @defgroup libmeos_temporal_cast Cast functions
 * @ingroup libmeos_temporal
 * @brief Cast functions for temporal types.
 *
 * @defgroup libmeos_temporal_accessor Accessor functions
 * @ingroup libmeos_temporal
 * @brief Accessor functions for temporal types.
 *
 * @defgroup libmeos_temporal_transf Transformation functions
 * @ingroup libmeos_temporal
 * @brief Transformation functions for temporal types.
 *
 * @defgroup libmeos_temporal_restrict Restriction functions
 * @ingroup libmeos_temporal
 * @brief Restriction functions for temporal types.
 *
 * @defgroup libmeos_temporal_modif Modification functions
 * @ingroup libmeos_temporal
 * @brief Modification functions for temporal types.
 *
 * @defgroup libmeos_temporal_bool Boolean functions
 * @ingroup libmeos_temporal
 * @brief Boolean functions for temporal types.
 *
 * @defgroup libmeos_temporal_math Mathematical functions
 * @ingroup libmeos_temporal
 * @brief Mathematical functions for temporal types.
 *
 * @defgroup libmeos_temporal_text Text functions
 * @ingroup libmeos_temporal
 * @brief Text functions for temporal types.
 *
 * @defgroup libmeos_temporal_dist Distance functions
 * @ingroup libmeos_temporal
 * @brief Distance functions for temporal types.
 *
 * @defgroup libmeos_temporal_ever Ever/always functions
 * @ingroup libmeos_temporal
 * @brief Ever/always functions for temporal types.
 *
 * @defgroup libmeos_temporal_comp Comparison functions
 * @ingroup libmeos_temporal
 * @brief Comparison functions for temporal types.
 *
 * @defgroup libmeos_temporal_spatial Spatial functions
 * @ingroup libmeos_temporal
 * @brief Spatial functions for temporal point types.
 *
 * @defgroup libmeos_temporal_spatial_accessor Spatial accessor functions
 * @ingroup libmeos_temporal_spatial
 * @brief Spatial accessor functions for temporal point types.
 *
 * @defgroup libmeos_temporal_spatial_transf Spatial transformation functions
 * @ingroup libmeos_temporal_spatial
 * @brief Spatial transformation functions for temporal point types.
 *
 * @defgroup libmeos_temporal_spatial_rel Spatial relationship functions
 * @ingroup libmeos_temporal_spatial
 * @brief Spatial relationship functions for temporal point types.
 *
 * @defgroup libmeos_temporal_agg Local and temporal aggregate functions
 * @ingroup libmeos_temporal
 * @brief Local and temporal aggregate functions for temporal types.
 *
 * @defgroup libmeos_temporal_tile Tile functions
 * @ingroup libmeos_temporal
 * @brief Tile functions for temporal types.
 *
 * @defgroup libmeos_temporal_similarity Similarity functions
 * @ingroup libmeos_temporal
 * @brief Similarity functions for temporal types.
 *
 * @defgroup libmeos_temporal_analytics Analytics functions
 * @ingroup libmeos_temporal
 * @brief Analytics functions for temporal types.
 */

/*****************************************************************************
 * Sections of the libMEOS internal functions
 *****************************************************************************/

/**
 * @defgroup libmeos_internal_typefuncs Generic type functions
 * @ingroup libmeos_internal
 * @brief Generic type functions.
 *
 * @defgroup libmeos_internal_setspan Functions for set and span types
 * @ingroup libmeos_internal
 * @brief Functions for set and span types.
 *
 * @defgroup libmeos_internal_setspan_inout Input/output functions
 * @ingroup libmeos_internal_setspan
 * @brief Input/output functions for set and span types.
 *
 * @defgroup libmeos_internal_setspan_constructor Constructor functions
 * @ingroup libmeos_internal_setspan
 * @brief Constructor functions for set and span types.
 *
 * @defgroup libmeos_internal_setspan_cast Cast functions
 * @ingroup libmeos_internal_setspan
 * @brief Cast functions for set and span types.
 *
 * @defgroup libmeos_internal_setspan_accessor Accessor functions
 * @ingroup libmeos_internal_setspan
 * @brief Accessor functions for set and span types.
 *
 * @defgroup libmeos_internal_setspan_transf Transformation functions
 * @ingroup libmeos_internal_setspan
 * @brief Transformation functions for set and span types.
 *
 * @defgroup libmeos_internal_setspan_bbox Bounding box functions
 * @ingroup libmeos_internal_setspan
 * @brief Bounding box functions for set and span types.
 *
 * @defgroup libmeos_internal_setspan_topo Topological functions
 * @ingroup libmeos_internal_setspan_bbox
 * @brief Topological functions for set and span types.
 *
 * @defgroup libmeos_internal_setspan_pos Position functions
 * @ingroup libmeos_internal_setspan_bbox
 * @brief Position functions for set and span types.
 *
 * @defgroup libmeos_internal_setspan_set Set functions
 * @ingroup libmeos_internal_setspan
 * @brief Set functions for set and span types.
 *
 * @defgroup libmeos_internal_setspan_dist Distance functions
 * @ingroup libmeos_internal_setspan
 * @brief Distance functions for set and span types.
 *
 * @defgroup libmeos_internal_setspan_agg Aggregate functions
 * @ingroup libmeos_internal_setspan
 * @brief Aggregate functions for set and span types.
 */

/**
 * @defgroup libmeos_internal_box Functions for box types
 * @ingroup libmeos_internal
 * @brief Functions for box types.
 *
 * @defgroup libmeos_internal_box_constructor Constructor functions
 * @ingroup libmeos_internal_box
 * @brief Constructor functions for box types.
 *
 * @defgroup libmeos_internal_box_cast Cast functions
 * @ingroup libmeos_internal_box
 * @brief Cast functions for box types.
 *
 * @defgroup libmeos_internal_box_set Set functions
 * @ingroup libmeos_internal_box
 * @brief Set functions for box types.
  */

/**
 * @defgroup libmeos_internal_temporal Functions for temporal types
 * @ingroup libmeos_internal
 * @brief Functions for temporal types.
 *
 * @defgroup libmeos_internal_temporal_inout Input/output functions
 * @ingroup libmeos_internal_temporal
 * @brief Input/output functions for temporal types.
 *
 * @defgroup libmeos_internal_temporal_constructor Constructor functions
 * @ingroup libmeos_internal_temporal
 * @brief Constructor functions for temporal types.
 *
 * @defgroup libmeos_internal_temporal_cast Cast functions
 * @ingroup libmeos_internal_temporal
 * @brief Cast functions for temporal types.
 *
 * @defgroup libmeos_internal_temporal_accessor Accessor functions
 * @ingroup libmeos_internal_temporal
 * @brief Accessor functions for temporal types.
 *
 * @defgroup libmeos_internal_temporal_transf Transformation functions
 * @ingroup libmeos_internal_temporal
 * @brief Transformation functions for temporal types.
 *
 * @defgroup libmeos_internal_temporal_restrict Restriction functions
 * @ingroup libmeos_internal_temporal
 * @brief Restriction functions for temporal types.
 *
 * @defgroup libmeos_internal_temporal_math Mathematical functions
 * @ingroup libmeos_internal_temporal
 * @brief Mathematical functions for temporal types.
 *
 * @defgroup libmeos_internal_temporal_dist Distance functions
 * @ingroup libmeos_internal_temporal
 * @brief Distance functions for temporal types.
 *
 * @defgroup libmeos_internal_temporal_ever Ever/always functions
 * @ingroup libmeos_internal_temporal
 * @brief Ever/always functions for temporal types.
 *
 * @defgroup libmeos_internal_temporal_comp Comparison functions
 * @ingroup libmeos_internal_temporal
 * @brief Comparison functions for temporal types.
 *
 * @defgroup libmeos_internal_temporal_spatial Spatial functions
 * @ingroup libmeos_internal_temporal
 * @brief Spatial functions for temporal point types.
 *
 * @defgroup libmeos_internal_temporal_spatial_accessor Spatial accessor functions
 * @ingroup libmeos_internal_temporal_spatial
 * @brief Spatial accessor functions for temporal point types.
 *
 * @defgroup libmeos_internal_temporal_spatial_transf Spatial transformation functions
 * @ingroup libmeos_internal_temporal_spatial
 * @brief Spatial transformation functions for temporal point types.
 *
 * @defgroup libmeos_internal_temporal_agg Local aggregate functions
 * @ingroup libmeos_internal_temporal
 * @brief Local aggregate functions for temporal types.
 */

/*****************************************************************************/
