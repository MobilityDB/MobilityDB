#ifndef __MEOS_ERROR_H__
#define __MEOS_ERROR_H__

typedef enum
{
  MEOS_SUCCESS                        = 0,  // Successful operation

  MEOS_ERR_INTERNAL_ERROR             = 1,  // Unspecified internal error
  MEOS_ERR_INTERNAL_TYPE_ERROR        = 2,  // Internal type error
  MEOS_ERR_VALUE_OUT_OF_RANGE         = 3,  // Internal out of range error
  MEOS_ERR_DIVISION_BY_ZERO           = 4,  // Internal division by zero error
  MEOS_ERR_MEMORY_ALLOC_ERROR         = 5,  // Internal malloc error
  MEOS_ERR_AGGREGATION_ERROR          = 6,  // Internal aggregation error
  MEOS_ERR_DIRECTORY_ERROR            = 7,  // Internal directory error
  MEOS_ERR_FILE_ERROR                 = 8,  // Internal file error
  MEOS_ERR_OUT_OF_MEMORY              = 9,  // Out of memory error

  MEOS_ERR_INVALID_ARG                = 10, // Invalid argument
  MEOS_ERR_INVALID_ARG_TYPE           = 11, // Invalid argument type
  MEOS_ERR_INVALID_ARG_VALUE          = 12, // Invalid argument value
  MEOS_ERR_FEATURE_NOT_SUPPORTED      = 13, // Feature not supported
  MEOS_ERR_INDETERMINATE_COLLATION    = 14, // Indeterminate collation
  MEOS_ERR_SYNTAX_ERROR               = 15, // Syntax error
  MEOS_ERR_NULL_RESULT                = 16, // Operation returned null

  MEOS_ERR_MFJSON_INPUT               = 20, // MFJSON input error
  MEOS_ERR_MFJSON_OUTPUT              = 21, // MFJSON output error
  MEOS_ERR_TEXT_INPUT                 = 22, // Text input error
  MEOS_ERR_TEXT_OUTPUT                = 23, // Text output error
  MEOS_ERR_WKB_INPUT                  = 24, // WKB input error
  MEOS_ERR_WKB_OUTPUT                 = 25, // WKB output error
  MEOS_ERR_GEOJSON_INPUT              = 26, // GEOJSON input error
  MEOS_ERR_GEOJSON_OUTPUT             = 27, // GEOJSON output error
  MEOS_ERR_SQL_JSON_ERROR             = 28, // SQL JSON error
  MEOS_ERR_INVALID_REGULAR_EXPRESSION = 29, // Regular expression error
} errorCode;

/**
 * Report an error at the given level/code with a printf-style message.
 *
 * @note Return-or-not contract is **undefined**: this function MAY
 * return control to the caller, depending on the installed error
 * handler. The default handler `exit(EXIT_FAILURE)`s on `ERROR` (safe
 * for one-shot CLI use), but any custom handler an embedder installs
 * via `meos_initialize_error_handler` may return. Code that assumes
 * `meos_error(ERROR, ...)` never returns dereferences freed or
 * uninitialized state on the fall-through path.
 *
 * **Convention** (enforced by a static-analysis CI check): every
 * `meos_error(ERROR, ...)` callsite MUST be immediately followed by a
 * `return` (with a sentinel value if the function has a non-void
 * return type), `goto cleanup_label`, `break`, or equivalent
 * control-transfer. NEVER let execution fall through and assume the
 * call did not return. This makes every callsite safe regardless of
 * which handler an embedder installs.
 */
extern void meos_error(int errlevel, int errcode, const char *format, ...)
/*
 * Give meos_error printf-style format checking. In the PostgreSQL-backed
 * build pg_attribute_printf is available and selects the gnu_printf
 * archetype (correct on MinGW, where the default printf archetype is MS
 * semantics). This public header is also consumed standalone, where that
 * macro may not be in scope, so fall back to the gnu_printf attribute
 * directly under GCC/Clang, and to nothing on compilers that lack it.
 */
#if defined(pg_attribute_printf)
  pg_attribute_printf(3, 4);
#elif defined(__GNUC__) && !defined(__clang__)
  __attribute__((format(gnu_printf, 3, 4)));
#elif defined(__GNUC__)
  __attribute__((format(printf, 3, 4)));
#else
  ;
#endif

/* Set or read error level */

extern int meos_errno(void);
extern int meos_errno_set(int err);
extern int meos_errno_restore(int err);
extern int meos_errno_reset(void);

#endif /* __MEOS_ERROR_H__ */
