/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * tnpoint_selfuncs.c
 * Functions for selectivity estimation of operators on temporal network points
 *
 *  These functions are only stubs, they need to be written TODO
 */

#include "npoint/tnpoint_selfuncs.h"

/*
 *  Selectivity functions for temporal types operators.  These are bogus --
 *  unless we know the actual key distribution in the index, we can't make
 *  a good prediction of the selectivity of these operators.
 *
 *  Note: the values used here may look unreasonably small.  Perhaps they
 *  are.  For now, we want to make sure that the optimizer will make use
 *  of a geometric index if one is available, so the selectivity had better
 *  be fairly small.
 *
 *  In general, GiST needs to search multiple subtrees in order to guarantee
 *  that all occurrences of the same key have been found.  Because of this,
 *  the estimated cost for scanning the index ought to be higher than the
 *  output selectivity would indicate.  gistcostestimate(), over in selfuncs.c,
 *  ought to be adjusted accordingly --- but until we can generate somewhat
 *  realistic numbers here, it hardly matters...
 */

/*****************************************************************************/

/*
 * Selectivity for operators for bounding box operators, i.e., overlaps (&&),
 * contains (@>), contained (<@), and, same (~=). These operators depend on
 * volume. Contains and contained are tighter contraints than overlaps, so
 * the former should produce lower estimates than the latter. Similarly,
 * equals is a tighter constrain tha contains and contained.
 */

PG_FUNCTION_INFO_V1(tnpoint_overlaps_sel);
/**
 * Estimate the selectivity value of the overlap operator for temporal
 * network points
 */
PGDLLEXPORT Datum
tnpoint_overlaps_sel(PG_FUNCTION_ARGS)
{
  PG_RETURN_FLOAT8(0.005);
}

PG_FUNCTION_INFO_V1(tnpoint_overlaps_joinsel);
/**
 * Estimate the join selectivity value of the overlap operator for temporal
 * network points
 */
PGDLLEXPORT Datum
tnpoint_overlaps_joinsel(PG_FUNCTION_ARGS)
{
  PG_RETURN_FLOAT8(0.005);
}

PG_FUNCTION_INFO_V1(tnpoint_contains_sel);
/**
 * Estimate the selectivity value of the contains operator for temporal
 * network points
 */
PGDLLEXPORT Datum
tnpoint_contains_sel(PG_FUNCTION_ARGS)
{
  PG_RETURN_FLOAT8(0.002);
}

PG_FUNCTION_INFO_V1(tnpoint_contains_joinsel);
/**
 * Estimate the join selectivity value of the contains operator for temporal
 * network points
 */
PGDLLEXPORT Datum
tnpoint_contains_joinsel(PG_FUNCTION_ARGS)
{
  PG_RETURN_FLOAT8(0.002);
}

PG_FUNCTION_INFO_V1(tnpoint_same_sel);
/**
 * Estimate the selectivity value of the same operator for temporal
 * network points
 */
PGDLLEXPORT Datum
tnpoint_same_sel(PG_FUNCTION_ARGS)
{
  PG_RETURN_FLOAT8(0.001);
}

PG_FUNCTION_INFO_V1(tnpoint_same_joinsel);
/**
 * Estimate the join selectivity value of the same operator for temporal
 * network points
 */
PGDLLEXPORT Datum
tnpoint_same_joinsel(PG_FUNCTION_ARGS)
{
  PG_RETURN_FLOAT8(0.001);
}

PG_FUNCTION_INFO_V1(tnpoint_adjacent_sel);
/**
 * Estimate the selectivity value of the adjacent operator for temporal
 * network points
 */
PGDLLEXPORT Datum
tnpoint_adjacent_sel(PG_FUNCTION_ARGS)
{
  PG_RETURN_FLOAT8(0.001);
}

PG_FUNCTION_INFO_V1(tnpoint_adjacent_joinsel);
/**
 * Estimate the join selectivity value of the adjacent operator for temporal
 * network points
 */
PGDLLEXPORT Datum
tnpoint_adjacent_joinsel(PG_FUNCTION_ARGS)
{
  PG_RETURN_FLOAT8(0.001);
}

/*****************************************************************************/

/*
 * Selectivity for operators for relative position box operators, i.e.,
 * left (<<), overleft (&<), right (>>), overright (&>),
 * below (<<|), overbelow (&<|), above (|>>), overabove (|&>),
 * front (<</), overfront (&</), back (/>>), overfront (/&>),
 * before (<<#), overbefore (&<#), after (#>>), overafter (#&>).
 */

PG_FUNCTION_INFO_V1(tnpoint_position_sel);
/**
 * Estimate the selectivity value of the relative position operators for
 * temporal network points
 */
PGDLLEXPORT Datum
tnpoint_position_sel(PG_FUNCTION_ARGS)
{
  PG_RETURN_FLOAT8(0.001);
}

PG_FUNCTION_INFO_V1(tnpoint_position_joinsel);
/**
 * Estimate the join selectivity value of the relative position operators for
 * temporal network points
 */
PGDLLEXPORT Datum
tnpoint_position_joinsel(PG_FUNCTION_ARGS)
{
  PG_RETURN_FLOAT8(0.001);
}

/*****************************************************************************/
