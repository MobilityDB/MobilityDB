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

extern DateADT date_in(const char *str);
extern char *date_out(DateADT d);
extern int interval_cmp(const Interval *interv1, const Interval *interv2);
extern Interval *interval_in(const char *str, int32 typmod);
extern Interval *interval_make(int32 years, int32 months, int32 weeks, int32 days, int32 hours, int32 mins, double secs);
extern char *interval_out(const Interval *interv);
extern double pg_exp(double arg1);
extern double pg_ln(double arg1);
extern double pg_log10(double arg1);
extern TimeADT time_in(const char *str, int32 typmod);
extern char *time_out(TimeADT t);
extern Timestamp timestamp_in(const char *str, int32 typmod);
extern char *timestamp_out(Timestamp t);
extern TimestampTz timestamptz_in(const char *str, int32 typmod);
extern char *timestamptz_out(TimestampTz t);

#endif              /* POSTGRES_H */
