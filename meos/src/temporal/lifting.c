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
 * @brief Generic functions for lifting functions and operators on temporal
 * types
 * @details These functions are used for lifting arithmetic operators (`+`,
 * `-`, `*`,
 * `/`), Boolean operators (`and`, `or`, `not`), comparisons (`<`, `<=`, `>`,
 * `>=`), distance (`<->`), spatial relationships (`tcontains`), etc.
 *
 * The lifting of functions and operators must take into account the following
 * characteristic of the function to be lifted
 * 1. The number of arguments of the function
 * - unary functions, such as `degrees` for temporal floats or `round`
 * for temporal points.
 * - binary functions and operators, such as arithmetic operators and comparisons (e.g.,
 * `+` or `<`) or spatial relationships functions (e.g.,`tintersects`).
 * 2. The type of the arguments for binary functions
 * - a temporal type and a base type. In this case the non-lifted function
 * is applied to each instant of the temporal type.
 * - two temporal types. In this case the operands must be synchronized
 * and the function is applied to each pair of synchronized instants.
 * 4. Whether the type of the arguments may vary. For example, temporal
 * numbers can be of different base type (that is, integer and float).
 * Therefore, the types of the arguments must be taken into account when
 * computing binary operators (e.g., `+` or `<`) for temporal numbers.
 * 5. The number of optional parameters of the function
 * - no arguments, such as most spatial relationships functions (e.g.,
 * `tintersects`).
 * - one argument, such as spatial relationships functions that need
 * an additional parameter (e.g., `tdwithin`).
 * - two arguments, e.g., when assembling a temporal point from two temporal
 * floats, the SRID and a boolean flag stating whether the resulting
 * temporal point is geometric or geographic are needed.
 * 6. Whether the function has instantaneous discontinuities at the crossings.
 * Examples of such functions are temporal comparisons for temporal floats
 * or spatiotemporal relationships since the value of the result may
 * change immediately before, at, or immediately after a crossing.
 * 7. Whether intermediate points between synchronized instants must be added
 * to take into account the crossings or the turning points (or local
 * minimum/maximum) of the function. For example, `tfloat + tfloat`
 * only needs to synchronize the arguments while `tfloat * tfloat` requires
 * in addition to add the turning point, which is the timestamp between the
 * two consecutive synchronized instants in which the linear functions
 * defined by the two segments are equal.
 *
 * Examples
 * - `tfloat_degrees => tfunc_temporal`
 * applies the `degrees` function to each instant.
 * - `tfloatseq + base => tfunc_tsequence_base`
 * applies the `+` operator to each instant and results in a `tfloatseq`.
 * - `tfloatseq < base => tfunc_tlinearseq_base_discfn`
 * applies the `<` operator to each instant, if the sequence is equal
 * to the base value in the middle of two consecutive instants add an
 * instantaneous sequence at the crossing. The result is a `tfloatseqset`.
 * - `tfloatseq + tfloatseq => tfunc_tcontseq_tcontseq`
 * synchronizes the sequences and applies the `+` operator to each instant.
 * - `tfloatseq * tfloatseq => tfunc_tcontseq_tcontseq`
 * synchronizes the sequences possibly adding the turning points between
 * two consecutive instants and applies the `*` operator to each instant.
 * The result is a `tfloatseq`.
 * - `tfloatseq < tfloatseq => tfunc_tcontseq_tcontseq_discfn`
 * synchronizes the sequences, applies the `<` operator to each instant,
 * and if there is a crossing in the middle of two consecutive pairs of
 * instants add an instant sequence at the crossing. The result is a
 * `tfloatseqset`.
 * - `tfloatseq <-> tfloatseq => tfunc_tcontseq_tcontseq_single`
 * synchronizes the sequences, applies the `<->` operator to each instant,
 * and if there is a crossing in the middle of two consecutive pairs of
 * instants add an instant sequence at the crossing. The result is a
 * `tfloatseq`.
 *
 * Under an error handler that does not exit, a function that raises an error
 * for one of the values returns its failure value, and the lifting goes on
 * building its result from it. The entry functions #tfunc_temporal,
 * #tfunc_temporal_base, #tfunc_temporal_temporal, #eafunc_temporal_base,
 * #eafunc_temporal_temporal, #lfunc_set and #lfunc_set_bool therefore clear
 * the error number on entry and read it on exit: when the function raised an
 * error they free the result and answer their own failure value, NULL or -1,
 * the error number keeping the code of the error; otherwise they restore the
 * error number found on entry, as #meos_errno_reset describes. A value for
 * which the function raised an error and returned no instant leaves the
 * temporal value it belongs to unbuilt, so that the constructor raises no
 * second error and the error number keeps the code the function raised. A
 * warning or a notice leaves the error number clear, so it leaves the answer
 * as it is. In the extension an error ends the statement where it is raised,
 * so the error number stays clear and the entry functions answer the result
 * of the lifting as it is.
 *
 * A struct named `LiftedFunctionInfo` is used to describe the above
 * characteristics of the function to be lifted. Such struct is filled by
 * the calling function and is passed through the dispatch functions.
 * To avoid code redundancy when coping with functions with 2, 3, and 4
 * arguments, variadic function pointers are used. The idea is sketched next.
 *
 * @note Before calling the function, we must cast it back to a fixed-parameter
 * function pointer. This is necessary because arm64 handles fixed and
 * variadic parameters differently.
 * See: https://developer.apple.com/documentation/apple-silicon/addressing-architectural-differences-in-your-macos-code#Dont-Redeclare-a-Function-to-Have-Variable-Parameters
 * and: https://developer.apple.com/documentation/apple-silicon/addressing-architectural-differences-in-your-macos-code#Enable-Strict-Type-Enforcement-for-Dynamic-Method-Dispatching
 *
 * @code
 * typedef Datum (*varfunc)    (Datum, ...);
 *
 * typedef Datum (*datum_func1)(Datum);
 * typedef Datum (*datum_func2)(Datum, Datum);
 *
 * TInstant *
 * tfunc_tinstant(const TInstant *inst, LiftedFunctionInfo *lfinfo)
 * {
 * Datum resvalue;
 * if (lfinfo->numparam == 0)
 * {
 * datum_func1 func1 = (datum_func1)(*lfinfo->func);
 * resvalue = func1(temporalinst_value(inst));
 * }
 * else if (lfinfo->numparam == 1)
 * {
 * datum_func2 func2 = (datum_func2)(*lfinfo->func);
 * resvalue = func2(temporalinst_value(inst), lfinfo->param[0]);
 * }
 * [...]
 * else
 * {
 * meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
 * "Number of function parameters not supported: %u", lfinfo->numparam);
 * return NULL;
 * }
 * return tinstant_make_free(resvalue, lfinfo->restype, inst->t);
 * }
 *
 * // Definitions for TSequence, TSequence, and TSequenceSet
 * [...]
 *
 * // Dispatch function
 * Temporal *
 * tfunc_temporal(const Temporal *temp, LiftedFunctionInfo *lfinfo)
 * {
 * // Dispatch depending on the temporal type
 * [...]
 * }
 * @endcode
 * An example of use of the lifting functions is given next.
 * @code
 * // Transform the geometry to a geography
 * PGDLLEXPORT Datum
 * tgeompoint_tgeogpoint(PG_FUNCTION_ARGS)
 * {
 * Temporal *temp = PG_GETARG_TEMPORAL_P(0);
 * LiftedFunctionInfo *lfinfo;
 * memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
 * lfinfo->func = (varfunc) &datum_geom_to_geog;
 * lfinfo->numparam = 1;
 * lfinfo->restype = T_TGEOGPOINT;
 * lfinfo->tpfn_base = NULL;
 * lfinfo->tpfn_temp = NULL;
 * Temporal *result = tfunc_temporal(temp, (Datum) NULL, lfinfo);
 * PG_FREE_IF_COPY(temp, 0);
 * PG_RETURN_TEMPORAL_P(result);
 * }
 * @endcode
 */

#include "temporal/lifting.h"

/* C */
#include <assert.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#if JSON
  #include <meos_json.h>
#endif
#include "temporal/temporal_restrict.h"
#include "temporal/tsequence.h"
#include "temporal/tsequenceset.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"

#if JSON
  /* Forward declarations for the JSON null-result lifting functions */
  extern Temporal *tfunc_null_tsequence(const TSequence *seq, LiftedFunctionInfo *lfinfo);
  extern TSequenceSet *tfunc_null_tsequenceset(const TSequenceSet *ss, LiftedFunctionInfo *lfinfo);
#endif

/**
 * @brief Return true when no value of an array is missing
 * @details A value is missing when the function lifted raised an error for
 * it, and building a temporal value from the array would raise a second error
 * that replaces the code of the first
 */
static bool
lifting_values_made(void **values, int count)
{
  for (int i = 0; i < count; i++)
  {
    if (! values[i])
      return false;
  }
  return true;
}

/**
 * @brief Construct a temporal sequence from an array of temporal instants,
 * or return NULL when one of them is missing
 * @note See #lifting_values_made
 */
static TSequence *
lifting_tsequence_make(TInstant **instants, int count, bool lower_inc,
  bool upper_inc, interpType interp, bool normalize)
{
  if (! lifting_values_made((void **) instants, count))
    return NULL;
  return tsequence_make(instants, count, lower_inc, upper_inc, interp,
    normalize);
}

/**
 * @brief Construct a temporal sequence from an array of temporal instants and
 * free the array and the instants, or return NULL when one of them is missing
 * @note See #lifting_values_made
 */
static TSequence *
lifting_tsequence_make_free(TInstant **instants, int count, bool lower_inc,
  bool upper_inc, interpType interp, bool normalize)
{
  if (! lifting_values_made((void **) instants, count))
  {
    pfree_array((void **) instants, count);
    return NULL;
  }
  return tsequence_make_free(instants, count, lower_inc, upper_inc, interp,
    normalize);
}

/**
 * @brief Construct a temporal sequence set from an array of temporal
 * sequences and free the array and the sequences, or return NULL when one of
 * them is missing
 * @note See #lifting_values_made
 */
static TSequenceSet *
lifting_tsequenceset_make_free(TSequence **sequences, int count,
  bool normalize)
{
  if (! lifting_values_made((void **) sequences, count))
  {
    pfree_array((void **) sequences, count);
    return NULL;
  }
  return tsequenceset_make_free(sequences, count, normalize);
}

/**
 * @brief Convert a temporal instant into a temporal sequence and free the
 * instant, or return NULL when the instant is missing
 * @note See #lifting_values_made
 */
static TSequence *
lifting_tinstant_to_tsequence_free(TInstant *inst, interpType interp)
{
  if (! inst)
    return NULL;
  return tinstant_to_tsequence_free(inst, interp);
}

/*****************************************************************************
 * Result of a lifted operation
 *****************************************************************************/

/**
 * @brief Return the result of a lifted operation, or NULL when the function
 * it applies raised an error for one of the values
 * @param[in] result Result built by the lifting, freed on an error
 * @param[in] last_errno Error number found on entry, returned by
 * #meos_errno_reset
 * @note See the note on errors in the file comment of lifting.c
 */
void *
lifting_result(void *result, int last_errno)
{
  if (meos_errno())
  {
    if (result)
      pfree(result);
    return NULL;
  }
  meos_errno_restore(last_errno);
  return result;
}

/**
 * @brief Return the result of a lifted ever or always operation, or -1 when
 * the function it applies raised an error for one of the values
 * @param[in] result Result of the operation
 * @param[in] last_errno Error number found on entry, returned by
 * #meos_errno_reset
 * @note See the note on errors in the file comment of lifting.c
 */
int
lifting_result_int(int result, int last_errno)
{
  if (meos_errno())
    return -1;
  meos_errno_restore(last_errno);
  return result;
}

/**
 * @brief Return a distance read from a temporal distance, or -1.0 when the
 * temporal distance raised an error
 * @details A temporal distance is NULL both where the operands share no time,
 * which the caller answers with @p DBL_MAX, and where it raises an error,
 * which the error number set since #meos_errno_reset tells apart
 * @param[in] result Distance, @p DBL_MAX where the temporal distance is NULL
 * @param[in] last_errno Error number found on entry, returned by
 * #meos_errno_reset
 */
double
distance_result(double result, int last_errno)
{
  if (meos_errno())
    return -1.0;
  meos_errno_restore(last_errno);
  return result;
}

/*****************************************************************************
 * Functions where the argument is a base type
 *****************************************************************************/

/**
 * @brief Apply the variadic function with the optional arguments to a base
 * value
 * @note We must cast the function pointer to a fixed-parameter function
 * pointer before calling it. See the note at the beginning of the file.
 */
static Datum
lfunc_base(Datum value, LiftedFunctionInfo *lfinfo)
{
  /* Lifted functions may have from 0 to MAX_PARAMS parameters */
  assert(lfinfo->numparam >= 0 && lfinfo->numparam <= MAX_PARAMS);
  if (lfinfo->numparam == 0)
  {
    datum_func1 func1 = (datum_func1)(*lfinfo->func);
    return func1(value);
  }
  else if (lfinfo->numparam == 1)
  {
    datum_func2 func2 = (datum_func2)(*lfinfo->func);
    return func2(value, lfinfo->param[0]);
  }
  else if (lfinfo->numparam == 2)
  {
    datum_func3 func3 = (datum_func3)(*lfinfo->func);
    return func3(value, lfinfo->param[0], lfinfo->param[1]);
  }
  else if (lfinfo->numparam == 3)
  {
    datum_func4 func4 = (datum_func4)(*lfinfo->func);
    return func4(value, lfinfo->param[0], lfinfo->param[1],
      lfinfo->param[2]);
  }
  else if (lfinfo->numparam == 4)
  {
    datum_func5 func5 = (datum_func5)(*lfinfo->func);
    return func5(value, lfinfo->param[0], lfinfo->param[1], lfinfo->param[2],
      lfinfo->param[3]);
  }
  else /* lfinfo->numparam == 5 */
  {
    datum_func6 func6 = (datum_func6)(*lfinfo->func);
    return func6(value, lfinfo->param[0], lfinfo->param[1], lfinfo->param[2],
      lfinfo->param[3], lfinfo->param[4]);
  }
}

/*****************************************************************************
 * Functions where the argument is a set type.
 * The function is applied to the composing elements.
 *****************************************************************************/

/**
 * @brief Apply a function to the elements of a set
 * @note The error number is read by #lfunc_set
 */
static Set *
lfunc_set_dispatch(const Set *set, LiftedFunctionInfo *lfinfo)
{
  assert(set); assert(lfinfo); 
  MeosType basetype = settype_basetype(lfinfo->restype);
  bool typbyval = basetype_byvalue(basetype);
  Datum *values = palloc(sizeof(Datum) * set->count);
  int count = 0;
  for (int i = 0; i < set->count; i++)
  {
    Datum resvalue = lfunc_base(SET_VAL_N(set, i), lfinfo);
    /* If the function does not return NULL */
    if (! lfinfo->resnull)
    {
      /* A non-byvalue result that is unexpectedly NULL means the lifted
       * function failed without flagging resnull (e.g. it raised a non-fatal
       * meos_error and returned NULL); abort cleanly instead of feeding a
       * NULL element to set_make. */
      if (! typbyval && ! resvalue)
      {
        for (int j = 0; j < count; j++)
          DATUM_FREE(values[j], basetype);
        pfree(values);
        return NULL;
      }
      values[count++] = resvalue;
    }
    else
    {
      /* Check if resulting value is a value error or a NULL pointer */
      if ((! typbyval && ! resvalue) ||
          (typbyval && datum_eq(resvalue, lfinfo->reserror, basetype)))
#if JSON
      {
        if (lfinfo->resnull == NULL_JSON_NULL)
        {
          const char *str = "null";
          values[count++] = (lfinfo->restype == T_TEXTSET) ? 
            PointerGetDatum(text_in(str)) : 
            PointerGetDatum(pg_jsonb_in(str));
        }
        else if (lfinfo->resnull == NULL_DELETE)
        {
          /* Nothing to do, ignore the value */
        }
        else
        {
          if (lfinfo->resnull == NULL_ERROR)
          {
            meos_error(ERROR, MEOS_ERR_NULL_RESULT,
              "The lifted operation returned NULL");
          }
          /* For both NULL_ERROR and NULL_RETURN */
          for (int j = 0; j < count; j++)
            DATUM_FREE(values[j], basetype);
          pfree(values);
          return NULL;
        }
      }
      else
        values[count++] = resvalue;
#else
    return NULL;
#endif /* JSON */
    }
  }
  /* A path query matching nothing (or every element deleted via NULL_DELETE)
   * leaves no values; sets must be non-empty, so there is no result. */
  if (count == 0)
  {
    pfree(values);
    return NULL;
  }
  /* set_make_exp copies the element data into the set inline; the per-element
   * results are freshly computed (owned -- the error paths above DATUM_FREE
   * them), so free the by-reference ones here. set_make_free would free only
   * the array, leaking the values. */
  Set *result = set_make_exp(values, count, count, basetype, ORDER);
  if (! typbyval)
    for (int i = 0; i < count; i++)
      DATUM_FREE(values[i], basetype);
  pfree(values);
  return result;
}

/**
 * @brief Apply a function to the elements of a set
 * @param[in] set Set
 * @param[in] lfinfo Information about the lifted function
 * @note An error raised by the function applied makes the answer NULL, see
 * the note on errors at the beginning of the file
 */
Set *
lfunc_set(const Set *set, LiftedFunctionInfo *lfinfo)
{
  int last_errno = meos_errno_reset();
  return (Set *) lifting_result(lfunc_set_dispatch(set, lfinfo),
    last_errno);
}

/**
 * @brief Apply a lifted predicate to the elements of a set
 * @param[in] set Set
 * @param[in] lfinfo Information about the lifted function
 * @param[out] count Number of values in the output array
 * @note A predicate returns a Boolean for every element of the set. There is
 * no Boolean set type, and a set would remove the duplicate results and
 * reorder them, so the result is an array in the order of the elements
 * @note An error raised by the function applied makes the answer NULL, see
 * the note on errors at the beginning of the file
 */
bool *
lfunc_set_bool(const Set *set, LiftedFunctionInfo *lfinfo, int *count)
{
  assert(set); assert(lfinfo); assert(count);
  int last_errno = meos_errno_reset();
  bool *result = palloc(sizeof(bool) * set->count);
  int nvalues = 0;
  for (int i = 0; i < set->count; i++)
  {
    Datum value = lfunc_base(SET_VAL_N(set, i), lfinfo);
    /* A value without an answer is left out with NULL_DELETE and leaves the
     * result without one otherwise, as in #lfunc_set_dispatch */
    if (lfinfo->resnull && datum_eq(value, lfinfo->reserror, T_BOOL))
    {
#if JSON
      if (lfinfo->resnull == NULL_DELETE)
        continue;
#endif /* JSON */
      pfree(result);
      meos_errno_restore(last_errno);
      return NULL;
    }
    result[nvalues++] = DatumGetBool(value);
  }
  /* Every value left out leaves no result, as in #lfunc_set_dispatch */
  if (nvalues == 0)
  {
    pfree(result);
    meos_errno_restore(last_errno);
    return NULL;
  }
  result = (bool *) lifting_result(result, last_errno);
  if (! result)
    return NULL;
  *count = nvalues;
  return result;
}

/*****************************************************************************
 * Functions where the argument is a temporal type.
 * The function is applied to the composing instants.
 *****************************************************************************/

/**
 * @brief Apply a lifted function with the optional arguments to a temporal
 * instant
 * @param[in] inst Temporal value
 * @param[in] lfinfo Information about the lifted function
 */
TInstant *
tfunc_tinstant(const TInstant *inst, LiftedFunctionInfo *lfinfo)
{
  Datum resvalue = lfunc_base(tinstant_value_p(inst), lfinfo);
#if JSON
  MeosType basetype = temptype_basetype(lfinfo->restype);
  bool typbyval = basetype_byvalue(basetype);
  /* If we must check NULLs, check if the resulting value is an error or a
     NULL pointer */
  if (lfinfo->resnull && ((! typbyval && ! resvalue) ||
      (typbyval && datum_eq(resvalue, lfinfo->reserror, basetype))))
  {
    if (lfinfo->resnull != NULL_JSON_NULL)
      return NULL;
    const char *str = "null";
    resvalue = (lfinfo->restype == T_TTEXT) ?
      PointerGetDatum(pg_json_in(str)) : PointerGetDatum(pg_jsonb_in(str));
  }
#endif /* JSON */
  return tinstant_make_free(resvalue, lfinfo->restype, inst->t);
}

