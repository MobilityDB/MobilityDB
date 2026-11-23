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
 * of H3 vendored under h3lib/, each prefixed by `meos_` as H3_PREFIX
 * prefixes its public names
 * @details The build of the object library h3core includes this header
 * before every source, so that no global symbol of the vendored library
 * keeps a name of libh3, which MEOS also links: a static libmeos linked
 * with a static libh3 then defines each name once. The names are those the
 * sources of h3lib/lib define without H3_PREFIX; the test
 * h3lib_names_unit fails when a global symbol of h3core carries none of
 * the prefixes `meos_` and `h3_` (the functions of h3_vertex.c).
 */

#ifndef __H3LIB_NAMES_H__
#define __H3LIB_NAMES_H__

#define FAILED_DIRECTIONS meos_FAILED_DIRECTIONS
#define PENTAGON_ROTATIONS meos_PENTAGON_ROTATIONS
#define PENTAGON_ROTATIONS_REVERSE meos_PENTAGON_ROTATIONS_REVERSE
#define PENTAGON_ROTATIONS_REVERSE_NONPOLAR meos_PENTAGON_ROTATIONS_REVERSE_NONPOLAR
#define PENTAGON_ROTATIONS_REVERSE_POLAR meos_PENTAGON_ROTATIONS_REVERSE_POLAR
#define _adjustOverageClassII meos__adjustOverageClassII
#define _adjustPentVertOverage meos__adjustPentVertOverage
#define _baseCellIsCwOffset meos__baseCellIsCwOffset
#define _baseCellToCCWrot60 meos__baseCellToCCWrot60
#define _baseCellToFaceIjk meos__baseCellToFaceIjk
#define _faceIjkPentToCellBoundary meos__faceIjkPentToCellBoundary
#define _faceIjkPentToVerts meos__faceIjkPentToVerts
#define _faceIjkToBaseCell meos__faceIjkToBaseCell
#define _faceIjkToBaseCellCCWrot60 meos__faceIjkToBaseCellCCWrot60
#define _faceIjkToCellBoundary meos__faceIjkToCellBoundary
#define _faceIjkToH3 meos__faceIjkToH3
#define _faceIjkToVec3 meos__faceIjkToVec3
#define _faceIjkToVerts meos__faceIjkToVerts
#define _getBaseCellDirection meos__getBaseCellDirection
#define _getBaseCellNeighbor meos__getBaseCellNeighbor
#define _getEdgeHexagons meos__getEdgeHexagons
#define _gridDiskDistancesInternal meos__gridDiskDistancesInternal
#define _gridRingInternal meos__gridRingInternal
#define _h3LeadingNonZeroDigit meos__h3LeadingNonZeroDigit
#define _h3Rotate60ccw meos__h3Rotate60ccw
#define _h3Rotate60cw meos__h3Rotate60cw
#define _h3RotatePent60ccw meos__h3RotatePent60ccw
#define _h3RotatePent60cw meos__h3RotatePent60cw
#define _h3ToFaceIjk meos__h3ToFaceIjk
#define _h3ToFaceIjkWithInitializedFijk meos__h3ToFaceIjkWithInitializedFijk
#define _hexRadiusKm meos__hexRadiusKm
#define _ipow meos__ipow
#define _isBaseCellPentagon meos__isBaseCellPentagon
#define _isBaseCellPolarPentagon meos__isBaseCellPolarPentagon
#define _iterInitParent meos__iterInitParent
#define _posAngleRads meos__posAngleRads
#define _setGeoRads meos__setGeoRads
#define _v2dAlmostEquals meos__v2dAlmostEquals
#define _v2dIntersect meos__v2dIntersect
#define _v2dMag meos__v2dMag
#define _vec3ToFaceIjk meos__vec3ToFaceIjk
#define _zeroIndexDigits meos__zeroIndexDigits
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
