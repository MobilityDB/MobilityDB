/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @brief Definitions of the sections for the MobilityDB API
 */

/*****************************************************************************
 * Definition of the modules of the MobilityDB API
 * These modules follow the sections of the MobilityDB documentation although
 * some subsections are merged into a single submodule
 *****************************************************************************/

/**
 * @defgroup mobilitydb_api MobilityDB API
 * @brief API of MobilityDB PostgreSQL extension.
 */

/**
 * @defgroup mobilitydb_spantime Functions for span and time types
 * @ingroup mobilitydb_api
 * @brief Functions for span and time types.
 *
 * @defgroup mobilitydb_spantime_in_out Input/output functions
 * @ingroup mobilitydb_spantime
 * @brief Input/output functions for span and time types.
 *
 * @defgroup mobilitydb_spantime_constructor Constructors functions
 * @ingroup mobilitydb_spantime
 * @brief Constructor functions for span and time types.
 *
 * @defgroup mobilitydb_spantime_cast Cast functions
 * @ingroup mobilitydb_spantime
 * @brief Cast functions for span and time types.
 *
 * @defgroup mobilitydb_spantime_accessor Accessor functions
 * @ingroup mobilitydb_spantime
 * @brief Accessor functions for span and time types.
 *
 * @defgroup mobilitydb_spantime_transf Transformation functions
 * @ingroup mobilitydb_spantime
 * @brief Transformation functions for span and time types.
 *
 * @defgroup mobilitydb_spantime_bbox Bounding box functions
 * @ingroup mobilitydb_spantime
 * @brief Bounding box functions for span and time types.
 *
 * @defgroup mobilitydb_spantime_topo Topological functions
 * @ingroup mobilitydb_spantime_bbox
 * @brief Topological functions for span and time types.
 *
 * @defgroup mobilitydb_spantime_pos Position functions
 * @ingroup mobilitydb_spantime_bbox
 * @brief Position functions for span and time types.
 *
 * @defgroup mobilitydb_spantime_set Set functions
 * @ingroup mobilitydb_spantime
 * @brief Set functions for span and time types.
 *
 * @defgroup mobilitydb_spantime_dist Distance functions
 * @ingroup mobilitydb_spantime
 * @brief Distance functions for span and time types.
 *
 * @defgroup mobilitydb_spantime_comp Comparison functions
 * @ingroup mobilitydb_spantime
 * @brief Comparison functions for span and time types.
 */

/**
 * @defgroup mobilitydb_box Functions for box types
 * @ingroup mobilitydb_api
 * @brief Functions for box types.
 *
 * @defgroup mobilitydb_box_in_out Input/output functions
 * @ingroup mobilitydb_box
 * @brief Input/output functions for box types.
 *
 * @defgroup mobilitydb_box_constructor Constructor functions
 * @ingroup mobilitydb_box
 * @brief Constructor functions for box types.
 *
 * @defgroup mobilitydb_box_cast Cast functions
 * @ingroup mobilitydb_box
 * @brief Cast functions for box types.
 *
 * @defgroup mobilitydb_box_accessor Accessor functions
 * @ingroup mobilitydb_box
 * @brief Accessor functions for box types.
 *
 * @defgroup mobilitydb_box_transf Transformation functions
 * @ingroup mobilitydb_box
 * @brief Transformation functions for box types.
 *
 * @defgroup mobilitydb_box_topo Topological functions
 * @ingroup mobilitydb_box
 * @brief Topological functions for box types.
 *
 * @defgroup mobilitydb_box_pos Position functions
 * @ingroup mobilitydb_box
 * @brief Position functions for box types.
 *
 * @defgroup mobilitydb_box_set Set functions
 * @ingroup mobilitydb_box
 * @brief Set functions for box types.
 *
 * @defgroup mobilitydb_box_comp Comparison functions
 * @ingroup mobilitydb_box
 * @brief Comparison functions for box types.
 */

