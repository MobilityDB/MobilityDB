/*****************************************************************************
 *
 * lifting.c
 *  Generic functions for lifting functions and operators on temporal types.
 *
 * Copyright (c) 2020, Université libre de Bruxelles and MobilityDB contributors
 *
 * Permission to use, copy, modify, and distribute this software and its documentation for any purpose, without fee, and without a written agreement is hereby
 * granted, provided that the above copyright notice and this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
 * PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO PROVIDE
 * MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

#ifndef __LIFTING_H__
#define __LIFTING_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include "temporal.h"

/**
 * Structure to represent the information about lifted functions
 *
 * The mandatory parameters are `func`, `numparam`, and `restypid`. These
 * parameters are used by function `tfunc_temporal`, which applies the lifted
 * function to every instant of the temporal value. The remaining parameters
 * are used by functions `tfunc_temporal_base` and `sync_tfunc_temporal_temporal`
 * that apply the lifted function to two base values.
 */
typedef struct
{
  Datum (*func)(Datum, ...); /**< Variadic function that is lifted */
  int numparam;              /**< Number of parameters of the function */
  Oid restypid;              /**< Base type of the result of the function */
  bool reslinear;            /**< True if the result has linear interpolation */
  bool invert;               /**< True if the arguments of the function must be inverted */
  bool discont;              /**< True if the function has instantaneaous discontinuities */
  bool (*tpfunc)(const TInstant *, const TInstant *, bool, const TInstant *,
     const TInstant *, bool, TimestampTz *);    /**< Turning point function */
} LiftedFunctionInfo;

/*****************************************************************************/

extern TInstant *tfunc_tinstant(const TInstant *inst, Datum param,
  LiftedFunctionInfo lfinfo);
extern TInstantSet *tfunc_tinstantset(const TInstantSet *ti, Datum param,
  LiftedFunctionInfo lfinfo);
extern TSequence *tfunc_tsequence(const TSequence *seq, Datum param,
  LiftedFunctionInfo lfinfo);
extern TSequenceSet *tfunc_tsequenceset(const TSequenceSet *ts, Datum param,
  LiftedFunctionInfo lfinfo);
extern Temporal *tfunc_temporal(const Temporal *temp, Datum param,
  LiftedFunctionInfo lfinfo);

extern TInstant *
tfunc_tinstant_base(const TInstant *inst, Datum value, Oid valuetypid, Datum param,
  LiftedFunctionInfo lfinfo);
extern TInstantSet *
tfunc_tinstantset_base(const TInstantSet *ti, Datum value, Oid valuetypid, Datum param,
  LiftedFunctionInfo lfinfo);
extern Temporal *
tfunc_tsequence_base(const TSequence *seq, Datum value, Oid valuetypid, Datum param,
  LiftedFunctionInfo lfinfo);
extern TSequenceSet *
tfunc_tsequenceset_base(const TSequenceSet *ts, Datum value, Oid valuetypid, Datum param,
  LiftedFunctionInfo lfinfo);
extern Temporal *
tfunc_temporal_base(const Temporal *temp, Datum value, Oid valuetypid, Datum param,
  LiftedFunctionInfo lfinfo);

extern TInstant *
sync_tfunc_tinstant_tinstant(const TInstant *inst1, const TInstant *inst2,
  Datum param, LiftedFunctionInfo lfinfo);
extern TInstantSet *
sync_tfunc_tinstantset_tinstantset(const TInstantSet *ti1, const TInstantSet *ti2,
  Datum param, LiftedFunctionInfo lfinfo);
extern Temporal *
sync_tfunc_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
  Datum param, LiftedFunctionInfo lfinfo);

/*****************************************************************************/

#endif