/*****************************************************************************
 * Turning-point densification of unary lifted functions.
 *
 * A unary function lifted over a LINEAR temporal float is itself emitted as a
 * piecewise-linear temporal float, so the result must carry a vertex at each of
 * the input segment's critical points. Two strategies are used, the exact one
 * first:
 *
 *   1. #tfunc_tlinearseq_turnpt (#tpfn_unary): inserts the turning points of
 *      each segment, located in closed form, however many there are. One
 *      suffices for the monotone lifts exp, ln and log10; sin and cos have one
 *      at each of their extrema, theta = (k + 1/2)*pi and theta = k*pi, so
 *      their result is exact.
 *   2. #tfunc_tlinearseq_adaptive (#tpfn_adaptive): adaptive recursive
 *      bisection, for functions whose critical points are NOT analytic. Used
 *      by tan, which is monotone within each branch but whose curvature grows
 *      without bound near its poles: a segment is bisected while a change
 *      metric across it exceeds a tolerance and the recursion depth is below a
 *      cap.
 *
 * The adaptive metric is the magnitude of the argument change across the
 * segment. The argument is the temporal float itself (an angle in radians for
 * tan), so the metric is the span of the float. Bounding each sub-segment to a
 * small argument span guarantees it spans only a small arc of the curve,
 * within which the lifted function is well approximated by its chord. An
 * output-space (chord-error) metric is deliberately NOT used: for a periodic
 * function the segment midpoint can land exactly on the endpoint chord (e.g. a
 * function equal to 0 at both endpoints and at the midpoint), which would
 * falsely report linearity and skip a whole period. The argument-span metric
 * never cancels. #NORMALIZE in #tsequence_make_free then collapses any
 * collinear runs the uniform-by-argument sampling produced.
 *****************************************************************************/

/**
 * @brief Adaptive sub-sampling tolerances for densifying a non-analytic unary
 * lifted function (tan) over a linear segment
 * @details @p MEOS_ADAPTIVE_TOL is the argument-span tolerance in the input
 * space: a segment is subdivided while its float argument spans more than this
 * many units. For tan the argument is radians, so 0.05 rad (~2.86 deg), the
 * turn a sample interval of the traversed area of a temporal rigid geometry
 * spans at most (`TRGEO_TRAVERSED_AREA_ANGLE_TOL`), bounds the per-segment
 * chord error away from a pole to about tol^2/8 ~ 3e-4. @p
 * MEOS_ADAPTIVE_MAX_DEPTH bounds the recursion so a pathological segment (one
 * straddling very many periods, or approaching a tan pole) cannot subdivide
 * without end: a single segment yields at most 2^depth sub-samples, but the
 * adaptive test stops far earlier wherever the argument span is already below
 * the tolerance.
 */
#define MEOS_ADAPTIVE_TOL       0.05
#define MEOS_ADAPTIVE_MAX_DEPTH 16

/**
 * @brief Return an array of instants with room for at least @p count of them,
 * enlarging @p instants when it holds fewer
 * @param[in] instants Array of instants
 * @param[in,out] maxcount Number of instants the array holds
 * @param[in] count Number of instants the array must hold
 */
static TInstant **
lifting_instants_room(TInstant **instants, int *maxcount, int count)
{
  if (count <= *maxcount)
    return instants;
  *maxcount = Max(count, 2 * (*maxcount));
  return repalloc(instants, sizeof(TInstant *) * (*maxcount));
}

/**
 * @brief Return an array of @p count values for a turning point function
 * that counts more instants than the array of #TPFN_STACK_CAP values it
 * filled, freeing those values, or NULL if it writes no values
 * @details As #lifting_instants_room() enlarges the array of instants, this
 * function replaces the array of values of a reader, whose turning point
 * function is called again with the larger one.
 * @param[in] values Values the function wrote, or NULL
 * @param[in] basetype Type of the values
 * @param[in] count Number of values of the array returned
 */
static Datum *
lifting_values_room(Datum *values, MeosType basetype, int count)
{
  if (! values)
    return NULL;
  if (! basetype_byvalue(basetype))
  {
    for (int k = 0; k < TPFN_STACK_CAP; k++)
      pfree(DatumGetPointer(values[k]));
  }
  return palloc(sizeof(Datum) * count);
}

/**
 * @brief Add to an array of instants those a turning point function writes
 * for a segment, its ends included (#LiftedFunctionInfo.tpfn_ends), and
 * return the number of instants of the array
 * @details An instant at the timestamp of the last one of the array, the end
 * of the segment before or an instant of this segment, is merged with it by
 * tpfn_merge, which keeps the instant of the array when it returns a value
 * equal to it.
 * @param[in,out] instants Array of instants, with room for @p ntps more
 * @param[in] ninsts Number of instants of the array
 * @param[in] tps,values Instants of the segment and their values, which the
 * function frees
 * @param[in] ntps Number of instants of the segment
 * @param[in] lfinfo Information about the lifted function
 */
static int
lifting_append_ends(TInstant **instants, int ninsts, const TimestampTz *tps,
  Datum *values, int ntps, const LiftedFunctionInfo *lfinfo)
{
  assert(lfinfo->tpfn_ends); assert(lfinfo->tpfn_merge);
  MeosType resbasetype = temptype_basetype(lfinfo->restype);
  /* A value passed by reference is freed once its instant holds a copy */
  bool byval = basetype_byvalue(resbasetype);
  /* Timestamp of the last instant of the array, if any */
  TimestampTz lastt = (ninsts > 0) ? instants[ninsts - 1]->t : 0;
  for (int k = 0; k < ntps; k++)
  {
    if (ninsts > 0 && lastt >= tps[k])
    {
      Datum last = tinstant_value_p(instants[ninsts - 1]);
      Datum merged = lfinfo->tpfn_merge(last, values[k]);
      if (merged != last && ! datum_eq(merged, last, resbasetype))
      {
        pfree(instants[ninsts - 1]);
        instants[ninsts - 1] = tinstant_make(merged, lfinfo->restype, tps[k]);
        lastt = tps[k];
      }
      if (! byval)
        pfree(DatumGetPointer(values[k]));
      continue;
    }
    instants[ninsts++] = tinstant_make(values[k], lfinfo->restype, tps[k]);
    lastt = tps[k];
    if (! byval)
      pfree(DatumGetPointer(values[k]));
  }
  return ninsts;
}

/**
 * @brief Append a lifted result instant to a growing buffer, doubling the
 * capacity as needed
 */
static void
tfunc_adaptive_emit(Datum resvalue, MeosType restype, TimestampTz t,
  TInstant ***buf, int *buf_n, int *buf_cap)
{
  if (*buf_n == *buf_cap)
  {
    *buf_cap *= 2;
    *buf = repalloc(*buf, sizeof(TInstant *) * (*buf_cap));
  }
  (*buf)[(*buf_n)++] = tinstant_make_free(resvalue, restype, t);
}

/**
 * @brief Recursively bisect a linear segment of a unary lifted function,
 * appending result instants on the half-open interval @p (t1, t2]
 * @details The convergence test is the argument span
 * across the segment: if the input float moves by less than #MEOS_ADAPTIVE_TOL
 * (or the depth cap is reached, or the half-segment is sub-microsecond) the
 * arc is small enough that the lifted chord is accurate, and only the segment
 * endpoint @p t2 is emitted. Otherwise the midpoint is computed, the left half
 * is recursed (which emits the midpoint as its terminal endpoint), and the
 * right half is recursed. The caller is responsible for emitting the very
 * first instant @p t1 (as the previous segment's endpoint or the sequence
 * start), so that consecutive segments do not duplicate the shared instant.
 * @param[in] value1,value2 Input base values at @p t1,@p t2 (segment endpoints)
 * @param[in] t1,t2 Segment endpoints
 * @param[in] temptype Temporal type of the input (for value interpolation)
 * @param[in] basetype Base type of the input (a continuous float)
 * @param[in] lfinfo Information about the lifted function
 * @param[in] depth Current recursion depth
 * @param[in,out] buf,buf_n,buf_cap Growing result-instant buffer
 */
static void
tfunc_adaptive_bisect(Datum value1, Datum value2, TimestampTz t1,
  TimestampTz t2, MeosType temptype, MeosType basetype,
  LiftedFunctionInfo *lfinfo, int depth, TInstant ***buf, int *buf_n,
  int *buf_cap)
{
  assert(basetype == T_FLOAT8);
  /* Argument span across the segment */
  double dv = fabs(DatumGetFloat8(value2) - DatumGetFloat8(value1));
  if (dv < MEOS_ADAPTIVE_TOL || depth >= MEOS_ADAPTIVE_MAX_DEPTH ||
      t2 - t1 <= 1)
  {
    /* The argument barely moves across [t1, t2], so the lifted chord is a
     * good approximation: emit only the segment endpoint t2 (t1 was emitted
     * by the caller). */
    tfunc_adaptive_emit(lfunc_base(value2, lfinfo), lfinfo->restype, t2, buf,
      buf_n, buf_cap);
    return;
  }
  /* Bisect: interpolate the argument at the midpoint, recurse on the left
   * half (which emits the midpoint as its terminal endpoint), then on the
   * right half. */
  TimestampTz tm = t1 + (t2 - t1) / 2;
  Datum vm = tsegment_value_at_timestamptz(value1, value2, temptype, t1, t2,
    tm);
  tfunc_adaptive_bisect(value1, vm, t1, tm, temptype, basetype, lfinfo,
    depth + 1, buf, buf_n, buf_cap);
  tfunc_adaptive_bisect(vm, value2, tm, t2, temptype, basetype, lfinfo,
    depth + 1, buf, buf_n, buf_cap);
  DATUM_FREE(vm, basetype);
}

/**
 * @brief Apply a unary lifted function to a temporal sequence with adaptive
 * densification
 * @details The fallback path for unary lifts whose critical points are not
 * known analytically (tan, whose curvature grows without bound near its poles):
 * each non-constant input segment is densified by adaptive recursive bisection
 * (#tfunc_adaptive_bisect). Constant segments are passed through unchanged.
 * Mirrors the structure of #tfunc_tlinearseq_turnpt, with the instants located
 * by a depth-capped bisection rather than in closed form.
 * @param[in] seq Temporal sequence with LINEAR interpolation
 * @param[in] lfinfo Information about the lifted function (tpfn_adaptive set)
 */
static TSequence *
tfunc_tlinearseq_adaptive(const TSequence *seq, LiftedFunctionInfo *lfinfo)
{
  MeosType basetype = temptype_basetype(seq->temptype);
  /* Generous initial capacity; the buffer doubles on demand */
  int buf_cap = seq->count * 4;
  TInstant **instants = palloc(sizeof(TInstant *) * buf_cap);
  int ninsts = 0;
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  Datum value1 = tinstant_value_p(inst1);
  /* Emit the first instant */
  tfunc_adaptive_emit(lfunc_base(value1, lfinfo), lfinfo->restype, inst1->t,
    &instants, &ninsts, &buf_cap);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Datum value2 = tinstant_value_p(inst2);
    if (datum_eq(value1, value2, basetype))
      /* Constant segment: just emit the endpoint */
      tfunc_adaptive_emit(lfunc_base(value2, lfinfo), lfinfo->restype,
        inst2->t, &instants, &ninsts, &buf_cap);
    else
      tfunc_adaptive_bisect(value1, value2, inst1->t, inst2->t,
        seq->temptype, basetype, lfinfo, 0, &instants, &ninsts, &buf_cap);
    inst1 = inst2;
    value1 = value2;
  }
  return lifting_tsequence_make_free(instants, ninsts, seq->period.lower_inc,
    seq->period.upper_inc, LINEAR, NORMALIZE);
}

/**
 * @brief Walk of #tfunc_tlinearseq_turnpt
 * @details The turning point function counts more than two instants in a
 * segment if @p many is true; each caller passes a literal flag, for which
 * the walk is compiled apart
 */
static pg_attribute_always_inline TSequence *
tfunc_tlinearseq_turnpt_walk(const TSequence *seq, LiftedFunctionInfo *lfinfo,
  bool many)
{
  MeosType basetype = temptype_basetype(seq->temptype);
  MeosType resbasetype = temptype_basetype(lfinfo->restype);
  /* A segment adds its start instant and at most two turning points unless
   * its turning point function counts more, which enlarges the array */
  int maxinsts = seq->count * 3;
  TInstant **instants = palloc(sizeof(TInstant *) * maxinsts);
  int ninsts = 0;
  TimestampTz tpbuf[TPFN_STACK_CAP];
  /* Capacity of the array a turning point function writes */
  int cap = many ? TPFN_STACK_CAP : 2;
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  Datum value1 = tinstant_value_p(inst1);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Datum value2 = tinstant_value_p(inst2);
    instants[ninsts++] = tfunc_tinstant(inst1, lfinfo);
    /* Skip densification on constant segments */
    if (! datum_eq(value1, value2, basetype))
    {
      TimestampTz *tps = tpbuf;
      int ntps = lfinfo->tpfn_unary(value1, value2, inst1->t, inst2->t, tps,
        cap);
      assert(many || ntps <= 2);
      if (many && unlikely(ntps > 2))
      {
        if (ntps > TPFN_STACK_CAP)
        {
          tps = palloc(sizeof(TimestampTz) * ntps);
          lfinfo->tpfn_unary(value1, value2, inst1->t, inst2->t, tps, ntps);
        }
        /* The array holds three instants for each segment left, two turning
         * points and the start instant, so it grows only when there are more */
        instants = lifting_instants_room(instants, &maxinsts,
          ninsts + ntps + 3 * (seq->count - i));
      }
      if (ntps == 1)
      {
        /* One turning point, the case of every lift but sin and cos */
        Datum tpvalue = tsegment_value_at_timestamptz(value1, value2,
          inst1->temptype, inst1->t, inst2->t, tpbuf[0]);
        Datum res = lfunc_base(tpvalue, lfinfo);
        instants[ninsts++] = tinstant_make(res, lfinfo->restype, tpbuf[0]);
        DATUM_FREE(tpvalue, basetype); DATUM_FREE(res, resbasetype);
      }
      else
      {
        for (int k = 0; k < ntps; k++)
        {
          Datum tpvalue = tsegment_value_at_timestamptz(value1, value2,
            inst1->temptype, inst1->t, inst2->t, tps[k]);
          Datum res = lfunc_base(tpvalue, lfinfo);
          instants[ninsts++] = tinstant_make(res, lfinfo->restype, tps[k]);
          DATUM_FREE(tpvalue, basetype); DATUM_FREE(res, resbasetype);
        }
        if (many && unlikely(ntps > TPFN_STACK_CAP))
          pfree(tps);
      }
    }
    inst1 = inst2;
    value1 = value2;
  }
  instants[ninsts++] = tfunc_tinstant(inst1, lfinfo);
  return lifting_tsequence_make_free(instants, ninsts, seq->period.lower_inc,
    seq->period.upper_inc, LINEAR, NORMALIZE);
}

/**
 * @brief Apply a unary lifted function to a temporal sequence with
 * turning-point densification
 * @details For each non-constant segment, @p tpfn_unary returns the turning
 * points strictly inside it, in increasing order and however many there are,
 * and the lifted value at each of them is inserted into the result sequence.
 * Mirrors the binary #tfunc_tlinearseq_base_turnpt path of
 * #tfunc_temporal_base.
 * @param[in] seq Temporal sequence with LINEAR interpolation
 * @param[in] lfinfo Information about the lifted function (tpfn_unary set)
 */
static TSequence *
tfunc_tlinearseq_turnpt(const TSequence *seq, LiftedFunctionInfo *lfinfo)
{
  assert(! lfinfo->tpfn_value);
  if (lfinfo->tpfn_many)
    return tfunc_tlinearseq_turnpt_walk(seq, lfinfo, true);
  return tfunc_tlinearseq_turnpt_walk(seq, lfinfo, false);
}

/**
 * @brief Apply a lifted function to a temporal sequence
 * @param[in] seq Temporal value
 * @param[in] lfinfo Information about the lifted function
 */
TSequence *
tfunc_tsequence(const TSequence *seq, LiftedFunctionInfo *lfinfo)
{
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  /* Densified LINEAR output at the turning points located in closed form,
   * however many a segment has (the sin/cos extrema) */
  if (lfinfo->tpfn_unary && interp == LINEAR)
    return tfunc_tlinearseq_turnpt(seq, lfinfo);
  /* Densified LINEAR output via adaptive recursive bisection, for unary
   * functions whose turning points are not analytic (the curvature and the
   * poles of tan) */
  if (lfinfo->tpfn_adaptive && interp == LINEAR)
    return tfunc_tlinearseq_adaptive(seq, lfinfo);

  /* Plain per-instant lift */
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = tfunc_tinstant(TSEQUENCE_INST_N(seq, i), lfinfo);
  /* Honor reslinear when the input is LINEAR and the producer declared the
   * result is not.  Same idiom as tfunc_tsequence_base. */
  if (interp == LINEAR && ! lfinfo->reslinear)
    interp = STEP;
  /* The last two values of sequences with step interpolation and exclusive
   * upper bound must be equal */
  if (! seq->period.upper_inc && interp == STEP && seq->count >= 2 &&
      instants[seq->count - 2] && instants[seq->count - 1])
  {
    TInstant *inst = instants[seq->count - 1];
    Datum value = tinstant_value_p(instants[seq->count - 2]);
    instants[seq->count - 1] = tinstant_make(value, lfinfo->restype, inst->t);
    pfree(inst);
  }
  return lifting_tsequence_make_free(instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc, interp, NORMALIZE);
}

/**
 * @brief Apply a lifted function to a temporal sequence set
 * @param[in] ss Temporal value
 * @param[in] lfinfo Information about the lifted function
 */
TSequenceSet *
tfunc_tsequenceset(const TSequenceSet *ss, LiftedFunctionInfo *lfinfo)
{
#if JSON
  /* If the function may return NULL */
  if (lfinfo->resnull)
    return tfunc_null_tsequenceset(ss, lfinfo);
#endif /* JSON */
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tfunc_tsequence(TSEQUENCESET_SEQ_N(ss, i), lfinfo);
  return lifting_tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @brief Apply a lifted function to a temporal value
 * @note The error number is read by #tfunc_temporal
 */
static Temporal *
tfunc_temporal_dispatch(const Temporal *temp, LiftedFunctionInfo *lfinfo)
{
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tfunc_tinstant((TInstant *) temp, lfinfo);
    case TSEQUENCE:
#if JSON
      return lfinfo->resnull ?
        (Temporal *) tfunc_null_tsequence((TSequence *) temp, lfinfo) :
        (Temporal *) tfunc_tsequence((TSequence *) temp, lfinfo);
#else
      return (Temporal *) tfunc_tsequence((TSequence *) temp, lfinfo);
#endif /* JSON */
    default: /* TSEQUENCESET */
      return (Temporal *) tfunc_tsequenceset((TSequenceSet *) temp, lfinfo);
  }
}

/**
 * @brief Apply a lifted function to a temporal value
 * @param[in] temp Temporal value
 * @param[in] lfinfo Information about the lifted function
 * @note An error raised by the function applied makes the answer the
 * failure value, see the note on errors at the beginning of the file
 */
Temporal *
tfunc_temporal(const Temporal *temp, LiftedFunctionInfo *lfinfo)
{
  int last_errno = meos_errno_reset();
  return (Temporal *) lifting_result(tfunc_temporal_dispatch(temp, lfinfo),
    last_errno);
}

/*****************************************************************************
 * Functions where the arguments are a temporal type and a base type.
 * Notice that their base type may be different, for example, tfloat + int
 *****************************************************************************/

/**
 * @brief Apply the variadic function with the optional arguments to the base
 * values, taking into account that their type may be different
 * @details The engines that call a base function directly rather than through
 * a lift, as the nearest approach and the temporal dwithin do, apply it with
 * its parameters through this function
 * @note We must cast the function pointer to a fixed-parameter function pointer
 * before calling it. See the note at the beginning of the file.
 */
Datum
tfunc_base_base(Datum value1, Datum value2, LiftedFunctionInfo *lfinfo)
{
  /* Lifted functions may have from 0 to MAX_PARAMS parameters */
  assert(lfinfo->numparam >= 0 && lfinfo->numparam <= MAX_PARAMS);
  /* The two base values are the arguments of the function, in the order the
   * lift states, and the parameters follow them as #lfunc_base passes them
   * after its one base value */
  Datum v1 = lfinfo->invert ? value2 : value1;
  Datum v2 = lfinfo->invert ? value1 : value2;
  const Datum *p = lfinfo->param;
  if (lfinfo->numparam == 0)
  {
    datum_func2 func2 = (datum_func2)(*lfinfo->func);
    return func2(v1, v2);
  }
  else if (lfinfo->numparam == 1)
  {
    datum_func3 func3 = (datum_func3)(*lfinfo->func);
    return func3(v1, v2, p[0]);
  }
  else if (lfinfo->numparam == 2)
  {
    datum_func4 func4 = (datum_func4)(*lfinfo->func);
    return func4(v1, v2, p[0], p[1]);
  }
  else if (lfinfo->numparam == 3)
  {
    datum_func5 func5 = (datum_func5)(*lfinfo->func);
    return func5(v1, v2, p[0], p[1], p[2]);
  }
  else if (lfinfo->numparam == 4)
  {
    datum_func6 func6 = (datum_func6)(*lfinfo->func);
    return func6(v1, v2, p[0], p[1], p[2], p[3]);
  }
  else /* lfinfo->numparam == 5 */
  {
    datum_func7 func7 = (datum_func7)(*lfinfo->func);
    return func7(v1, v2, p[0], p[1], p[2], p[3], p[4]);
  }
}

