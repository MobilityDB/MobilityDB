#ifndef POSTGRES_H
#define POSTGRES_H

#define DatumGetPointer(X) ((Pointer) (X))

typedef char *Pointer;
typedef uintptr_t Datum;

typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
typedef long int int64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long int uint64;

typedef int32 DateADT;
typedef int64 TimeADT;
typedef int64 Timestamp;
typedef int64 TimestampTz;
typedef int64 TimeOffset;
typedef int32 fsec_t;      /* fractional seconds (in microseconds) */

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

typedef varlena text;
typedef struct varlena bytea;

/* The following functions have the same name as external PostgreSQL functions */

extern DateADT date_in(const char *str);
extern char *date_out(DateADT date);
extern int interval_cmp(const Interval *interv1, const Interval *interv2);
extern Interval *interval_in(const char *str, int32 typmod);
extern char *interval_out(const Interval *interv);
extern TimeADT time_in(const char *str, int32 typmod);
extern char *time_out(TimeADT time);
extern Timestamp timestamp_in(const char *str, int32 typmod);
extern char *timestamp_out(Timestamp ts);
extern TimestampTz timestamptz_in(const char *str, int32 typmod);
extern char *timestamptz_out(TimestampTz tstz);
/* Base-type operations (comparison, arithmetic, math, conversion) — the public
 * MEOS API must provide all operations over the base types; impls in pgtypes/ */
extern int int32_cmp(int32 l, int32 r);
extern int int64_cmp(int64 l, int64 r);
extern double float8_exp(double num);
extern double float8_ln(double num);
extern double float8_log10(double num);
extern Interval *add_interval_interval(const Interval *interv1, const Interval *interv2);
extern Interval *mul_interval_double(const Interval *interv, double factor);
extern int32 minus_date_date(DateADT date1, DateADT date2);
extern DateADT minus_date_int(DateADT date, int32 days);
extern TimestampTz minus_timestamptz_interval(TimestampTz tstz, const Interval *interv);
extern Interval *minus_timestamptz_timestamptz(TimestampTz tstz1, TimestampTz tstz2);
extern Timestamp date_to_timestamp(DateADT date);
extern TimestampTz date_to_timestamptz(DateADT date);

#endif /* POSTGRES_H */
