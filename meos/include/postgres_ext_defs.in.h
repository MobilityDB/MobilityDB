/* The scalar types come from the header that states them once, so this file and
 * every pg_*.h answer the same type for a name. It is read BEFORE POSTGRES_H is
 * defined below, because that macro is what tells it PostgreSQL has already
 * supplied them. This file states the structures alone, which a second
 * definition would conflict with. */
#include <pg_basetypes.h>

#ifndef POSTGRES_H
#define POSTGRES_H

#define DatumGetPointer(X) ((Pointer) (X))

#include <stdint.h>

/* State vector of the PostgreSQL pseudo-random number generator, mirroring the
 * definition in <common/pg_prng.h>. Its include guard is defined here as well
 * so that pulling in the PostgreSQL header afterwards does not redefine the
 * type, exactly as this file does for <postgres.h>. */

#ifndef PG_PRNG_H
#define PG_PRNG_H

typedef struct pg_prng_state
{
  uint64 s0,
         s1;
} pg_prng_state;

#endif /* PG_PRNG_H */

typedef struct
{
  TimeOffset time;  /* all time units other than days, months and years */
  int32 day;        /* days, after time for alignment */
  int32 month;      /* months and years, after time for alignment */
} Interval;

typedef struct varlena
{
  char vl_len_[4];  /* Do not touch this field directly! */
  char vl_dat[];    /* Data content is here */
} varlena;

/* The following functions have the same name as external PostgreSQL functions */

extern bool bool_in(const char *str);
extern char *bool_out(bool b, size_t *size_out);
extern char *float8_out(double num, int maxdd, size_t *size_out);
extern DateADT date_in(const char *str);
extern char *date_out(DateADT date, size_t *size_out);
extern int interval_cmp(const Interval *interv1, const Interval *interv2);
extern Interval *interval_in(const char *str, int32 typmod);
extern char *interval_out(const Interval *interv, size_t *size_out);
extern TimeADT time_in(const char *str, int32 typmod);
extern char *time_out(TimeADT time, size_t *size_out);
extern Timestamp timestamp_in(const char *str, int32 typmod);
extern char *timestamp_out(Timestamp ts, size_t *size_out);
extern TimestampTz timestamptz_in(const char *str, int32 typmod);
extern char *timestamptz_out(TimestampTz tstz, size_t *size_out);
extern text *cstring_to_text(const char *str);
extern char *text_to_cstring(const text *txt);
extern text *text_in(const char *str);
extern char *text_out(const text *txt, size_t *size_out);
extern int text_cmp(const text *txt1, const text *txt2, Oid collid);
extern text *text_copy(const text *txt);
extern text *text_initcap(const text *txt);
extern text *text_lower(const text *txt);
extern text *text_upper(const text *txt);
extern text *textcat_text_text(const text *txt1, const text *txt2);

#endif /* POSTGRES_H */