/**
 * @brief Apply a lifted function to a temporal value and the base value
 * @param[in] inst Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 */
TInstant *
tfunc_tinstant_base(const TInstant *inst, Datum value,
  LiftedFunctionInfo *lfinfo)
{
  Datum resvalue = tfunc_base_base(tinstant_value_p(inst), value, lfinfo);
  return tinstant_make_free(resvalue, lfinfo->restype, inst->t);
}

/**
 * @brief Apply a lifted function to a temporal value and a base value
 * @param[in] seq Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 */
TSequence *
tfunc_tsequence_base(const TSequence *seq, Datum value,
  LiftedFunctionInfo *lfinfo)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = tfunc_tinstant_base(TSEQUENCE_INST_N(seq, i), value, lfinfo);
  /* Set the interpolation depending on the one of the sequence and the result
   * as stated in the `lfinfo` structure */
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  if (interp == LINEAR && ! lfinfo->reslinear)
    interp = STEP;
  /* The last two values of sequences with step interpolation and exclusive
     upper bound must be equal */
  if (! seq->period.upper_inc && interp == STEP &&
      instants[seq->count - 2] && instants[seq->count - 1])
  {
    TInstant *inst = instants[seq->count - 1];
    value = tinstant_value_p(instants[seq->count - 2]);
    instants[seq->count - 1] = tinstant_make(value, lfinfo->restype,
      instants[seq->count - 1]->t);
    pfree(inst);
  }
  /* Create the result */
  return lifting_tsequence_make_free(instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc, interp, NORMALIZE);
}

/**
 * @brief Walk of #tfunc_tlinearseq_base_turnpt
 * @details The turning point function counts more than two instants in a
 * segment if @p many is true and writes their values if @p values is true;
 * each caller passes literal flags, for which the walk is compiled apart
 */
static pg_attribute_always_inline int
tfunc_tlinearseq_base_turnpt_walk(const TSequence *seq, Datum value,
  LiftedFunctionInfo *lfinfo, TSequence **result,
  bool many, bool values)
{
  MeosType basetype = temptype_basetype(seq->temptype);
  MeosType resbasetype = temptype_basetype(lfinfo->restype);
  int ninsts = 0;
  /* A segment adds its start instant and at most two turning points unless
   * its turning point function counts more, which enlarges the array */
  int maxinsts = seq->count * 3;
  TInstant **instants = palloc(sizeof(TInstant *) * maxinsts);
  TimestampTz tpbuf[TPFN_STACK_CAP];
  /* Capacity of the array a turning point function writes */
  int cap = many ? TPFN_STACK_CAP : 2;
  Datum valbuf[TPFN_STACK_CAP];
  Datum *valstack = values ? valbuf : NULL;
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  Datum value1 = tinstant_value_p(inst1);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  /* A turning point function that writes the ends of the segments gives
   * every instant of a segment, a constant one included */
  bool ends = values && lfinfo->tpfn_ends && lfinfo->tpfn_base &&
    interp == LINEAR;
  assert(! ends || lfinfo->tpfn_value);
  for (int i = 1; i < seq->count; i++)
  {
    /* Each iteration of the loop adds the start instant of the segment and
     * its turning points */
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Datum value2 = tinstant_value_p(inst2);
    if (ends)
    {
      TimestampTz *tps = tpbuf;
      Datum *vals = valstack;
      int ntps = lfinfo->tpfn_base(value1, value2, value, inst1->t, inst2->t,
        tps, vals, cap);
      assert(many || ntps <= 2);
      if (ntps > TPFN_STACK_CAP)
      {
        tps = palloc(sizeof(TimestampTz) * ntps);
        vals = lifting_values_room(vals, resbasetype, ntps);
        lfinfo->tpfn_base(value1, value2, value, inst1->t, inst2->t, tps,
          vals, ntps);
      }
      instants = lifting_instants_room(instants, &maxinsts,
        ninsts + ntps + 3 * (seq->count - i));
      ninsts = lifting_append_ends(instants, ninsts, tps, vals, ntps, lfinfo);
      if (ntps > TPFN_STACK_CAP)
      {
        pfree(tps); pfree(vals);
      }
      inst1 = inst2;
      value1 = value2;
      continue;
    }
    instants[ninsts++] = tfunc_tinstant_base(inst1, value, lfinfo);
    /* If not constant segment and linear compute the function on the potential
       intermediate turning points before adding the new instant */
    if (lfinfo->tpfn_base && interp == LINEAR &&
      ! datum_eq(value1, value2, temptype_basetype(seq->temptype)))
    {
      TimestampTz *tps = tpbuf;
      Datum *vals = valstack;
      int ntps = lfinfo->tpfn_base(value1, value2, value, inst1->t, inst2->t,
        tps, vals, cap);
      assert(many || ntps <= 2);
      if (many && unlikely(ntps > 2))
      {
        if (ntps > TPFN_STACK_CAP)
        {
          tps = palloc(sizeof(TimestampTz) * ntps);
          vals = lifting_values_room(vals, resbasetype, ntps);
          lfinfo->tpfn_base(value1, value2, value, inst1->t, inst2->t, tps,
            vals, ntps);
        }
        /* The array holds three instants for each segment left, two turning
         * points and the start instant, so it grows only when there are more */
        instants = lifting_instants_room(instants, &maxinsts,
          ninsts + ntps + 3 * (seq->count - i));
      }
      if (ntps == 1)
      {
        /* One turning point, the case of every function of this arity */
        Datum res;
        if (vals)
          res = valbuf[0];
        else
        {
          Datum tpvalue = tsegment_value_at_timestamptz(value1, value2,
            inst1->temptype, inst1->t, inst2->t, tpbuf[0]);
          res = tfunc_base_base(tpvalue, value, lfinfo);
          DATUM_FREE(tpvalue, basetype);
        }
        instants[ninsts++] = tinstant_make(res, lfinfo->restype, tpbuf[0]);
        DATUM_FREE(res, resbasetype);
      }
      else
      {
        for (int k = 0; k < ntps; k++)
        {
          Datum res;
          if (vals)
            res = vals[k];
          else
          {
            Datum tpvalue = tsegment_value_at_timestamptz(value1, value2,
              inst1->temptype, inst1->t, inst2->t, tps[k]);
            res = tfunc_base_base(tpvalue, value, lfinfo);
            DATUM_FREE(tpvalue, basetype);
          }
          instants[ninsts++] = tinstant_make(res, lfinfo->restype, tps[k]);
          DATUM_FREE(res, resbasetype);
        }
        if (many && unlikely(ntps > TPFN_STACK_CAP))
        {
          pfree(tps);
          if (vals)
            pfree(vals);
        }
      }
    }
    inst1 = inst2;
    value1 = value2;
  }
  /* The last instant ends the last segment, unless the turning point
   * function wrote it, or none if the function wrote none, which it does
   * when it raises an error */
  if (! ends || ninsts == 0 || instants[ninsts - 1]->t != inst1->t)
    instants[ninsts++] = tfunc_tinstant_base(inst1, value, lfinfo);
  result[0] = lifting_tsequence_make_free(instants, ninsts,
    seq->period.lower_inc, seq->period.upper_inc, interp,
    lfinfo->reskeep ? NORMALIZE_NO : NORMALIZE);
  return 1;
}

/**
 * @brief Apply a lifted function to a temporal value and a base value when
 * turning points should be added
 * @param[in] seq Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 */
static int
tfunc_tlinearseq_base_turnpt(const TSequence *seq, Datum value,
  LiftedFunctionInfo *lfinfo, TSequence **result)
{
  if (lfinfo->tpfn_many || lfinfo->tpfn_value)
    return tfunc_tlinearseq_base_turnpt_walk(seq, value, lfinfo, result,
      true, lfinfo->tpfn_value);
  return tfunc_tlinearseq_base_turnpt_walk(seq, value, lfinfo, result, false,
    false);
}

/**
 * @brief Apply a lifted function to a temporal value and a base value when the
 * function has instantaneuous discontinuties
 * @param[in] seq Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @note The current version of the function supposes that the basetype
 * is passed by value and thus it is not necessary to create and pfree
 * each pair of instants used for constructing a segment of the result.
 * Similarly, it is not necessary to pfree the values resulting from
 * the function func.
 */
static int
tfunc_tlinearseq_base_discfn(const TSequence *seq, Datum value,
  LiftedFunctionInfo *lfinfo, TSequence **result)
{
  assert(temptype_basetype(seq->temptype) == lfinfo->argtype[1]);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == LINEAR);
  const TInstant *start = TSEQUENCE_INST_N(seq, 0);
  Datum startvalue = tinstant_value_p(start);
  Datum startresult = tfunc_base_base(startvalue, value, lfinfo);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count * 2);
  MeosType resbasetype = temptype_basetype(lfinfo->restype);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    instants[0] = tinstant_make_free(startresult, lfinfo->restype, start->t);
    result[0] = lifting_tinstant_to_tsequence_free(instants[0], STEP);
    return 1;
  }

  /* General case */
  MeosType basetype = temptype_basetype(seq->temptype);
  MeosType restype = lfinfo->restype;
  bool lower_inc = seq->period.lower_inc;
  int ninsts = 0, nseqs = 0;
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *end = TSEQUENCE_INST_N(seq, i);
    Datum endvalue = tinstant_value_p(end);
    Datum endresult = tfunc_base_base(endvalue, value, lfinfo);
    Datum tpresult;
    bool lower_eq;
    TimestampTz tpt1, tpt2;

    /* Determine whether there is a crossing in the middle of the segment
     * and compute the value at the crossing if there is one */
    bool constant = datum_eq(startvalue, endvalue, basetype);
    bool midpoint = false;
    int cross = 0;
    if (! constant)
    {
      if (datum_eq(startvalue, value, basetype) ||
          datum_eq(endvalue, value, basetype))
        midpoint = true;
      else
      {
        /* See the cross_type comment in eafunc_tlinearseq_base: when the
         * right-hand value is a different type from the temporal's basetype,
         * datumsegm_locate would reinterpret its bytes and trip a garbage
         * SRID-equality check.  Skip the intersection search and treat the
         * segment as having no crossing. */
        cross = lfinfo->cross_type ? 0 :
          tsegment_intersection_value(startvalue, endvalue, value,
            start->temptype, start->t, end->t, &tpt1, &tpt2);
        /* A crossing dated at a bound is not reported, see
         * #tfloatsegm_crossing. If the results at the bounds differ, the
         * result changes at a bound, and the value in the middle tells at
         * which one. */
        if (! cross && ! datum_eq(startresult, endresult, resbasetype))
          midpoint = true;
      }
    }

    /* If the segment is constant continue the current sequence */
    if (constant)
    {
      instants[ninsts++] = tinstant_make(startresult, restype, start->t);
      if (i == seq->count - 1)
        instants[ninsts++] = tinstant_make(startresult, restype, end->t);
    }
    /* If either the start or the end value is equal to the value, or the
     * result changes at a bound, compute the function at the middle time
     * between the start and end instants */
    else if (midpoint)
    {
      tpt1 = start->t + ((end->t - start->t) / 2);
      Datum tpvalue1 = tsegment_value_at_timestamptz(startvalue, endvalue,
        start->temptype, start->t, end->t, tpt1);
      tpresult = tfunc_base_base(tpvalue1, value, lfinfo);
      DATUM_FREE(tpvalue1, basetype);
      lower_eq = datum_eq(startresult, tpresult, resbasetype);
      if (lower_eq)
      {
        /* Continue the current sequence */
        instants[ninsts++] = tinstant_make(startresult, restype, start->t);
        if (i == seq->count - 1)
          instants[ninsts++] = tinstant_make(endresult, restype, end->t);
      }
      else /* ! lower_eq => upper_eq */
      {
        /* Close the current sequence at the start */
        instants[ninsts++] = tinstant_make(startresult, restype, start->t);
        result[nseqs++] = lifting_tsequence_make(instants, ninsts, lower_inc,
          true, STEP, NORMALIZE);
        for (int j = 0; j < ninsts; j++)
          pfree(instants[j]);
        ninsts = 0;
        lower_inc = false;
        /* Start a new sequence */
        instants[ninsts++] = tinstant_make_free(tpresult, restype, start->t);
        if (i == seq->count - 1)
          instants[ninsts++] = tinstant_make(endresult, restype, end->t);
      }
      DATUM_FREE(tpresult, resbasetype);
    }
    else
    {
      if (! cross)
      {
        /* Continue the current sequence */
        instants[ninsts++] = tinstant_make(startresult, restype, start->t);
        if (i == seq->count - 1)
          instants[ninsts++] = tinstant_make(endresult, restype, end->t);
      }
      else /* cross */
      {
        /* The segment takes the value at the crossing, so the function is
         * computed on the value itself, as #eafunc_tlinearseq_base does. The
         * value interpolated at the microsecond dating the crossing differs
         * from it by a rounding, which a comparison would read as a
         * different value */
        tpresult = tfunc_base_base(value, value, lfinfo);
        lower_eq = datum_eq(startresult, tpresult, resbasetype);
        bool upper_eq = datum_eq(tpresult, endresult, resbasetype);
        /* If the value at the crossing is equal to the start or to the end
         * value close the current sequence at the start or end instant.
         * Notice that lower_eq and upper_eq can be both true */
        if (lower_eq || upper_eq)
        {
          instants[ninsts++] = tinstant_make(startresult, restype, start->t);
          instants[ninsts++] = tinstant_make(startresult, restype, tpt1);
          /* The upper_inc bound of the closing sequence is true if lower_eq,
           * false if upper_eq */
          result[nseqs++] = lifting_tsequence_make(instants, ninsts, lower_inc,
            lower_eq, STEP, NORMALIZE);
          for (int j = 0; j < ninsts; j++)
            pfree(instants[j]);
          ninsts = 0;
          /* The lower_inc of the new sequence is false if lower_eq is true */
          lower_inc = ! lower_eq;
          /* Start a new sequence */
          instants[ninsts++] = tinstant_make(endresult, restype, tpt1);
          if (i == seq->count - 1)
            instants[ninsts++] = tinstant_make(endresult, restype, end->t);
        }
        else /* ! lower_eq && ! upper_eq */
        {
          /* The crossing is at the middle: close the current sequence at the
           * crossing, which takes the value at the crossing, the sequence a
           * closing sequence exclusive at the crossing and a singleton
           * sequence at the crossing join into (#tsequence_join_test) */
          instants[ninsts++] = tinstant_make(startresult, restype, start->t);
          instants[ninsts++] = tinstant_make_free(tpresult, restype, tpt1);
          result[nseqs++] = lifting_tsequence_make(instants, ninsts, lower_inc,
            true, STEP, NORMALIZE);
          for (int j = 0; j < ninsts; j++)
            pfree(instants[j]);
          ninsts = 0;
          /* Start a new sequence from the crossing to the end of the segment */
          lower_inc = false;
          instants[ninsts++] = tinstant_make(endresult, restype, tpt1);
          if (i == seq->count - 1)
            instants[ninsts++] = tinstant_make(endresult, restype, end->t);
        }
        DATUM_FREE(tpresult, resbasetype);
      }
    }
    start = end;
    startvalue = endvalue;
    DATUM_FREE(startresult, resbasetype);
    startresult = endresult;
  }
  /* Build the last sequence */
  if (ninsts > 0)
  {
    /* The last two values of sequences with step interpolation and exclusive
       upper bound must be equal */
    if (! seq->period.upper_inc && ninsts > 1 && instants[ninsts - 2] &&
        instants[ninsts - 1])
    {
      TInstant *inst = instants[ninsts - 1];
      value = tinstant_value_p(instants[ninsts - 2]);
      instants[ninsts - 1] = tinstant_make(value, lfinfo->restype,
        instants[ninsts - 1]->t);
      pfree(inst);
    }
    result[nseqs++] = lifting_tsequence_make(instants, ninsts,
      (ninsts == 1) ? true : lower_inc,
      (ninsts == 1) ? true : seq->period.upper_inc, STEP, NORMALIZE);
  }
  pfree_array((void *) instants, ninsts);
  return nseqs;
}

/**
 * @brief Apply a lifted function to a temporal value and a base value
 * @details Dispatch function depending on whether the function has
 * instantaneous discontinuities
 */
Temporal *
tfunc_tlinearseq_base(const TSequence *seq, Datum value,
  LiftedFunctionInfo *lfinfo)
{
  int count = (lfinfo->discont) ? seq->count * 3 : 1;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  if (lfinfo->discont)
  {
    int nseqs = tfunc_tlinearseq_base_discfn(seq, value, lfinfo, sequences);
    /* We are sure that nseqs > 0 */
    return (Temporal *) lifting_tsequenceset_make_free(sequences, nseqs,
      NORMALIZE);
  }
  else
  {
    /* We are sure that the result is a single sequence */
    if (lfinfo->tpfn_base)
      tfunc_tlinearseq_base_turnpt(seq, value, lfinfo, sequences);
    else
      sequences[0] = tfunc_tsequence_base(seq, value, lfinfo);
    Temporal *result = (Temporal *) sequences[0];
    pfree(sequences);
    return result;
  }
}

/**
 * @brief Apply a lifted function to a temporal value and a base value
 * @param[in] ss Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 */
TSequenceSet *
tfunc_tsequenceset_base(const TSequenceSet *ss, Datum value,
  LiftedFunctionInfo *lfinfo)
{
  int count;
  if (lfinfo->discont)
    count = ss->totalcount * 3;
  else
    count = ss->count;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    if (lfinfo->discont)
      nseqs += tfunc_tlinearseq_base_discfn(seq, value, lfinfo,
        &sequences[nseqs]);
    else if (lfinfo->tpfn_base)
      nseqs += tfunc_tlinearseq_base_turnpt(seq, value, lfinfo,
        &sequences[nseqs]);
    else
      sequences[nseqs++] = tfunc_tsequence_base(seq, value, lfinfo);
  }
  /* We are sure that nseqs > 0 */
  return lifting_tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @brief Apply a lifted function to a temporal value and a base value
 * @note The error number is read by #tfunc_temporal_base
 */
static Temporal *
tfunc_temporal_base_dispatch(const Temporal *temp, Datum value,
  LiftedFunctionInfo *lfinfo)
{
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tfunc_tinstant_base((TInstant *) temp, value, lfinfo);
    case TSEQUENCE:
      return ! MEOS_FLAGS_LINEAR_INTERP(temp->flags) ?
        (Temporal *) tfunc_tsequence_base((TSequence *) temp, value, lfinfo) :
        (Temporal *) tfunc_tlinearseq_base((TSequence *) temp, value, lfinfo);
    default: /* TSEQUENCESET */
      return (Temporal *) tfunc_tsequenceset_base((TSequenceSet *) temp, value,
        lfinfo);
  }
}

/**
 * @brief Apply a lifted function to a temporal value and a base value
 * @param[in] temp Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 * @note An error raised by the function applied makes the answer the
 * failure value, see the note on errors at the beginning of the file
 */
Temporal *
tfunc_temporal_base(const Temporal *temp, Datum value,
  LiftedFunctionInfo *lfinfo)
{
  int last_errno = meos_errno_reset();
  return (Temporal *) lifting_result(
    tfunc_temporal_base_dispatch(temp, value, lfinfo), last_errno);
}

/*****************************************************************************
 * Functions that synchronize two temporal values and apply a function in
 * a single pass.
 *****************************************************************************/

/**
 * @brief Invert the arguments of the lfinfo struct
 */
