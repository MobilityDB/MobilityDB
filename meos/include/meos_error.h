#ifndef __MEOS_ERROR_H__
#define __MEOS_ERROR_H__

typedef enum
{
  MEOS_SUCCESS                   = 0,  // Successful operation

  MEOS_ERR_INTERNAL_ERROR        = 1,  // Unspecified internal error
  MEOS_ERR_INTERNAL_TYPE_ERROR   = 2,  // Internal type error
  MEOS_ERR_VALUE_OUT_OF_RANGE    = 3,  // Internal out of range error
  MEOS_ERR_DIVISION_BY_ZERO      = 4,  // Internal division by zero error
  MEOS_ERR_MEMORY_ALLOC_ERROR    = 5,  // Internal malloc error
  MEOS_ERR_AGGREGATION_ERROR     = 6,  // Internal aggregation error
  MEOS_ERR_DIRECTORY_ERROR       = 7,  // Internal directory error
  MEOS_ERR_FILE_ERROR            = 8,  // Internal file error
  MEOS_ERR_OUT_OF_MEMORY         = 9,  // Out of memory error

  MEOS_ERR_INVALID_ARG           = 10, // Invalid argument
  MEOS_ERR_INVALID_ARG_TYPE      = 11, // Invalid argument type
  MEOS_ERR_INVALID_ARG_VALUE     = 12, // Invalid argument value
  MEOS_ERR_FEATURE_NOT_SUPPORTED = 13, // Feature not supported

  MEOS_ERR_MFJSON_INPUT          = 20, // MFJSON input error
  MEOS_ERR_MFJSON_OUTPUT         = 21, // MFJSON output error
  MEOS_ERR_TEXT_INPUT            = 22, // Text input error
  MEOS_ERR_TEXT_OUTPUT           = 23, // Text output error
  MEOS_ERR_WKB_INPUT             = 24, // WKB input error
  MEOS_ERR_WKB_OUTPUT            = 25, // WKB output error
  MEOS_ERR_GEOJSON_INPUT         = 26, // GEOJSON input error
  MEOS_ERR_GEOJSON_OUTPUT        = 27, // GEOJSON output error

} errorCode;

extern void meos_error(int errlevel, int errcode, const char *format, ...);

/* Set or read error level */

extern int meos_errno(void);
extern int meos_errno_set(int err);
extern int meos_errno_restore(int err);
extern int meos_errno_reset(void);

#endif /* __MEOS_ERROR_H__ */
