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
 * @brief Strategy numbers for MEOS indexes
 * @note Definitions borrowed from startnum.h
 */

#ifndef __STRATNUM_H__
#define __STRATNUM_H__

/* PostgreSQL */
#include <postgres.h>

/*****************************************************************************/

#if ! MEOS
/*
 * Strategy numbers identify the semantics that particular operators have
 * with respect to particular operator classes.  In some cases a strategy
 * subtype (an OID) is used as further information.
 */
typedef uint16 StrategyNumber;

#define InvalidStrategy ((StrategyNumber) 0)

/*
 * Strategy numbers common to (some) GiST, SP-GiST and BRIN opclasses.
 *
 * The first few of these come from the R-Tree indexing method (hence the
 * names); the others have been added over time as they have been needed.
 */
#define RTLeftStrategyNumber			1	/* for << */
#define RTOverLeftStrategyNumber		2	/* for &< */
#define RTOverlapStrategyNumber			3	/* for && */
#define RTOverRightStrategyNumber		4	/* for &> */
#define RTRightStrategyNumber			5	/* for >> */
#define RTSameStrategyNumber			6	/* for ~= */
#define RTContainsStrategyNumber		7	/* for @> */
#define RTContainedByStrategyNumber		8	/* for <@ */
#define RTOverBelowStrategyNumber		9	/* for &<| */
#define RTBelowStrategyNumber			10	/* for <<| */
#define RTAboveStrategyNumber			11	/* for |>> */
#define RTOverAboveStrategyNumber		12	/* for |&> */
#define RTOldContainsStrategyNumber		13	/* for old spelling of @> */
#define RTOldContainedByStrategyNumber	14	/* for old spelling of <@ */
#define RTKNNSearchStrategyNumber		15	/* for <-> (distance) */
#define RTContainsElemStrategyNumber	16	/* for range types @> elem */
#define RTAdjacentStrategyNumber		17	/* for -|- */
#define RTEqualStrategyNumber			18	/* for = */
#define RTNotEqualStrategyNumber		19	/* for != */
#define RTLessStrategyNumber			20	/* for < */
#define RTLessEqualStrategyNumber		21	/* for <= */
#define RTGreaterStrategyNumber			22	/* for > */
#define RTGreaterEqualStrategyNumber	23	/* for >= */
#define RTSubStrategyNumber				24	/* for inet >> */
#define RTSubEqualStrategyNumber		25	/* for inet <<= */
#define RTSuperStrategyNumber			26	/* for inet << */
#define RTSuperEqualStrategyNumber		27	/* for inet >>= */
#define RTPrefixStrategyNumber			28	/* for text ^@ */
#define RTOldBelowStrategyNumber		29	/* for old spelling of <<| */
#define RTOldAboveStrategyNumber		30	/* for old spelling of |>> */

#endif /* ! MEOS */

/* Additional operator strategy numbers used for temporal types */

#define RTOverBeforeStrategyNumber    28    /* for &<# */
#define RTBeforeStrategyNumber        29    /* for <<# */
#define RTAfterStrategyNumber         30    /* for #>> */
#define RTOverAfterStrategyNumber     31    /* for #&> */
#define RTOverFrontStrategyNumber     32    /* for &</ */
#define RTFrontStrategyNumber         33    /* for <</ */
#define RTBackStrategyNumber          34    /* for />> */
#define RTOverBackStrategyNumber      35    /* for /&> */

/*****************************************************************************/

#endif /* __STRATNUM_H__ */