static void
lfinfo_invert_args(LiftedFunctionInfo *lfinfo)
{
  lfinfo->invert = ! lfinfo->invert;
  MeosType temp = lfinfo->argtype[0];
  lfinfo->argtype[0] = lfinfo->argtype[1];
  lfinfo->argtype[1] = temp;
  return;
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 *
 * @param[in] inst1,inst2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @note This function is called by other functions besides the dispatch
 * function tfunc_temporal_temporal and thus the overlappint test is
 * repeated
 */
TInstant *
tfunc_tinstant_tinstant(const TInstant *inst1, const TInstant *inst2,
  LiftedFunctionInfo *lfinfo)
{
  /* The following is ensured by the period bound test in the dispatch function
   * or by the synchronizaton performed before this call */
  assert(inst1->t == inst2->t);
  Datum resvalue = tfunc_base_base(tinstant_value_p(inst1), tinstant_value_p(inst2),
    lfinfo);
  return tinstant_make_free(resvalue, lfinfo->restype, inst1->t);
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] seq,inst Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
tfunc_tdiscseq_tinstant(const TSequence *seq, const TInstant *inst,
  LiftedFunctionInfo *lfinfo)
{
  Datum value1;
  if (! tdiscseq_value_at_timestamptz(seq, inst->t, &value1))
    return NULL;

  Datum resvalue = tfunc_base_base(value1, tinstant_value_p(inst), lfinfo);
  return tinstant_make_free(resvalue, lfinfo->restype, inst->t);
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] inst,seq Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
tfunc_tinstant_tdiscseq(const TInstant *inst, const TSequence *seq,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return tfunc_tdiscseq_tinstant(seq, inst, lfinfo);
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * with an optional argument
 * @param[in] seq,inst Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
tfunc_tcontseq_tinstant(const TSequence *seq, const TInstant *inst,
  LiftedFunctionInfo *lfinfo)
{
  Datum value;
  /* The following call is ensured to return true due to the period bound test
   * in the dispatch function */
  tsequence_value_at_timestamptz(seq, inst->t, true, &value);
  Datum resvalue = tfunc_base_base(value, tinstant_value_p(inst), lfinfo);
  DATUM_FREE(value, temptype_basetype(seq->temptype));
  return tinstant_make_free(resvalue, lfinfo->restype, inst->t);
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] inst,seq Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
tfunc_tinstant_tcontseq(const TInstant *inst, const TSequence *seq,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return tfunc_tcontseq_tinstant(seq, inst, lfinfo);
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] ss,inst Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
tfunc_tsequenceset_tinstant(const TSequenceSet *ss, const TInstant *inst,
  LiftedFunctionInfo *lfinfo)
{
  Datum value1;
  if (! tsequenceset_value_at_timestamptz(ss, inst->t, true, &value1))
    return NULL;

  Datum resvalue = tfunc_base_base(value1, tinstant_value_p(inst), lfinfo);
  DATUM_FREE(value1, temptype_basetype(inst->temptype));
  return tinstant_make_free(resvalue, lfinfo->restype, inst->t);
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] inst,ss Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
tfunc_tinstant_tsequenceset(const TInstant *inst, const TSequenceSet *ss,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return tfunc_tsequenceset_tinstant(ss, inst, lfinfo);
}

/*****************************************************************************/

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @note This function is called by other functions besides the dispatch
 * function tfunc_temporal_temporal and thus the bounding period test is
 * repeated
 */
TSequence *
tfunc_tdiscseq_tdiscseq(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo)
{
  TInstant **instants = palloc(sizeof(TInstant *) *
    Min(seq1->count, seq2->count));
  int i = 0, j = 0, ninsts = 0;
  const TInstant *inst1 = TSEQUENCE_INST_N(seq1, i);
  const TInstant *inst2 = TSEQUENCE_INST_N(seq2, j);
  while (i < seq1->count && j < seq2->count)
  {
    int cmp = timestamptz_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      Datum resvalue = tfunc_base_base(tinstant_value_p(inst1),
        tinstant_value_p(inst2), lfinfo);
      instants[ninsts++] = tinstant_make_free(resvalue, lfinfo->restype,
        inst1->t);
      inst1 = TSEQUENCE_INST_N(seq1, ++i);
      inst2 = TSEQUENCE_INST_N(seq2, ++j);
    }
    else if (cmp < 0)
      inst1 = TSEQUENCE_INST_N(seq1, ++i);
    else
      inst2 = TSEQUENCE_INST_N(seq2, ++j);
  }
  return lifting_tsequence_make_free(instants, ninsts, true, true, DISCRETE,
    NORMALIZE_NO);
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TSequence *
tfunc_tcontseq_tdiscseq(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq2->count);
  int ninsts = 0;
  TimestampTz upper1 = DatumGetTimestampTz(seq1->period.upper);
  for (int i = 0; i < seq2->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq2, i);
    if (contains_span_timestamptz(&seq1->period, inst->t))
    {
      Datum value;
      tsequence_value_at_timestamptz(seq1, inst->t, true, &value);
      Datum resvalue = tfunc_base_base(value, tinstant_value_p(inst), lfinfo);
      DATUM_FREE(value, temptype_basetype(seq1->temptype));
      instants[ninsts++] = tinstant_make_free(resvalue, lfinfo->restype,
        inst->t);
    }
    if (upper1 < inst->t)
      break;
  }
  return lifting_tsequence_make_free(instants, ninsts, true, true, DISCRETE,
    NORMALIZE_NO);
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TSequence *
tfunc_tdiscseq_tcontseq(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return tfunc_tcontseq_tdiscseq(seq2, seq1, lfinfo);
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] ss,seq Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TSequence *
tfunc_tsequenceset_tdiscseq(const TSequenceSet *ss, const TSequence *seq,
  LiftedFunctionInfo *lfinfo)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int i = 0, j = 0, ninsts = 0;
  MeosType basetype = temptype_basetype(ss->temptype);
  while (i < ss->count && j < seq->count)
  {
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss, i);
    const TInstant *inst = TSEQUENCE_INST_N(seq, j);
    if (contains_span_timestamptz(&seq1->period, inst->t))
    {
      Datum value;
      tsequenceset_value_at_timestamptz(ss, inst->t, true, &value);
      Datum resvalue = tfunc_base_base(value, tinstant_value_p(inst), lfinfo);
      DATUM_FREE(value, basetype);
      instants[ninsts++] = tinstant_make_free(resvalue, lfinfo->restype,
        inst->t);
    }
    int cmp = timestamptz_cmp_internal(DatumGetTimestampTz(seq1->period.upper),
      inst->t);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
      i++;
    else
      j++;
  }
  return lifting_tsequence_make_free(instants, ninsts, true, true, DISCRETE,
    NORMALIZE_NO);
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] seq,ss Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TSequence *
tfunc_tdiscseq_tsequenceset(const TSequence *seq, const TSequenceSet *ss,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return tfunc_tsequenceset_tdiscseq(ss, seq, lfinfo);
}

/*****************************************************************************/

/**
 * @brief Walk of #tfunc_tcontseq_tcontseq_single
 * @details The turning point function counts more than two instants in a
 * segment if @p many is true and writes their values if @p values is true;
 * each caller passes literal flags, for which the walk is compiled apart
 */
