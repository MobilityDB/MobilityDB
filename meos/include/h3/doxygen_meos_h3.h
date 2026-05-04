/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief Sections for the documentation of the MEOS API: temporal H3 cell
 * indices.
 *
 * Mirrors the section layout of `doxygen_meos_cbuffer.h`. The
 * grouping follows the chapter divisions of the user manual
 * (`doc/temporal_h3_index.xml`).
 */

/*****************************************************************************
 * Modules of the MEOS API for the th3index type
 *****************************************************************************/

/**
 * @defgroup meos_h3_base Functions for static H3 cell indices
 * @ingroup meos_h3
 * @brief Functions for static H3 cell indices
 *
 * @defgroup meos_h3_inout Input and output functions
 * @ingroup meos_h3
 * @brief Input and output functions for temporal H3 cell indices
 *
 * @defgroup meos_h3_constructor Constructor functions
 * @ingroup meos_h3
 * @brief Constructor functions for temporal H3 cell indices
 *
 * @defgroup meos_h3_conversion Conversion functions
 * @ingroup meos_h3
 * @brief Conversion functions for temporal H3 cell indices
 *
 * @defgroup meos_h3_accessor Accessor functions
 * @ingroup meos_h3
 * @brief Accessor functions for temporal H3 cell indices
 *
 * @defgroup meos_h3_inspection Index-inspection functions
 * @ingroup meos_h3
 * @brief Per-instant inspection of the underlying H3 cell payload
 *
 * @defgroup meos_h3_hierarchy Hierarchy functions
 * @ingroup meos_h3
 * @brief Per-instant parent / center-child / child-position helpers
 *
 * @defgroup meos_h3_latlng Lat/Lng-conversion functions
 * @ingroup meos_h3
 * @brief Per-instant centroid / boundary / latlng-to-cell helpers
 *
 * @defgroup meos_h3_edges Directed-edge functions
 * @ingroup meos_h3
 * @brief Per-instant directed-edge construction and accessors
 *
 * @defgroup meos_h3_vertex Vertex functions
 * @ingroup meos_h3
 * @brief Per-instant H3-vertex construction and accessors
 *
 * @defgroup meos_h3_traversal Grid-traversal functions
 * @ingroup meos_h3
 * @brief Per-instant grid distance and local-IJ helpers
 *
 * @defgroup meos_h3_metrics Metric functions
 * @ingroup meos_h3
 * @brief Per-instant area / edge-length / great-circle distance helpers
 *
 * @defgroup meos_h3_comp Comparison functions
 * @ingroup meos_h3
 * @brief Comparison functions for temporal H3 cell indices
 *
 *   @defgroup meos_h3_comp_ever Ever and always comparison functions
 *   @ingroup meos_h3_comp
 *   @brief Ever / always comparison helpers for temporal H3 cells
 *
 *   @defgroup meos_h3_comp_temp Temporal comparison functions
 *   @ingroup meos_h3_comp
 *   @brief Temporal comparison helpers (returning a temporal boolean)
 */

/*****************************************************************************/

/**
 * @defgroup meos_h3_base_inout Input and output functions
 * @ingroup meos_h3_base
 * @brief Input and output functions for static H3 cell indices
 *
 * @defgroup meos_h3_base_accessor Accessor functions
 * @ingroup meos_h3_base
 * @brief Accessor functions for static H3 cell indices
 *
 * @defgroup meos_h3_base_comp Comparison functions
 * @ingroup meos_h3_base
 * @brief Comparison functions for static H3 cell indices
 */

/*****************************************************************************/

/**
 * @defgroup meos_h3_internal Internal helpers
 * @ingroup meos_h3
 * @brief Internal datum wrappers and adapter declarations exposed only
 * inside the th3index implementation.
 */

/*****************************************************************************/
