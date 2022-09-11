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