static pg_attribute_always_inline int
tfunc_tcontseq_tcontseq_single_walk(const TSequence *seq1,
  const TSequence *seq2, LiftedFunctionInfo *lfinfo, Span *inter,
  TSequence **result, bool many, bool values)
{
  /*
   * General case
   * seq1 =  ...    *       *       *>
   * seq2 =    <*       *   *   * ...
   * result =  <S T S T S T * T S T S>
   * where S, T, and * are values computed, respectively, at the
   * synchronization points, optional turning points, and common points
   */
  MeosType basetype = temptype_basetype(seq1->temptype);
  MeosType basetype_res = temptype_basetype(lfinfo->restype);
  TInstant *inst1 = (TInstant *) TSEQUENCE_INST_N(seq1, 0);
  TInstant *inst2 = (TInstant *) TSEQUENCE_INST_N(seq2, 0);
  TInstant *prev1 = NULL; /* make compiler quiet */
  const TInstant *prev2 = NULL;
  TimestampTz lower = DatumGetTimestampTz(inter->lower);
  TimestampTz upper = DatumGetTimestampTz(inter->upper);
  interpType interp1 = MEOS_FLAGS_GET_INTERP(seq1->flags);
  interpType interp2 = MEOS_FLAGS_GET_INTERP(seq2->flags);
  int i = 0, j = 0, ninsts = 0, nfree = 0;
  if (inst1->t < lower)
  {
    i = tcontseq_find_timestamptz(seq1, inter->lower) + 1;
    inst1 = (TInstant *) TSEQUENCE_INST_N(seq1, i);
  }
  else if (inst2->t < lower)
  {
    j = tcontseq_find_timestamptz(seq2, inter->lower) + 1;
    inst2 = (TInstant *) TSEQUENCE_INST_N(seq2, j);
  }
  int count = (seq1->count - i + seq2->count - j) * 3;
  /* A segment adds its end instant and at most two turning points unless its
   * turning point function counts more, which enlarges the array */
  int maxinsts = count;
  TInstant **instants = palloc(sizeof(TInstant *) * maxinsts);
  TInstant **tofree = palloc(sizeof(TInstant *) * count);
  TimestampTz tpbuf[TPFN_STACK_CAP];
  /* Capacity of the array a turning point function writes */
  int cap = many ? TPFN_STACK_CAP : 2;
  Datum valbuf[TPFN_STACK_CAP];
  Datum *valstack = values ? valbuf : NULL;
  Datum value;
  /* A turning point function that writes the ends of the segments gives
   * every instant of a segment, and the function is evaluated only at an
   * instant that ends no segment */
  bool ends = values && lfinfo->tpfn_ends && lfinfo->tpfn_temp &&
    interp1 == LINEAR;
  assert(! ends || lfinfo->tpfn_value);
  /* Number of synchronized instants read when the turning point function
   * writes the ends of the segments, else the instants added count them */
  int nsync = 0;
  while (i < seq1->count && j < seq2->count &&
    (inst1->t <= upper || inst2->t <= upper))
  {
    /* Synchronize the two start instants */
    int cmp = timestamptz_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
    {
      /* Interpolate seq2 at inst1->t on its current segment, known from the
       * merge position [j-1, j], avoiding a per-instant binary search */
      i++;
      const TInstant *b1 = TSEQUENCE_INST_N(seq2, j - 1);
      inst2 = tsegment_at_timestamptz(b1, inst2, interp2, inst1->t);
      tofree[nfree++] = inst2;
    }
    else
    {
      /* Interpolate seq1 at inst2->t on its current segment [i-1, i] */
      j++;
      const TInstant *a1 = TSEQUENCE_INST_N(seq1, i - 1);
      inst1 = tsegment_at_timestamptz(a1, inst1, interp1, inst2->t);
      tofree[nfree++] = inst1;
    }
    /* If not the first instant compute the function on the potential
       turning point before adding the new instants */
    if (lfinfo->tpfn_temp && (ends ? nsync : ninsts) > 0)
    {
      Datum start1 = tinstant_value_p(prev1);
      Datum end1 = tinstant_value_p(inst1);
      Datum start2 = tinstant_value_p(prev2);
      Datum end2 = tinstant_value_p(inst2);
      TimestampTz *tps = tpbuf;
      Datum *vals = valstack;
      int ntps = lfinfo->tpfn_temp(start1, end1, start2, end2,
        lfinfo->param[0], prev1->t, inst1->t, tps, vals, cap);
      assert(many || ntps <= 2);
      if (ends)
      {
        if (ntps > TPFN_STACK_CAP)
        {
          tps = palloc(sizeof(TimestampTz) * ntps);
          vals = lifting_values_room(vals, basetype_res, ntps);
          lfinfo->tpfn_temp(start1, end1, start2, end2, lfinfo->param[0],
            prev1->t, inst1->t, tps, vals, ntps);
        }
        instants = lifting_instants_room(instants, &maxinsts, ninsts + ntps +
          3 * (seq1->count - i + seq2->count - j + 1));
        ninsts = lifting_append_ends(instants, ninsts, tps, vals, ntps,
          lfinfo);
        if (ntps > TPFN_STACK_CAP)
        {
          pfree(tps); pfree(vals);
        }
      }
      else if (ntps == 1)
      {
        /* One turning point, the case of every function of this arity; one
         * at the start of the segment is the instant added there */
        if (tpbuf[0] != prev1->t)
        {
          Datum tpresult;
          if (vals)
            tpresult = valbuf[0];
          else
          {
            Datum tpvalue1 = tsegment_value_at_timestamptz(start1, end1,
              prev1->temptype, prev1->t, inst1->t, tpbuf[0]);
            Datum tpvalue2 = tsegment_value_at_timestamptz(start2, end2,
              prev1->temptype, prev1->t, inst1->t, tpbuf[0]);
            tpresult = tfunc_base_base(tpvalue1, tpvalue2, lfinfo);
            DATUM_FREE(tpvalue1, basetype); DATUM_FREE(tpvalue2, basetype);
          }
          instants[ninsts++] = tinstant_make_free(tpresult, lfinfo->restype,
            tpbuf[0]);
        }
        else if (vals)
          DATUM_FREE(valbuf[0], basetype_res);
      }
      else if (ntps > 1)
      {
        if (many && unlikely(ntps > 2))
        {
          if (ntps > TPFN_STACK_CAP)
          {
            tps = palloc(sizeof(TimestampTz) * ntps);
            vals = lifting_values_room(vals, basetype_res, ntps);
            lfinfo->tpfn_temp(start1, end1, start2, end2, lfinfo->param[0],
              prev1->t, inst1->t, tps, vals, ntps);
          }
          /* The array holds three instants for each synchronized instant
           * left, two turning points and the instant, so it grows only when
           * there are more */
          instants = lifting_instants_room(instants, &maxinsts, ninsts +
            ntps + 3 * (seq1->count - i + seq2->count - j + 1));
        }
        for (int k = 0; k < ntps; k++)
        {
          /* Avoid adding a turning point at the same timestamp added next */
          if (k == 0 && tps[0] == prev1->t)
          {
            if (vals)
              DATUM_FREE(vals[0], basetype_res);
            continue;
          }
          Datum tpresult;
          if (vals)
            tpresult = vals[k];
          else
          {
            Datum tpvalue1 = tsegment_value_at_timestamptz(start1, end1,
              prev1->temptype, prev1->t, inst1->t, tps[k]);
            Datum tpvalue2 = tsegment_value_at_timestamptz(start2, end2,
              prev1->temptype, prev1->t, inst1->t, tps[k]);
            tpresult = tfunc_base_base(tpvalue1, tpvalue2, lfinfo);
            DATUM_FREE(tpvalue1, basetype); DATUM_FREE(tpvalue2, basetype);
          }
          instants[ninsts++] = tinstant_make_free(tpresult, lfinfo->restype,
            tps[k]);
        }
        if (many && unlikely(ntps > TPFN_STACK_CAP))
        {
          pfree(tps);
          if (vals)
            pfree(vals);
        }
      }
    }
    /* Compute the function on the synchronized instants */
    if (! ends)
    {
      value = tfunc_base_base(tinstant_value_p(inst1),
        tinstant_value_p(inst2), lfinfo);
      instants[ninsts++] = tinstant_make_free(value, lfinfo->restype,
        inst1->t);
    }
    if (ends)
      nsync++;
    if (i == seq1->count || j == seq2->count)
      break;
    prev1 = inst1;
    prev2 = inst2;
    inst1 = (TInstant *) TSEQUENCE_INST_N(seq1, i);
    inst2 = (TInstant *) TSEQUENCE_INST_N(seq2, j);
  }
  /* The last instant ends the last segment, unless the turning point
   * function wrote it, or none if the function wrote none, which it does
   * when it raises an error */
  if (ends && (ninsts == 0 || instants[ninsts - 1]->t != inst1->t))
  {
    value = tfunc_base_base(tinstant_value_p(inst1), tinstant_value_p(inst2),
      lfinfo);
    instants[ninsts++] = tinstant_make_free(value, lfinfo->restype, inst1->t);
  }
  /* We are sure that ninsts != 0 due to the period intersection test above */
  /* The last two values of sequences with step interpolation and exclusive
     upper bound must be equal */
  if (! lfinfo->reslinear && ! inter->upper_inc && ninsts > 1 &&
      instants[ninsts - 2] && instants[ninsts - 1])
  {
    tofree[nfree++] = instants[ninsts - 1];
    value = tinstant_value_p(instants[ninsts - 2]);
    instants[ninsts - 1] = tinstant_make(value, lfinfo->restype,
      instants[ninsts - 1]->t);
  }
  pfree_array((void **) tofree, nfree);
  interpType interp = Min(MEOS_FLAGS_GET_INTERP(seq1->flags),
    MEOS_FLAGS_GET_INTERP(seq2->flags));
  if (interp == LINEAR && ! lfinfo->reslinear)
    interp = STEP;
  result[0] = lifting_tsequence_make_free(instants, ninsts, inter->lower_inc,
    inter->upper_inc, interp, lfinfo->reskeep ? NORMALIZE_NO : NORMALIZE);
  return 1;
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @details This function is applied when the result is a single sequence and
 * thus it is used when
 * - the function to lift has NOT instantaneous discontinuities
 * - the temporal values have equal interpolation.
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @param[in] inter Overlapping period of the two sequences
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @pre The sequences are both linear or both step
 */
static int
tfunc_tcontseq_tcontseq_single(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo, Span *inter, TSequence **result)
{
  if (lfinfo->tpfn_many || lfinfo->tpfn_value)
    return tfunc_tcontseq_tcontseq_single_walk(seq1, seq2, lfinfo, inter,
      result, true, lfinfo->tpfn_value);
  return tfunc_tcontseq_tcontseq_single_walk(seq1, seq2, lfinfo, inter,
    result, false, false);
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @details This function is applied when the result is an array of sequences
 * and thus it is used when
 * - the function to lift has instantaneous discontinuities
 * - one temporal value has linear and the other has step interpolation.
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @param[in] inter Overlapping period of the two sequences
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 */
static int
tfunc_tcontseq_tcontseq_discfn(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo, Span *inter, TSequence **result)
{
  assert(seq1->temptype == seq2->temptype);
  int count = seq1->count + seq2->count;
  /* Array that keeps the new instants to be accumulated */
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  /* Array that keeps the new instants added for synchronization */
  TInstant **tofree = palloc(sizeof(TInstant *) * count);
  interpType interp1 = MEOS_FLAGS_GET_INTERP(seq1->flags);
  interpType interp2 = MEOS_FLAGS_GET_INTERP(seq2->flags);
  MeosType basetype = temptype_basetype(seq1->temptype);
  MeosType restype = lfinfo->restype;
  MeosType resbasetype = temptype_basetype(lfinfo->restype);
  interpType interp = lfinfo->reslinear ? LINEAR : STEP;

  TInstant *start1 = (TInstant *) TSEQUENCE_INST_N(seq1, 0);
  TInstant *start2 = (TInstant *) TSEQUENCE_INST_N(seq2, 0);
  int i = 1, j = 1, nseqs = 0, ninsts = 0, nfree = 0;
  /* Synchronize the two start instants */
  if (start1->t < DatumGetTimestampTz(inter->lower))
  {
    start1 = tsequence_at_timestamptz(seq1, inter->lower);
    tofree[nfree++] = start1;
    i = tcontseq_find_timestamptz(seq1, inter->lower) + 1;
  }
  else if (start2->t < DatumGetTimestampTz(inter->lower))
  {
    start2 = tsequence_at_timestamptz(seq2, inter->lower);
    tofree[nfree++] = start2;
    j = tcontseq_find_timestamptz(seq2, inter->lower) + 1;
  }
  bool lower_inc = inter->lower_inc;
  while (i < seq1->count && j < seq2->count)
  {
    /* Compute the function at the start instant. Notice that we cannot reuse
     * the values from the previous iteration since one of the two sequences
     * may have step interpolation */
    Datum startvalue1 = tinstant_value_p(start1);
    Datum startvalue2 = tinstant_value_p(start2);
    Datum startresult = tfunc_base_base(startvalue1, startvalue2, lfinfo);
    /* Synchronize the two end instants */
    TInstant *end1 = (TInstant *) TSEQUENCE_INST_N(seq1, i);
    TInstant *end2 = (TInstant *) TSEQUENCE_INST_N(seq2, j);
    int cmp = timestamptz_cmp_internal(end1->t, end2->t);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
    {
      i++;
      end2 = tsegment_at_timestamptz(start2, end2, interp2, end1->t);
      tofree[nfree++] = end2;
    }
    else
    {
      j++;
      end1 = tsegment_at_timestamptz(start1, end1, interp1, end2->t);
      tofree[nfree++] = end1;
    }
    /* Compute the function at the end instant */
    Datum endvalue1 = (interp1 == LINEAR) ? tinstant_value_p(end1) : startvalue1;
    Datum endvalue2 = (interp2 == LINEAR) ? tinstant_value_p(end2) : startvalue2;
    Datum endresult = tfunc_base_base(endvalue1, endvalue2, lfinfo);
    Datum tpvalue1, tpvalue2, tpresult;
    TimestampTz tpt1 = 0; /* make compiler quiet */
    bool lower_eq;

    /* Determine whether there is a crossing in the middle of the segments
     * and compute the value at the crossing if there is one */
    bool constant = (datum_eq(startvalue1, endvalue1, basetype) &&
         datum_eq(startvalue2, endvalue2, basetype)) ||
        (datum_eq(startvalue1, startvalue2, basetype) &&
         datum_eq(endvalue1, endvalue2, basetype));
    bool midpoint = false;
    int cross = 0;
    if (! constant)
    {
      if (datum_eq(startvalue1, startvalue2, basetype) ||
          datum_eq(endvalue1, endvalue2, basetype))
        midpoint = true;
      else
      {
        /* The interval of the segment during which the predicate holds, one
         * at most, of which the start is read */
        TimestampTz tps[2];
        if (lfinfo->tpfn_temp)
        {
          cross = lfinfo->tpfn_temp(startvalue1, endvalue1, startvalue2,
            endvalue2, lfinfo->param[0], start1->t, end1->t, tps, NULL, 2);
          assert(cross <= 2);
        }
        else
          cross = tsegment_intersection(startvalue1, endvalue1, startvalue2,
            endvalue2, start1->temptype, start1->t, end1->t, &tps[0], &tps[1]);
        if (cross)
          tpt1 = tps[0];
        /* A crossing dated at a bound is not reported, see
         * #tnumbersegm_crossing. If the results at the bounds differ, the
         * result changes at a bound, and the value in the middle tells at
         * which one. */
        if (! cross && ! datum_eq(startresult, endresult, resbasetype))
          midpoint = true;
      }
    }

    /* If the segments are both constants OR are both equal, compute the
     * function at the start and end instants and continue the current
     * sequence */
    if (constant)
    {
      instants[ninsts++] = tinstant_make(startresult, restype, start1->t);
    }
    /* If either the start values or the end values are equal, or the result
     * changes at a bound, compute the function at the start instant, at an
     * intermediate point, and at the end instant */
    else if (midpoint)
    {
      /* Compute the function at the middle time between the start and end
       * instants */
      tpt1 = start1->t + ((end1->t - start1->t) / 2);
      tpvalue1 = tsegment_value_at_timestamptz(startvalue1, endvalue1,
        start1->temptype, start1->t, end1->t, tpt1);
      tpvalue2 = tsegment_value_at_timestamptz(startvalue2, endvalue2,
        start1->temptype, start1->t, end1->t, tpt1);
      tpresult = tfunc_base_base(tpvalue1, tpvalue2, lfinfo);
      // DATUM_FREE(tpvalue1, basetype);  DATUM_FREE(tpvalue1, basetype); 
      lower_eq = datum_eq(startresult, tpresult, resbasetype);
      if (lower_eq)
      {
        /* Continue the current sequence */
        instants[ninsts++] = tinstant_make(startresult, restype, start1->t);
      }
      else /* ! lower_eq => upper_eq */
      {
        /* Close the current sequence at the start instant */
        instants[ninsts++] = tinstant_make(startresult, restype, start1->t);
        result[nseqs++] = lifting_tsequence_make(instants, ninsts, lower_inc,
          true, interp, NORMALIZE);
        for (int k = 0; k < ninsts; k++)
          pfree(instants[k]);
        ninsts = 0;
        lower_inc = false;
        /* Start a new sequence */
        instants[ninsts++] = tinstant_make(tpresult, restype, start1->t);
      }
      // DATUM_FREE(tpresult, resbasetype);
    }
    else
    {
      if (! cross)
      {
        instants[ninsts++] = tinstant_make(startresult, restype, start1->t);
      }
      else /* cross */
      {
        tpvalue1 = tsegment_value_at_timestamptz(startvalue1, endvalue1,
          start1->temptype, start1->t, end1->t, tpt1);
        /* Two segments take one value at their crossing, which the function
         * reads. The values interpolated at the microsecond dating it differ
         * by their rounding for two float segments, and two point segments
         * construct two points apart by their rounding where the meeting
         * falls between two microseconds, a meeting their intersection
         * decides (#tgeompointsegm_meet, #tgeogpointsegm_intersection) */
        tpvalue2 = (! lfinfo->tpfn_temp &&
            (start1->temptype == T_TFLOAT ||
             tpoint_type(start1->temptype))) ?
          datum_copy(tpvalue1, basetype) :
          tsegment_value_at_timestamptz(startvalue2, endvalue2,
            start1->temptype, start1->t, end1->t, tpt1);
        tpresult = tfunc_base_base(tpvalue1, tpvalue2, lfinfo);
        DATUM_FREE(tpvalue1, basetype); DATUM_FREE(tpvalue2, basetype);
        lower_eq = datum_eq(startresult, tpresult, resbasetype);
        bool upper_eq = datum_eq(tpresult, endresult, resbasetype);
        /* If the value at the crossing is equal to the start or to the end
         * value close the current sequence at the start or end instant.
         * Notice that lower_eq and upper_eq can be both true */
        if (lower_eq || upper_eq)
        {
          instants[ninsts++] = tinstant_make(startresult, restype, start1->t);
          instants[ninsts++] = tinstant_make(startresult, restype, tpt1);
          /* The upper_inc bound of the closing sequence is true if lower_eq,
           * false if upper_eq */
          result[nseqs++] = lifting_tsequence_make(instants, ninsts, lower_inc,
            lower_eq, interp, NORMALIZE);
          for (int k = 0; k < ninsts; k++)
            pfree(instants[k]);
          ninsts = 0;
          /* The lower_inc of the new sequence is false if lower_eq is true */
          lower_inc = ! lower_eq;
          /* Start a new sequence */
          instants[ninsts++] = tinstant_make(endresult, restype, tpt1);
        }
        else if (interp == STEP)
        {
          /* The crossing is at the middle: close the current sequence at the
           * crossing, which takes the value at the crossing, the sequence a
           * closing sequence exclusive at the crossing and a singleton
           * sequence at the crossing join into (#tsequence_join_test) */
          instants[ninsts++] = tinstant_make(startresult, restype, start1->t);
          instants[ninsts++] = tinstant_make_free(tpresult, restype, tpt1);
          result[nseqs++] = lifting_tsequence_make(instants, ninsts, lower_inc,
            true, interp, NORMALIZE);
          for (int k = 0; k < ninsts; k++)
            pfree(instants[k]);
          ninsts = 0;
          /* Start a new sequence from the crossing to the end of the segment */
          lower_inc = false;
          instants[ninsts++] = tinstant_make(endresult, restype, tpt1);
        }
        else /* ! lower_eq && ! upper_eq */
        {
          /* The crossing is at the middle: close the current sequence */
          instants[ninsts++] = tinstant_make(startresult, restype, start1->t);
          instants[ninsts++] = tinstant_make(startresult, restype, tpt1);
          result[nseqs++] = lifting_tsequence_make(instants, ninsts, lower_inc,
            false, interp, NORMALIZE);
          for (int k = 0; k < ninsts; k++)
            pfree(instants[k]);
          ninsts = 0;
          /* Add a singleton sequence with the crossing */
          instants[0] = tinstant_make_free(tpresult, restype, tpt1);
          result[nseqs++] = lifting_tsequence_make(instants, 1, true, true,
            interp, NORMALIZE);
          pfree(instants[0]);
          /* Start a new sequence from the crossing to the end of the segment */
          lower_inc = false;
          instants[ninsts++] = tinstant_make(endresult, restype, tpt1);
        }
        DATUM_FREE(tpresult, resbasetype);
      }
    }
    DATUM_FREE(startresult, resbasetype);
    DATUM_FREE(endresult, resbasetype);
    start1 = end1; start2 = end2;
  }
  /* Add the last instant */
  Datum lastresult = tfunc_base_base(tinstant_value_p(start1),
    tinstant_value_p(start2), lfinfo);
  instants[ninsts++] = tinstant_make_free(lastresult, restype, start1->t);

  /* The last two values of sequences with step interpolation and exclusive
     upper bound must be equal */
  if (! inter->upper_inc && interp == STEP && instants[ninsts - 2] &&
      instants[ninsts - 1])
  {
    TInstant *inst = instants[ninsts - 1];
    Datum value = tinstant_value_p(instants[ninsts - 2]);
    instants[ninsts - 1] = tinstant_make(value, lfinfo->restype,
      instants[ninsts - 1]->t);
    pfree(inst);
  }
  
  result[nseqs++] = lifting_tsequence_make_free(instants, ninsts,
    (ninsts == 1) ? true : lower_inc,
    (ninsts == 1) ? true : inter->upper_inc, interp, NORMALIZE);
  pfree_array((void *) tofree, nfree);
  return nseqs;
}

/**
 * @brief Walk of #tfunc_tlinearseq_tstepseq
 * @details The turning point function counts more than two instants in a
 * segment if @p many is true and writes their values if @p values is true;
 * each caller passes literal flags, for which the walk is compiled apart
 */
static pg_attribute_always_inline int
tfunc_tlinearseq_tstepseq_walk(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo, Span *inter, TSequence **result,
  bool many, bool values)
{
  interpType interp1 = MEOS_FLAGS_GET_INTERP(seq1->flags);
  interpType interp2 = MEOS_FLAGS_GET_INTERP(seq2->flags);
  assert(interp1 != interp2);
  bool step1 = (interp1 != LINEAR);
  MeosType basetype = temptype_basetype(seq1->temptype);
  /* Array that keeps the instants of a sequence of the result, which it
   * gathers from the instants of both inputs and from the turning points
   * between them, at most two per segment unless a turning point function
   * counts more, which enlarges the array, as in
   * #tfunc_tcontseq_tcontseq_single */
  int maxinsts = (seq1->count + seq2->count) * 3;
  TInstant **instants = palloc(sizeof(TInstant *) * maxinsts);
  TimestampTz tpbuf[TPFN_STACK_CAP];
  /* Capacity of the array a turning point function writes */
  int cap = many ? TPFN_STACK_CAP : 2;
  Datum valbuf[TPFN_STACK_CAP];
  Datum *valstack = values ? valbuf : NULL;
  /* Array that keeps the new instants added for synchronization */
  TInstant **tofree = palloc(sizeof(TInstant *) *
    (seq1->count + seq2->count) * 2);
  TInstant *start1 = (TInstant *) TSEQUENCE_INST_N(seq1, 0);
  TInstant *start2 = (TInstant *) TSEQUENCE_INST_N(seq2, 0);
  int i = 1, j = 1, nfree = 0, ninsts = 0, nseqs = 0;
  /* Synchronize the two start instant */
  if (start1->t < DatumGetTimestampTz(inter->lower))
  {
    start1 = tsequence_at_timestamptz(seq1, inter->lower);
    tofree[nfree++] = start1;
    i = tcontseq_find_timestamptz(seq1, inter->lower) + 1;
  }
  else if (start2->t < DatumGetTimestampTz(inter->lower))
  {
    start2 = tsequence_at_timestamptz(seq2, inter->lower);
    tofree[nfree++] = start2;
    j = tcontseq_find_timestamptz(seq2, inter->lower) + 1;
  }
  bool lower_inc = inter->lower_inc;
  MeosType restype = lfinfo->restype;
  MeosType resbasetype = temptype_basetype(restype);
  /* A turning point function that writes the ends of the segments gives
   * every instant of a segment, and the function is evaluated only at an
   * instant that ends no segment */
  bool ends = values && lfinfo->tpfn_ends && lfinfo->tpfn_temp;
  assert(! ends || lfinfo->tpfn_value);
  /* Compute the function at the start instant */
  Datum startvalue1 = tinstant_value_p(start1);
  Datum startvalue2 = tinstant_value_p(start2);
  Datum startresult = 0;
  if (! ends)
    startresult = tfunc_base_base(startvalue1, startvalue2, lfinfo);
  /* One sequence is produced for each instant of the step sequence, or for
   * each segment (#LiftedFunctionInfo.ressegs) */
  while (i < seq1->count && j < seq2->count)
  {
    /* Synchronize the two end instants */
    TInstant *end1 = (TInstant *) TSEQUENCE_INST_N(seq1, i);
    TInstant *end2 = (TInstant *) TSEQUENCE_INST_N(seq2, j);
    int cmp = timestamptz_cmp_internal(end1->t, end2->t);
    bool makeseq = false;
    if (cmp == 0)
    {
      i++; j++;
      makeseq = true;
    }
    else if (cmp < 0)
    {
      i++;
      end2 = tsegment_at_timestamptz(start2, end2, interp2, end1->t);
      tofree[nfree++] = end2;
      makeseq = step1;
    }
    else
    {
      j++;
      end1 = tsegment_at_timestamptz(start1, end1, interp1, end2->t);
      tofree[nfree++] = end1;
      makeseq = ! step1;
    }
    /* Compute the function at the end instant */
    Datum endvalue1 = tinstant_value_p(end1);
    Datum endvalue2 = tinstant_value_p(end2);
    Datum endresult = 0;
    if (! ends)
    {
      endresult = tfunc_base_base(endvalue1, endvalue2, lfinfo);
      instants[ninsts++] = tinstant_make(startresult, restype, start1->t);
    }
    /* Add the turning points of the segment, where the step sequence keeps
     * its start value up to the end instant, as #tfunc_tcontseq_tcontseq_single
     * adds them for two linear sequences */
    if (lfinfo->tpfn_temp)
    {
      Datum segend1 = step1 ? startvalue1 : endvalue1;
      Datum segend2 = step1 ? endvalue2 : startvalue2;
      TimestampTz *tps = tpbuf;
      Datum *vals = valstack;
      int ntps = lfinfo->tpfn_temp(startvalue1, segend1, startvalue2, segend2,
        lfinfo->param[0], start1->t, end1->t, tps, vals, cap);
      assert(many || ntps <= 2);
      if (ends)
      {
        if (ntps > TPFN_STACK_CAP)
        {
          tps = palloc(sizeof(TimestampTz) * ntps);
          vals = lifting_values_room(vals, resbasetype, ntps);
          lfinfo->tpfn_temp(startvalue1, segend1, startvalue2, segend2,
            lfinfo->param[0], start1->t, end1->t, tps, vals, ntps);
        }
        instants = lifting_instants_room(instants, &maxinsts, ninsts + ntps +
          3 * (seq1->count - i + seq2->count - j + 1));
        ninsts = lifting_append_ends(instants, ninsts, tps, vals, ntps,
          lfinfo);
        if (ntps > TPFN_STACK_CAP)
        {
          pfree(tps); pfree(vals);
        }
        ntps = 0;
      }
      if (many && unlikely(ntps > 2))
      {
        if (ntps > TPFN_STACK_CAP)
        {
          tps = palloc(sizeof(TimestampTz) * ntps);
          vals = lifting_values_room(vals, resbasetype, ntps);
          lfinfo->tpfn_temp(startvalue1, segend1, startvalue2, segend2,
            lfinfo->param[0], start1->t, end1->t, tps, vals, ntps);
        }
        /* The array holds three instants for each segment left, two turning
         * points and the instant closing the sequence or starting the next
         * segment, so it grows only when there are more */
        instants = lifting_instants_room(instants, &maxinsts, ninsts + ntps +
          3 * (seq1->count - i + seq2->count - j + 1));
      }
      for (int k = 0; k < ntps; k++)
      {
        /* A turning point on a bound is the instant added there */
        if (tps[k] <= start1->t || tps[k] >= end1->t ||
            (k > 0 && tps[k] == tps[k - 1]))
        {
          if (vals)
            DATUM_FREE(vals[k], resbasetype);
          continue;
        }
        Datum tpresult;
        if (vals)
          tpresult = vals[k];
        else
        {
          Datum tpvalue1 = tsegment_value_at_timestamptz(startvalue1, segend1,
            start1->temptype, start1->t, end1->t, tps[k]);
          Datum tpvalue2 = tsegment_value_at_timestamptz(startvalue2, segend2,
            start2->temptype, start1->t, end1->t, tps[k]);
          tpresult = tfunc_base_base(tpvalue1, tpvalue2, lfinfo);
          DATUM_FREE(tpvalue1, basetype); DATUM_FREE(tpvalue2, basetype);
        }
        instants[ninsts++] = tinstant_make_free(tpresult, restype, tps[k]);
      }
      if (many && unlikely(ntps > TPFN_STACK_CAP))
      {
        pfree(tps);
        if (vals)
          pfree(vals);
      }
    }
    /* Close the current sequence if the step sequence changed value, or at
     * the end of each segment */
    if (makeseq || lfinfo->ressegs)
    {
      /* The turning point function wrote the end of the segment, where the
       * step sequence keeps its start value */
      if (! ends)
      {
        Datum closeresult = (interp1 == LINEAR) ?
          tfunc_base_base(endvalue1, startvalue2, lfinfo) :
          tfunc_base_base(startvalue1, endvalue2, lfinfo);
        instants[ninsts++] = tinstant_make_free(closeresult, restype,
          end1->t);
      }
      result[nseqs++] = lifting_tsequence_make(instants, ninsts, lower_inc,
        false, LINEAR, NORMALIZE_NO);
      for (int k = 0; k < ninsts; k++)
        pfree(instants[k]);
      ninsts = 0;
    }
    start1 = end1; start2 = end2;
    startvalue1 = endvalue1; startvalue2 = endvalue2;
    if (! ends)
    {
      DATUM_FREE(startresult, resbasetype);
      startresult = endresult;
    }
    lower_inc = true;
  }
  /* Add a final sequence if any */
  if (inter->upper_inc)
  {
    if (ends)
      startresult = tfunc_base_base(startvalue1, startvalue2, lfinfo);
    instants[ninsts++] = tinstant_make_free(startresult, restype, start1->t);
  }
  else if (! ends)
    DATUM_FREE(startresult, resbasetype);
  if (ninsts > 0)
    result[nseqs++] = lifting_tsequence_make(instants, ninsts,
      (ninsts == 1) ? true : lower_inc, (ninsts == 1) ? true : inter->upper_inc,
      LINEAR, lfinfo->reskeep ? NORMALIZE_NO : NORMALIZE);
  pfree_array((void **) tofree, nfree);
  pfree(instants);
  return nseqs;
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @note This function is called when one sequence has linear and the other
 * has step interpolation, in either order, and #tfunc_tcontseq_tcontseq_single
 * when both have the same interpolation. The value of the step sequence
 * changes at its instants, so a sequence of the result closes at each of
 * them, while the instants of the linear sequence are kept inside it, unless
 * the result has one sequence for each segment (ressegs).
 */
static int
tfunc_tlinearseq_tstepseq(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo, Span *inter, TSequence **result)
{
  if (lfinfo->tpfn_many || lfinfo->tpfn_value)
    return tfunc_tlinearseq_tstepseq_walk(seq1, seq2, lfinfo, inter, result,
      true, lfinfo->tpfn_value);
  return tfunc_tlinearseq_tstepseq_walk(seq1, seq2, lfinfo, inter, result,
    false, false);
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @note This function is called for each composing sequence of a temporal
 * sequence set and therefore the bounding period test is repeated
 */
static int
tfunc_tcontseq_tcontseq_dispatch(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo, TSequence **result)
{
  /* Test whether the bounding period of the two temporal values overlap */
  Span inter;
  if (! inter_span_span(&seq1->period, &seq2->period, &inter))
    return 0;

  /* If the two sequences intersect at an instant */
  if (inter.lower == inter.upper)
  {
    Datum value1, value2;
    tsequence_value_at_timestamptz(seq1, inter.lower, true, &value1);
    tsequence_value_at_timestamptz(seq2, inter.lower, true, &value2);
    Datum resvalue = tfunc_base_base(value1, value2, lfinfo);
    TInstant *inst = tinstant_make_free(resvalue, lfinfo->restype, inter.lower);
    interpType interp = lfinfo->reslinear ? LINEAR : STEP;
    result[0] = lifting_tinstant_to_tsequence_free(inst, interp);
    DATUM_FREE(value1, temptype_basetype(seq1->temptype));
    DATUM_FREE(value2, temptype_basetype(seq2->temptype));
    return 1;
  }

  if (lfinfo->discont)
    return tfunc_tcontseq_tcontseq_discfn(seq1, seq2, lfinfo, &inter, result);

  bool linear1 = MEOS_FLAGS_LINEAR_INTERP(seq1->flags);
  bool linear2 = MEOS_FLAGS_LINEAR_INTERP(seq2->flags);
  if (linear1 == linear2)
    return tfunc_tcontseq_tcontseq_single(seq1, seq2, lfinfo, &inter, result);
  else
    return tfunc_tlinearseq_tstepseq(seq1, seq2, lfinfo, &inter, result);
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 */
Temporal *
tfunc_tcontseq_tcontseq(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo)
{
  int count;
  if (lfinfo->discont)
    count = (seq1->count + seq2->count) * 3;
  else
  {
    if (MEOS_FLAGS_LINEAR_INTERP(seq1->flags) ==
        MEOS_FLAGS_LINEAR_INTERP(seq2->flags))
      count = 1;
    else
      count = (seq1->count + seq2->count) * 2;
  }
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int nseqs = tfunc_tcontseq_tcontseq_dispatch(seq1, seq2, lfinfo, sequences);
  /* The following is ensured by the period bound test in the dispatch
   * function */
  assert(nseqs > 0);
  if (nseqs == 1)
  {
    Temporal *result = (Temporal *) sequences[0];
    pfree(sequences);
    return result;
  }
  else
  {
    TSequenceSet *result = lifting_tsequenceset_make_free(sequences, nseqs,
      lfinfo->reskeep ? NORMALIZE_NO : NORMALIZE);
    if (result && result->count == 1)
    {
      Temporal *resultseq = (Temporal *) tsequenceset_as_tsequence(result);
      pfree(result);
      return resultseq;
    }
    return (Temporal *) result;
  }
}

/*****************************************************************************/

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] ss,seq Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static Temporal *
tfunc_tsequenceset_tcontseq(const TSequenceSet *ss, const TSequence *seq,
  LiftedFunctionInfo *lfinfo)
{
  int loc;
  tsequenceset_find_timestamptz(ss, seq->period.lower, &loc);
  /* We are sure that loc < ss->count due to the bounding period test made
   * in the dispatch function */
  int count;
  if (lfinfo->discont)
    count = (ss->totalcount + seq->count) * 3;
  else
  {
    if (MEOS_FLAGS_LINEAR_INTERP(ss->flags) ==
        MEOS_FLAGS_LINEAR_INTERP(seq->flags))
      count = ss->count - loc;
    else
      count = (ss->totalcount + seq->count) * 2;
  }
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  TimestampTz upper = DatumGetTimestampTz(seq->period.upper);
  int nseqs = 0;
  for (int i = loc; i < ss->count; i++)
  {
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss, i);
    nseqs += tfunc_tcontseq_tcontseq_dispatch(seq1, seq, lfinfo,
      &sequences[nseqs]);
    int cmp = timestamptz_cmp_internal(upper,
      DatumGetTimestampTz(seq1->period.upper));
    if (cmp < 0 ||
      (cmp == 0 && (! seq->period.upper_inc || seq1->period.upper_inc)))
      break;
  }
  /* We need to normalize when discont is true */
  return (Temporal *) lifting_tsequenceset_make_free(sequences, nseqs,
    NORMALIZE);
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] seq,ss Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static Temporal *
tfunc_tcontseq_tsequenceset(const TSequence *seq, const TSequenceSet *ss,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return tfunc_tsequenceset_tcontseq(ss, seq, lfinfo);
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] ss1,ss2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
TSequenceSet *
tfunc_tsequenceset_tsequenceset(const TSequenceSet *ss1,
  const TSequenceSet *ss2, LiftedFunctionInfo *lfinfo)
{
  int count = ss1->totalcount + ss2->totalcount;
  if (lfinfo->discont)
    count *= 3;
  else
  {
    if (MEOS_FLAGS_LINEAR_INTERP(ss1->flags) !=
        MEOS_FLAGS_LINEAR_INTERP(ss2->flags))
      count *= 2;
  }
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int i = 0, j = 0, nseqs = 0;
  while (i < ss1->count && j < ss2->count)
  {
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss1, i);
    const TSequence *seq2 = TSEQUENCESET_SEQ_N(ss2, j);
    nseqs += tfunc_tcontseq_tcontseq_dispatch(seq1, seq2, lfinfo,
      &sequences[nseqs]);
    int cmp = timestamptz_cmp_internal(DatumGetTimestampTz(seq1->period.upper),
      DatumGetTimestampTz(seq2->period.upper));
    if (cmp == 0)
    {
      if (! seq1->period.upper_inc && seq2->period.upper_inc)
        cmp = -1;
      else if (seq1->period.upper_inc && !seq2->period.upper_inc)
        cmp = 1;
    }
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
      i++;
    else
      j++;
  }
  /* We need to normalize if the function has instantaneous discontinuities */
  return lifting_tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/*****************************************************************************/

/**
 * @brief Apply a lifted function to two temporal values
 * @note The error number is read by #tfunc_temporal_temporal
 */
static Temporal *
tfunc_temporal_temporal_dispatch(const Temporal *temp1, const Temporal *temp2,
  LiftedFunctionInfo *lfinfo)
{
  /* Bounding box test */
  Span s1, s2;
  temporal_set_tstzspan(temp1, &s1);
  temporal_set_tstzspan(temp2, &s2);
  if (! overlaps_span_span(&s1, &s2))
    return NULL;

  tempSubtype subtype1 = temp1->subtype;
  tempSubtype subtype2 = temp2->subtype;
  assert(temptype_subtype(subtype1));
  assert(temptype_subtype(subtype2));
  switch (subtype1)
  {
    case TINSTANT:
    {
      switch (subtype2)
      {
        case TINSTANT:
          return (Temporal *) tfunc_tinstant_tinstant(
            (TInstant *) temp1, (TInstant *) temp2, lfinfo);
        case TSEQUENCE:
          return MEOS_FLAGS_DISCRETE_INTERP(temp2->flags) ?
            (Temporal *) tfunc_tinstant_tdiscseq(
              (TInstant *) temp1, (TSequence *) temp2, lfinfo) :
            (Temporal *) tfunc_tinstant_tcontseq(
              (TInstant *) temp1, (TSequence *) temp2, lfinfo);
        default: /* TSEQUENCESET */
          return (Temporal *) tfunc_tinstant_tsequenceset(
            (TInstant *) temp1, (TSequenceSet *) temp2, lfinfo);
      }
    }
    case TSEQUENCE:
    {
      const TSequence *seq1 = (const TSequence *) temp1;
      interpType interp1 = MEOS_FLAGS_GET_INTERP(seq1->flags);
      switch (subtype2)
      {
        case TINSTANT:
          return (interp1 == DISCRETE) ?
            (Temporal *) tfunc_tdiscseq_tinstant(
              (TSequence *) temp1, (TInstant *) temp2, lfinfo) :
            (Temporal *) tfunc_tcontseq_tinstant(
              (TSequence *) temp1, (TInstant *) temp2, lfinfo);
        case TSEQUENCE:
        {
          const TSequence *seq2 = (const TSequence *) temp2;
          interpType interp2 = MEOS_FLAGS_GET_INTERP(temp2->flags);
          if (interp1 == DISCRETE)
          {
            if (interp2 == DISCRETE)
              return (Temporal *) tfunc_tdiscseq_tdiscseq(seq1, seq2, lfinfo);
            else /* interp2 != DISCRETE */
              return (Temporal *) tfunc_tdiscseq_tcontseq(seq1, seq2, lfinfo);
          }
          else /* interp1 != DISCRETE */
          {
            if (interp2 == DISCRETE)
              return (Temporal *) tfunc_tcontseq_tdiscseq(seq1, seq2, lfinfo);
            else /* interp2 = DISCRETE */
              return (Temporal *) tfunc_tcontseq_tcontseq(seq1, seq2, lfinfo);
          }
        }
        default: /* TSEQUENCESET */
          return MEOS_FLAGS_DISCRETE_INTERP(temp1->flags) ?
            (Temporal *) tfunc_tdiscseq_tsequenceset(
              (TSequence *) temp1, (TSequenceSet *) temp2, lfinfo) :
            (Temporal *) tfunc_tcontseq_tsequenceset(
              (TSequence *) temp1, (TSequenceSet *) temp2, lfinfo);
      }
    }
    default: /* TSEQUENCESET */
    {
      switch (subtype2)
      {
        case TINSTANT:
          return (Temporal *) tfunc_tsequenceset_tinstant(
            (TSequenceSet *) temp1, (TInstant *) temp2, lfinfo);
        case TSEQUENCE:
          return MEOS_FLAGS_DISCRETE_INTERP(temp2->flags) ?
            (Temporal *) tfunc_tsequenceset_tdiscseq(
              (TSequenceSet *) temp1, (TSequence *) temp2, lfinfo) :
            (Temporal *) tfunc_tsequenceset_tcontseq(
              (TSequenceSet *) temp1, (TSequence *) temp2, lfinfo);
        default: /* TSEQUENCESET */
          return (Temporal *) tfunc_tsequenceset_tsequenceset(
              (TSequenceSet *) temp1, (TSequenceSet *) temp2, lfinfo);
      }
    }
  }
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] temp1,temp2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @note An error raised by the function applied makes the answer the
 * failure value, see the note on errors at the beginning of the file
 */
Temporal *
tfunc_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
  LiftedFunctionInfo *lfinfo)
{
  int last_errno = meos_errno_reset();
  return (Temporal *) lifting_result(
    tfunc_temporal_temporal_dispatch(temp1, temp2, lfinfo), last_errno);
}

/*****************************************************************************
 * Functions that take either (1) a temporal value and a base value, or (2) two
 * temporal values and apply to them a Boolean functio using the ever/always
 * semantics, that is, it stops when a true value (for ever) or a false value
 * (for always) is found. In the case of two temporal values the functions
 * synchronize two temporal values and apply the function in a single pass.
 *****************************************************************************/

/**
 * @brief Apply a lifted function to a temporal instant and a base value
 * @param[in] inst Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 */
static int
eafunc_tinstant_base(const TInstant *inst, Datum value,
  LiftedFunctionInfo *lfinfo)
{
  assert(inst);
  /* Result is the same for both EVER and ALWAYS */
  return DatumGetBool(tfunc_base_base(tinstant_value_p(inst), value, lfinfo)) ?
    1 : 0;
}

/**
 * @brief Apply a lifted function to a temporal discrete or step sequence and a
 * base value
 * @param[in] seq Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 */
static int
eafunc_tdiscstepseq_base(const TSequence *seq, Datum value,
  LiftedFunctionInfo *lfinfo)
{
  assert(seq); assert(MEOS_FLAGS_GET_INTERP(seq->flags) != LINEAR);
  for (int i = 0; i < seq->count; i++)
  {
    Datum value1 = tinstant_value_p(TSEQUENCE_INST_N(seq, i));
    bool res = DatumGetBool(tfunc_base_base(value1, value, lfinfo));
    if (lfinfo->ever && res)
      return 1;
    else if (! lfinfo->ever && ! res)
      return 0;
  }
  return lfinfo->ever ? 0 : 1;
}

/**
 * @brief Apply a lifted function to a temporal continuous sequence and a base
 * value
 * @param[in] seq Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 * @pre The lifted function is discontinuous
 * @note All ever/always functions currently available, that is, comparisons
 * (=, <, ...) and spatial relationship (contains, intersects, ...), are
 * discontinuous. When this would be no longer the case, additional functions
 * must be added to take account of the remaining cases
 */
/**
 * @brief Apply the lifted function to a value that a segment takes and return
 * whether the answer for the whole sequence is thereby decided
 * @param[in] value1 Value that the segment takes
 * @param[in] value2 Value that the lifted function is applied against
 * @param[in] lfinfo Information about the lifted function
 * @param[out] result Answer for the sequence, set when the answer is decided
 */
static bool
eafunc_value_decides(Datum value1, Datum value2, LiftedFunctionInfo *lfinfo,
  int *result)
{
  bool res = DatumGetBool(tfunc_base_base(value1, value2, lfinfo));
  if (lfinfo->ever && res)
  {
    *result = 1;
    return true;
  }
  if (! lfinfo->ever && ! res)
  {
    *result = 0;
    return true;
  }
  return false;
}

/**
 * @brief Apply the lifted function to the value that a segment takes midway
 * between two instants of it and return whether the answer is thereby decided
 * @param[in] startvalue,endvalue Values defining the segment
 * @param[in] temptype Temporal type
 * @param[in] lower,upper Timestamps defining the segment
 * @param[in] t1,t2 Timestamps defining the piece to take a value from
 * @param[in] value Value that the lifted function is applied against
 * @param[in] lfinfo Information about the lifted function
 * @param[out] result Answer for the sequence, set when the answer is decided
 * @note The midpoint of a piece shorter than two microseconds is a bound of
 * it, which is as finely as the time type resolves a piece
 */
static bool
eafunc_midpoint_decides(Datum startvalue, Datum endvalue, MeosType temptype,
  TimestampTz lower, TimestampTz upper, TimestampTz t1, TimestampTz t2,
  Datum value, LiftedFunctionInfo *lfinfo, int *result)
{
  TimestampTz tmid = t1 + (TimestampTz) ((t2 - t1) / 2);
  Datum midvalue = tsegment_value_at_timestamptz(startvalue, endvalue,
    temptype, lower, upper, tmid);
  bool decided = eafunc_value_decides(midvalue, value, lfinfo, result);
  DATUM_FREE(midvalue, temptype_basetype(temptype));
  return decided;
}

static int
eafunc_tlinearseq_base(const TSequence *seq, Datum value,
  LiftedFunctionInfo *lfinfo)
{
  assert(seq); assert(MEOS_FLAGS_GET_INTERP(seq->flags) == LINEAR);

  /* Instantaneous sequence */
  if (seq->count == 1)
    return eafunc_tinstant_base(TSEQUENCE_INST_N(seq, 0), value, lfinfo);

  /* General case */
  bool lower_inc = seq->period.lower_inc;
  MeosType basetype = temptype_basetype(seq->temptype);
  TInstant *start = (TInstant *) TSEQUENCE_INST_N(seq, 0);
  int result;
  for (int i = 1; i < seq->count; i++)
  {
    /* Compute the function at the start instant, which the segment takes
     * only when its lower bound is inclusive */
    Datum startvalue = tinstant_value_p(start);
    if (lower_inc &&
        eafunc_value_decides(startvalue, value, lfinfo, &result))
      return result;
    /* Compute the function at the end instant, which the segment takes only
     * when its upper bound is inclusive */
    TInstant *end = (TInstant *) TSEQUENCE_INST_N(seq, i);
    Datum endvalue = tinstant_value_p(end);
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    if (upper_inc &&
        eafunc_value_decides(endvalue, value, lfinfo, &result))
      return result;
    /* A constant segment takes its single value at every instant of it, the
     * interior included, so a bound it excludes leaves that value taken */
    if (datum_eq(startvalue, endvalue, basetype))
    {
      if (eafunc_value_decides(startvalue, value, lfinfo, &result))
        return result;
      continue;
    }
    /* Cross-type comparison (e.g. trgeometry vs geometry): skip the
     * segment-locate intersection finder. datumsegm_locate dispatches
     * by the temporal type's basetype (Pose for trgeometry), but the
     * right-hand `value` is a different type (a GSERIALIZED), so the
     * dispatcher would reinterpret the bytes through the wrong base
     * type and trip the SRID-equality check on garbage SRID bits.
     * The bounds-only checks above already cover the common case;
     * mid-segment crossings are conservatively missed for ?= / ?<>. */
    if (lfinfo->cross_type)
    {
      start = end;
      lower_inc = true;
      continue;
    }
    /* A float segment crosses the value strictly inside, in continuous time,
     * where the value lies strictly between its bound values, a comparison
     * of the doubles, whichever microsecond dates the crossing. On each side
     * of the crossing the segment compares with the value as its bound value
     * there does */
    if (basetype == T_FLOAT8)
    {
      double dstart = DatumGetFloat8(startvalue);
      double dend = DatumGetFloat8(endvalue);
      double dvalue = DatumGetFloat8(value);
      if (Min(dstart, dend) < dvalue && dvalue < Max(dstart, dend))
      {
        if (eafunc_value_decides(value, value, lfinfo, &result) ||
            eafunc_value_decides(startvalue, value, lfinfo, &result) ||
            eafunc_value_decides(endvalue, value, lfinfo, &result))
          return result;
        start = end;
        lower_inc = true;
        continue;
      }
    }
    /* Determine whether the segment crosses the value. The crossing is
     * strictly inside the segment, since the function answers 0 when the
     * value is the one of a bound, so the segment takes the value there
     * whichever bounds it excludes */
    TimestampTz tpt1, tpt2;
    int cross = tsegment_intersection_value(startvalue, endvalue, value,
      start->temptype, start->t, end->t, &tpt1, &tpt2);
    if (cross && eafunc_value_decides(value, value, lfinfo, &result))
      return result;
    /* The crossing cuts the segment into pieces on which the lifted function
     * is constant, the function comparing against a single value and the
     * segment being linear. One value taken strictly inside a piece therefore
     * decides that piece, and the midpoint is such a value */
    if (cross)
    {
      if (eafunc_midpoint_decides(startvalue, endvalue, start->temptype,
            start->t, end->t, start->t, tpt1, value, lfinfo, &result))
        return result;
      if (eafunc_midpoint_decides(startvalue, endvalue, start->temptype,
            start->t, end->t, tpt1, end->t, value, lfinfo, &result))
        return result;
    }
    else if (eafunc_midpoint_decides(startvalue, endvalue, start->temptype,
               start->t, end->t, start->t, end->t, value, lfinfo, &result))
      return result;
    start = end;
    lower_inc = true;
  }
  return lfinfo->ever ? 0 : 1;
}


/**
 * @brief Apply a lifted function to a temporal sequence and a base value
 * (dispatch function)
 * @param[in] seq Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 */
static int
eafunc_tsequence_base(const TSequence *seq, Datum value,
  LiftedFunctionInfo *lfinfo)
{
  assert(seq);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  if (interp == DISCRETE || interp == STEP)
    return eafunc_tdiscstepseq_base(seq, value, lfinfo);
  else
    return eafunc_tlinearseq_base(seq, value, lfinfo);
}

/**
 * @brief Apply a lifted function to a temporal sequence set and a base value
 * @param[in] ss Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 */
static int
eafunc_tsequenceset_base(const TSequenceSet *ss, Datum value,
  LiftedFunctionInfo *lfinfo)
{
  assert(ss);
  /* Singleton sequence set */
  if (ss->count == 1)
    return eafunc_tsequence_base(TSEQUENCESET_SEQ_N(ss, 0), value, lfinfo);

  /* General case */
  for (int i = 0; i < ss->count; i++)
  {
    int res = eafunc_tsequence_base(TSEQUENCESET_SEQ_N(ss, i), value, lfinfo);
    if (lfinfo->ever && res == 1)
      return 1;
    else if (! lfinfo->ever && res != 1)
      return 0;
  }
  return lfinfo->ever ? 0 : 1;
}

/*****************************************************************************/

/**
 * @brief Apply a lifted ever or always function to a temporal value and a
 * base value
 * @note The error number is read by #eafunc_temporal_base
 */
static int
eafunc_temporal_base_dispatch(const Temporal *temp, Datum value,
  LiftedFunctionInfo *lfinfo)
{
  assert(temp);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return eafunc_tinstant_base((TInstant *) temp, value, lfinfo);
    case TSEQUENCE:
      return eafunc_tsequence_base((TSequence *) temp, value, lfinfo);
    default: /* TSEQUENCESET */
      return eafunc_tsequenceset_base((TSequenceSet *) temp, value, lfinfo);
  }
}

/**
 * @brief Apply a lifted function to a temporal value and a base value
 * @param[in] temp Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 * @note An error raised by the function applied makes the answer the
 * failure value, see the note on errors at the beginning of the file
 */
int
eafunc_temporal_base(const Temporal *temp, Datum value,
  LiftedFunctionInfo *lfinfo)
{
  int last_errno = meos_errno_reset();
  return lifting_result_int(
    eafunc_temporal_base_dispatch(temp, value, lfinfo), last_errno);
}

/*****************************************************************************/

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] inst1,inst2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
eafunc_tinstant_tinstant(const TInstant *inst1, const TInstant *inst2,
  LiftedFunctionInfo *lfinfo)
{
  assert(inst1); assert(inst2); assert(inst1->temptype == inst2->temptype);
  /* The following is ensured by the period bound test in the dispatch
   * function */
  assert(inst1->t == inst2->t);
  /* Result is the same for both EVER and ALWAYS */
  return DatumGetBool(tfunc_base_base(tinstant_value_p(inst1), tinstant_value_p(inst2),
    lfinfo)) ? 1 : 0;
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] seq,inst Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
eafunc_tdiscseq_tinstant(const TSequence *seq, const TInstant *inst,
  LiftedFunctionInfo *lfinfo)
{
  assert(seq); assert(inst); assert(seq->temptype == inst->temptype);
  Datum value1;
  if (! tdiscseq_value_at_timestamptz(seq, inst->t, &value1))
    return -1;
  /* Result is the same for both EVER and ALWAYS */
  return DatumGetBool(tfunc_base_base(value1, tinstant_value_p(inst), lfinfo)) ?
    1 : 0;
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] inst,seq Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
eafunc_tinstant_tdiscseq(const TInstant *inst, const TSequence *seq,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return eafunc_tdiscseq_tinstant(seq, inst, lfinfo);
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] seq,inst Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
eafunc_tcontseq_tinstant(const TSequence *seq, const TInstant *inst,
  LiftedFunctionInfo *lfinfo)
{
  assert(seq); assert(inst); assert(seq->temptype == inst->temptype);
  Datum value1;
  /* The following call is ensured to return true due to the period bound test
   * in the dispatch function */
  tsequence_value_at_timestamptz(seq, inst->t, true, &value1);
  /* Result is the same for both EVER and ALWAYS */
  bool result = DatumGetBool(tfunc_base_base(value1, tinstant_value_p(inst),
    lfinfo)) ? 1 : 0;
  DATUM_FREE(value1, temptype_basetype(seq->temptype));
  return result;
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] inst,seq Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
eafunc_tinstant_tcontseq(const TInstant *inst, const TSequence *seq,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return eafunc_tcontseq_tinstant(seq, inst, lfinfo);
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] ss,inst Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
eafunc_tsequenceset_tinstant(const TSequenceSet *ss, const TInstant *inst,
  LiftedFunctionInfo *lfinfo)
{
  assert(ss); assert(inst); assert(ss->temptype == inst->temptype);
  Datum value1;
  if (! tsequenceset_value_at_timestamptz(ss, inst->t, true, &value1))
    return -1;
  /* Result is the same for both EVER and ALWAYS */
  return DatumGetBool(tfunc_base_base(value1, tinstant_value_p(inst), lfinfo)) ?
    1 : 0;
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] inst,ss Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
eafunc_tinstant_tsequenceset(const TInstant *inst, const TSequenceSet *ss,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return eafunc_tsequenceset_tinstant(ss, inst, lfinfo);
}

