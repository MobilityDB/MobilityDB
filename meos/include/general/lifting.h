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
 * @brief Generic functions for lifting functions and operators on temporal
 * types.
 */

#ifndef __LIFTING_H__
#define __LIFTING_H__

/* MobilityDB */
#include "general/temporal.h"

/**
 * Structure to represent the information about lifted functions
 *
 * The mandatory parameters are `func`, `numparam`, and `restype`. These
 * parameters are used by function `tfunc_temporal`, which applies the lifted
 * function to every instant of the temporal value. The remaining parameters
 * are used by functions `tfunc_temporal_base` and `tfunc_temporal_temporal`
 * that apply the lifted function to two base values.
 */

#define MAX_PARAMS 3

typedef struct
{
  Datum (*func)(Datum, ...); /**< Variadic function that is lifted */
  int numparam;              /**< Number of parameters of the function */
  Datum param[MAX_PARAMS];   /**< Datum array for the parameters of the function */
  bool args;                 /**< True if the lifted function requires arguments */
  mobdbType argtype[2];      /**< Base type of the arguments */
  mobdbType restype;         /**< Temporal type of the result of the function */
  bool reslinear;            /**< True if the result has linear interpolation */
  bool invert;               /**< True if the arguments of the function must be inverted */
  bool discont;              /**< True if the function has instantaneous discontinuities */
  bool (*tpfunc_base)(const TInstant *, const TInstant *, Datum, mobdbType,
    Datum *, TimestampTz *); /**< Turning point function for temporal and base types*/
  bool (*tpfunc)(const TInstant *, const TInstant *, const TInstant *,
    const TInstant *, Datum *,
    TimestampTz *);          /**< Turning point function for two temporal types */
} LiftedFunctionInfo;

/*****************************************************************************/

extern TInstant *tfunc_tinstant(const TInstant *inst,
  LiftedFunctionInfo *lfinfo);
extern TSequence *tfunc_tdiscseq(const TSequence *is,
  LiftedFunctionInfo *lfinfo);
extern TSequence *tfunc_tsequence(const TSequence *seq,
  LiftedFunctionInfo *lfinfo);
extern TSequenceSet *tfunc_tsequenceset(const TSequenceSet *ss,
  LiftedFunctionInfo *lfinfo);
extern Temporal *tfunc_temporal(const Temporal *temp,
  LiftedFunctionInfo *lfinfo);

extern TInstant *tfunc_tinstant_base(const TInstant *inst, Datum value,
  LiftedFunctionInfo *lfinfo);
extern TSequence *tfunc_tsequence_base(const TSequence *seq, Datum value,
  LiftedFunctionInfo *lfinfo);
extern TSequenceSet *tfunc_tsequenceset_base(const TSequenceSet *ss, Datum value,
  LiftedFunctionInfo *lfinfo);
extern Temporal *tfunc_temporal_base(const Temporal *temp, Datum value,
  LiftedFunctionInfo *lfinfo);

extern TInstant *tfunc_tinstant_tinstant(const TInstant *inst1,
  const TInstant *inst2, LiftedFunctionInfo *lfinfo);
extern TSequence *tfunc_tdiscseq_tdiscseq(const TSequence *is1,
  const TSequence *is2, LiftedFunctionInfo *lfinfo);
extern Temporal *tfunc_temporal_temporal(const Temporal *temp1,
  const Temporal *temp2, LiftedFunctionInfo *lfinfo);

/*****************************************************************************/

extern int efunc_temporal_temporal(const Temporal *temp1,
  const Temporal *temp2, LiftedFunctionInfo *lfinfo);

/*****************************************************************************/

#endif
