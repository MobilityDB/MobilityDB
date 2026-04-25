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
 * Top-level submodules under @ref meos_pointcloud
 *****************************************************************************/

/**
 * @defgroup meos_pointcloud_base Functions for static pcpoint / pcpatch
 * @ingroup meos_pointcloud
 * @brief Functions for the static pgpointcloud base types pcpoint and pcpatch
 *
 * @defgroup meos_pointcloud_set Functions for pcpointset / pcpatchset
 * @ingroup meos_pointcloud
 * @brief Functions for the pgpointcloud set types pcpointset and pcpatchset
 *
 * @defgroup meos_pointcloud_box Functions for the TPCBox bounding-box type
 * @ingroup meos_pointcloud
 * @brief Functions for the TPCBox spatiotemporal bounding-box type
 *
 * @defgroup meos_pointcloud_schema_cache Schema cache helpers
 * @ingroup meos_pointcloud
 * @brief Process-global cache of parsed pgpointcloud PCSCHEMA values
 *
 * @note tpcpoint and tpcpatch have no type-specific public MEOS API:
 *   the lifted types delegate everything to the generic @ref meos_temporal
 *   surface (numInstants, atTime, valueAtTimestamp, …).  Their per-instant
 *   bbox is a @ref meos_pointcloud_box value; type-specific helpers
 *   (`pcid`, per-dimension projections, `tgeompoint` cast) live in the
 *   PG wrapper layer because they need PG datum unpacking.
 */

/*****************************************************************************
 * Submodules under @ref meos_pointcloud_base
 *
 * The static pcpoint / pcpatch surface is intentionally narrow at the
 * MEOS layer (opaque byte blobs).  Schema-aware helpers (getX/Y/Z/Dim)
 * live alongside the basic accessors.
 *****************************************************************************/

/**
 * @defgroup meos_pointcloud_inout Input and output functions
 * @ingroup meos_pointcloud_base
 * @brief Hex-WKB input / output functions for pcpoint and pcpatch
 *
 * @defgroup meos_pointcloud_accessor Accessor functions
 * @ingroup meos_pointcloud_base
 * @brief Schema-aware coordinate accessors and metadata for pcpoint / pcpatch
 */

/*****************************************************************************
 * Submodules under @ref meos_pointcloud_set
 *****************************************************************************/

/**
 * @defgroup meos_pointcloud_set_inout Input and output functions
 * @ingroup meos_pointcloud_set
 * @brief Input and output functions for pcpointset / pcpatchset
 *
 * @defgroup meos_pointcloud_set_constructor Constructor functions
 * @ingroup meos_pointcloud_set
 * @brief Constructor functions for pcpointset / pcpatchset
 *
 * @defgroup meos_pointcloud_set_conversion Conversion functions
 * @ingroup meos_pointcloud_set
 * @brief Conversion functions for pcpointset / pcpatchset
 *
 * @defgroup meos_pointcloud_set_accessor Accessor functions
 * @ingroup meos_pointcloud_set
 * @brief Accessor functions for pcpointset / pcpatchset
 *
 * @defgroup meos_pointcloud_set_setops Set operations
 * @ingroup meos_pointcloud_set
 * @brief Set operations on pcpointset / pcpatchset
 */

/*****************************************************************************
 * Submodules under @ref meos_pointcloud_box
 *****************************************************************************/

/**
 * @defgroup meos_pointcloud_box_inout Input and output functions
 * @ingroup meos_pointcloud_box
 * @brief Input and output functions for the TPCBox type
 *
 * @defgroup meos_pointcloud_box_constructor Constructor functions
 * @ingroup meos_pointcloud_box
 * @brief Constructor functions for the TPCBox type
 *
 * @defgroup meos_pointcloud_box_conversion Conversion functions
 * @ingroup meos_pointcloud_box
 * @brief Conversion functions for the TPCBox type
 *
 * @defgroup meos_pointcloud_box_accessor Accessor functions
 * @ingroup meos_pointcloud_box
 * @brief Accessor functions for the TPCBox type
 *
 * @defgroup meos_pointcloud_box_transf Transformation functions
 * @ingroup meos_pointcloud_box
 * @brief Transformation (expand, etc.) functions for the TPCBox type
 *
 * @defgroup meos_pointcloud_box_setops Set operations
 * @ingroup meos_pointcloud_box
 * @brief Union and intersection of TPCBox values
 *
 * @defgroup meos_pointcloud_box_topo Topological predicates
 * @ingroup meos_pointcloud_box
 * @brief Contains / contained / overlaps / same / adjacent for TPCBox
 *
 * @defgroup meos_pointcloud_box_pos Position predicates
 * @ingroup meos_pointcloud_box
 * @brief Axis-aligned position predicates (left / right / above / below /
 *   front / back / before / after) for TPCBox
 *
 * @defgroup meos_pointcloud_box_comp Comparison functions
 * @ingroup meos_pointcloud_box
 * @brief Equality, ordering, and hashing for the TPCBox type
 */

/*****************************************************************************/