/*****************************************************************************/

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
eafunc_tdiscseq_tdiscseq(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo)
{
  assert(seq1); assert(seq2); assert(seq1->temptype == seq2->temptype);
  /* We need to verify that the sequences intersect in time in addition
   * to the bounding period test done in the dispatch function
   * #eafunc_temporal_temporal to return -1 if they do not intersect in time
   */
  SpanSet *s1 = tsequence_time(seq1);
  SpanSet *s2 = tsequence_time(seq2);
  bool found = overlaps_spanset_spanset(s1, s2);
  pfree(s1); pfree(s2);
  if (! found)
      return -1;

  int i = 0, j = 0;
  const TInstant *inst1 = TSEQUENCE_INST_N(seq1, i);
  const TInstant *inst2 = TSEQUENCE_INST_N(seq2, j);
  while (i < seq1->count && j < seq2->count)
  {
    int cmp = timestamptz_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      bool res = DatumGetBool(tfunc_base_base(tinstant_value_p(inst1),
        tinstant_value_p(inst2), lfinfo));
      if (lfinfo->ever && res)
        return 1;
      else if (! lfinfo->ever && ! res)
        return 0;
      /* Both values move on to their next instant, as #tfunc_tdiscseq_tdiscseq
       * does, so that every common instant is read */
      inst1 = TSEQUENCE_INST_N(seq1, ++i);
      inst2 = TSEQUENCE_INST_N(seq2, ++j);
    }
    else if (cmp < 0)
      inst1 = TSEQUENCE_INST_N(seq1, ++i);
    else
      inst2 = TSEQUENCE_INST_N(seq2, ++j);
  }
  return lfinfo->ever ? 0 : 1;
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @note The bounding period test in the dispatch function
 * #eafunc_temporal_temporal ensures that the sequences are not disjoint and
 * thus we are sure that we do not need to return -1 in this function
 */
