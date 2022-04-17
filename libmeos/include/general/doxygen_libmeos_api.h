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
 * @defgroup libmeos_time_oper Operators
 * @ingroup libmeos_time
 * @brief Operators for time types.
 *
 * @defgroup libmeos_time_oper_topo Topological operators
 * @ingroup libmeos_time_oper
 * @brief Topological operators for time types.
 *
 * @defgroup libmeos_time_oper_pos Position operators
 * @ingroup libmeos_time_oper
 * @brief Position operators for time types.
 *
 * @defgroup libmeos_time_oper_set Set operators
 * @ingroup libmeos_time_oper
 * @brief Set operators for time types.
 *
 * @defgroup libmeos_time_oper_dist Distance operators
 * @ingroup libmeos_time_oper
 * @brief Distance operators for time types.
 *
 * @defgroup libmeos_time_oper_comp Comparison operators
 * @ingroup libmeos_time_oper
 * @brief Comparison operators for time types.
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
 * @defgroup libmeos_box_oper Operators
 * @ingroup libmeos_box
 * @brief Operators for box types.
 *
 * @defgroup libmeos_box_oper_topo Topological operators
 * @ingroup libmeos_box_oper
 * @brief Topological operators for box types.
 *
 * @defgroup libmeos_box_oper_pos Position operators
 * @ingroup libmeos_box_oper
 * @brief Position operators for box types.
 *
 * @defgroup libmeos_box_oper_set Set operators
 * @ingroup libmeos_box_oper
 * @brief Set operators for box types.
 *
 * @defgroup libmeos_box_oper_comp Comparison operators
 * @ingroup libmeos_box_oper
 * @brief Comparison operators for box types.
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
 * @defgroup libmeos_temporal_tiling Multidimensional tiling functions
 * @ingroup libmeos_temporal
 * @brief Multidimensional tiling functions for temporal types.
 *
 * @defgroup libmeos_temporal_similarity Similarity functions
 * @ingroup libmeos_temporal
 * @brief Similarity functions for temporal types.
 *
 * @defgroup libmeos_temporal_oper Operators
 * @ingroup libmeos_temporal
 * @brief Operators for temporal types.
 *
 * @defgroup libmeos_temporal_oper_bool Boolean operators
 * @ingroup libmeos_temporal_oper
 * @brief Boolean operators for temporal types.
 *
 * @defgroup libmeos_temporal_oper_math Mathematical operators
 * @ingroup libmeos_temporal_oper
 * @brief Mathematical operators for temporal types.
 *
 * @defgroup libmeos_temporal_oper_topo Topological operators
 * @ingroup libmeos_temporal_oper
 * @brief Topological operators for temporal types.
 *
 * @defgroup libmeos_temporal_oper_pos Position operators
 * @ingroup libmeos_temporal_oper
 * @brief Position operators for temporal types.
 *
 * @defgroup libmeos_temporal_oper_dist Distance operators
 * @ingroup libmeos_temporal_oper
 * @brief Distance operators for temporal types.
 *
 * @defgroup libmeos_temporal_oper_ever Ever/always operators
 * @ingroup libmeos_temporal_oper
 * @brief Comparison operators for temporal types.
 *
 * @defgroup libmeos_temporal_oper_comp Comparison operators
 * @ingroup libmeos_temporal_oper
 * @brief Comparison operators for temporal types.
 */

/*****************************************************************************/
