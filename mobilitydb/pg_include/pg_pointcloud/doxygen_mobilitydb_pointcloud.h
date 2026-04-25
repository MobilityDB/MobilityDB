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

/*****************************************************************************
 * Top-level submodules under @ref mobilitydb_pointcloud
 *****************************************************************************/

/**
 * @defgroup mobilitydb_pointcloud_base PG wrappers for static pcpoint / pcpatch
 * @ingroup mobilitydb_pointcloud
 * @brief PG wrappers for the static pgpointcloud base types
 *
 * @defgroup mobilitydb_pointcloud_set PG wrappers for pcpointset / pcpatchset
 * @ingroup mobilitydb_pointcloud
 * @brief PG wrappers for the pgpointcloud set types
 *
 * @defgroup mobilitydb_pointcloud_box PG wrappers for the TPCBox type
 * @ingroup mobilitydb_pointcloud
 * @brief PG wrappers for the TPCBox spatiotemporal bounding-box type
 *
 * @defgroup mobilitydb_pointcloud_temp PG wrappers for tpcpoint / tpcpatch
 * @ingroup mobilitydb_pointcloud
 * @brief PG wrappers for the lifted temporal pgpointcloud types
 *
 * @defgroup mobilitydb_pointcloud_index GiST / SP-GiST opclass support
 * @ingroup mobilitydb_pointcloud
 * @brief GiST opclass support functions for the TPCBox type
 */

/*****************************************************************************
 * Submodules under @ref mobilitydb_pointcloud_base
 *****************************************************************************/

/**
 * @defgroup mobilitydb_pointcloud_base_accessor Accessor functions
 * @ingroup mobilitydb_pointcloud_base
 * @brief Schema-aware accessors (pcid, getX, getY, getZ, getDim) for
 *   pcpoint / pcpatch
 */

/*****************************************************************************
 * Submodules under @ref mobilitydb_pointcloud_temp
 *****************************************************************************/

/**
 * @defgroup mobilitydb_pointcloud_accessor Accessor functions
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Per-type accessors for tpcpoint and tpcpatch (pcid, npoints,
 *   per-dimension projections)
 *
 * @defgroup mobilitydb_pointcloud_conversion Conversion functions
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Conversions between tpcpoint / tpcpatch and tgeompoint / tgeometry
 */

/*****************************************************************************
 * Submodules under @ref mobilitydb_pointcloud_box
 *****************************************************************************/

/**
 * @defgroup mobilitydb_pointcloud_box_inout Input and output functions
 * @ingroup mobilitydb_pointcloud_box
 * @brief Input / output PG wrappers for the TPCBox type
 *
 * @defgroup mobilitydb_pointcloud_box_constructor Constructor functions
 * @ingroup mobilitydb_pointcloud_box
 * @brief Constructor PG wrappers for the TPCBox type
 *
 * @defgroup mobilitydb_pointcloud_box_accessor Accessor functions
 * @ingroup mobilitydb_pointcloud_box
 * @brief Accessor PG wrappers for the TPCBox type
 *
 * @defgroup mobilitydb_pointcloud_box_setops Set operations
 * @ingroup mobilitydb_pointcloud_box
 * @brief Union / intersection PG wrappers for the TPCBox type
 *
 * @defgroup mobilitydb_pointcloud_box_topo Topological predicates
 * @ingroup mobilitydb_pointcloud_box
 * @brief Topological predicate PG wrappers (contains / overlaps / same /
 *   adjacent) for TPCBox
 *
 * @defgroup mobilitydb_pointcloud_box_pos Position predicates
 * @ingroup mobilitydb_pointcloud_box
 * @brief Position predicate PG wrappers (left / right / above / below /
 *   front / back / before / after) for TPCBox
 *
 * @defgroup mobilitydb_pointcloud_box_comp Comparison functions
 * @ingroup mobilitydb_pointcloud_box
 * @brief Comparison PG wrappers for the TPCBox type
 */

/*****************************************************************************/
