/*****************************************************************************
 *
 * tbool_boolops.c
 *    Temporal Boolean operators (and, or, not).
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *     Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tbool_boolops.h"

#include "temporaltypes.h"
#include "lifting.h"

/*****************************************************************************
 * Boolean operations functions on datums
 *****************************************************************************/

/**
 * Returns the Boolean and of the two values
 */
Datum
datum_and(Datum l, Datum r)
{
  return BoolGetDatum(DatumGetBool(l) && DatumGetBool(r));
}


/**
 * Returns the Boolean or of the two values
 */
Datum
datum_or(Datum l, Datum r)
{
  return BoolGetDatum(DatumGetBool(l) || DatumGetBool(r));
}

/*****************************************************************************
 * Generic functions 
 *****************************************************************************/

Temporal *
boolop_tbool_bool(Temporal *temp, Datum b, Datum (*func)(Datum, Datum),
  bool invert)
{
  LiftedFunctionInfo lfinfo;
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 2;
  lfinfo.restypid = BOOLOID;
  lfinfo.reslinear = STEP;
  lfinfo.invert = invert;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal_base(temp, b, BOOLOID, (Datum) NULL, lfinfo);
}

Temporal *
boolop_tbool_tbool(Temporal *temp1, Temporal *temp2,
  Datum (*func)(Datum, Datum))
{
  LiftedFunctionInfo lfinfo;
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 2;
  lfinfo.restypid = BOOLOID;
  lfinfo.reslinear = STEP;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc = NULL;
  return sync_tfunc_temporal_temporal(temp1, temp2, (Datum) NULL, lfinfo);
}

/*****************************************************************************
 * Temporal and
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tand_bool_tbool);
/**
 * Returns the temporal boolean and of the value and the temporal value
 */
PGDLLEXPORT Datum
tand_bool_tbool(PG_FUNCTION_ARGS)
{
  Datum b = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Temporal *result = boolop_tbool_bool(temp, b, &datum_and, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tand_tbool_bool);
/**
 * Returns the temporal boolean and of the temporal value and the value
 */
PGDLLEXPORT Datum
tand_tbool_bool(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum b = PG_GETARG_DATUM(1);
  Temporal *result = boolop_tbool_bool(temp, b, &datum_and, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tand_tbool_tbool);
/**
 * Returns the temporal boolean and of the temporal values
 */
PGDLLEXPORT Datum
tand_tbool_tbool(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  Temporal *result = boolop_tbool_tbool(temp1, temp2, &datum_and);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal or
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tor_bool_tbool);
/**
 * Returns the temporal boolean or of the value and the temporal value
 */
PGDLLEXPORT Datum
tor_bool_tbool(PG_FUNCTION_ARGS)
{
  Datum b = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Temporal *result = boolop_tbool_bool(temp, b, &datum_or, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tor_tbool_bool);
/**
 * Returns the temporal boolean or of the temporal value and the value
 */
PGDLLEXPORT Datum
tor_tbool_bool(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum b = PG_GETARG_DATUM(1);
  Temporal *result = boolop_tbool_bool(temp, b, &datum_or, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tor_tbool_tbool);
/**
 * Returns the temporal boolean or of the temporal values
 */
PGDLLEXPORT Datum
tor_tbool_tbool(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  Temporal *result = boolop_tbool_tbool(temp1, temp2, &datum_or);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal not
 *****************************************************************************/

/**
 * Returns the temporal boolean not of the temporal value
 */
static TInstant *
tnot_tboolinst(const TInstant *inst)
{
  TInstant *result = tinstant_copy(inst);
  Datum *value_ptr = tinstant_value_ptr(result);
  *value_ptr = BoolGetDatum(!DatumGetBool(tinstant_value(inst)));
  return result;
}

/**
 * Returns the temporal boolean not of the temporal value
 */
static TInstantSet *
tnot_tbooli(const TInstantSet *ti)
{
  TInstantSet *result = tinstantset_copy(ti);
  for (int i = 0; i < ti->count; i++)
  {
    TInstant *inst = tinstantset_inst_n(result, i);
    Datum *value_ptr = tinstant_value_ptr(inst);
    *value_ptr = BoolGetDatum(!DatumGetBool(tinstant_value(inst)));
  }
  return result;

}

/**
 * Returns the temporal boolean not of the temporal value
 */
static TSequence *
tnot_tboolseq(const TSequence *seq)
{
  TSequence *result = tsequence_copy(seq);
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = tsequence_inst_n(result, i);
    Datum *value_ptr = tinstant_value_ptr(inst);
    *value_ptr = BoolGetDatum(!DatumGetBool(tinstant_value(inst)));
  }
  return result;
}

/**
 * Returns the temporal boolean not of the temporal value
 */
static TSequenceSet *
tnot_tbools(const TSequenceSet *ts)
{
  TSequenceSet *result = tsequenceset_copy(ts);
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(result, i);
    for (int j = 0; j < seq->count; j++)
    {
      TInstant *inst = tsequence_inst_n(seq, j);
      Datum *value_ptr = tinstant_value_ptr(inst);
      *value_ptr = BoolGetDatum(!DatumGetBool(tinstant_value(inst)));
    }
  }
  return result;
}

/**
 * Returns the temporal boolean not of the temporal value
 * (dispatch function)
 */
Temporal *
tnot_tbool_internal(const Temporal *temp)
{
  Temporal *result;
  ensure_valid_duration(temp->duration);
  if (temp->duration == INSTANT)
    result = (Temporal *)tnot_tboolinst((TInstant *)temp);
  else if (temp->duration == INSTANTSET)
    result = (Temporal *)tnot_tbooli((TInstantSet *)temp);
  else if (temp->duration == SEQUENCE)
    result = (Temporal *)tnot_tboolseq((TSequence *)temp);
  else /* temp->duration == SEQUENCESET */
    result = (Temporal *)tnot_tbools((TSequenceSet *)temp);
  return result;
}

PG_FUNCTION_INFO_V1(tnot_tbool);
/**
 * Returns the temporal boolean not of the temporal value
 */
PGDLLEXPORT Datum
tnot_tbool(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Temporal *result = tnot_tbool_internal(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