/**
 * @defgroup mobilitydb_temporal Functions for temporal types
 * @ingroup mobilitydb_api
 * @brief Functions for temporal types.
 *
 * @defgroup mobilitydb_temporal_in_out Input/output functions
 * @ingroup mobilitydb_temporal
 * @brief Input/output functions for temporal types.
 *
 * @defgroup mobilitydb_temporal_constructor Constructor functions
 * @ingroup mobilitydb_temporal
 * @brief Constructor functions for temporal types.
 *
 * @defgroup mobilitydb_temporal_cast Cast functions
 * @ingroup mobilitydb_temporal
 * @brief Cast functions for temporal types.
 *
 * @defgroup mobilitydb_temporal_accessor Accessor functions
 * @ingroup mobilitydb_temporal
 * @brief Accessor functions for temporal types.
 *
 * @defgroup mobilitydb_temporal_transf Transformation functions
 * @ingroup mobilitydb_temporal
 * @brief Transformation functions for temporal types.
 *
 * @defgroup mobilitydb_temporal_restrict Restriction functions
 * @ingroup mobilitydb_temporal
 * @brief Restriction functions for temporal types.
 *
 * @defgroup mobilitydb_temporal_bool Boolean functions
 * @ingroup mobilitydb_temporal
 * @brief Boolean functions for temporal types.
 *
 * @defgroup mobilitydb_temporal_math Mathematical functions
 * @ingroup mobilitydb_temporal
 * @brief Mathematical functions for temporal types.
 *
 * @defgroup mobilitydb_temporal_text Text functions
 * @ingroup mobilitydb_temporal
 * @brief Text functions for temporal types.
 *
 * @defgroup mobilitydb_temporal_bbox Bounding box functions
 * @ingroup mobilitydb_temporal
 * @brief Bounding box functions for temporal types.
 *
 * @defgroup mobilitydb_temporal_topo Topological functions
 * @ingroup mobilitydb_temporal_bbox
 * @brief Topological functions for temporal types.
 *
 * @defgroup mobilitydb_temporal_pos Position functions
 * @ingroup mobilitydb_temporal_bbox
 * @brief Position functions for temporal types.
 *
 * @defgroup mobilitydb_temporal_dist Distance functions
 * @ingroup mobilitydb_temporal
 * @brief Distance functions for temporal types.
 *
 * @defgroup mobilitydb_temporal_ever Ever/always functions
 * @ingroup mobilitydb_temporal
 * @brief Ever/always functions for temporal types.
 *
 * @defgroup mobilitydb_temporal_comp Comparison functions
 * @ingroup mobilitydb_temporal
 * @brief Comparison functions for temporal types.
 *
 * @defgroup mobilitydb_temporal_spatial Spatial functions
 * @ingroup mobilitydb_temporal
 * @brief Spatial functions for temporal point types.
 *
 * @defgroup mobilitydb_temporal_spatial_accessor Spatial accessor functions
 * @ingroup mobilitydb_temporal_spatial
 * @brief Spatial accessor functions for temporal point types.
 *
 * @defgroup mobilitydb_temporal_spatial_transf Spatial transformation functions
 * @ingroup mobilitydb_temporal_spatial
 * @brief Spatial transformation functions for temporal point types.
 *
 * @defgroup mobilitydb_temporal_spatial_rel Spatial relationship functions
 * @ingroup mobilitydb_temporal_spatial
 * @brief Spatial relationship functions for temporal point types.
 *
 * @defgroup mobilitydb_temporal_time Time functions
 * @ingroup mobilitydb_temporal
 * @brief Time functions for temporal types.
 *
 * @defgroup mobilitydb_temporal_agg Local aggregate functions
 * @ingroup mobilitydb_temporal
 * @brief Local aggregate functions for temporal types.
 *
 * @defgroup mobilitydb_temporal_tile Tile functions
 * @ingroup mobilitydb_temporal
 * @brief Tile functions for temporal types.
 *
 * @defgroup mobilitydb_temporal_similarity Similarity functions
 * @ingroup mobilitydb_temporal
 * @brief Similarity functions for temporal types.
 */

/*****************************************************************************/
