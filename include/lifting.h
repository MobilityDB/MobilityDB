/*****************************************************************************
 *
 * lifting.c
 *  Generic functions for lifting functions and operators on temporal types.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *    Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __LIFTING_H__
#define __LIFTING_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include "temporal.h"

/**
 * Structure to represent the information about lifted functions
 */
typedef struct 
{
  Datum (*func)(Datum, ...); /**< Variadic function that is lifted*/
  int numparam;              /**< Number of parameters of the function */
  Oid restypid;              /**< Oid of the result type of function */
  bool reslinear;            /**< True if the resulting value has linear interpolation */
  bool discont;              /**< True if the function has instantaneaous discontinuities */
  bool invert;               /**< True if the arguments of the function must be inverted */
  bool (*tpfunc)(const TInstant *, const TInstant *, const TInstant *,
     const TInstant *, TimestampTz *);    /**< Turning point function */
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
extern TSequence *
tfunc_tsequence_base(const TSequence *seq, Datum value, Oid valuetypid, Datum param,
  LiftedFunctionInfo lfinfo);
extern TSequenceSet *
tfunc_tsequenceset_base(const TSequenceSet *ts, Datum value, Oid valuetypid, Datum param,
  LiftedFunctionInfo lfinfo);
extern Temporal *
tfunc_temporal_base(const Temporal *temp, Datum value, Oid valuetypid, Datum param,
  LiftedFunctionInfo lfinfo);

extern TSequenceSet *
tfunc4_tsequence_base_discont(const TSequence *seq, Datum value, Oid valuetypid,
  LiftedFunctionInfo lfinfo);
extern TSequenceSet *
tfunc4_tsequenceset_base_discont(const TSequenceSet *ts, Datum value, Oid valuetypid,
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
