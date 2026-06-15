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

typedef unsigned int Oid;

/* The following functions have the same name as external PostgreSQL functions */

extern bool bool_in(const char *str);
extern char *bool_out(bool b);
extern char *float8_out(double num, int maxdd);
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
extern text *cstring_to_text(const char *str);
extern char *text_to_cstring(const text *txt);
extern text *text_in(const char *str);
extern char *text_out(const text *txt);
extern int text_cmp(const text *txt1, const text *txt2, Oid collid);
extern text *text_copy(const text *txt);
extern text *text_initcap(const text *txt);
extern text *text_lower(const text *txt);
extern text *text_upper(const text *txt);
extern text *textcat_text_text(const text *txt1, const text *txt2);

#endif /* POSTGRES_H */
