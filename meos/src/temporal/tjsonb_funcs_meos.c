

/**
 * @file
 * @brief Temporal JSONB functions
 */

#include "temporal/tjsonb_funcs.h"
#include "utils/jsonb.h"

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "utils/array.h"

/*****************************************************************************
 * JSONB concat / set 
 *****************************************************************************/



/**
 * @ingroup meos_temporal_jsonb
 * @brief Return the concatenation of a JSONB and a temporal JSONB
 * @param[in] jb   JSONB constant
 * @param[in] temp Temporal JSONB value
 * @csqlfn #Jsonb_concat_jsonb_tjsonb()
 */
Temporal *
jsonb_concat_jsonb_tjsonb(const Jsonb *jb, const Temporal *temp)
{
  VALIDATE_TJSONB(temp, NULL);
  VALIDATE_NOT_NULL(jb, NULL);
  /* concat: func( const, instant ) */
  return jsonbfunc_tjsonb_jsonb(temp, PointerGetDatum(jb),
    &datum_jsonb_concat, INVERT);
}




/**
 * @ingroup meos_temporal_jsonb
 * @brief Return the concatenation of a temporal JSONB and a JSONB
 * @param[in] temp Temporal JSONB value
 * @param[in] jb   JSONB constant
 * @csqlfn #Jsonb_concat_tjsonb_jsonb()
 */
Temporal *
jsonb_concat_tjsonb_jsonb(const Temporal *temp, const Jsonb *jb)
{
  VALIDATE_TJSONB(temp, NULL);
  VALIDATE_NOT_NULL(jb, NULL);
  /* concat: func( instant, const ) */
  return jsonbfunc_tjsonb_jsonb(temp, PointerGetDatum(jb),
    &datum_jsonb_concat, INVERT_NO);
}




/**
 * @ingroup meos_temporal_jsonb
 * @brief Return the concatenation of two temporal JSONB values
 * @param[in] temp1, temp2 Temporal JSONB values
 * @csqlfn #Jsonb_concat_tjsonb_tjsonb()
 */
Temporal *
jsonb_concat_tjsonb_tjsonb(const Temporal *temp1, const Temporal *temp2)
{
  VALIDATE_TJSONB(temp1, NULL); VALIDATE_TJSONB(temp2, NULL);
  return jsonbfunc_tjsonb_tjsonb(temp1, temp2, &datum_jsonb_concat);
}








