/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief The names of the internal functions and tables of the core library
 * of H3 vendored under h3lib/, each prefixed as H3_PREFIX prefixes its
 * public names
 * @details The build of the object library h3core includes this header
 * before every source, so that no global symbol of the vendored library
 * keeps a name of libh3, which MEOS also links: a static libmeos linked
 * with a static libh3 then defines each name once. The names are those the
 * sources of h3lib/lib define without H3_PREFIX. A name H3 begins with an
 * underscore, its mark of a name private to the library, takes the prefix
 * `meos_h3_` in place of the underscore (`_iterInitParent` reads
 * `meos_h3_iterInitParent`), and any other name the prefix `meos_`
 * (`iterInitParent` reads `meos_iterInitParent`), so no name carries two
 * underscores and the two names H3 tells apart by the underscore stay
 * apart. The test h3lib_names_unit fails when a global symbol of h3core
 * carries none of the prefixes `meos_` and `h3_` (the functions of
 * h3_vertex.c), or carries `meos__`.
 */

#ifndef __H3LIB_NAMES_H__
#define __H3LIB_NAMES_H__

#define FAILED_DIRECTIONS meos_FAILED_DIRECTIONS
#define PENTAGON_ROTATIONS meos_PENTAGON_ROTATIONS
#define PENTAGON_ROTATIONS_REVERSE meos_PENTAGON_ROTATIONS_REVERSE
#define PENTAGON_ROTATIONS_REVERSE_NONPOLAR meos_PENTAGON_ROTATIONS_REVERSE_NONPOLAR
#define PENTAGON_ROTATIONS_REVERSE_POLAR meos_PENTAGON_ROTATIONS_REVERSE_POLAR
#define _adjustOverageClassII meos_h3_adjustOverageClassII
#define _adjustPentVertOverage meos_h3_adjustPentVertOverage
#define _baseCellIsCwOffset meos_h3_baseCellIsCwOffset
#define _baseCellToCCWrot60 meos_h3_baseCellToCCWrot60
#define _baseCellToFaceIjk meos_h3_baseCellToFaceIjk
#define _faceIjkPentToCellBoundary meos_h3_faceIjkPentToCellBoundary
#define _faceIjkPentToVerts meos_h3_faceIjkPentToVerts
#define _faceIjkToBaseCell meos_h3_faceIjkToBaseCell
#define _faceIjkToBaseCellCCWrot60 meos_h3_faceIjkToBaseCellCCWrot60
#define _faceIjkToCellBoundary meos_h3_faceIjkToCellBoundary
#define _faceIjkToH3 meos_h3_faceIjkToH3
#define _faceIjkToVec3 meos_h3_faceIjkToVec3
#define _faceIjkToVerts meos_h3_faceIjkToVerts
#define _getBaseCellDirection meos_h3_getBaseCellDirection
#define _getBaseCellNeighbor meos_h3_getBaseCellNeighbor
#define _getEdgeHexagons meos_h3_getEdgeHexagons
#define _gridDiskDistancesInternal meos_h3_gridDiskDistancesInternal
#define _gridRingInternal meos_h3_gridRingInternal
#define _h3LeadingNonZeroDigit meos_h3_h3LeadingNonZeroDigit
#define _h3Rotate60ccw meos_h3_h3Rotate60ccw
#define _h3Rotate60cw meos_h3_h3Rotate60cw
#define _h3RotatePent60ccw meos_h3_h3RotatePent60ccw
#define _h3RotatePent60cw meos_h3_h3RotatePent60cw
#define _h3ToFaceIjk meos_h3_h3ToFaceIjk
#define _h3ToFaceIjkWithInitializedFijk meos_h3_h3ToFaceIjkWithInitializedFijk
#define _hexRadiusKm meos_h3_hexRadiusKm
#define _ipow meos_h3_ipow
#define _isBaseCellPentagon meos_h3_isBaseCellPentagon
#define _isBaseCellPolarPentagon meos_h3_isBaseCellPolarPentagon
#define _iterInitParent meos_h3_iterInitParent
#define _posAngleRads meos_h3_posAngleRads
#define _setGeoRads meos_h3_setGeoRads
#define _v2dAlmostEquals meos_h3_v2dAlmostEquals
#define _v2dIntersect meos_h3_v2dIntersect
#define _v2dMag meos_h3_v2dMag
#define _vec3ToFaceIjk meos_h3_vec3ToFaceIjk
#define _zeroIndexDigits meos_h3_zeroIndexDigits
#define addLinkedCoord meos_addLinkedCoord
#define addLinkedLoop meos_addLinkedLoop
#define addNewLinkedLoop meos_addNewLinkedLoop
#define addNewLinkedPolygon meos_addNewLinkedPolygon
#define baseCellData meos_baseCellData
#define baseCellNeighbor60CCWRots meos_baseCellNeighbor60CCWRots
#define baseCellNeighbors meos_baseCellNeighbors
#define baseCellNumToCell meos_baseCellNumToCell
#define bboxCenter meos_bboxCenter
#define bboxContains meos_bboxContains
#define bboxContainsBBox meos_bboxContainsBBox
#define bboxEquals meos_bboxEquals
#define bboxFromGeoLoop meos_bboxFromGeoLoop
#define bboxFromLinkedGeoLoop meos_bboxFromLinkedGeoLoop
#define bboxHeightRads meos_bboxHeightRads
#define bboxHexEstimate meos_bboxHexEstimate
#define bboxIsTransmeridian meos_bboxIsTransmeridian
#define bboxNormalization meos_bboxNormalization
#define bboxOverlapsBBox meos_bboxOverlapsBBox
#define bboxToCellBoundary meos_bboxToCellBoundary
#define bboxWidthRads meos_bboxWidthRads
#define bboxesFromGeoPolygon meos_bboxesFromGeoPolygon
#define cellBoundaryCrossesGeoLoop meos_cellBoundaryCrossesGeoLoop
#define cellBoundaryCrossesPolygon meos_cellBoundaryCrossesPolygon
#define cellBoundaryInsidePolygon meos_cellBoundaryInsidePolygon
#define cellToBBox meos_cellToBBox
#define cellToLocalIjk meos_cellToLocalIjk
#define cellToVec3 meos_cellToVec3
#define constrainLat meos_constrainLat
#define constrainLng meos_constrainLng
#define countLinkedCoords meos_countLinkedCoords
#define countLinkedLoops meos_countLinkedLoops
#define countLinkedPolygons meos_countLinkedPolygons
#define destroyGeoLoop meos_destroyGeoLoop
#define destroyGeoPolygon meos_destroyGeoPolygon
#define destroyLinkedGeoLoop meos_destroyLinkedGeoLoop
#define directionForNeighbor meos_directionForNeighbor
#define directionForVertexNum meos_directionForVertexNum
#define geoAlmostEqual meos_geoAlmostEqual
#define geoAlmostEqualThreshold meos_geoAlmostEqualThreshold
#define geoLoopAreaRads2 meos_geoLoopAreaRads2
#define geoMultiPolygonAreaRads2 meos_geoMultiPolygonAreaRads2
#define geoMultiPolygonToLinkedGeoPolygon meos_geoMultiPolygonToLinkedGeoPolygon
#define geoPolygonAreaRads2 meos_geoPolygonAreaRads2
#define h3NeighborRotations meos_h3NeighborRotations
#define isClockwiseGeoLoop meos_isClockwiseGeoLoop
#define isClockwiseLinkedGeoLoop meos_isClockwiseLinkedGeoLoop
#define isResolutionClassIII meos_isResolutionClassIII
#define iterDestroyPolygon meos_iterDestroyPolygon
#define iterDestroyPolygonCompact meos_iterDestroyPolygonCompact
#define iterInitBaseCellNum meos_iterInitBaseCellNum
#define iterInitGosper meos_iterInitGosper
#define iterInitParent meos_iterInitParent
#define iterInitPolygon meos_iterInitPolygon
#define iterInitPolygonCompact meos_iterInitPolygonCompact
#define iterInitRes meos_iterInitRes
#define iterStepChild meos_iterStepChild
#define iterStepGosper meos_iterStepGosper
#define iterStepPolygon meos_iterStepPolygon
#define iterStepPolygonCompact meos_iterStepPolygonCompact
#define iterStepRes meos_iterStepRes
#define lineCrossesLine meos_lineCrossesLine
#define lineHexEstimate meos_lineHexEstimate
#define linkedGeoPolygonToGeoMultiPolygon meos_linkedGeoPolygonToGeoMultiPolygon
#define localIjkToCell meos_localIjkToCell
#define makeDirectChild meos_makeDirectChild
#define normalizeLng meos_normalizeLng
#define normalizeMultiPolygon meos_normalizeMultiPolygon
#define pentagonRadiusKm meos_pentagonRadiusKm
#define pointInsideGeoLoop meos_pointInsideGeoLoop
#define pointInsideLinkedGeoLoop meos_pointInsideLinkedGeoLoop
#define pointInsidePolygon meos_pointInsidePolygon
#define scaleBBox meos_scaleBBox
#define setGeoDegs meos_setGeoDegs
#define setH3Index meos_setH3Index
#define validatePolygonFlags meos_validatePolygonFlags
#define vec3ToCell meos_vec3ToCell
#define vertexNumForDirection meos_vertexNumForDirection

#endif /* __H3LIB_NAMES_H__ */