static int
eafunc_tcontseq_tdiscseq(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo)
{
  assert(seq1); assert(seq2); assert(seq1->temptype == seq2->temptype);
  /* We need to verify that the sequences intersect in time in addition
   * to the bounding period test done in the dispatch function
   * #eafunc_temporal_temporal to return -1 if they do not intersect in time
   */
  SpanSet *s1 = tsequence_time(seq1);
  SpanSet *s2 = tsequence_time(seq2);
  bool found = overlaps_spanset_spanset(s1, s2);
  pfree(s1); pfree(s2);
  if (! found)
      return -1;

  TimestampTz upper1 = DatumGetTimestampTz(seq1->period.upper);
  for (int i = 0; i < seq2->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq2, i);
    if (contains_span_timestamptz(&seq1->period, inst->t))
    {
      Datum value1;
      tsequence_value_at_timestamptz(seq1, inst->t, true, &value1);
      bool res = DatumGetBool(tfunc_base_base(value1, tinstant_value_p(inst),
        lfinfo));
      DATUM_FREE(value1, temptype_basetype(seq1->temptype));
      if (lfinfo->ever && res)
        return 1;
      else if (! lfinfo->ever && ! res)
        return 0;
    }
    if (upper1 < inst->t)
      break;
  }
  return lfinfo->ever ? 0 : 1;
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
eafunc_tdiscseq_tcontseq(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return eafunc_tcontseq_tdiscseq(seq2, seq1, lfinfo);
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] ss,seq Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
eafunc_tsequenceset_tdiscseq(const TSequenceSet *ss, const TSequence *seq,
  LiftedFunctionInfo *lfinfo)
{
  assert(ss); assert(seq); assert(ss->temptype == seq->temptype);
  /* We need to verify that the sequences intersect in time in addition
   * to the bounding period test done in the dispatch function
   * #eafunc_temporal_temporal to return -1 if they do not intersect in time
   */
  SpanSet *s1 = tsequenceset_time(ss);
  SpanSet *s2 = tsequence_time(seq);
  bool found = overlaps_spanset_spanset(s1, s2);
  pfree(s1); pfree(s2);
  if (! found)
      return -1;

  int i = 0, j = 0;
  while (i < ss->count && j < seq->count)
  {
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss, i);
    const TInstant *inst = TSEQUENCE_INST_N(seq, j);
    if (contains_span_timestamptz(&seq1->period, inst->t))
    {
      Datum value1;
      tsequenceset_value_at_timestamptz(ss, inst->t, true, &value1);
      bool res = DatumGetBool(tfunc_base_base(value1, tinstant_value_p(inst),
        lfinfo));
      if (lfinfo->ever && res)
        return 1;
      else if (! lfinfo->ever && ! res)
        return 0;
    }
    int cmp = timestamptz_cmp_internal(DatumGetTimestampTz(seq1->period.upper),
      inst->t);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
      i++;
    else
      j++;
  }
  return lfinfo->ever ? 0 : 1;
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] seq,ss Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
eafunc_tdiscseq_tsequenceset(const TSequence *seq, const TSequenceSet *ss,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return eafunc_tsequenceset_tdiscseq(ss, seq, lfinfo);
}

/**
 * @brief Return the lifted function applied to the values that two
 * synchronized segments take at a timestamp, or the answer that decides
 * neither quantifier when the segments do not take values there
 * @param[in] startvalue1,endvalue1 Values defining the first segment
 * @param[in] startvalue2,endvalue2 Values defining the second segment
 * @param[in] start,end Instants whose timestamps define the segments
 * @param[in] t Timestamp
 * @param[in] lower_inc True when the segments take the start timestamp
 * @param[in] lfinfo Information about the lifted function
 * @note The middle of a piece shorter than two microseconds is its start,
 * which is as finely as the time type resolves a piece, as for
 * #eafunc_midpoint_decides
 */
static bool
eafunc_segments_at(Datum startvalue1, Datum endvalue1, Datum startvalue2,
  Datum endvalue2, const TInstant *start, const TInstant *end, TimestampTz t,
  bool lower_inc, LiftedFunctionInfo *lfinfo)
{
  assert(start); assert(end); assert(start->t <= t); assert(t < end->t);
  if (t == start->t && ! lower_inc)
    return ! lfinfo->ever;
  MeosType basetype = temptype_basetype(start->temptype);
  Datum value1 = tsegment_value_at_timestamptz(startvalue1, endvalue1,
    start->temptype, start->t, end->t, t);
  Datum value2 = tsegment_value_at_timestamptz(startvalue2, endvalue2,
    start->temptype, start->t, end->t, t);
  bool result = DatumGetBool(tfunc_base_base(value1, value2, lfinfo));
  DATUM_FREE(value1, basetype);
  DATUM_FREE(value2, basetype);
  return result;
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @details This function is applied for functions with instantaneous
 * discontinuities and when at least one temporal value has linear
 * interpolation
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @param[in] inter Overlapping period of the two sequences
 * @note The bounding period test in the dispatch function
 * #eafunc_temporal_temporal ensures that the sequences are not disjoint and
 * thus we are sure that we do not need to return -1 in this function
 */
static int
eafunc_tcontseq_tcontseq_discfn(const TSequence *seq1,
  const TSequence *seq2, LiftedFunctionInfo *lfinfo, Span *inter)
{
  assert(seq1); assert(seq2); assert(seq1->temptype == seq2->temptype);
  /* Array that keeps the new instants added for synchronization */
  TInstant **tofree = palloc(sizeof(TInstant *) *
    (seq1->count + seq2->count) * 3);
  TInstant *start1 = (TInstant *) TSEQUENCE_INST_N(seq1, 0);
  TInstant *start2 = (TInstant *) TSEQUENCE_INST_N(seq2, 0);
  int i = 1, j = 1, nfree = 0;
  /* Synchronize the two start instant */
  if (start1->t < DatumGetTimestampTz(inter->lower))
  {
    start1 = tsequence_at_timestamptz(seq1, inter->lower);
    tofree[nfree++] = start1;
    i = tcontseq_find_timestamptz(seq1, inter->lower) + 1;
  }
  else if (start2->t < DatumGetTimestampTz(inter->lower))
  {
    start2 = tsequence_at_timestamptz(seq2, inter->lower);
    tofree[nfree++] = start2;
    j = tcontseq_find_timestamptz(seq2, inter->lower) + 1;
  }
  bool lower_inc = inter->lower_inc;
  interpType interp1 = MEOS_FLAGS_GET_INTERP(seq1->flags);
  interpType interp2 = MEOS_FLAGS_GET_INTERP(seq2->flags);
  MeosType basetype = temptype_basetype(seq1->temptype);
  /* Whether the values of two linear sequences are equal at the start of
   * the segment, as read for a comparison */
  bool linear = (interp1 == LINEAR && interp2 == LINEAR);
  bool prevendeq = linear && ! lfinfo->tpfn_temp &&
    datum_eq(tinstant_value_p(start1), tinstant_value_p(start2), basetype);
  bool res;
  while (i < seq1->count && j < seq2->count)
  {
    /* Compute the function at the start instant */
    Datum startvalue1 = tinstant_value_p(start1);
    Datum startvalue2 = tinstant_value_p(start2);
    if (lower_inc)
    {
      res = DatumGetBool(tfunc_base_base(startvalue1, startvalue2, lfinfo));
      if ((lfinfo->ever && res) || (! lfinfo->ever && ! res))
      {
        pfree_array((void **) tofree, nfree);
        return lfinfo->ever ? 1 : 0;
      }
    }
    /* Synchronize the two end instants */
    TInstant *end1 = (TInstant *) TSEQUENCE_INST_N(seq1, i);
    TInstant *end2 = (TInstant *) TSEQUENCE_INST_N(seq2, j);
    int cmp = timestamptz_cmp_internal(end1->t, end2->t);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
    {
      i++;
      end2 = tsegment_at_timestamptz(start2, end2, interp2, end1->t);
      tofree[nfree++] = end2;
    }
    else
    {
      j++;
      end1 = tsegment_at_timestamptz(start1, end1, interp1, end2->t);
      tofree[nfree++] = end1;
    }
    /* Values that the segments tend to at the end instant. A step segment
     * jumps there and the period may exclude it, so the end instant is read
     * only where the sequences take it, as the start of the next segment or
     * as the final instant below */
    Datum endvalue1 = (interp1 == LINEAR) ? tinstant_value_p(end1) : startvalue1;
    Datum endvalue2 = (interp2 == LINEAR) ? tinstant_value_p(end2) : startvalue2;
    /* Read the function right before the end instant and, when the period
     * excludes the start instant, right after it, where the segments take
     * values whichever bounds the period excludes */
    TimestampTz tpt1, tpt2;
    int cross = 0;
    if (lfinfo->tpfn_temp)
    {
      /* The function changes only at the bounds of the period [tpt1, tpt2]
       * that the turning point function returns, so it is read in the middle
       * of the pieces of the segment between them and the segment bounds,
       * the whole segment without a turning point */
      TimestampTz tpi[2];
      cross = lfinfo->tpfn_temp(startvalue1, endvalue1, startvalue2,
        endvalue2, lfinfo->param[0], start1->t, end1->t, tpi, NULL, 2);
      assert(cross <= 2);
      bool decides;
      if (lfinfo->tpfn_temp_bound)
      {
        /* A lift with a bound function states with its turning point
         * function the period where the predicate holds inside the segment,
         * a touch at one instant or an interval: it holds inside the segment
         * where the period meets its interior, and fails there where the
         * period does not cover it */
        if (cross == 1)
          tpi[1] = tpi[0];
        bool holds = cross && (tpi[0] < tpi[1] ||
          (tpi[0] > start1->t && tpi[0] < end1->t));
        bool fails = ! cross || tpi[0] > start1->t || tpi[1] < end1->t;
        decides = lfinfo->ever ? holds : fails;
        cross = 0;
      }
      else if (cross == 0)
        decides = eafunc_segments_at(startvalue1, endvalue1, startvalue2,
          endvalue2, start1, end1, start1->t + (end1->t - start1->t) / 2,
          lower_inc, lfinfo) == lfinfo->ever;
      else
      {
        tpt1 = tpi[0];
        tpt2 = (cross > 1) ? tpi[1] : tpt1;
        TimestampTz first = end1->t, last = start1->t;
        if (start1->t < tpt2 && tpt2 < end1->t)
          first = last = tpt2;
        if (start1->t < tpt1 && tpt1 < end1->t)
        {
          first = tpt1;
          if (last == start1->t)
            last = tpt1;
        }
        TimestampTz before = last + (end1->t - last) / 2;
        decides = (! lower_inc && eafunc_segments_at(startvalue1, endvalue1,
            startvalue2, endvalue2, start1, end1,
            start1->t + (first - start1->t) / 2, lower_inc, lfinfo) ==
            lfinfo->ever) ||
          eafunc_segments_at(startvalue1, endvalue1, startvalue2, endvalue2,
            start1, end1, before, lower_inc, lfinfo) == lfinfo->ever;
        /* The crossing below reads only tpt1 where the middle of the period
         * [tpt1, tpt2] is the timestamp read right before the end */
        if (tpt1 + (tpt2 - tpt1) / 2 == before)
          tpt2 = tpt1;
      }
      if (decides)
      {
        pfree_array((void **) tofree, nfree);
        return lfinfo->ever ? 1 : 0;
      }
    }
    else
    {
      /* The function compares the two values, and their gap, linear over
       * the segment, keeps one sign between a bound where it is zero and the
       * other bound. The segments therefore compare right after the start as
       * at the start where they differ there and as at the end otherwise,
       * and symmetrically right before the end, which is the start instant
       * read above where the period takes it and the end values are equal.
       * Two linear segments start with the end values of the segments
       * before them, so their end values are compared at every segment, and
       * those of other segments only where a read depends on them */
      bool starteq = linear ? prevendeq :
        datum_eq(startvalue1, startvalue2, basetype);
      bool endknown = linear || ! lower_inc;
      bool endeq = endknown && datum_eq(endvalue1, endvalue2, basetype);
      prevendeq = endeq;
      if ((lower_inc && ! endeq &&
            DatumGetBool(tfunc_base_base(endvalue1, endvalue2, lfinfo)) ==
              lfinfo->ever &&
            (endknown || ! datum_eq(endvalue1, endvalue2, basetype))) ||
          (! lower_inc && (DatumGetBool(tfunc_base_base(
              starteq ? endvalue1 : startvalue1,
              starteq ? endvalue2 : startvalue2, lfinfo)) == lfinfo->ever ||
            DatumGetBool(tfunc_base_base(endeq ? startvalue1 : endvalue1,
              endeq ? startvalue2 : endvalue2, lfinfo)) == lfinfo->ever)))
      {
        pfree_array((void **) tofree, nfree);
        return lfinfo->ever ? 1 : 0;
      }
      /* Two float segments cross strictly inside, in continuous time, where
       * their gap has opposite signs at the bounds, comparisons of the
       * doubles, whichever microsecond dates the crossing, and take one
       * value there */
      if (start1->temptype == T_TFLOAT)
      {
        double s1 = DatumGetFloat8(startvalue1);
        double s2 = DatumGetFloat8(startvalue2);
        double e1 = DatumGetFloat8(endvalue1);
        double e2 = DatumGetFloat8(endvalue2);
        int sstart = (s1 > s2) - (s1 < s2);
        int send = (e1 > e2) - (e1 < e2);
        if (sstart != 0 && sstart == - send &&
            DatumGetBool(tfunc_base_base(startvalue1, startvalue1,
              lfinfo)) == lfinfo->ever)
        {
          pfree_array((void **) tofree, nfree);
          return lfinfo->ever ? 1 : 0;
        }
      }
      /* Two planar point segments meet strictly inside where their meeting
       * is decided exactly (#tgeompointsegm_meet), whichever microsecond
       * dates it, and take one point there */
      else if (start1->temptype == T_TGEOMPOINT && ! starteq &&
          tgeompointsegm_meet(startvalue1, endvalue1, startvalue2, endvalue2))
      {
        if (DatumGetBool(tfunc_base_base(startvalue1, startvalue1, lfinfo)) ==
            lfinfo->ever)
        {
          pfree_array((void **) tofree, nfree);
          return lfinfo->ever ? 1 : 0;
        }
      }
      /* Two geography point segments meet strictly inside where the doubles
       * of their ends prove it (#tgeogpointsegm_meet), whichever microsecond
       * dates it, and take one point there */
      else if (start1->temptype == T_TGEOGPOINT && ! starteq &&
          tgeogpointsegm_meet(startvalue1, endvalue1, startvalue2, endvalue2,
            start1->t, end1->t))
      {
        if (DatumGetBool(tfunc_base_base(startvalue1, startvalue1, lfinfo)) ==
            lfinfo->ever)
        {
          pfree_array((void **) tofree, nfree);
          return lfinfo->ever ? 1 : 0;
        }
      }
      /* If either the start values differ or the end values are equal,
       * determine whether there is a crossing */
      else if (! starteq ||
          (endknown ? endeq : datum_eq(endvalue1, endvalue2, basetype)))
        cross = tsegment_intersection(startvalue1, endvalue1, startvalue2,
          endvalue2, start1->temptype, start1->t, end1->t, &tpt1, &tpt2);
    }
    /* If there is a crossing compute the function at it */
    if (cross)
    {
      /* The predicate may hold only strictly inside the turning-point
       * interval (t1, t2) -- for a within-distance predicate over a
       * trajectory crossing the values are within the band between t1 and
       * t2 but exactly on the band at the bounds, whose value rounded to
       * the microsecond can fall just outside. Evaluate at the midpoint,
       * where the predicate holds robustly, in addition to the bound t1. */
      TimestampTz tpts[2];
      int ntpts = 0;
      tpts[ntpts++] = tpt1;
      if (tpt2 != tpt1)
        tpts[ntpts++] = tpt1 + (tpt2 - tpt1) / 2;
      for (int k = 0; k < ntpts; k++)
      {
        /* A crossing at a bound of the segment is read where the sequences
         * take that bound, as above */
        if (tpts[k] <= start1->t || tpts[k] >= end1->t)
          continue;
        Datum tpvalue1 = tsegment_value_at_timestamptz(startvalue1, endvalue1,
          start1->temptype, start1->t, end1->t, tpts[k]);
        /* Two point segments meeting at the crossing construct two points
         * apart by their rounding where the meeting falls between two
         * microseconds; the crossing is a meeting their intersection
         * decides (#tgeompointsegm_meet, #tgeogpointsegm_intersection), so
         * the function reads the one point they meet at */
        Datum tpvalue2 =
          (! lfinfo->tpfn_temp && tpoint_type(start1->temptype)) ?
          datum_copy(tpvalue1, temptype_basetype(seq1->temptype)) :
          tsegment_value_at_timestamptz(startvalue2, endvalue2,
            start2->temptype, start2->t, end2->t, tpts[k]);
        res = DatumGetBool(tfunc_base_base(tpvalue1, tpvalue2, lfinfo));
        DATUM_FREE(tpvalue1, temptype_basetype(seq1->temptype));
        DATUM_FREE(tpvalue2, temptype_basetype(seq1->temptype));
        if ((lfinfo->ever && res) || (! lfinfo->ever && ! res))
        {
          pfree_array((void **) tofree, nfree);
          return lfinfo->ever ? 1 : 0;
        }
      }
    }
    start1 = end1; start2 = end2;
    lower_inc = true;
  }
  /* Add a final instant if any */
  if (inter->upper_inc)
  {
    res = DatumGetBool(tfunc_base_base(tinstant_value_p(start1),
      tinstant_value_p(start2), lfinfo));
    if ((lfinfo->ever && res) || (! lfinfo->ever && ! res))
    {
      pfree_array((void **) tofree, nfree);
      return lfinfo->ever ? 1 : 0;
    }
  }
  pfree_array((void **) tofree, nfree);
  return lfinfo->ever ? 0 : 1;
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @details This function is applied when both temporal values have step
 * interpolation. The other  possible case when one temporal value has step
 * interpolation and the other has linear interpolation is NOT YET IMPLEMENTED
 * since this case does not currently occur.
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @param[in] inter Overlapping period of the two sequences
 * @pre The sequences are both linear or both step
 */
static int
eafunc_tstepseq_tstepseq(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo, Span *inter)
{
  /*
   * General case
   * seq1 =  ...    *       *       *>
   * seq2 =    <*       *   *   * ...
   * result =  <S   S   S   *   S   S>
   * where S and * are values computed, respectively, at the synchronization
   * points and common points
   */
  assert(MEOS_FLAGS_GET_INTERP(seq1->flags) == STEP);
  assert(MEOS_FLAGS_GET_INTERP(seq2->flags) == STEP);
  TInstant *inst1 = (TInstant *) TSEQUENCE_INST_N(seq1, 0);
  TInstant *inst2 = (TInstant *) TSEQUENCE_INST_N(seq2, 0);
  TimestampTz lower = DatumGetTimestampTz(inter->lower);
  /* The last timestamp read, a whole number of microseconds: the sequences
   * take the values of their instants at the upper bound only when the
   * period includes it, and up to it the values read at the instants before,
   * whose first is the lower bound */
  TimestampTz upper = DatumGetTimestampTz(inter->upper) -
    (inter->upper_inc ? 0 : 1);
  int i = 0, j = 0, nfree = 0;
  if (inst1->t < lower)
  {
    i = tcontseq_find_timestamptz(seq1, inter->lower) + 1;
    inst1 = (TInstant *) TSEQUENCE_INST_N(seq1, i);
  }
  else if (inst2->t < lower)
  {
    j = tcontseq_find_timestamptz(seq2, inter->lower) + 1;
    inst2 = (TInstant *) TSEQUENCE_INST_N(seq2, j);
  }
  int count = (seq1->count - i + seq2->count - j) * 2;
  TInstant **tofree = palloc(sizeof(TInstant *) * count);
  bool res = false;
  while (i < seq1->count && j < seq2->count &&
    (inst1->t <= upper || inst2->t <= upper))
  {
    /* Synchronize the two start instants */
    int cmp = timestamptz_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
    {
      /* Hold seq2 at inst1->t on its current step segment, known from the
       * merge position [j-1, j], avoiding a per-instant binary search */
      i++;
      const TInstant *b1 = TSEQUENCE_INST_N(seq2, j - 1);
      inst2 = tsegment_at_timestamptz(b1, inst2, STEP, inst1->t);
      tofree[nfree++] = inst2;
    }
    else
    {
      /* Hold seq1 at inst2->t on its current step segment [i-1, i] */
      j++;
      const TInstant *a1 = TSEQUENCE_INST_N(seq1, i - 1);
      inst1 = tsegment_at_timestamptz(a1, inst1, STEP, inst2->t);
      tofree[nfree++] = inst1;
    }
    /* Compute the function on the synchronized instants */
    res = DatumGetBool(tfunc_base_base(tinstant_value_p(inst1),
      tinstant_value_p(inst2), lfinfo));
    /* Stop at the first instant that decides the quantifier, one where the
     * relationship holds for ever and one where it fails for always, as
     * #eafunc_tdiscseq_tdiscseq does */
    if (res == lfinfo->ever || i == seq1->count || j == seq2->count)
      break;
    inst1 = (TInstant *) TSEQUENCE_INST_N(seq1, i);
    inst2 = (TInstant *) TSEQUENCE_INST_N(seq2, j);
  }
  pfree_array((void **) tofree, nfree);
  if (lfinfo->ever && res)
    return 1;
  else if (! lfinfo->ever && ! res)
    return 0;
  return lfinfo->ever ? 0 : 1;
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * (dispatch function)
 * @note This function is called for each composing sequence of a temporal
 * sequence set and therefore the bounding period test is repeated
 */
static int
eafunc_tcontseq_tcontseq(const TSequence *seq1,
  const TSequence *seq2, LiftedFunctionInfo *lfinfo)
{
  assert(seq1); assert(seq2); assert(seq1->temptype == seq2->temptype);
  /* Test whether the bounding period of the two temporal values overlap */
  Span inter;
  if (! inter_span_span(&seq1->period, &seq2->period, &inter))
    return -1;

  /* If the two sequences intersect at an instant */
  if (inter.lower == inter.upper)
  {
    Datum value1, value2;
    tsequence_value_at_timestamptz(seq1, inter.lower, true, &value1);
    tsequence_value_at_timestamptz(seq2, inter.lower, true, &value2);
    int result = DatumGetBool(tfunc_base_base(value1, value2, lfinfo)) ? 1 : 0;
    DATUM_FREE(value1, temptype_basetype(seq1->temptype));
    DATUM_FREE(value2, temptype_basetype(seq2->temptype));
    return result;
  }

  /* If the two sequences have step interpolation */
  if (MEOS_FLAGS_GET_INTERP(seq1->flags) == STEP &&
      MEOS_FLAGS_GET_INTERP(seq2->flags) == STEP)
    return eafunc_tstepseq_tstepseq(seq1, seq2, lfinfo, &inter);

  /* General case */
  return eafunc_tcontseq_tcontseq_discfn(seq1, seq2, lfinfo, &inter);
}

/*****************************************************************************/

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] ss,seq Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
eafunc_tsequenceset_tcontseq(const TSequenceSet *ss, const TSequence *seq,
  LiftedFunctionInfo *lfinfo)
{
  assert(ss); assert(seq); assert(ss->temptype == seq->temptype);
  /* We need to verify that the sequences intersect in time in addition
   * to the bounding period test done in the dispatch function
   * #eafunc_temporal_temporal to return -1 if they do not intersect in time
   */
  SpanSet *s1 = tsequenceset_time(ss);
  SpanSet *s2 = tsequence_time(seq);
  bool found = overlaps_spanset_spanset(s1, s2);
  pfree(s1); pfree(s2);
  if (! found)
      return -1;

  TimestampTz upper = DatumGetTimestampTz(seq->period.upper);
  int loc;
  tsequenceset_find_timestamptz(ss, seq->period.lower, &loc);
  /* We are sure that loc < ss->count due to the bounding period test made
   * in the dispatch function */
  for (int i = loc; i < ss->count; i++)
  {
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss, i);
    int res = eafunc_tcontseq_tcontseq(seq1, seq, lfinfo);
    /* A piece sharing no time with the sequence (-1) decides neither
     * quantifier */
    if (lfinfo->ever && res == 1)
      return 1;
    else if (! lfinfo->ever && res == 0)
      return 0;
    int cmp = timestamptz_cmp_internal(upper,
      DatumGetTimestampTz(seq1->period.upper));
    if (cmp < 0 ||
      (cmp == 0 && (! seq->period.upper_inc || seq1->period.upper_inc)))
      break;
  }
  return lfinfo->ever ? 0 : 1;
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] seq,ss Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
eafunc_tcontseq_tsequenceset(const TSequence *seq, const TSequenceSet *ss,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return eafunc_tsequenceset_tcontseq(ss, seq, lfinfo);
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] ss1,ss2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
eafunc_tsequenceset_tsequenceset(const TSequenceSet *ss1,
  const TSequenceSet *ss2, LiftedFunctionInfo *lfinfo)
{
  assert(ss1); assert(ss2); assert(ss1->temptype == ss2->temptype);
  /* We need to verify that the sequence sets intersect in time in addition
   * to the bounding period test done in the dispatch function
   * #eafunc_temporal_temporal to return -1 if they do not intersect in time
   */
  SpanSet *s1 = tsequenceset_time(ss1);
  SpanSet *s2 = tsequenceset_time(ss2);
  bool found = overlaps_spanset_spanset(s1, s2);
  pfree(s1); pfree(s2);
  if (! found)
      return -1;

  int i = 0, j = 0;
  while (i < ss1->count && j < ss2->count)
  {
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss1, i);
    const TSequence *seq2 = TSEQUENCESET_SEQ_N(ss2, j);
    int res = eafunc_tcontseq_tcontseq(seq1, seq2, lfinfo);
    /* Two pieces sharing no time (-1) decide neither quantifier */
    if (lfinfo->ever && res == 1)
      return 1;
    else if (! lfinfo->ever && res == 0)
      return 0;
    int cmp = timestamptz_cmp_internal(DatumGetTimestampTz(seq1->period.upper),
      DatumGetTimestampTz(seq2->period.upper));
    if (cmp == 0)
    {
      if (! seq1->period.upper_inc && seq2->period.upper_inc)
        cmp = -1;
      else if (seq1->period.upper_inc && ! seq2->period.upper_inc)
        cmp = 1;
    }
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
      i++;
    else
      j++;
  }
  return lfinfo->ever ? 0 : 1;
}

