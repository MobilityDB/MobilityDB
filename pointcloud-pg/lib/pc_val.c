/***********************************************************************
 * pc_val.c
 *
 *  Pointclound value handling. Create, get and set values.
 *
 *  PgSQL Pointcloud is free and open source software provided
 *  by the Government of Canada
 *  Copyright (c) 2013 Natural Resources Canada
 *
 ***********************************************************************/

#include "pc_api_internal.h"
#include <math.h>
#include <stdint.h>

double pc_value_unscale_unoffset(double val, const PCDIMENSION *dim)
{
  /* Offset value */
  if (dim->offset)
    val -= dim->offset;

  /* Scale value */
  if (dim->scale != 1)
    val /= dim->scale;

  return val;
}

double pc_value_scale_offset(double val, const PCDIMENSION *dim)
{
  /* Scale value */
  if (dim->scale != 1)
    val *= dim->scale;

  /* Offset value */
  if (dim->offset)
    val += dim->offset;

  return val;
}

double pc_value_from_ptr(const uint8_t *ptr, const PCDIMENSION *dim)
{
  double val = pc_double_from_ptr(ptr, dim->interpretation);
  return pc_value_scale_offset(val, dim);
}

double pc_double_from_ptr(const uint8_t *ptr, uint32_t interpretation)
{
  switch (interpretation)
  {
  case PC_UINT8:
  {
    uint8_t v;
    memcpy(&(v), ptr, sizeof(uint8_t));
    return (double)v;
  }
  case PC_UINT16:
  {
    uint16_t v;
    memcpy(&(v), ptr, sizeof(uint16_t));
    return (double)v;
  }
  case PC_UINT32:
  {
    uint32_t v;
    memcpy(&(v), ptr, sizeof(uint32_t));
    return (double)v;
  }
  case PC_UINT64:
  {
    uint64_t v;
    memcpy(&(v), ptr, sizeof(uint64_t));
    return (double)v;
  }
  case PC_INT8:
  {
    int8_t v;
    memcpy(&(v), ptr, sizeof(int8_t));
    return (double)v;
  }
  case PC_INT16:
  {
    int16_t v;
    memcpy(&(v), ptr, sizeof(int16_t));
    return (double)v;
  }
  case PC_INT32:
  {
    int32_t v;
    memcpy(&(v), ptr, sizeof(int32_t));
    return (double)v;
  }
  case PC_INT64:
  {
    int64_t v;
    memcpy(&(v), ptr, sizeof(int64_t));
    return (double)v;
  }
  case PC_FLOAT:
  {
    float v;
    memcpy(&(v), ptr, sizeof(float));
    return (double)v;
  }
  case PC_DOUBLE:
  {
    double v;
    memcpy(&(v), ptr, sizeof(double));
    return v;
  }
  default:
  {
    pcerror("unknown interpretation type %d encountered in pc_double_from_ptr",
            interpretation);
  }
  }
  return 0.0;
}

#define CLAMP(v, min, max, t, format)                                          \
  do                                                                           \
  {                                                                            \
    if (v > max)                                                               \
    {                                                                          \
      pcwarn("Value %g truncated to " format " to fit in " t, v, max);         \
      v = max;                                                                 \
    }                                                                          \
    else if (v < min)                                                          \
    {                                                                          \
      pcwarn("Value %g truncated to " format " to fit in " t, v, min);         \
      v = min;                                                                 \
    }                                                                          \
  } while (0)

int pc_double_to_ptr(uint8_t *ptr, uint32_t interpretation, double val)
{
  switch (interpretation)
  {
  case PC_UINT8:
  {
    uint8_t v;
    CLAMP(val, 0, UINT8_MAX, "uint8_t", "%u");
    v = (uint8_t)lround(val);
    memcpy(ptr, &(v), sizeof(uint8_t));
    break;
  }
  case PC_UINT16:
  {
    uint16_t v;
    CLAMP(val, 0, UINT16_MAX, "uint16_t", "%u");
    v = (uint16_t)lround(val);
    memcpy(ptr, &(v), sizeof(uint16_t));
    break;
  }
  case PC_UINT32:
  {
    uint32_t v;
    CLAMP(val, 0, UINT32_MAX, "uint32", "%u");
    v = (uint32_t)lround(val);
    memcpy(ptr, &(v), sizeof(uint32_t));
    break;
  }
  case PC_UINT64:
  {
    uint64_t v;
    CLAMP(val, 0, UINT64_MAX, "uint64", "%u");
    v = (uint64_t)lround(val);
    memcpy(ptr, &(v), sizeof(uint64_t));
    break;
  }
  case PC_INT8:
  {
    int8_t v;
    CLAMP(val, INT8_MIN, INT8_MAX, "int8", "%d");
    v = (int8_t)lround(val);
    memcpy(ptr, &(v), sizeof(int8_t));
    break;
  }
  case PC_INT16:
  {
    int16_t v;
    CLAMP(val, INT16_MIN, INT16_MAX, "int16", "%d");
    v = (int16_t)lround(val);
    memcpy(ptr, &(v), sizeof(int16_t));
    break;
  }
  case PC_INT32:
  {
    int32_t v;
    CLAMP(val, INT32_MIN, INT32_MAX, "int32", "%d");
    v = (int32_t)lround(val);
    memcpy(ptr, &(v), sizeof(int32_t));
    break;
  }
  case PC_INT64:
  {
    int64_t v;
    CLAMP(val, INT64_MIN, INT64_MAX, "int64", "%d");
    v = (int64_t)lround(val);
    memcpy(ptr, &(v), sizeof(int64_t));
    break;
  }
  case PC_FLOAT:
  {
    float v = (float)val;
    memcpy(ptr, &(v), sizeof(float));
    break;
  }
  case PC_DOUBLE:
  {
    double v = val;
    memcpy(ptr, &(v), sizeof(double));
    break;
  }
  default:
  {
    pcerror("unknown interpretation type %d encountered in pc_double_to_ptr",
            interpretation);
    return PC_FAILURE;
  }
  }

  return PC_SUCCESS;
}
