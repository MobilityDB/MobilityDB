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
 * @file doxygen_libmeos_api.h
 * @brief Definitions of the sections for the MEOS library API
 */

/*****************************************************************************
 * Definition of the modules of the libMEOS API
 * These modules follow the sections of the MobilityDB documentation although
 * some subsections are merged into a single submodule
 *****************************************************************************/

/**
 * @defgroup libmeos_api MEOS library API
 * @brief API of Mobility Engine Open Source (MEOS) library.
 */

/**
 * @defgroup libmeos_time Functions for time types
 * @ingroup libmeos_api
 * @brief Functions for time types.
 *
 * @defgroup libmeos_time_input_output Input/output functions
 * @ingroup libmeos_time
 * @brief Input/output functions for time types.
 *
 * @defgroup libmeos_time_constructor Constructors functions
 * @ingroup libmeos_time
 * @brief Constructor functions for time types.
 *
 * @defgroup libmeos_time_cast Cast functions
 * @ingroup libmeos_time
 * @brief Cast functions for time types.
 *
 * @defgroup libmeos_time_accessor Accessor functions
 * @ingroup libmeos_time
 * @brief Accessor functions for time types.
 *
 * @defgroup libmeos_time_transf Transformation functions
 * @ingroup libmeos_time
 * @brief Transformation functions for time types.
 *
 * @defgroup libmeos_time_bbox Bounding box functions
 * @ingroup libmeos_time
 * @brief Bounding box functions for time types.
 *
 * @defgroup libmeos_time_topo Topological functions
 * @ingroup libmeos_time_bbox
 * @brief Topological functions for time types.
 *
 * @defgroup libmeos_time_pos Position functions
 * @ingroup libmeos_time_bbox
 * @brief Position functions for time types.
 *
 * @defgroup libmeos_time_set Set functions
 * @ingroup libmeos_time
 * @brief Set functions for time types.
 *
 * @defgroup libmeos_time_dist Distance functions
 * @ingroup libmeos_time
 * @brief Distance functions for time types.
 *
 * @defgroup libmeos_time_comp Comparison functions
 * @ingroup libmeos_time
 * @brief Comparison functions for time types.
 */

/**
 * @defgroup libmeos_box Functions for box types
 * @ingroup libmeos_api
 * @brief Functions for box types.
 *
 * @defgroup libmeos_box_input_output Input/output functions
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
 * @defgroup libmeos_temporal_input_output Input/output functions
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
 * @defgroup libmeos_temporal_bbox Bounding box functions
 * @ingroup libmeos_temporal
 * @brief Bounding box functions for temporal types.
 *
 * @defgroup libmeos_temporal_topo Topological functions
 * @ingroup libmeos_temporal_bbox
 * @brief Topological functions for temporal types.
 *
 * @defgroup libmeos_temporal_pos Position functions
 * @ingroup libmeos_temporal_bbox
 * @brief Position functions for temporal types.
 *
 * @defgroup libmeos_temporal_dist Distance functions
 * @ingroup libmeos_temporal
 * @brief Distance functions for temporal types.
 *
 * @defgroup libmeos_temporal_ever Ever/always functions
 * @ingroup libmeos_temporal
 * @brief Comparison functions for temporal types.
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
 * @defgroup libmeos_temporal_time Time functions
 * @ingroup libmeos_temporal
 * @brief Time functions for temporal types.
 *
 * @defgroup libmeos_temporal_agg Local aggregate functions
 * @ingroup libmeos_temporal
 * @brief Local aggregate functions for temporal types.
 *
 * @defgroup libmeos_temporal_tiling Multidimensional tiling functions
 * @ingroup libmeos_temporal
 * @brief Multidimensional tiling functions for temporal types.
 *
 * @defgroup libmeos_temporal_similarity Similarity functions
 * @ingroup libmeos_temporal
 * @brief Similarity functions for temporal types.
 */

/*****************************************************************************/
