
#ifndef __TJSONB_JSONBFUNCS_H__
#define __TJSONB_JSONBFUNCS_H__



/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include "temporal/temporal.h"




/*----------------------------------------------------------------------------
 * Datum‐level JSONB operations
 *---------------------------------------------------------------------------*/

/** Concatenate two JSONB values (objects or arrays). */
extern Datum datum_jsonb_concat(Datum left, Datum right);


/*----------------------------------------------------------------------------
 * Temporal wrappers for JSONB operations
 *---------------------------------------------------------------------------*/

// Apply a unary JSONB→JSONB function to each instant of a T_TJSONB.
extern Temporal *jsonbfunc_tjsonb(const Temporal *temp, datum_func1 func);

 // Apply a binary JSONB→JSONB function between each instant and a constant.
extern Temporal *jsonbfunc_tjsonb_jsonb(const Temporal *temp,Datum value,datum_func2 func,bool invert);

 //Apply a binary JSONB→JSONB function instant‐by‐instant between two T_TJSONB.
extern Temporal *jsonbfunc_tjsonb_tjsonb(const Temporal *temp1, const Temporal *temp2, datum_func2 func);

#endif 
