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
 * @brief Sections for the documentation of the MobilityDB API: Poses
 */

/*****************************************************************************
 * Definition of the modules of the MobilityDB API
 * These modules follow the sections of the MobilityDB documentation although
 * some subsections are merged into a single submodule
 *****************************************************************************/

/**
 * @defgroup mobilitydb_pose_base Functions for static poses
 * @ingroup mobilitydb_pose
 * @brief Functions for static poses
 *
 * @defgroup mobilitydb_pose_set Functions for pose sets
 * @ingroup mobilitydb_pose
 * @brief Functions for pose sets
 *
 * @defgroup mobilitydb_pose_inout Input and output functions
 * @ingroup mobilitydb_pose
 * @brief Input and output functions for temporal poses
 *
 * @defgroup mobilitydb_pose_conversion Conversion functions
 * @ingroup mobilitydb_pose
 * @brief Conversion functions for temporal poses
 *
 * @defgroup mobilitydb_pose_constructor Constructor functions
 * @ingroup mobilitydb_pose
 * @brief Constructor functions for temporal poses
 *
 * @defgroup mobilitydb_pose_accessor Accessor functions
 * @ingroup mobilitydb_pose
 * @brief Accessor functions for temporal poses
 *
 * @defgroup mobilitydb_pose_transf Transformation functions
 * @ingroup mobilitydb_pose
 * @brief Transformation functions for temporal poses
 *
 * @defgroup mobilitydb_pose_restrict Restriction functions
 * @ingroup mobilitydb_pose
 * @brief Restriction functions for temporal poses
 *
 * @ingroup mobilitydb_pose
 * @brief Distance functions for temporal poses
 *
 * @defgroup mobilitydb_pose_comp Comparison functions
 * @ingroup mobilitydb_pose
 * @brief Comparison functions for temporal poses
 *
 *   @defgroup mobilitydb_pose_comp_ever Ever and always comparison functions
 *   @ingroup mobilitydb_pose_comp
 *   @brief Ever and always comparison functions for temporal poses
 *
 *   @defgroup mobilitydb_pose_comp_temp Temporal comparison functions
 *   @ingroup mobilitydb_pose_comp
 *   @brief Comparison functions for temporal poses
 *
 * @defgroup mobilitydb_pose_srid Spatial reference system functions
 * @ingroup mobilitydb_pose
 * @brief Spatial reference system functions for temporal poses
 *
 * @defgroup mobilitydb_pose_dist Distance functions
 * @ingroup mobilitydb_pose
 * @brief Distance functions for temporal poses
 *
 * @defgroup mobilitydb_pose_rel Spatial relationship functions
 * @ingroup mobilitydb_pose
 * @brief Spatial relationship functions for temporal poses
 *
 *   @defgroup mobilitydb_pose_rel_ever Ever and always spatial relationship functions
 *   @ingroup mobilitydb_pose_rel
 *   @brief Ever and always spatial relationship functions for temporal poses
 *
 *   @defgroup mobilitydb_pose_rel_temp Spatiotemporal relationship functions
 *   @ingroup mobilitydb_pose_rel
 *   @brief Spatiotemporal relationship functions for temporal poses
 *
 * @defgroup mobilitydb_pose_agg Aggregate functions
 * @ingroup mobilitydb_pose
 * @brief Aggregate functions for temporal poses
 */

/*****************************************************************************/
/**
 * @defgroup mobilitydb_pose_base_inout Input and output functions
 * @ingroup mobilitydb_pose_base
 * @brief Input and output functions for static poses
 *
 * @defgroup mobilitydb_pose_base_constructor Constructor functions
 * @ingroup mobilitydb_pose_base
 * @brief Constructor functions for static poses
 *
 * @defgroup mobilitydb_pose_base_conversion Conversion functions
 * @ingroup mobilitydb_pose_base
 * @brief Conversion functions for static poses
 *
 * @defgroup mobilitydb_pose_base_accessor Accessor functions
 * @ingroup mobilitydb_pose_base
 * @brief Accessor functions for static poses
 *
 * @defgroup mobilitydb_pose_base_transf Transformation functions
 * @ingroup mobilitydb_pose_base
 * @brief Transformation functions for static poses
 *
 * @defgroup mobilitydb_pose_base_box Bounding box functions
 * @ingroup mobilitydb_pose_base
 * @brief Bounding box functions for static poses
 *
 * @defgroup mobilitydb_pose_base_srid Spatial reference system functions
 * @ingroup mobilitydb_pose_base
 * @brief Spatial reference system functions for static poses
 *
 * @defgroup mobilitydb_pose_base_comp Comparison functions
 * @ingroup mobilitydb_pose_base
 * @brief Comparison functions for static poses
 */

/*****************************************************************************/