/*****************************************************************************/

/**
 * @brief Apply a lifted ever or always function to two temporal values
 * @note The error number is read by #eafunc_temporal_temporal
 */
static int
eafunc_temporal_temporal_dispatch(const Temporal *temp1, const Temporal *temp2,
  LiftedFunctionInfo *lfinfo)
{
  assert(temp1); assert(temp2); assert(temp1->temptype == temp2->temptype);
  /* Bounding box test */
  Span s1, s2;
  temporal_set_tstzspan(temp1, &s1);
  temporal_set_tstzspan(temp2, &s2);
  if (! overlaps_span_span(&s1, &s2))
    return -1;

  assert(temptype_subtype(temp1->subtype));
  assert(temptype_subtype(temp2->subtype));
  switch (temp1->subtype)
  {
    case TINSTANT:
    {
      switch (temp2->subtype)
      {
        case TINSTANT:
          return eafunc_tinstant_tinstant((TInstant *) temp1,
            (TInstant *) temp2, lfinfo);
        case TSEQUENCE:
          return MEOS_FLAGS_DISCRETE_INTERP(temp2->flags) ?
            eafunc_tinstant_tdiscseq((TInstant *) temp1,
              (TSequence *) temp2, lfinfo) :
            eafunc_tinstant_tcontseq((TInstant *) temp1,
              (TSequence *) temp2, lfinfo);
        default: /* TSEQUENCESET */
          return eafunc_tinstant_tsequenceset((TInstant *) temp1,
            (TSequenceSet *) temp2, lfinfo);
      }
    }
    case TSEQUENCE:
    {
      switch (temp2->subtype)
      {
        case TINSTANT:
          return  MEOS_FLAGS_DISCRETE_INTERP(temp1->flags) ?
            eafunc_tdiscseq_tinstant((TSequence *) temp1,
              (TInstant *) temp2, lfinfo) :
            eafunc_tcontseq_tinstant((TSequence *) temp1,
              (TInstant *) temp2, lfinfo);
        case TSEQUENCE:
        {
          interpType interp1 = MEOS_FLAGS_GET_INTERP(temp1->flags);
          interpType interp2 = MEOS_FLAGS_GET_INTERP(temp2->flags);
          if (interp1 == DISCRETE)
          {
            if (interp2 == DISCRETE )
              return eafunc_tdiscseq_tdiscseq((TSequence *) temp1,
                (TSequence *) temp2, lfinfo);
            else /* interp2 != DISCRETE */
              return eafunc_tdiscseq_tcontseq((TSequence *) temp1,
                (TSequence *) temp2, lfinfo);
          }
          else /* interp1 != DISCRETE */
          {
            if (interp2 == DISCRETE)
              return eafunc_tcontseq_tdiscseq((TSequence *) temp1,
                (TSequence *) temp2, lfinfo);
            else /* interp2 != DISCRETE */
              return eafunc_tcontseq_tcontseq((TSequence *) temp1,
                (TSequence *) temp2, lfinfo);
          }
        }
        default: /* TSEQUENCESET */
          return MEOS_FLAGS_DISCRETE_INTERP(temp1->flags) ?
            eafunc_tdiscseq_tsequenceset((TSequence *) temp1,
              (TSequenceSet *) temp2, lfinfo) :
            eafunc_tcontseq_tsequenceset((TSequence *) temp1,
              (TSequenceSet *) temp2, lfinfo);
      }
    }
    default: /* TSEQUENCESET */
    {
      switch (temp2->subtype)
      {
        case TINSTANT:
          return eafunc_tsequenceset_tinstant((TSequenceSet *) temp1,
            (TInstant *) temp2, lfinfo);
        case TSEQUENCE:
          return MEOS_FLAGS_DISCRETE_INTERP(temp2->flags) ?
            eafunc_tsequenceset_tdiscseq((TSequenceSet *) temp1,
              (TSequence *) temp2, lfinfo) :
            eafunc_tsequenceset_tcontseq((TSequenceSet *) temp1,
              (TSequence *) temp2, lfinfo);
        default: /* TSEQUENCESET */
          return eafunc_tsequenceset_tsequenceset((TSequenceSet *) temp1,
            (TSequenceSet *) temp2, lfinfo);
      }
    }
  }
}

/**
 * @brief Synchronize two temporal values and apply to them a lifted function
 * @param[in] temp1,temp2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @note An error raised by the function applied makes the answer the
 * failure value, see the note on errors at the beginning of the file
 */
int
eafunc_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
  LiftedFunctionInfo *lfinfo)
{
  int last_errno = meos_errno_reset();
  return lifting_result_int(
    eafunc_temporal_temporal_dispatch(temp1, temp2, lfinfo), last_errno);
}

/*****************************************************************************
 * The following functions are only allowed for JSON since currently it is
 * the only base type that can produce null values on input
 *****************************************************************************/
#if JSON
/**
 * @brief Apply a lifted function to a temporal sequence with discrete
 * interpolation
 * @param[in] seq Temporal value
 * @param[in] lfinfo Information about the lifted function
 */
TSequence *
tfunc_null_tdiscseq(const TSequence *seq, LiftedFunctionInfo *lfinfo)
{
  assert(seq); assert(lfinfo);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);

  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int count = 0;
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = tfunc_tinstant(TSEQUENCE_INST_N(seq, i), lfinfo);
    if (inst)
      instants[count++] = inst;
    else
    {
      if (lfinfo->resnull == NULL_DELETE)
      {
        /* This option is ONLY allowed for STEP interpolation */
        assert(MEOS_FLAGS_GET_INTERP(seq->flags) == STEP);
        /* Nothing to do, ignore the instant */
      }
      else
      {
        if (lfinfo->resnull == NULL_ERROR)
        {
          meos_error(ERROR, MEOS_ERR_NULL_RESULT,
            "The lifted operation returned NULL");
        }
        /* For both NULL_ERROR and NULL_RETURN */
        for (int j = 0; j < count; j++)
          pfree(instants[j]);
        pfree(instants);
        return NULL;
      }
    }
  }
  if (! count)
    return NULL;
  return lifting_tsequence_make_free(instants, count, true, true, DISCRETE,
    NORMALIZE);
}

/**
 * @brief Apply a lifted function to a temporal sequence with continuous
 * interpolation (iterator function)
 * @param[in] seq Temporal sequence
 * @param[in] lfinfo Information about the lifted function
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set
 */
int
tfunc_null_tcontseq_iter(const TSequence *seq, LiftedFunctionInfo *lfinfo,
  TSequence **result)
{
  assert(seq); assert(lfinfo); assert(result);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);

  /* General case */
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int ninsts = 0, nseqs = 0;
  /* The interpolation of the result is the one the lifted function declares,
   * as in #tfunc_tcontseq_tcontseq_discfn, so that a value extracted from a
   * temporal type with step interpolation may be interpolated linearly */
  interpType interp = lfinfo->reslinear ? LINEAR : STEP;
  bool lower_inc = seq->period.lower_inc, upper_inc = seq->period.upper_inc;
  for (int i = 0; i < seq->count; i++)
  {
    upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    TInstant *inst1 = tfunc_tinstant(inst, lfinfo);
    if (inst1)
      instants[ninsts++] = inst1;
    else
    {
      if (lfinfo->resnull == NULL_DELETE)
      {
        /* This option is ONLY allowed for STEP interpolation */
        assert(MEOS_FLAGS_GET_INTERP(seq->flags) == STEP);
        /* Close the current sequence and start a new one. With step
         * interpolation the last value holds until the missing one, with
         * linear interpolation the sequence ends at the last value */
        if (ninsts > 0)
        {
          if (interp == STEP)
          {
            inst1 = instants[ninsts - 1];
            instants[ninsts++] = tinstant_make(tinstant_value_p(inst1),
              lfinfo->restype, inst->t);
            upper_inc = false;
          }
          else
          {
            upper_inc = true;
            if (ninsts == 1)
              lower_inc = true;
          }
          result[nseqs++] = lifting_tsequence_make(instants, ninsts, lower_inc,
            upper_inc, interp, NORMALIZE);
          ninsts = 0;
        }
      }
      else 
      {
        if (lfinfo->resnull == NULL_ERROR)
        {
          meos_error(ERROR, MEOS_ERR_NULL_RESULT,
            "The lifted operation returned NULL");
        }
        /* For both NULL_ERROR and NULL_RETURN */
        for (int j = 0; j < ninsts; j++)
          pfree(instants[j]);
        pfree(instants);
        for (int j = 0; j < nseqs; j++)
          pfree(result[j]);
        return 0;
      }
    }
    lower_inc = true;
  }
  if (ninsts > 0)
  {
    /* If instantaneous sequence */
    if (ninsts == 1)
    {
      lower_inc = upper_inc = true;
    }
    result[nseqs++] = lifting_tsequence_make_free(instants, ninsts, lower_inc,
      upper_inc, interp, NORMALIZE);
  }
  else
    pfree(instants);
  return nseqs;
}

/**
 * @brief Apply a lifted function to a temporal sequence with continuous
 * interpolation
 * @param[in] seq Temporal sequence
 * @param[in] lfinfo Information about the lifted function
 */
TSequenceSet *
tfunc_null_tcontseq(const TSequence *seq, LiftedFunctionInfo *lfinfo)
{
  assert(seq); assert(lfinfo);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);

  /* For step interpolation and NULL_DELETE we need the half of the count */
  int maxseqs = 1;
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == STEP &&
      lfinfo->resnull == NULL_DELETE)
    maxseqs = (int) ceil((double) seq->count / 2.0);
  TSequence **sequences = palloc(sizeof(TSequence *) * maxseqs);
  int count = tfunc_null_tcontseq_iter(seq, lfinfo, sequences);
  return lifting_tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @brief Apply a lifted function to a temporal sequence
 * @param[in] seq Temporal value
 * @param[in] lfinfo Information about the lifted function
 */
Temporal *
tfunc_null_tsequence(const TSequence *seq, LiftedFunctionInfo *lfinfo)
{
  assert(seq); assert(lfinfo); assert(lfinfo->resnull);
  /* The function may return NULL */
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
    return (Temporal *) tfunc_null_tdiscseq(seq, lfinfo);
  TSequenceSet *ss = tfunc_null_tcontseq(seq, lfinfo);
  /* A sequence none of whose instants is left out answers a sequence, as
   * #tfunc_tsequence does; one with instants left out answers a sequence
   * set, as a restriction of a sequence does */
  if (! ss || ss->count > 1 || ! span_eq(&ss->period, &seq->period))
    return (Temporal *) ss;
  TSequence *result = tsequence_copy(TSEQUENCESET_SEQ_N(ss, 0));
  pfree(ss);
  return (Temporal *) result;
}

/**
 * @brief Apply a lifted function to a temporal sequence set
 * @param[in] ss Temporal value
 * @param[in] lfinfo Information about the lifted function
 */
TSequenceSet *
tfunc_null_tsequenceset(const TSequenceSet *ss, LiftedFunctionInfo *lfinfo)
{
  assert(ss); assert(lfinfo);
  /* Singleton sequence set */
  if (ss->count == 1)
    return tfunc_null_tcontseq(TSEQUENCESET_SEQ_N(ss, 0), lfinfo);

  /* For step interpolation and NULL_DELETE we need the half of the count */
  int maxseqs = ss->totalcount;
  if (MEOS_FLAGS_GET_INTERP(ss->flags) == STEP &&
      lfinfo->resnull == NULL_DELETE)
    maxseqs = (int) ceil((double) ss->totalcount / 2.0);
  TSequence **sequences = palloc(sizeof(TSequence *) * maxseqs);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    int count = tfunc_null_tcontseq_iter(TSEQUENCESET_SEQ_N(ss, i), lfinfo,
      &sequences[nseqs]);
    if (count)
      nseqs += count;
    else
    {
      if (lfinfo->resnull == NULL_DELETE)
      {
        /* This option is ONLY allowed for STEP interpolation */
        assert(MEOS_FLAGS_GET_INTERP(ss->flags) == STEP);
        /* Ignore the current sequence and continue with the next one */
      }
      else
      {
        if (lfinfo->resnull == NULL_ERROR)
        {
          meos_error(ERROR, MEOS_ERR_NULL_RESULT,
            "The lifted operation returned NULL");
        }
        /* For both NULL_ERROR and NULL_RETURN */
        for (int j = 0; j < nseqs; j++)
          pfree(sequences[j]);
        pfree(sequences);
        return NULL;
      }
    }
  }
  return lifting_tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

#endif /* JSON */

/*****************************************************************************/
