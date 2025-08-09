
/**
 * @file
 * @brief Generic lifting functions for temporal JSONB
 */

#include "temporal/tjsonb_funcs.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/lifting.h"

/*****************************************************************************
 * Generic functions on temporal JSONB
 *****************************************************************************/


/**
 * @brief Apply a unary JSONB→JSONB function to each instant of a T_TJSONB.
 */
Temporal *
jsonbfunc_tjsonb(const Temporal *temp, datum_func1 func)
{
  /* Ensure arguments are valid */
  assert(temp);
  assert(func);
  assert(temp->temptype == T_TJSONB);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func      = (varfunc) func;
  lfinfo.numparam  = 0;
  lfinfo.argtype[0] = T_TJSONB;
  lfinfo.restype   = T_TJSONB;
  return tfunc_temporal(temp, &lfinfo);
}



/**
 * @brief Apply a binary JSONB→JSONB function between each instant of a
 *        T_TJSONB and a constant JSONB.
 */
Temporal *
jsonbfunc_tjsonb_jsonb(const Temporal *temp,
                       Datum value,
                       datum_func2 func,
                       bool invert)
{
  /* Ensure arguments are valid */
  assert(temp);
  assert(temp->temptype == T_TJSONB);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func      = (varfunc) func;
  lfinfo.numparam  = 0;
  lfinfo.argtype[0] = T_TJSONB;
  lfinfo.argtype[1] = T_JSONB;
  lfinfo.restype   = T_TJSONB;
  lfinfo.reslinear = false;    /* JSONB ops are discontinuous */
  lfinfo.invert    = invert;
  lfinfo.discont   = CONTINUOUS;
  return tfunc_temporal_base(temp, value, &lfinfo);
}



/**
 * @brief Apply a binary JSONB→JSONB function instant-by-instant between two
 *        T_TJSONB values.
 */
Temporal *
jsonbfunc_tjsonb_tjsonb(const Temporal *temp1,
                        const Temporal *temp2,
                        datum_func2 func)
{
  /* Ensure arguments are valid */
  assert(temp1); assert(temp2);
  assert(temp1->temptype == temp2->temptype);
  assert(temp1->temptype == T_TJSONB);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func        = (varfunc) func;
  lfinfo.numparam    = 0;
  lfinfo.argtype[0]  = lfinfo.argtype[1] = T_TJSONB;
  lfinfo.restype     = T_TJSONB;
  lfinfo.reslinear   = false;
  lfinfo.invert      = INVERT_NO;
  lfinfo.discont     = CONTINUOUS;
  return tfunc_temporal_temporal(temp1, temp2, &lfinfo);
}

/*****************************************************************************/
