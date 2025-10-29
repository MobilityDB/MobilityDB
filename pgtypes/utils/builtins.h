/*-------------------------------------------------------------------------
 *
 * builtins.h
 *    Declarations for operations on built-in types.
 *
 *
 * Portions Copyright (c) 1996-2025, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/utils/builtins.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef BUILTINS_H
#define BUILTINS_H

// #include "fmgr.h"
#include "nodes/nodes.h"
// #include "utils/fmgrprotos.h"

/* Sign + the most decimal digits an 8-byte number could have */
#define MAXINT8LEN 20

/* bool.c */
extern bool parse_bool(const char *value, bool *result);
extern bool parse_bool_with_len(const char *value, size_t len, bool *result);

/* encode.c */
extern uint64 hex_encode(const char *src, size_t len, char *dst);
extern uint64 hex_decode(const char *src, size_t len, char *dst);
extern uint64 hex_decode_safe(const char *src, size_t len, char *dst,
                Node *escontext);

/* numutils.c */
extern int16 pg_strtoint16(const char *s);
extern int16 pg_strtoint16_safe(const char *s, Node *escontext);
extern int32 pg_strtoint32(const char *s);
extern int32 pg_strtoint32_safe(const char *s, Node *escontext);
extern int64 pg_strtoint64(const char *s);
extern int64 pg_strtoint64_safe(const char *s, Node *escontext);
extern uint32 uint32in_subr(const char *s, char **endloc,
              const char *typname, Node *escontext);
extern uint64 uint64in_subr(const char *s, char **endloc,
              const char *typname, Node *escontext);
extern int  pg_itoa(int16 i, char *a);
extern int  pg_ultoa_n(uint32 value, char *a);
extern int  pg_ulltoa_n(uint64 value, char *a);
extern int  pg_ltoa(int32 value, char *a);
extern int  pg_lltoa(int64 value, char *a);
extern char *pg_ultostr_zeropad(char *str, uint32 value, int32 minwidth);
extern char *pg_ultostr(char *str, uint32 value);

/* varchar.c */
extern int  bpchartruelen(char *s, int len);

/* popular functions from varlena.c */
extern text *cstring_to_text(const char *s);
extern text *cstring_to_text_with_len(const char *s, int len);
extern char *text_to_cstring(const text *t);
extern void text_to_cstring_buffer(const text *src, char *dst, size_t dst_len);

#define CStringGetTextDatum(s) PointerGetDatum(cstring_to_text(s))
#define TextDatumGetCString(d) text_to_cstring((text *) DatumGetPointer(d))

/* numeric.c */
// extern Datum numeric_float8_no_overflow(PG_FUNCTION_ARGS);

/* format_type.c */

/* Control flags for format_type_extended */
#define FORMAT_TYPE_TYPEMOD_GIVEN	0x01	/* typemod defined by caller */
#define FORMAT_TYPE_ALLOW_INVALID	0x02	/* allow invalid types */
#define FORMAT_TYPE_FORCE_QUALIFY	0x04	/* force qualification of type */
#define FORMAT_TYPE_INVALID_AS_NULL	0x08	/* NULL if undefined */
extern char *format_type_extended(Oid type_oid, int32 typemod, bits16 flags);

extern char *format_type_be(Oid type_oid);
extern char *format_type_be_qualified(Oid type_oid);
extern char *format_type_with_typemod(Oid type_oid, int32 typemod);

extern int32 type_maximum_size(Oid type_oid, int32 typemod);

/* quote.c */
extern char *quote_literal_cstr(const char *rawstr);

#endif              /* BUILTINS_H */
