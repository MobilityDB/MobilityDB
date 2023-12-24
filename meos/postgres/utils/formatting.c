/* -----------------------------------------------------------------------
 * formatting.c
 *
 * src/backend/utils/adt/formatting.c
 *
 *
 *   Portions Copyright (c) 1999-2020, PostgreSQL Global Development Group
 *
 *
 *   TO_CHAR(); TO_TIMESTAMP(); TO_DATE(); TO_NUMBER();
 *
 *   The PostgreSQL routines for a timestamp/int/float/numeric formatting,
 *   inspired by the Oracle TO_CHAR() / TO_DATE() / TO_NUMBER() routines.
 *
 *
 *   Cache & Memory:
 *  Routines use (itself) internal cache for format pictures.
 *
 *  The cache uses a static buffer and is persistent across transactions.  If
 *  the format-picture is bigger than the cache buffer, the parser is called
 *  always.
 *
 *   NOTE for Number version:
 *  All in this version is implemented as keywords ( => not used
 *  suffixes), because a format picture is for *one* item (number)
 *  only. It not is as a timestamp version, where each keyword (can)
 *  has suffix.
 *
 *   NOTE for Timestamp routines:
 *  In this module the POSIX 'struct tm' type is *not* used, but rather
 *  PgSQL type, which has tm_mon based on one (*non* zero) and
 *  year *not* based on 1900, but is used full year number.
 *  Module supports AD / BC / AM / PM.
 *
 *  Supported types for to_char():
 *
 *    Timestamp, Numeric, int4, int8, float4, float8
 *
 *  Supported types for reverse conversion:
 *
 *    Timestamp  - to_timestamp()
 *    Date    - to_date()
 *    Numeric    - to_number()
 *
 *
 *  Karel Zak
 *
 * TODO
 *  - better number building (formatting) / parsing, now it isn't
 *      ideal code
 *  - use Assert()
 *  - add support for roman number to standard number conversion
 *  - add support for number spelling
 *  - add support for string to string formatting (we must be better
 *    than Oracle :-),
 *    to_char('Hello', 'X X X X X') -> 'H e l l o'
 *
 * -----------------------------------------------------------------------
 */

#include "postgres.h"

#include <limits.h>

#include "utils/timestamp_def.h"
#include "utils/datetime.h"
#include "utils/date.h"
#include "utils/float.h"

#include "../../include/meos.h"

#define DEFAULT_COLLATION_OID 100

/*
 * towlower() and friends should be in <wctype.h>, but some pre-C99 systems
 * declare them in <wchar.h>, so include that too.
 */
// #include <wchar.h>
// #ifdef HAVE_WCTYPE_H
// #include <wctype.h>
// #endif

// #ifdef USE_ICU
// #include <unicode/ustring.h>
// #endif

// #include "catalog/pg_collation.h"
// #include "catalog/pg_type.h"
// #include "mb/pg_wchar.h"
// #include "parser/scansup.h"
// #include "utils/builtins.h"
// #include "utils/formatting.h"
// #include "utils/int8.h"
// #include "utils/memutils.h"
// #include "utils/numeric.h"
// #include "utils/pg_locale.h"



extern char *text2cstring(const text *textptr);
extern text *cstring2text(const char *cstring);
extern char *pnstrdup(const char *in, Size size);

extern void MEOSAdjustTimestampForTypmod(Timestamp *time, int32 typmod);


/* ----------
 * Convenience macros for error handling
 * ----------
 *
 * Two macros below help to handle errors in functions that take
 * 'bool *have_error' argument.  When this argument is not NULL, it's expected
 * that function will suppress elogs when possible.  Instead it should
 * return some default value and set *have_error flag.
 *
 * RETURN_ERROR() macro intended to wrap elog() calls.  When have_error
 * function argument is not NULL, then instead of elog'ing we set
 * *have_error flag and go to on_error label.  It's supposed that jump
 * resources will be freed and some 'default' value returned.
 *
 * CHECK_ERROR() jumps on_error label when *have_error flag is defined and set.
 * It's supposed to be used for immediate exit from the function on error
 * after call of another function with 'bool *have_error' argument.
 */
#define RETURN_ERROR(throw_error) \
do { \
  if (have_error) \
  { \
    *have_error = true; \
    goto on_error; \
  } \
  else \
  { \
    throw_error; \
  } \
} while (0)

#define CHECK_ERROR \
do { \
  if (have_error && *have_error) \
    goto on_error; \
} while (0)

/* ----------
 * Routines flags
 * ----------
 */
#define DCH_FLAG    0x1    /* DATE-TIME flag  */
#define NUM_FLAG    0x2    /* NUMBER flag  */
#define STD_FLAG    0x4    /* STANDARD flag  */

/* ----------
 * KeyWord Index (ascii from position 32 (' ') to 126 (~))
 * ----------
 */
#define KeyWord_INDEX_SIZE    ('~' - ' ')
#define KeyWord_INDEX_FILTER(_c)  ((_c) <= ' ' || (_c) >= '~' ? 0 : 1)

/* ----------
 * Maximal length of one node
 * ----------
 */
#define DCH_MAX_ITEM_SIZ     12  /* max localized day name    */

/* ----------
 * Format parser structs
 * ----------
 */
typedef struct
{
  const char *name;      /* suffix string    */
  int      len,      /* suffix length    */
        id,        /* used in node->suffix */
        type;      /* prefix / postfix    */
} KeySuffix;

/* ----------
 * FromCharDateMode
 * ----------
 *
 * This value is used to nominate one of several distinct (and mutually
 * exclusive) date conventions that a keyword can belong to.
 */
typedef enum
{
  FROM_CHAR_DATE_NONE = 0,  /* Value does not affect date mode. */
  FROM_CHAR_DATE_GREGORIAN,  /* Gregorian (day, month, year) style date */
  FROM_CHAR_DATE_ISOWEEK    /* ISO 8601 week date */
} FromCharDateMode;

typedef struct
{
  const char *name;
  int      len;
  int      id;
  bool    is_digit;
  FromCharDateMode date_mode;
} KeyWord;

/*
 * Maximum byte length of multibyte characters in any backend encoding
 */
#define MAX_MULTIBYTE_CHAR_LEN  4

typedef struct
{
  uint8    type;      /* NODE_TYPE_XXX, see below */
  char    character[MAX_MULTIBYTE_CHAR_LEN + 1];  /* if type is CHAR */
  uint8    suffix;      /* keyword prefix/suffix code, if any */
  const KeyWord *key;      /* if type is ACTION */
} FormatNode;

#define NODE_TYPE_END    1
#define NODE_TYPE_ACTION  2
#define NODE_TYPE_CHAR    3
#define NODE_TYPE_SEPARATOR  4
#define NODE_TYPE_SPACE    5

#define SUFFTYPE_PREFIX    1
#define SUFFTYPE_POSTFIX  2

#define CLOCK_24_HOUR    0
#define CLOCK_12_HOUR    1


/* ----------
 * Full months
 * ----------
 */
static const char *const months_full[] = {
  "January", "February", "March", "April", "May", "June", "July",
  "August", "September", "October", "November", "December", NULL
};

static const char *const days_short[] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", NULL
};

/* ----------
 * AD / BC
 * ----------
 *  There is no 0 AD.  Years go from 1 BC to 1 AD, so we make it
 *  positive and map year == -1 to year zero, and shift all negative
 *  years up one.  For interval years, we just return the year.
 */
#define ADJUST_YEAR(year, is_interval)  ((is_interval) ? (year) : ((year) <= 0 ? -((year) - 1) : (year)))

#define A_D_STR    "A.D."
#define a_d_STR    "a.d."
#define AD_STR    "AD"
#define ad_STR    "ad"

#define B_C_STR    "B.C."
#define b_c_STR    "b.c."
#define BC_STR    "BC"
#define bc_STR    "bc"

/*
 * AD / BC strings for seq_search.
 *
 * These are given in two variants, a long form with periods and a standard
 * form without.
 *
 * The array is laid out such that matches for AD have an even index, and
 * matches for BC have an odd index.  So the boolean value for BC is given by
 * taking the array index of the match, modulo 2.
 */
static const char *const adbc_strings[] = {ad_STR, bc_STR, AD_STR, BC_STR, NULL};
static const char *const adbc_strings_long[] = {a_d_STR, b_c_STR, A_D_STR, B_C_STR, NULL};

/* ----------
 * AM / PM
 * ----------
 */
#define A_M_STR    "A.M."
#define a_m_STR    "a.m."
#define AM_STR    "AM"
#define am_STR    "am"

#define P_M_STR    "P.M."
#define p_m_STR    "p.m."
#define PM_STR    "PM"
#define pm_STR    "pm"

/*
 * AM / PM strings for seq_search.
 *
 * These are given in two variants, a long form with periods and a standard
 * form without.
 *
 * The array is laid out such that matches for AM have an even index, and
 * matches for PM have an odd index.  So the boolean value for PM is given by
 * taking the array index of the match, modulo 2.
 */
static const char *const ampm_strings[] = {am_STR, pm_STR, AM_STR, PM_STR, NULL};
static const char *const ampm_strings_long[] = {a_m_STR, p_m_STR, A_M_STR, P_M_STR, NULL};

/* ----------
 * Months in roman-numeral
 * (Must be in reverse order for seq_search (in FROM_CHAR), because
 *  'VIII' must have higher precedence than 'V')
 * ----------
 */
static const char *const rm_months_upper[] =
{"XII", "XI", "X", "IX", "VIII", "VII", "VI", "V", "IV", "III", "II", "I", NULL};

static const char *const rm_months_lower[] =
{"xii", "xi", "x", "ix", "viii", "vii", "vi", "v", "iv", "iii", "ii", "i", NULL};

/* ----------
 * Roman numbers
 * ----------
 */
static const char *const rm1[] = {"I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", NULL};
static const char *const rm10[] = {"X", "XX", "XXX", "XL", "L", "LX", "LXX", "LXXX", "XC", NULL};
static const char *const rm100[] = {"C", "CC", "CCC", "CD", "D", "DC", "DCC", "DCCC", "CM", NULL};

/* ----------
 * Ordinal postfixes
 * ----------
 */
static const char *const numTH[] = {"ST", "ND", "RD", "TH", NULL};
static const char *const numth[] = {"st", "nd", "rd", "th", NULL};

/* ----------
 * Flags & Options:
 * ----------
 */
#define TH_UPPER    1
#define TH_LOWER    2


/* ----------
 * Format picture cache
 *
 * We will cache datetime format pictures up to DCH_CACHE_SIZE bytes long;
 * likewise number format pictures up to NUM_CACHE_SIZE bytes long.
 *
 * For simplicity, the cache entries are fixed-size, so they allow for the
 * worst case of a FormatNode for each byte in the picture string.
 *
 * The CACHE_SIZE constants are computed to make sizeof(DCHCacheEntry) and
 * sizeof(NUMCacheEntry) be powers of 2, or just less than that, so that
 * we don't waste too much space by palloc'ing them individually.  Be sure
 * to adjust those macros if you add fields to those structs.
 *
 * The max number of entries in each cache is DCH_CACHE_ENTRIES
 * resp. NUM_CACHE_ENTRIES.
 * ----------
 */
#define DCH_CACHE_OVERHEAD \
  MAXALIGN(sizeof(bool) + sizeof(int))

#define DCH_CACHE_SIZE \
  ((2048 - DCH_CACHE_OVERHEAD) / (sizeof(FormatNode) + sizeof(char)) - 1)

#define DCH_CACHE_ENTRIES  20

typedef struct
{
  FormatNode  format[DCH_CACHE_SIZE + 1];
  char    str[DCH_CACHE_SIZE + 1];
  bool    std;
  bool    valid;
  int      age;
} DCHCacheEntry;


/* global cache for date/time format pictures */
static DCHCacheEntry *DCHCache[DCH_CACHE_ENTRIES];
static int  n_DCHCache = 0;    /* current number of entries */
static int  DCHCounter = 0;    /* aging-event counter */


/* ----------
 * For char->date/time conversion
 * ----------
 */
typedef struct
{
  FromCharDateMode mode;
  int      hh,
        pm,
        mi,
        ss,
        ssss,
        d,        /* stored as 1-7, Sunday = 1, 0 means missing */
        dd,
        ddd,
        mm,
        ms,
        year,
        bc,
        ww,
        w,
        cc,
        j,
        us,
        yysz,      /* is it YY or YYYY ? */
        clock,      /* 12 or 24 hour clock? */
        tzsign,      /* +1, -1 or 0 if timezone info is absent */
        tzh,
        tzm,
        ff;        /* fractional precision */
} TmFromChar;

#define ZERO_tmfc(_X) memset(_X, 0, sizeof(TmFromChar))

/* ----------
 * Datetime to char conversion
 * ----------
 */
typedef struct TmToChar
{
  struct pg_tm tm;      /* classic 'tm' struct */
  fsec_t    fsec;      /* fractional seconds */
  const char *tzn;      /* timezone */
} TmToChar;

#define tmtcTm(_X)  (&(_X)->tm)
#define tmtcTzn(_X) ((_X)->tzn)
#define tmtcFsec(_X)  ((_X)->fsec)

#define ZERO_tm(_X) \
do {  \
  (_X)->tm_sec  = (_X)->tm_year = (_X)->tm_min = (_X)->tm_wday = \
  (_X)->tm_hour = (_X)->tm_yday = (_X)->tm_isdst = 0; \
  (_X)->tm_mday = (_X)->tm_mon  = 1; \
  (_X)->tm_zone = NULL; \
} while(0)

#define ZERO_tmtc(_X) \
do { \
  ZERO_tm( tmtcTm(_X) ); \
  tmtcFsec(_X) = 0; \
  tmtcTzn(_X) = NULL; \
} while(0)

/*
 *  to_char(time) appears to to_char() as an interval, so this check
 *  is really for interval and time data types.
 */
#define INVALID_FOR_INTERVAL  \
do { \
  if (is_interval) \
  { \
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, \
        "invalid format specification for an interval value"); \
    return; \
  } \
} while(0)

/*****************************************************************************
 *      KeyWord definitions
 *****************************************************************************/

/* ----------
 * Suffixes (FormatNode.suffix is an OR of these codes)
 * ----------
 */
#define DCH_S_FM  0x01
#define DCH_S_TH  0x02
#define DCH_S_th  0x04
#define DCH_S_SP  0x08
#define DCH_S_TM  0x10

/* ----------
 * Suffix tests
 * ----------
 */
#define S_THth(_s)  ((((_s) & DCH_S_TH) || ((_s) & DCH_S_th)) ? 1 : 0)
#define S_TH(_s)  (((_s) & DCH_S_TH) ? 1 : 0)
#define S_th(_s)  (((_s) & DCH_S_th) ? 1 : 0)
#define S_TH_TYPE(_s)  (((_s) & DCH_S_TH) ? TH_UPPER : TH_LOWER)

/* Oracle toggles FM behavior, we don't; see docs. */
#define S_FM(_s)  (((_s) & DCH_S_FM) ? 1 : 0)
#define S_SP(_s)  (((_s) & DCH_S_SP) ? 1 : 0)
#define S_TM(_s)  (((_s) & DCH_S_TM) ? 1 : 0)

/* ----------
 * Suffixes definition for DATE-TIME TO/FROM CHAR
 * ----------
 */
#define TM_SUFFIX_LEN  2

static const KeySuffix DCH_suff[] = {
  {"FM", 2, DCH_S_FM, SUFFTYPE_PREFIX},
  {"fm", 2, DCH_S_FM, SUFFTYPE_PREFIX},
  {"TM", TM_SUFFIX_LEN, DCH_S_TM, SUFFTYPE_PREFIX},
  {"tm", 2, DCH_S_TM, SUFFTYPE_PREFIX},
  {"TH", 2, DCH_S_TH, SUFFTYPE_POSTFIX},
  {"th", 2, DCH_S_th, SUFFTYPE_POSTFIX},
  {"SP", 2, DCH_S_SP, SUFFTYPE_POSTFIX},
  /* last */
  {NULL, 0, 0, 0}
};


/* ----------
 * Format-pictures (KeyWord).
 *
 * The KeyWord field; alphabetic sorted, *BUT* strings alike is sorted
 *      complicated -to-> easy:
 *
 *  (example: "DDD","DD","Day","D" )
 *
 * (this specific sort needs the algorithm for sequential search for strings,
 * which not has exact end; -> How keyword is in "HH12blabla" ? - "HH"
 * or "HH12"? You must first try "HH12", because "HH" is in string, but
 * it is not good.
 *
 * (!)
 *   - Position for the keyword is similar as position in the enum DCH/NUM_poz.
 * (!)
 *
 * For fast search is used the 'int index[]', index is ascii table from position
 * 32 (' ') to 126 (~), in this index is DCH_ / NUM_ enums for each ASCII
 * position or -1 if char is not used in the KeyWord. Search example for
 * string "MM":
 *  1)  see in index to index['M' - 32],
 *  2)  take keywords position (enum DCH_MI) from index
 *  3)  run sequential search in keywords[] from this position
 *
 * ----------
 */

typedef enum
{
  DCH_A_D,
  DCH_A_M,
  DCH_AD,
  DCH_AM,
  DCH_B_C,
  DCH_BC,
  DCH_CC,
  DCH_DAY,
  DCH_DDD,
  DCH_DD,
  DCH_DY,
  DCH_Day,
  DCH_Dy,
  DCH_D,
  DCH_FF1,
  DCH_FF2,
  DCH_FF3,
  DCH_FF4,
  DCH_FF5,
  DCH_FF6,
  DCH_FX,            /* global suffix */
  DCH_HH24,
  DCH_HH12,
  DCH_HH,
  DCH_IDDD,
  DCH_ID,
  DCH_IW,
  DCH_IYYY,
  DCH_IYY,
  DCH_IY,
  DCH_I,
  DCH_J,
  DCH_MI,
  DCH_MM,
  DCH_MONTH,
  DCH_MON,
  DCH_MS,
  DCH_Month,
  DCH_Mon,
  DCH_OF,
  DCH_P_M,
  DCH_PM,
  DCH_Q,
  DCH_RM,
  DCH_SSSSS,
  DCH_SSSS,
  DCH_SS,
  DCH_TZH,
  DCH_TZM,
  DCH_TZ,
  DCH_US,
  DCH_WW,
  DCH_W,
  DCH_Y_YYY,
  DCH_YYYY,
  DCH_YYY,
  DCH_YY,
  DCH_Y,
  DCH_a_d,
  DCH_a_m,
  DCH_ad,
  DCH_am,
  DCH_b_c,
  DCH_bc,
  DCH_cc,
  DCH_day,
  DCH_ddd,
  DCH_dd,
  DCH_dy,
  DCH_d,
  DCH_ff1,
  DCH_ff2,
  DCH_ff3,
  DCH_ff4,
  DCH_ff5,
  DCH_ff6,
  DCH_fx,
  DCH_hh24,
  DCH_hh12,
  DCH_hh,
  DCH_iddd,
  DCH_id,
  DCH_iw,
  DCH_iyyy,
  DCH_iyy,
  DCH_iy,
  DCH_i,
  DCH_j,
  DCH_mi,
  DCH_mm,
  DCH_month,
  DCH_mon,
  DCH_ms,
  DCH_p_m,
  DCH_pm,
  DCH_q,
  DCH_rm,
  DCH_sssss,
  DCH_ssss,
  DCH_ss,
  DCH_tz,
  DCH_us,
  DCH_ww,
  DCH_w,
  DCH_y_yyy,
  DCH_yyyy,
  DCH_yyy,
  DCH_yy,
  DCH_y,

  /* last */
  _DCH_last_
}      DCH_poz;


/* ----------
 * KeyWords for DATE-TIME version
 * ----------
 */
static const KeyWord DCH_keywords[] = {
/*  name, len, id, is_digit, date_mode */
  {"A.D.", 4, DCH_A_D, false, FROM_CHAR_DATE_NONE},  /* A */
  {"A.M.", 4, DCH_A_M, false, FROM_CHAR_DATE_NONE},
  {"AD", 2, DCH_AD, false, FROM_CHAR_DATE_NONE},
  {"AM", 2, DCH_AM, false, FROM_CHAR_DATE_NONE},
  {"B.C.", 4, DCH_B_C, false, FROM_CHAR_DATE_NONE},  /* B */
  {"BC", 2, DCH_BC, false, FROM_CHAR_DATE_NONE},
  {"CC", 2, DCH_CC, true, FROM_CHAR_DATE_NONE},  /* C */
  {"DAY", 3, DCH_DAY, false, FROM_CHAR_DATE_NONE},  /* D */
  {"DDD", 3, DCH_DDD, true, FROM_CHAR_DATE_GREGORIAN},
  {"DD", 2, DCH_DD, true, FROM_CHAR_DATE_GREGORIAN},
  {"DY", 2, DCH_DY, false, FROM_CHAR_DATE_NONE},
  {"Day", 3, DCH_Day, false, FROM_CHAR_DATE_NONE},
  {"Dy", 2, DCH_Dy, false, FROM_CHAR_DATE_NONE},
  {"D", 1, DCH_D, true, FROM_CHAR_DATE_GREGORIAN},
  {"FF1", 3, DCH_FF1, false, FROM_CHAR_DATE_NONE},  /* F */
  {"FF2", 3, DCH_FF2, false, FROM_CHAR_DATE_NONE},
  {"FF3", 3, DCH_FF3, false, FROM_CHAR_DATE_NONE},
  {"FF4", 3, DCH_FF4, false, FROM_CHAR_DATE_NONE},
  {"FF5", 3, DCH_FF5, false, FROM_CHAR_DATE_NONE},
  {"FF6", 3, DCH_FF6, false, FROM_CHAR_DATE_NONE},
  {"FX", 2, DCH_FX, false, FROM_CHAR_DATE_NONE},
  {"HH24", 4, DCH_HH24, true, FROM_CHAR_DATE_NONE},  /* H */
  {"HH12", 4, DCH_HH12, true, FROM_CHAR_DATE_NONE},
  {"HH", 2, DCH_HH, true, FROM_CHAR_DATE_NONE},
  {"IDDD", 4, DCH_IDDD, true, FROM_CHAR_DATE_ISOWEEK},  /* I */
  {"ID", 2, DCH_ID, true, FROM_CHAR_DATE_ISOWEEK},
  {"IW", 2, DCH_IW, true, FROM_CHAR_DATE_ISOWEEK},
  {"IYYY", 4, DCH_IYYY, true, FROM_CHAR_DATE_ISOWEEK},
  {"IYY", 3, DCH_IYY, true, FROM_CHAR_DATE_ISOWEEK},
  {"IY", 2, DCH_IY, true, FROM_CHAR_DATE_ISOWEEK},
  {"I", 1, DCH_I, true, FROM_CHAR_DATE_ISOWEEK},
  {"J", 1, DCH_J, true, FROM_CHAR_DATE_NONE}, /* J */
  {"MI", 2, DCH_MI, true, FROM_CHAR_DATE_NONE},  /* M */
  {"MM", 2, DCH_MM, true, FROM_CHAR_DATE_GREGORIAN},
  {"MONTH", 5, DCH_MONTH, false, FROM_CHAR_DATE_GREGORIAN},
  {"MON", 3, DCH_MON, false, FROM_CHAR_DATE_GREGORIAN},
  {"MS", 2, DCH_MS, true, FROM_CHAR_DATE_NONE},
  {"Month", 5, DCH_Month, false, FROM_CHAR_DATE_GREGORIAN},
  {"Mon", 3, DCH_Mon, false, FROM_CHAR_DATE_GREGORIAN},
  {"OF", 2, DCH_OF, false, FROM_CHAR_DATE_NONE},  /* O */
  {"P.M.", 4, DCH_P_M, false, FROM_CHAR_DATE_NONE},  /* P */
  {"PM", 2, DCH_PM, false, FROM_CHAR_DATE_NONE},
  {"Q", 1, DCH_Q, true, FROM_CHAR_DATE_NONE}, /* Q */
  {"RM", 2, DCH_RM, false, FROM_CHAR_DATE_GREGORIAN}, /* R */
  {"SSSSS", 5, DCH_SSSS, true, FROM_CHAR_DATE_NONE},  /* S */
  {"SSSS", 4, DCH_SSSS, true, FROM_CHAR_DATE_NONE},
  {"SS", 2, DCH_SS, true, FROM_CHAR_DATE_NONE},
  {"TZH", 3, DCH_TZH, false, FROM_CHAR_DATE_NONE},  /* T */
  {"TZM", 3, DCH_TZM, true, FROM_CHAR_DATE_NONE},
  {"TZ", 2, DCH_TZ, false, FROM_CHAR_DATE_NONE},
  {"US", 2, DCH_US, true, FROM_CHAR_DATE_NONE},  /* U */
  {"WW", 2, DCH_WW, true, FROM_CHAR_DATE_GREGORIAN},  /* W */
  {"W", 1, DCH_W, true, FROM_CHAR_DATE_GREGORIAN},
  {"Y,YYY", 5, DCH_Y_YYY, true, FROM_CHAR_DATE_GREGORIAN},  /* Y */
  {"YYYY", 4, DCH_YYYY, true, FROM_CHAR_DATE_GREGORIAN},
  {"YYY", 3, DCH_YYY, true, FROM_CHAR_DATE_GREGORIAN},
  {"YY", 2, DCH_YY, true, FROM_CHAR_DATE_GREGORIAN},
  {"Y", 1, DCH_Y, true, FROM_CHAR_DATE_GREGORIAN},
  {"a.d.", 4, DCH_a_d, false, FROM_CHAR_DATE_NONE},  /* a */
  {"a.m.", 4, DCH_a_m, false, FROM_CHAR_DATE_NONE},
  {"ad", 2, DCH_ad, false, FROM_CHAR_DATE_NONE},
  {"am", 2, DCH_am, false, FROM_CHAR_DATE_NONE},
  {"b.c.", 4, DCH_b_c, false, FROM_CHAR_DATE_NONE},  /* b */
  {"bc", 2, DCH_bc, false, FROM_CHAR_DATE_NONE},
  {"cc", 2, DCH_CC, true, FROM_CHAR_DATE_NONE},  /* c */
  {"day", 3, DCH_day, false, FROM_CHAR_DATE_NONE},  /* d */
  {"ddd", 3, DCH_DDD, true, FROM_CHAR_DATE_GREGORIAN},
  {"dd", 2, DCH_DD, true, FROM_CHAR_DATE_GREGORIAN},
  {"dy", 2, DCH_dy, false, FROM_CHAR_DATE_NONE},
  {"d", 1, DCH_D, true, FROM_CHAR_DATE_GREGORIAN},
  {"ff1", 3, DCH_FF1, false, FROM_CHAR_DATE_NONE},  /* f */
  {"ff2", 3, DCH_FF2, false, FROM_CHAR_DATE_NONE},
  {"ff3", 3, DCH_FF3, false, FROM_CHAR_DATE_NONE},
  {"ff4", 3, DCH_FF4, false, FROM_CHAR_DATE_NONE},
  {"ff5", 3, DCH_FF5, false, FROM_CHAR_DATE_NONE},
  {"ff6", 3, DCH_FF6, false, FROM_CHAR_DATE_NONE},
  {"fx", 2, DCH_FX, false, FROM_CHAR_DATE_NONE},
  {"hh24", 4, DCH_HH24, true, FROM_CHAR_DATE_NONE},  /* h */
  {"hh12", 4, DCH_HH12, true, FROM_CHAR_DATE_NONE},
  {"hh", 2, DCH_HH, true, FROM_CHAR_DATE_NONE},
  {"iddd", 4, DCH_IDDD, true, FROM_CHAR_DATE_ISOWEEK},  /* i */
  {"id", 2, DCH_ID, true, FROM_CHAR_DATE_ISOWEEK},
  {"iw", 2, DCH_IW, true, FROM_CHAR_DATE_ISOWEEK},
  {"iyyy", 4, DCH_IYYY, true, FROM_CHAR_DATE_ISOWEEK},
  {"iyy", 3, DCH_IYY, true, FROM_CHAR_DATE_ISOWEEK},
  {"iy", 2, DCH_IY, true, FROM_CHAR_DATE_ISOWEEK},
  {"i", 1, DCH_I, true, FROM_CHAR_DATE_ISOWEEK},
  {"j", 1, DCH_J, true, FROM_CHAR_DATE_NONE}, /* j */
  {"mi", 2, DCH_MI, true, FROM_CHAR_DATE_NONE},  /* m */
  {"mm", 2, DCH_MM, true, FROM_CHAR_DATE_GREGORIAN},
  {"month", 5, DCH_month, false, FROM_CHAR_DATE_GREGORIAN},
  {"mon", 3, DCH_mon, false, FROM_CHAR_DATE_GREGORIAN},
  {"ms", 2, DCH_MS, true, FROM_CHAR_DATE_NONE},
  {"p.m.", 4, DCH_p_m, false, FROM_CHAR_DATE_NONE},  /* p */
  {"pm", 2, DCH_pm, false, FROM_CHAR_DATE_NONE},
  {"q", 1, DCH_Q, true, FROM_CHAR_DATE_NONE}, /* q */
  {"rm", 2, DCH_rm, false, FROM_CHAR_DATE_GREGORIAN}, /* r */
  {"sssss", 5, DCH_SSSS, true, FROM_CHAR_DATE_NONE},  /* s */
  {"ssss", 4, DCH_SSSS, true, FROM_CHAR_DATE_NONE},
  {"ss", 2, DCH_SS, true, FROM_CHAR_DATE_NONE},
  {"tz", 2, DCH_tz, false, FROM_CHAR_DATE_NONE},  /* t */
  {"us", 2, DCH_US, true, FROM_CHAR_DATE_NONE},  /* u */
  {"ww", 2, DCH_WW, true, FROM_CHAR_DATE_GREGORIAN},  /* w */
  {"w", 1, DCH_W, true, FROM_CHAR_DATE_GREGORIAN},
  {"y,yyy", 5, DCH_Y_YYY, true, FROM_CHAR_DATE_GREGORIAN},  /* y */
  {"yyyy", 4, DCH_YYYY, true, FROM_CHAR_DATE_GREGORIAN},
  {"yyy", 3, DCH_YYY, true, FROM_CHAR_DATE_GREGORIAN},
  {"yy", 2, DCH_YY, true, FROM_CHAR_DATE_GREGORIAN},
  {"y", 1, DCH_Y, true, FROM_CHAR_DATE_GREGORIAN},

  /* last */
  {NULL, 0, 0, 0, 0}
};


/* ----------
 * KeyWords index for DATE-TIME version
 * ----------
 */
static const int DCH_index[KeyWord_INDEX_SIZE] = {
/*
0  1  2  3  4  5  6  7  8  9
*/
  /*---- first 0..31 chars are skipped ----*/

  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, DCH_A_D, DCH_B_C, DCH_CC, DCH_DAY, -1,
  DCH_FF1, -1, DCH_HH24, DCH_IDDD, DCH_J, -1, -1, DCH_MI, -1, DCH_OF,
  DCH_P_M, DCH_Q, DCH_RM, DCH_SSSSS, DCH_TZH, DCH_US, -1, DCH_WW, -1, DCH_Y_YYY,
  -1, -1, -1, -1, -1, -1, -1, DCH_a_d, DCH_b_c, DCH_cc,
  DCH_day, -1, DCH_ff1, -1, DCH_hh24, DCH_iddd, DCH_j, -1, -1, DCH_mi,
  -1, -1, DCH_p_m, DCH_q, DCH_rm, DCH_sssss, DCH_tz, DCH_us, -1, DCH_ww,
  -1, DCH_y_yyy, -1, -1, -1, -1

  /*---- chars over 126 are skipped ----*/
};


/* Return flags for DCH_from_char() */
#define DCH_DATED  0x01
#define DCH_TIMED  0x02
#define DCH_ZONED  0x04

/* ----------
 * Functions
 * ----------
 */
static const KeyWord *index_seq_search(const char *str, const KeyWord *kw,
                     const int *index);
static const KeySuffix *suff_search(const char *str, const KeySuffix *suf, int type);
static bool is_separator_char(const char *str);
static void parse_format(FormatNode *node, const char *str, const KeyWord *kw,
             const KeySuffix *suf, const int *index, uint32 flags);

static void DCH_to_char(FormatNode *node, bool is_interval,
            TmToChar *in, char *out, Oid collid);
static void DCH_from_char(FormatNode *node, const char *in, TmFromChar *out,
              Oid collid, bool std, bool *have_error);

static const char *get_th(char *num, int type);
static char *str_numth(char *dest, char *num, int type);
static int  adjust_partial_year_to_2020(int year);
static int  strspace_len(const char *str);
static void from_char_set_mode(TmFromChar *tmfc, const FromCharDateMode mode,
                 bool *have_error);
static void from_char_set_int(int *dest, const int value, const FormatNode *node,
                bool *have_error);
static int  from_char_parse_int_len(int *dest, const char **src, const int len,
                  FormatNode *node, bool *have_error);
static int  from_char_parse_int(int *dest, const char **src, FormatNode *node,
                bool *have_error);
static int  seq_search_ascii(const char *name, const char *const *array, int *len);
static int  seq_search_localized(const char *name, char **array, int *len,
                 Oid collid);
static int  from_char_seq_search(int *dest, const char **src,
                 const char *const *array,
                 char **localized_array, Oid collid,
                 FormatNode *node, bool *have_error);
static void do_to_timestamp(text *date_txt, text *fmt, Oid collid, bool std,
              struct pg_tm *tm, fsec_t *fsec, int *fprec,
              uint32 *flags, bool *have_error);
static DCHCacheEntry *DCH_cache_getnew(const char *str, bool std);
static DCHCacheEntry *DCH_cache_search(const char *str, bool std);
static DCHCacheEntry *DCH_cache_fetch(const char *str, bool std);


/* ----------
 * Fast sequential search, use index for data selection which
 * go to seq. cycle (it is very fast for unwanted strings)
 * (can't be used binary search in format parsing)
 * ----------
 */
static const KeyWord *
index_seq_search(const char *str, const KeyWord *kw, const int *index)
{
  int      poz;

  if (!KeyWord_INDEX_FILTER(*str))
    return NULL;

  if ((poz = *(index + (*str - ' '))) > -1)
  {
    const KeyWord *k = kw + poz;

    do
    {
      if (strncmp(str, k->name, k->len) == 0)
        return k;
      k++;
      if (!k->name)
        return NULL;
    } while (*str == *k->name);
  }
  return NULL;
}

static const KeySuffix *
suff_search(const char *str, const KeySuffix *suf, int type)
{
  const KeySuffix *s;

  for (s = suf; s->name != NULL; s++)
  {
    if (s->type != type)
      continue;

    if (strncmp(str, s->name, s->len) == 0)
      return s;
  }
  return NULL;
}

static bool
is_separator_char(const char *str)
{
  /* ASCII printable character, but not letter or digit */
  return (*str > 0x20 && *str < 0x7F &&
      !(*str >= 'A' && *str <= 'Z') &&
      !(*str >= 'a' && *str <= 'z') &&
      !(*str >= '0' && *str <= '9'));
}


/* ----------
 * Format parser, search small keywords and keyword's suffixes, and make
 * format-node tree.
 *
 * for DATE-TIME & NUMBER version
 * ----------
 */
static void
parse_format(FormatNode *node, const char *str, const KeyWord *kw,
       const KeySuffix *suf, const int *index, uint32 flags)
{
  FormatNode *n;

  n = node;

  while (*str)
  {
    int      suffix = 0;
    const KeySuffix *s;

    /*
     * Prefix
     */
    if ((flags & DCH_FLAG) &&
      (s = suff_search(str, suf, SUFFTYPE_PREFIX)) != NULL)
    {
      suffix |= s->id;
      if (s->len)
        str += s->len;
    }

    /*
     * Keyword
     */
    if (*str && (n->key = index_seq_search(str, kw, index)) != NULL)
    {
      n->type = NODE_TYPE_ACTION;
      n->suffix = suffix;
      if (n->key->len)
        str += n->key->len;

      /*
       * Postfix
       */
      if ((flags & DCH_FLAG) && *str &&
        (s = suff_search(str, suf, SUFFTYPE_POSTFIX)) != NULL)
      {
        n->suffix |= s->id;
        if (s->len)
          str += s->len;
      }

      n++;
    }
    else if (*str)
    {
      int      chlen;

      if ((flags & STD_FLAG) && *str != '"')
      {
        /*
         * Standard mode, allow only following separators: "-./,':; ".
         * However, we support double quotes even in standard mode
         * (see below).  This is our extension of standard mode.
         */
        if (strchr("-./,':; ", *str) == NULL)
        {
          meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
            "invalid datetime format separator: \"%s\"",
            pnstrdup(str, sizeof(char)));
          return;
        }

        if (*str == ' ')
          n->type = NODE_TYPE_SPACE;
        else
          n->type = NODE_TYPE_SEPARATOR;

        n->character[0] = *str;
        n->character[1] = '\0';
        n->key = NULL;
        n->suffix = 0;
        n++;
        str++;
      }
      else if (*str == '"')
      {
        /*
         * Process double-quoted literal string, if any
         */
        str++;
        while (*str)
        {
          if (*str == '"')
          {
            str++;
            break;
          }
          /* backslash quotes the next character, if any */
          if (*str == '\\' && *(str + 1))
            str++;
          chlen = sizeof(char);
          n->type = NODE_TYPE_CHAR;
          memcpy(n->character, str, chlen);
          n->character[chlen] = '\0';
          n->key = NULL;
          n->suffix = 0;
          n++;
          str += chlen;
        }
      }
      else
      {
        /*
         * Outside double-quoted strings, backslash is only special if
         * it immediately precedes a double quote.
         */
        if (*str == '\\' && *(str + 1) == '"')
          str++;
        chlen = sizeof(char);

        if ((flags & DCH_FLAG) && is_separator_char(str))
          n->type = NODE_TYPE_SEPARATOR;
        else if (isspace((unsigned char) *str))
          n->type = NODE_TYPE_SPACE;
        else
          n->type = NODE_TYPE_CHAR;

        memcpy(n->character, str, chlen);
        n->character[chlen] = '\0';
        n->key = NULL;
        n->suffix = 0;
        n++;
        str += chlen;
      }
    }
  }

  n->type = NODE_TYPE_END;
  n->suffix = 0;
}

/*****************************************************************************
 *      Private utils
 *****************************************************************************/

/* ----------
 * Return ST/ND/RD/TH for simple (1..9) numbers
 * type --> 0 upper, 1 lower
 * ----------
 */
static const char *
get_th(char *num, int type)
{
  int      len = strlen(num),
        last,
        seclast;

  last = *(num + (len - 1));
  if (!isdigit((unsigned char) last))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "\"%s\" is not a number", num);
    return NULL;
  }

  /*
   * All "teens" (<x>1[0-9]) get 'TH/th', while <x>[02-9][123] still get
   * 'ST/st', 'ND/nd', 'RD/rd', respectively
   */
  if ((len > 1) && ((seclast = num[len - 2]) == '1'))
    last = 0;

  switch (last)
  {
    case '1':
      if (type == TH_UPPER)
        return numTH[0];
      return numth[0];
    case '2':
      if (type == TH_UPPER)
        return numTH[1];
      return numth[1];
    case '3':
      if (type == TH_UPPER)
        return numTH[2];
      return numth[2];
    default:
      if (type == TH_UPPER)
        return numTH[3];
      return numth[3];
  }
}

/* ----------
 * Convert string-number to ordinal string-number
 * type --> 0 upper, 1 lower
 * ----------
 */
static char *
str_numth(char *dest, char *num, int type)
{
  if (dest != num)
    strcpy(dest, num);
  strcat(dest, get_th(num, type));
  return dest;
}

/*****************************************************************************
 *      upper/lower/initcap functions
 * MEOS currently does not support neither locale nor wide characters
 *****************************************************************************/

/*
 * ASCII-only lower function
 *
 * We pass the number of bytes so we can pass varlena and char*
 * to this function.  The result is a palloc'd, null-terminated string.
 */
char *
asc_tolower(const char *buff, size_t nbytes)
{
  char     *result;
  char     *p;

  if (!buff)
    return NULL;

  result = pnstrdup(buff, nbytes);

  for (p = result; *p; p++)
    *p = pg_ascii_tolower((unsigned char) *p);

  return result;
}

/*
 * ASCII-only upper function
 *
 * We pass the number of bytes so we can pass varlena and char*
 * to this function.  The result is a palloc'd, null-terminated string.
 */
char *
asc_toupper(const char *buff, size_t nbytes)
{
  char     *result;
  char     *p;

  if (!buff)
    return NULL;

  result = pnstrdup(buff, nbytes);

  for (p = result; *p; p++)
    *p = pg_ascii_toupper((unsigned char) *p);

  return result;
}

/*
 * ASCII-only initcap function
 *
 * We pass the number of bytes so we can pass varlena and char*
 * to this function.  The result is a palloc'd, null-terminated string.
 */
char *
asc_initcap(const char *buff, size_t nbytes)
{
  char     *result;
  char     *p;
  int      wasalnum = false;

  if (!buff)
    return NULL;

  result = pnstrdup(buff, nbytes);

  for (p = result; *p; p++)
  {
    char    c;

    if (wasalnum)
      *p = c = pg_ascii_tolower((unsigned char) *p);
    else
      *p = c = pg_ascii_toupper((unsigned char) *p);
    /* we don't trust isalnum() here */
    wasalnum = ((c >= 'A' && c <= 'Z') ||
          (c >= 'a' && c <= 'z') ||
          (c >= '0' && c <= '9'));
  }

  return result;
}

/* convenience routines for when the input is null-terminated */

static char *
str_tolower_z(const char *buff, Oid collid __attribute__((__unused__)) )
{
  return asc_tolower(buff, strlen(buff));
}

static char *
str_toupper_z(const char *buff, Oid collid __attribute__((__unused__)) )
{
  return asc_toupper(buff, strlen(buff));
}

static char *
str_initcap_z(const char *buff, Oid collid __attribute__((__unused__)) )
{
  return asc_initcap(buff, strlen(buff));
}

static char *
asc_tolower_z(const char *buff)
{
  return asc_tolower(buff, strlen(buff));
}

static char *
asc_toupper_z(const char *buff)
{
  return asc_toupper(buff, strlen(buff));
}

/* asc_initcap_z is not currently needed */


/* ----------
 * Skip TM / th in FROM_CHAR
 *
 * If S_THth is on, skip two chars, assuming there are two available
 * ----------
 */
#define SKIP_THth(ptr, _suf) \
  do { \
    if (S_THth(_suf)) \
    { \
      if (*(ptr)) (ptr) += sizeof(char); \
      if (*(ptr)) (ptr) += sizeof(char); \
    } \
  } while (0)


/* ----------
 * Return true if next format picture is not digit value
 * ----------
 */
static bool
is_next_separator(FormatNode *n)
{
  if (n->type == NODE_TYPE_END)
    return false;

  if (n->type == NODE_TYPE_ACTION && S_THth(n->suffix))
    return true;

  /*
   * Next node
   */
  n++;

  /* end of format string is treated like a non-digit separator */
  if (n->type == NODE_TYPE_END)
    return true;

  if (n->type == NODE_TYPE_ACTION)
  {
    if (n->key->is_digit)
      return false;

    return true;
  }
  else if (n->character[1] == '\0' &&
       isdigit((unsigned char) n->character[0]))
    return false;

  return true;        /* some non-digit input (separator) */
}


static int
adjust_partial_year_to_2020(int year)
{
  /*
   * Adjust all dates toward 2020; this is effectively what happens when we
   * assume '70' is 1970 and '69' is 2069.
   */
  /* Force 0-69 into the 2000's */
  if (year < 70)
    return year + 2000;
  /* Force 70-99 into the 1900's */
  else if (year < 100)
    return year + 1900;
  /* Force 100-519 into the 2000's */
  else if (year < 520)
    return year + 2000;
  /* Force 520-999 into the 1000's */
  else if (year < 1000)
    return year + 1000;
  else
    return year;
}


static int
strspace_len(const char *str)
{
  int      len = 0;

  while (*str && isspace((unsigned char) *str))
  {
    str++;
    len++;
  }
  return len;
}

/*
 * Set the date mode of a from-char conversion.
 *
 * Puke if the date mode has already been set, and the caller attempts to set
 * it to a conflicting mode.
 *
 * If 'have_error' is NULL, then errors are thrown, else '*have_error' is set.
 */
static void
from_char_set_mode(TmFromChar *tmfc, const FromCharDateMode mode, bool *have_error)
{
  if (mode != FROM_CHAR_DATE_NONE)
  {
    if (tmfc->mode == FROM_CHAR_DATE_NONE)
      tmfc->mode = mode;
    else if (tmfc->mode != mode)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "invalid combination of date conventions");
      return;
    }
  }

on_error:
  return;
}

/*
 * Set the integer pointed to by 'dest' to the given value.
 *
 * Puke if the destination integer has previously been set to some other
 * non-zero value.
 *
 * If 'have_error' is NULL, then errors are thrown, else '*have_error' is set.
 */
static void
from_char_set_int(int *dest, const int value, const FormatNode *node,
          bool *have_error)
{
  if (*dest != 0 && *dest != value)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "conflicting values for \"%s\" field in formatting string",
      node->key->name);
    return;
  }

  *dest = value;

on_error:
  return;
}

/*
 * Read a single integer from the source string, into the int pointed to by
 * 'dest'. If 'dest' is NULL, the result is discarded.
 *
 * In fixed-width mode (the node does not have the FM suffix), consume at most
 * 'len' characters.  However, any leading whitespace isn't counted in 'len'.
 *
 * We use strtol() to recover the integer value from the source string, in
 * accordance with the given FormatNode.
 *
 * If the conversion completes successfully, src will have been advanced to
 * point at the character immediately following the last character used in the
 * conversion.
 *
 * Return the number of characters consumed.
 *
 * Note that from_char_parse_int() provides a more convenient wrapper where
 * the length of the field is the same as the length of the format keyword (as
 * with DD and MI).
 *
 * If 'have_error' is NULL, then errors are thrown, else '*have_error' is set
 * and -1 is returned.
 */
static int
from_char_parse_int_len(int *dest, const char **src, const int len,
  FormatNode *node, bool *have_error)
{
  long    result;
  char *copy;   // char copy[DCH_MAX_ITEM_SIZ + 1];
  const char *init = *src;
  int      used;

  /*
   * Skip any whitespace before parsing the integer.
   */
  *src += strspace_len(*src);

  Assert(len <= DCH_MAX_ITEM_SIZ);

  /*
   * MEOS note: palloc before strncpy otherwise *copy is not re-init
   * for next from_char_parse_int_len( ) call
   * and chars >n are not removed from str
   */
  copy = palloc(DCH_MAX_ITEM_SIZ + 1);
  memcpy(copy, *src, len);
  copy[len] = '\0';
  used = strlen(*src);

  if (S_FM(node->suffix) || is_next_separator(node))
  {
    /*
     * This node is in Fill Mode, or the next node is known to be a
     * non-digit value, so we just slurp as many characters as we can get.
     */
    char     *endptr;

    errno = 0;
    result = strtol(init, &endptr, 10);
    *src = endptr;
  }
  else
  {
    /*
     * We need to pull exactly the number of characters given in 'len' out
     * of the string, and convert those.
     */
    char     *last;

    if (used < len)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "source string too short for \"%s\" formatting field",
        node->key->name);
      goto on_error;
    }

    errno = 0;
    result = strtol(copy, &last, 10);
    used = last - copy;

    if (used > 0 && used < len)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "invalid value \"%s\" for \"%s\"", copy, node->key->name);
      goto on_error;
    }

    *src += used;
  }

  if (*src == init)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "invalid value \"%s\" for \"%s\"", copy, node->key->name);
    goto on_error;
  }

  if (errno == ERANGE || result < INT_MIN || result > INT_MAX)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "value for \"%s\" in source string is out of range", node->key->name);
    goto on_error;
  }

  if (dest != NULL)
  {
    from_char_set_int(dest, (int) result, node, have_error);
    CHECK_ERROR;
  }
  pfree(copy);
  return *src - init;

on_error:
  pfree(copy);
  return -1;
}

/*
 * Call from_char_parse_int_len(), using the length of the format keyword as
 * the expected length of the field.
 *
 * Don't call this function if the field differs in length from the format
 * keyword (as with HH24; the keyword length is 4, but the field length is 2).
 * In such cases, call from_char_parse_int_len() instead to specify the
 * required length explicitly.
 */
static int
from_char_parse_int(int *dest, const char **src, FormatNode *node, bool *have_error)
{
  return from_char_parse_int_len(dest, src, node->key->len, node, have_error);
}

/*
 * Sequentially search null-terminated "array" for a case-insensitive match
 * to the initial character(s) of "name".
 *
 * Returns array index of match, or -1 for no match.
 *
 * *len is set to the length of the match, or 0 for no match.
 *
 * Case-insensitivity is defined per pg_ascii_tolower, so this is only
 * suitable for comparisons to ASCII strings.
 */
static int
seq_search_ascii(const char *name, const char *const *array, int *len)
{
  unsigned char firstc;
  const char *const *a;

  *len = 0;

  /* empty string can't match anything */
  if (!*name)
    return -1;

  /* we handle first char specially to gain some speed */
  firstc = pg_ascii_tolower((unsigned char) *name);

  for (a = array; *a != NULL; a++)
  {
    const char *p;
    const char *n;

    /* compare first chars */
    if (pg_ascii_tolower((unsigned char) **a) != firstc)
      continue;

    /* compare rest of string */
    for (p = *a + 1, n = name + 1;; p++, n++)
    {
      /* return success if we matched whole array entry */
      if (*p == '\0')
      {
        *len = n - name;
        return a - array;
      }
      /* else, must have another character in "name" ... */
      if (*n == '\0')
        break;
      /* ... and it must match */
      if (pg_ascii_tolower((unsigned char) *p) !=
        pg_ascii_tolower((unsigned char) *n))
        break;
    }
  }

  return -1;
}

/*
 * Sequentially search an array of possibly non-English words for
 * a case-insensitive match to the initial character(s) of "name".
 *
 * This has the same API as seq_search_ascii(), but we use a more general
 * case-folding transformation to achieve case-insensitivity.  Case folding
 * is done per the rules of the collation identified by "collid".
 *
 * The array is treated as const, but we don't declare it that way because
 * the arrays exported by pg_locale.c aren't const.
 */
static int
seq_search_localized(const char *name, char **array, int *len, Oid collid)
{
  char    **a;
  char     *upper_name;
  char     *lower_name;

  *len = 0;

  /* empty string can't match anything */
  if (!*name)
    return -1;

  /*
   * The case-folding processing done below is fairly expensive, so before
   * doing that, make a quick pass to see if there is an exact match.
   */
  for (a = array; *a != NULL; a++)
  {
    int      element_len = strlen(*a);

    if (strncmp(name, *a, element_len) == 0)
    {
      *len = element_len;
      return a - array;
    }
  }

  /*
   * Fold to upper case, then to lower case, so that we can match reliably
   * even in languages in which case conversions are not injective.
   */
  upper_name = asc_toupper(unconstify(char *, name), strlen(name));
  lower_name = asc_tolower(upper_name, strlen(upper_name));
  pfree(upper_name);

  for (a = array; *a != NULL; a++)
  {
    char     *upper_element;
    char     *lower_element;
    int      element_len;

    /* Likewise upper/lower-case array element */
    upper_element = asc_toupper(*a, strlen(*a));
    lower_element = asc_tolower(upper_element, strlen(upper_element));
    pfree(upper_element);
    element_len = strlen(lower_element);

    /* Match? */
    if (strncmp(lower_name, lower_element, element_len) == 0)
    {
      *len = element_len;
      pfree(lower_element);
      pfree(lower_name);
      return a - array;
    }
    pfree(lower_element);
  }

  pfree(lower_name);
  return -1;
}

/*
 * scanner_isspace() --- return true if flex scanner considers char whitespace
 *
 * This should be used instead of the potentially locale-dependent isspace()
 * function when it's important to match the lexer's behavior.
 *
 * In principle we might need similar functions for isalnum etc, but for the
 * moment only isspace seems needed.
 */
bool
scanner_isspace(char ch)
{
  /* This must match scan.l's list of {space} characters */
  if (ch == ' ' ||
    ch == '\t' ||
    ch == '\n' ||
    ch == '\r' ||
    ch == '\f')
    return true;
  return false;
}

/*
 * Perform a sequential search in 'array' (or 'localized_array', if that's
 * not NULL) for an entry matching the first character(s) of the 'src'
 * string case-insensitively.
 *
 * The 'array' is presumed to be English words (all-ASCII), but
 * if 'localized_array' is supplied, that might be non-English
 * so we need a more expensive case-folding transformation
 * (which will follow the rules of the collation 'collid').
 *
 * If a match is found, copy the array index of the match into the integer
 * pointed to by 'dest', advance 'src' to the end of the part of the string
 * which matched, and return the number of characters consumed.
 *
 * If the string doesn't match, throw an error if 'have_error' is NULL,
 * otherwise set '*have_error' and return -1.
 *
 * 'node' is used only for error reports: node->key->name identifies the
 * field type we were searching for.
 */
static int
from_char_seq_search(int *dest, const char **src, const char *const *array,
           char **localized_array, Oid collid,
           FormatNode *node, bool *have_error)
{
  int      len;

  if (localized_array == NULL)
    *dest = seq_search_ascii(*src, array, &len);
  else
    *dest = seq_search_localized(*src, localized_array, &len, collid);

  if (len <= 0)
  {
    /*
     * In the error report, truncate the string at the next whitespace (if
     * any) to avoid including irrelevant data.
     */
    char     *copy = pstrdup(*src);
    char     *c;

    for (c = copy; *c; c++)
    {
      if (scanner_isspace(*c))
      {
        *c = '\0';
        break;
      }
    }

    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "invalid value \"%s\" for \"%s\"", copy, node->key->name);
    return -1;
  }
  *src += len;
  return len;

on_error:
  return -1;
}

/* ----------
 * Process a TmToChar struct as denoted by a list of FormatNodes.
 * The formatted data is written to the string pointed to by 'out'.
 * ----------
 */
static void
DCH_to_char(FormatNode *node, bool is_interval, TmToChar *in, char *out,
  Oid collid)
{
  FormatNode *n;
  char     *s;
  struct pg_tm *tm = &in->tm;
  int      i;

  /* cache localized days and months */
  // cache_locale_time();

  s = out;
  for (n = node; n->type != NODE_TYPE_END; n++)
  {
    if (n->type != NODE_TYPE_ACTION)
    {
      strcpy(s, n->character);
      s += strlen(s);
      continue;
    }

    switch (n->key->id)
    {
      case DCH_A_M:
      case DCH_P_M:
        strcpy(s, (tm->tm_hour % HOURS_PER_DAY >= HOURS_PER_DAY / 2)
             ? P_M_STR : A_M_STR);
        s += strlen(s);
        break;
      case DCH_AM:
      case DCH_PM:
        strcpy(s, (tm->tm_hour % HOURS_PER_DAY >= HOURS_PER_DAY / 2)
             ? PM_STR : AM_STR);
        s += strlen(s);
        break;
      case DCH_a_m:
      case DCH_p_m:
        strcpy(s, (tm->tm_hour % HOURS_PER_DAY >= HOURS_PER_DAY / 2)
             ? p_m_STR : a_m_STR);
        s += strlen(s);
        break;
      case DCH_am:
      case DCH_pm:
        strcpy(s, (tm->tm_hour % HOURS_PER_DAY >= HOURS_PER_DAY / 2)
             ? pm_STR : am_STR);
        s += strlen(s);
        break;
      case DCH_HH:
      case DCH_HH12:

        /*
         * display time as shown on a 12-hour clock, even for
         * intervals
         */
        sprintf(s, "%0*d", S_FM(n->suffix) ? 0 : (tm->tm_hour >= 0) ? 2 : 3,
            tm->tm_hour % (HOURS_PER_DAY / 2) == 0 ? HOURS_PER_DAY / 2 :
            tm->tm_hour % (HOURS_PER_DAY / 2));
        if (S_THth(n->suffix))
          str_numth(s, s, S_TH_TYPE(n->suffix));
        s += strlen(s);
        break;
      case DCH_HH24:
        sprintf(s, "%0*d", S_FM(n->suffix) ? 0 : (tm->tm_hour >= 0) ? 2 : 3,
            tm->tm_hour);
        if (S_THth(n->suffix))
          str_numth(s, s, S_TH_TYPE(n->suffix));
        s += strlen(s);
        break;
      case DCH_MI:
        sprintf(s, "%0*d", S_FM(n->suffix) ? 0 : (tm->tm_min >= 0) ? 2 : 3,
            tm->tm_min);
        if (S_THth(n->suffix))
          str_numth(s, s, S_TH_TYPE(n->suffix));
        s += strlen(s);
        break;
      case DCH_SS:
        sprintf(s, "%0*d", S_FM(n->suffix) ? 0 : (tm->tm_sec >= 0) ? 2 : 3,
            tm->tm_sec);
        if (S_THth(n->suffix))
          str_numth(s, s, S_TH_TYPE(n->suffix));
        s += strlen(s);
        break;

#define DCH_to_char_fsec(frac_fmt, frac_val) \
        sprintf(s, frac_fmt, (int) (frac_val)); \
        if (S_THth(n->suffix)) \
          str_numth(s, s, S_TH_TYPE(n->suffix)); \
        s += strlen(s)

      case DCH_FF1:    /* tenth of second */
        DCH_to_char_fsec("%01d", in->fsec / 100000);
        break;
      case DCH_FF2:    /* hundredth of second */
        DCH_to_char_fsec("%02d", in->fsec / 10000);
        break;
      case DCH_FF3:
      case DCH_MS:    /* millisecond */
        DCH_to_char_fsec("%03d", in->fsec / 1000);
        break;
      case DCH_FF4:    /* tenth of a millisecond */
        DCH_to_char_fsec("%04d", in->fsec / 100);
        break;
      case DCH_FF5:    /* hundredth of a millisecond */
        DCH_to_char_fsec("%05d", in->fsec / 10);
        break;
      case DCH_FF6:
      case DCH_US:    /* microsecond */
        DCH_to_char_fsec("%06d", in->fsec);
        break;
#undef DCH_to_char_fsec
      case DCH_SSSS:
        sprintf(s, "%d", tm->tm_hour * SECS_PER_HOUR +
            tm->tm_min * SECS_PER_MINUTE +
            tm->tm_sec);
        if (S_THth(n->suffix))
          str_numth(s, s, S_TH_TYPE(n->suffix));
        s += strlen(s);
        break;
      case DCH_tz:
        INVALID_FOR_INTERVAL;
        if (tmtcTzn(in))
        {
          /* We assume here that timezone names aren't localized */
          char     *p = asc_tolower_z(tmtcTzn(in));

          strcpy(s, p);
          pfree(p);
          s += strlen(s);
        }
        break;
      case DCH_TZ:
        INVALID_FOR_INTERVAL;
        if (tmtcTzn(in))
        {
          strcpy(s, tmtcTzn(in));
          s += strlen(s);
        }
        break;
      case DCH_TZH:
        INVALID_FOR_INTERVAL;
        sprintf(s, "%c%02d",
            (tm->tm_gmtoff >= 0) ? '+' : '-',
            abs((int) tm->tm_gmtoff) / SECS_PER_HOUR);
        s += strlen(s);
        break;
      case DCH_TZM:
        INVALID_FOR_INTERVAL;
        sprintf(s, "%02d",
            (abs((int) tm->tm_gmtoff) % SECS_PER_HOUR) / SECS_PER_MINUTE);
        s += strlen(s);
        break;
      case DCH_OF:
        INVALID_FOR_INTERVAL;
        sprintf(s, "%c%0*d",
            (tm->tm_gmtoff >= 0) ? '+' : '-',
            S_FM(n->suffix) ? 0 : 2,
            abs((int) tm->tm_gmtoff) / SECS_PER_HOUR);
        s += strlen(s);
        if (abs((int) tm->tm_gmtoff) % SECS_PER_HOUR != 0)
        {
          sprintf(s, ":%02d",
              (abs((int) tm->tm_gmtoff) % SECS_PER_HOUR) / SECS_PER_MINUTE);
          s += strlen(s);
        }
        break;
      case DCH_A_D:
      case DCH_B_C:
        INVALID_FOR_INTERVAL;
        strcpy(s, (tm->tm_year <= 0 ? B_C_STR : A_D_STR));
        s += strlen(s);
        break;
      case DCH_AD:
      case DCH_BC:
        INVALID_FOR_INTERVAL;
        strcpy(s, (tm->tm_year <= 0 ? BC_STR : AD_STR));
        s += strlen(s);
        break;
      case DCH_a_d:
      case DCH_b_c:
        INVALID_FOR_INTERVAL;
        strcpy(s, (tm->tm_year <= 0 ? b_c_STR : a_d_STR));
        s += strlen(s);
        break;
      case DCH_ad:
      case DCH_bc:
        INVALID_FOR_INTERVAL;
        strcpy(s, (tm->tm_year <= 0 ? bc_STR : ad_STR));
        s += strlen(s);
        break;
      case DCH_MONTH:
        INVALID_FOR_INTERVAL;
        if (!tm->tm_mon)
          break;
        if (S_TM(n->suffix))
        {
          char *str = str_toupper_z(months_full[tm->tm_mon - 1], collid);

          if (strlen(str) <= (n->key->len + TM_SUFFIX_LEN) * DCH_MAX_ITEM_SIZ)
            strcpy(s, str);
          else
          {
            meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
              "localized string format value too long");
            return;
          }
        }
        else
          sprintf(s, "%*s", S_FM(n->suffix) ? 0 : -9,
              asc_toupper_z(months_full[tm->tm_mon - 1]));
        s += strlen(s);
        break;
      case DCH_Month:
        INVALID_FOR_INTERVAL;
        if (!tm->tm_mon)
          break;
        if (S_TM(n->suffix))
        {
          char     *str = str_initcap_z(months_full[tm->tm_mon - 1], collid);

          if (strlen(str) <= (n->key->len + TM_SUFFIX_LEN) * DCH_MAX_ITEM_SIZ)
            strcpy(s, str);
          else
          {
            meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
              "localized string format value too long");
            return;
          }
        }
        else
          sprintf(s, "%*s", S_FM(n->suffix) ? 0 : -9,
              months_full[tm->tm_mon - 1]);
        s += strlen(s);
        break;
      case DCH_month:
        INVALID_FOR_INTERVAL;
        if (!tm->tm_mon)
          break;
        if (S_TM(n->suffix))
        {
          char     *str = str_tolower_z(months_full[tm->tm_mon - 1], collid);

          if (strlen(str) <= (n->key->len + TM_SUFFIX_LEN) * DCH_MAX_ITEM_SIZ)
            strcpy(s, str);
          else
          {
            meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
              "localized string format value too long");
            return;
          }
        }
        else
          sprintf(s, "%*s", S_FM(n->suffix) ? 0 : -9,
              asc_tolower_z(months_full[tm->tm_mon - 1]));
        s += strlen(s);
        break;
      case DCH_MON:
        INVALID_FOR_INTERVAL;
        if (!tm->tm_mon)
          break;
        if (S_TM(n->suffix))
        {
          char     *str = str_toupper_z(months[tm->tm_mon - 1], collid);

          if (strlen(str) <= (n->key->len + TM_SUFFIX_LEN) * DCH_MAX_ITEM_SIZ)
            strcpy(s, str);
          else
          {
            meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
              "localized string format value too long");
            return;
          }
        }
        else
          strcpy(s, asc_toupper_z(months[tm->tm_mon - 1]));
        s += strlen(s);
        break;
      case DCH_Mon:
        INVALID_FOR_INTERVAL;
        if (!tm->tm_mon)
          break;
        if (S_TM(n->suffix))
        {
          char     *str = str_initcap_z(months[tm->tm_mon - 1], collid);

          if (strlen(str) <= (n->key->len + TM_SUFFIX_LEN) * DCH_MAX_ITEM_SIZ)
            strcpy(s, str);
          else
          {
            meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
              "localized string format value too long");
            return;
          }
        }
        else
          strcpy(s, months[tm->tm_mon - 1]);
        s += strlen(s);
        break;
      case DCH_mon:
        INVALID_FOR_INTERVAL;
        if (!tm->tm_mon)
          break;
        if (S_TM(n->suffix))
        {
          char     *str = str_tolower_z(months[tm->tm_mon - 1], collid);

          if (strlen(str) <= (n->key->len + TM_SUFFIX_LEN) * DCH_MAX_ITEM_SIZ)
            strcpy(s, str);
          else
          {
            meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
              "localized string format value too long");
            return;
          }
        }
        else
          strcpy(s, asc_tolower_z(months[tm->tm_mon - 1]));
        s += strlen(s);
        break;
      case DCH_MM:
        sprintf(s, "%0*d", S_FM(n->suffix) ? 0 : (tm->tm_mon >= 0) ? 2 : 3,
            tm->tm_mon);
        if (S_THth(n->suffix))
          str_numth(s, s, S_TH_TYPE(n->suffix));
        s += strlen(s);
        break;
      case DCH_DAY:
        INVALID_FOR_INTERVAL;
        if (S_TM(n->suffix))
        {
          char     *str = str_toupper_z(days[tm->tm_wday], collid);

          if (strlen(str) <= (n->key->len + TM_SUFFIX_LEN) * DCH_MAX_ITEM_SIZ)
            strcpy(s, str);
          else
          {
            meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
              "localized string format value too long");
            return;
          }
        }
        else
          sprintf(s, "%*s", S_FM(n->suffix) ? 0 : -9,
              asc_toupper_z(days[tm->tm_wday]));
        s += strlen(s);
        break;
      case DCH_Day:
        INVALID_FOR_INTERVAL;
        if (S_TM(n->suffix))
        {
          char     *str = str_initcap_z(days[tm->tm_wday], collid);

          if (strlen(str) <= (n->key->len + TM_SUFFIX_LEN) * DCH_MAX_ITEM_SIZ)
            strcpy(s, str);
          else
          {
            meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
              "localized string format value too long");
            return;
          }
        }
        else
          sprintf(s, "%*s", S_FM(n->suffix) ? 0 : -9,
              days[tm->tm_wday]);
        s += strlen(s);
        break;
      case DCH_day:
        INVALID_FOR_INTERVAL;
        if (S_TM(n->suffix))
        {
          char     *str = str_tolower_z(days[tm->tm_wday], collid);

          if (strlen(str) <= (n->key->len + TM_SUFFIX_LEN) * DCH_MAX_ITEM_SIZ)
            strcpy(s, str);
          else
          {
            meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
              "localized string format value too long");
            return;
          }
        }
        else
          sprintf(s, "%*s", S_FM(n->suffix) ? 0 : -9,
              asc_tolower_z(days[tm->tm_wday]));
        s += strlen(s);
        break;
      case DCH_DY:
        INVALID_FOR_INTERVAL;
        if (S_TM(n->suffix))
        {
          char     *str = str_toupper_z(days_short[tm->tm_wday], collid);

          if (strlen(str) <= (n->key->len + TM_SUFFIX_LEN) * DCH_MAX_ITEM_SIZ)
            strcpy(s, str);
          else
          {
            meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
              "localized string format value too long");
            return;
          }
        }
        else
          strcpy(s, asc_toupper_z(days_short[tm->tm_wday]));
        s += strlen(s);
        break;
      case DCH_Dy:
        INVALID_FOR_INTERVAL;
        if (S_TM(n->suffix))
        {
          char     *str = str_initcap_z(days_short[tm->tm_wday], collid);

          if (strlen(str) <= (n->key->len + TM_SUFFIX_LEN) * DCH_MAX_ITEM_SIZ)
            strcpy(s, str);
          else
          {
            meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
              "localized string format value too long");
            return;
          }
        }
        else
          strcpy(s, days_short[tm->tm_wday]);
        s += strlen(s);
        break;
      case DCH_dy:
        INVALID_FOR_INTERVAL;
        if (S_TM(n->suffix))
        {
          char     *str = str_tolower_z(days_short[tm->tm_wday], collid);

          if (strlen(str) <= (n->key->len + TM_SUFFIX_LEN) * DCH_MAX_ITEM_SIZ)
            strcpy(s, str);
          else
          {
            meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
              "localized string format value too long");
            return;
          }
        }
        else
          strcpy(s, asc_tolower_z(days_short[tm->tm_wday]));
        s += strlen(s);
        break;
      case DCH_DDD:
      case DCH_IDDD:
        sprintf(s, "%0*d", S_FM(n->suffix) ? 0 : 3,
            (n->key->id == DCH_DDD) ?
            tm->tm_yday :
            date2isoyearday(tm->tm_year, tm->tm_mon, tm->tm_mday));
        if (S_THth(n->suffix))
          str_numth(s, s, S_TH_TYPE(n->suffix));
        s += strlen(s);
        break;
      case DCH_DD:
        sprintf(s, "%0*d", S_FM(n->suffix) ? 0 : 2, tm->tm_mday);
        if (S_THth(n->suffix))
          str_numth(s, s, S_TH_TYPE(n->suffix));
        s += strlen(s);
        break;
      case DCH_D:
        INVALID_FOR_INTERVAL;
        sprintf(s, "%d", tm->tm_wday + 1);
        if (S_THth(n->suffix))
          str_numth(s, s, S_TH_TYPE(n->suffix));
        s += strlen(s);
        break;
      case DCH_ID:
        INVALID_FOR_INTERVAL;
        sprintf(s, "%d", (tm->tm_wday == 0) ? 7 : tm->tm_wday);
        if (S_THth(n->suffix))
          str_numth(s, s, S_TH_TYPE(n->suffix));
        s += strlen(s);
        break;
      case DCH_WW:
        sprintf(s, "%0*d", S_FM(n->suffix) ? 0 : 2,
            (tm->tm_yday - 1) / 7 + 1);
        if (S_THth(n->suffix))
          str_numth(s, s, S_TH_TYPE(n->suffix));
        s += strlen(s);
        break;
      case DCH_IW:
        sprintf(s, "%0*d", S_FM(n->suffix) ? 0 : 2,
            date2isoweek(tm->tm_year, tm->tm_mon, tm->tm_mday));
        if (S_THth(n->suffix))
          str_numth(s, s, S_TH_TYPE(n->suffix));
        s += strlen(s);
        break;
      case DCH_Q:
        if (!tm->tm_mon)
          break;
        sprintf(s, "%d", (tm->tm_mon - 1) / 3 + 1);
        if (S_THth(n->suffix))
          str_numth(s, s, S_TH_TYPE(n->suffix));
        s += strlen(s);
        break;
      case DCH_CC:
        if (is_interval)  /* straight calculation */
          i = tm->tm_year / 100;
        else
        {
          if (tm->tm_year > 0)
            /* Century 20 == 1901 - 2000 */
            i = (tm->tm_year - 1) / 100 + 1;
          else
            /* Century 6BC == 600BC - 501BC */
            i = tm->tm_year / 100 - 1;
        }
        if (i <= 99 && i >= -99)
          sprintf(s, "%0*d", S_FM(n->suffix) ? 0 : (i >= 0) ? 2 : 3, i);
        else
          sprintf(s, "%d", i);
        if (S_THth(n->suffix))
          str_numth(s, s, S_TH_TYPE(n->suffix));
        s += strlen(s);
        break;
      case DCH_Y_YYY:
        i = ADJUST_YEAR(tm->tm_year, is_interval) / 1000;
        sprintf(s, "%d,%03d", i,
            ADJUST_YEAR(tm->tm_year, is_interval) - (i * 1000));
        if (S_THth(n->suffix))
          str_numth(s, s, S_TH_TYPE(n->suffix));
        s += strlen(s);
        break;
      case DCH_YYYY:
      case DCH_IYYY:
        sprintf(s, "%0*d",
            S_FM(n->suffix) ? 0 :
            (ADJUST_YEAR(tm->tm_year, is_interval) >= 0) ? 4 : 5,
            (n->key->id == DCH_YYYY ?
             ADJUST_YEAR(tm->tm_year, is_interval) :
             ADJUST_YEAR(date2isoyear(tm->tm_year,
                          tm->tm_mon,
                          tm->tm_mday),
                   is_interval)));
        if (S_THth(n->suffix))
          str_numth(s, s, S_TH_TYPE(n->suffix));
        s += strlen(s);
        break;
      case DCH_YYY:
      case DCH_IYY:
        sprintf(s, "%0*d",
            S_FM(n->suffix) ? 0 :
            (ADJUST_YEAR(tm->tm_year, is_interval) >= 0) ? 3 : 4,
            (n->key->id == DCH_YYY ?
             ADJUST_YEAR(tm->tm_year, is_interval) :
             ADJUST_YEAR(date2isoyear(tm->tm_year,
                          tm->tm_mon,
                          tm->tm_mday),
                   is_interval)) % 1000);
        if (S_THth(n->suffix))
          str_numth(s, s, S_TH_TYPE(n->suffix));
        s += strlen(s);
        break;
      case DCH_YY:
      case DCH_IY:
        sprintf(s, "%0*d",
            S_FM(n->suffix) ? 0 :
            (ADJUST_YEAR(tm->tm_year, is_interval) >= 0) ? 2 : 3,
            (n->key->id == DCH_YY ?
             ADJUST_YEAR(tm->tm_year, is_interval) :
             ADJUST_YEAR(date2isoyear(tm->tm_year,
                          tm->tm_mon,
                          tm->tm_mday),
                   is_interval)) % 100);
        if (S_THth(n->suffix))
          str_numth(s, s, S_TH_TYPE(n->suffix));
        s += strlen(s);
        break;
      case DCH_Y:
      case DCH_I:
        sprintf(s, "%1d",
            (n->key->id == DCH_Y ?
             ADJUST_YEAR(tm->tm_year, is_interval) :
             ADJUST_YEAR(date2isoyear(tm->tm_year,
                          tm->tm_mon,
                          tm->tm_mday),
                   is_interval)) % 10);
        if (S_THth(n->suffix))
          str_numth(s, s, S_TH_TYPE(n->suffix));
        s += strlen(s);
        break;
      case DCH_RM:
        /* FALLTHROUGH */
      case DCH_rm:

        /*
         * For intervals, values like '12 month' will be reduced to 0
         * month and some years.  These should be processed.
         */
        if (!tm->tm_mon && !tm->tm_year)
          break;
        else
        {
          int      mon = 0;
          const char *const *months;

          if (n->key->id == DCH_RM)
            months = rm_months_upper;
          else
            months = rm_months_lower;

          /*
           * Compute the position in the roman-numeral array.  Note
           * that the contents of the array are reversed, December
           * being first and January last.
           */
          if (tm->tm_mon == 0)
          {
            /*
             * This case is special, and tracks the case of full
             * interval years.
             */
            mon = tm->tm_year >= 0 ? 0 : MONTHS_PER_YEAR - 1;
          }
          else if (tm->tm_mon < 0)
          {
            /*
             * Negative case.  In this case, the calculation is
             * reversed, where -1 means December, -2 November,
             * etc.
             */
            mon = -1 * (tm->tm_mon + 1);
          }
          else
          {
            /*
             * Common case, with a strictly positive value.  The
             * position in the array matches with the value of
             * tm_mon.
             */
            mon = MONTHS_PER_YEAR - tm->tm_mon;
          }

          sprintf(s, "%*s", S_FM(n->suffix) ? 0 : -4,
              months[mon]);
          s += strlen(s);
        }
        break;
      case DCH_W:
        sprintf(s, "%d", (tm->tm_mday - 1) / 7 + 1);
        if (S_THth(n->suffix))
          str_numth(s, s, S_TH_TYPE(n->suffix));
        s += strlen(s);
        break;
      case DCH_J:
        sprintf(s, "%d", date2j(tm->tm_year, tm->tm_mon, tm->tm_mday));
        if (S_THth(n->suffix))
          str_numth(s, s, S_TH_TYPE(n->suffix));
        s += strlen(s);
        break;
    }
  }

  *s = '\0';
  return;
}

/*
 * Process the string 'in' as denoted by the array of FormatNodes 'node[]'.
 * The TmFromChar struct pointed to by 'out' is populated with the results.
 *
 * 'collid' identifies the collation to use, if needed.
 * 'std' specifies standard parsing mode.
 * If 'have_error' is NULL, then errors are thrown, else '*have_error' is set.
 *
 * Note: we currently don't have any to_interval() function, so there
 * is no need here for INVALID_FOR_INTERVAL checks.
 */
static void
DCH_from_char(FormatNode *node, const char *in, TmFromChar *out,
        Oid collid, bool std, bool *have_error)
{
  FormatNode *n;
  const char *s;
  int      len,
        value;
  bool    fx_mode = std;

  /* number of extra skipped characters (more than given in format string) */
  int      extra_skip = 0;

  /* cache localized days and months */
  // cache_locale_time();

  for (n = node, s = in; n->type != NODE_TYPE_END && *s != '\0'; n++)
  {
    /*
     * Ignore spaces at the beginning of the string and before fields when
     * not in FX (fixed width) mode.
     */
    if (!fx_mode && (n->type != NODE_TYPE_ACTION || n->key->id != DCH_FX) &&
      (n->type == NODE_TYPE_ACTION || n == node))
    {
      while (*s != '\0' && isspace((unsigned char) *s))
      {
        s++;
        extra_skip++;
      }
    }

    if (n->type == NODE_TYPE_SPACE || n->type == NODE_TYPE_SEPARATOR)
    {
      if (std)
      {
        /*
         * Standard mode requires strict matching between format
         * string separators/spaces and input string.
         */
        Assert(n->character[0] && !n->character[1]);

        if (*s == n->character[0])
          s++;
        else
        {
          meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
            "unmatched format separator \"%c\"",  n->character[0]);
          return;
        }
      }
      else if (!fx_mode)
      {
        /*
         * In non FX (fixed format) mode one format string space or
         * separator match to one space or separator in input string.
         * Or match nothing if there is no space or separator in the
         * current position of input string.
         */
        extra_skip--;
        if (isspace((unsigned char) *s) || is_separator_char(s))
        {
          s++;
          extra_skip++;
        }
      }
      else
      {
        /*
         * In FX mode, on format string space or separator we consume
         * exactly one character from input string.  Notice we don't
         * insist that the consumed character match the format's
         * character.
         */
        s += sizeof(char);
      }
      continue;
    }
    else if (n->type != NODE_TYPE_ACTION)
    {
      /*
       * Text character, so consume one character from input string.
       * Notice we don't insist that the consumed character match the
       * format's character.
       */
      if (!fx_mode)
      {
        /*
         * In non FX mode we might have skipped some extra characters
         * (more than specified in format string) before.  In this
         * case we don't skip input string character, because it might
         * be part of field.
         */
        if (extra_skip > 0)
          extra_skip--;
        else
          s += sizeof(char);
      }
      else
      {
        int      chlen = sizeof(char);

        /*
         * Standard mode requires strict match of format characters.
         */
        if (std && n->type == NODE_TYPE_CHAR &&
          strncmp(s, n->character, chlen) != 0)
        {
          meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
            "unmatched format character \"%s\"", n->character);
          return;
        }

        s += chlen;
      }
      continue;
    }

    from_char_set_mode(out, n->key->date_mode, have_error);
    CHECK_ERROR;

    switch (n->key->id)
    {
      case DCH_FX:
        fx_mode = true;
        break;
      case DCH_A_M:
      case DCH_P_M:
      case DCH_a_m:
      case DCH_p_m:
        from_char_seq_search(&value, &s, ampm_strings_long,
                   NULL, InvalidOid,
                   n, have_error);
        CHECK_ERROR;
        from_char_set_int(&out->pm, value % 2, n, have_error);
        CHECK_ERROR;
        out->clock = CLOCK_12_HOUR;
        break;
      case DCH_AM:
      case DCH_PM:
      case DCH_am:
      case DCH_pm:
        from_char_seq_search(&value, &s, ampm_strings,
                   NULL, InvalidOid,
                   n, have_error);
        CHECK_ERROR;
        from_char_set_int(&out->pm, value % 2, n, have_error);
        CHECK_ERROR;
        out->clock = CLOCK_12_HOUR;
        break;
      case DCH_HH:
      case DCH_HH12:
        from_char_parse_int_len(&out->hh, &s, 2, n, have_error);
        CHECK_ERROR;
        out->clock = CLOCK_12_HOUR;
        SKIP_THth(s, n->suffix);
        break;
      case DCH_HH24:
        from_char_parse_int_len(&out->hh, &s, 2, n, have_error);
        CHECK_ERROR;
        SKIP_THth(s, n->suffix);
        break;
      case DCH_MI:
        from_char_parse_int(&out->mi, &s, n, have_error);
        CHECK_ERROR;
        SKIP_THth(s, n->suffix);
        break;
      case DCH_SS:
        from_char_parse_int(&out->ss, &s, n, have_error);
        CHECK_ERROR;
        SKIP_THth(s, n->suffix);
        break;
      case DCH_MS:    /* millisecond */
        len = from_char_parse_int_len(&out->ms, &s, 3, n, have_error);
        CHECK_ERROR;

        /*
         * 25 is 0.25 and 250 is 0.25 too; 025 is 0.025 and not 0.25
         */
        out->ms *= len == 1 ? 100 :
          len == 2 ? 10 : 1;

        SKIP_THth(s, n->suffix);
        break;
      case DCH_FF1:
      case DCH_FF2:
      case DCH_FF3:
      case DCH_FF4:
      case DCH_FF5:
      case DCH_FF6:
        out->ff = n->key->id - DCH_FF1 + 1;
        /* fall through */
      case DCH_US:    /* microsecond */
        len = from_char_parse_int_len(&out->us, &s,
                        n->key->id == DCH_US ? 6 :
                        out->ff, n, have_error);
        CHECK_ERROR;

        out->us *= len == 1 ? 100000 :
          len == 2 ? 10000 :
          len == 3 ? 1000 :
          len == 4 ? 100 :
          len == 5 ? 10 : 1;

        SKIP_THth(s, n->suffix);
        break;
      case DCH_SSSS:
        from_char_parse_int(&out->ssss, &s, n, have_error);
        CHECK_ERROR;
        SKIP_THth(s, n->suffix);
        break;
      case DCH_tz:
      case DCH_TZ:
      case DCH_OF:
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "formatting field \"%s\" is only supported in to_char",
                       n->key->name);
        return;
        // CHECK_ERROR;
        // break;
      case DCH_TZH:

        /*
         * Value of TZH might be negative.  And the issue is that we
         * might swallow minus sign as the separator.  So, if we have
         * skipped more characters than specified in the format
         * string, then we consider prepending last skipped minus to
         * TZH.
         */
        if (*s == '+' || *s == '-' || *s == ' ')
        {
          out->tzsign = *s == '-' ? -1 : +1;
          s++;
        }
        else
        {
          if (extra_skip > 0 && *(s - 1) == '-')
            out->tzsign = -1;
          else
            out->tzsign = +1;
        }

        from_char_parse_int_len(&out->tzh, &s, 2, n, have_error);
        CHECK_ERROR;
        break;
      case DCH_TZM:
        /* assign positive timezone sign if TZH was not seen before */
        if (!out->tzsign)
          out->tzsign = +1;
        from_char_parse_int_len(&out->tzm, &s, 2, n, have_error);
        CHECK_ERROR;
        break;
      case DCH_A_D:
      case DCH_B_C:
      case DCH_a_d:
      case DCH_b_c:
        from_char_seq_search(&value, &s, adbc_strings_long,
                   NULL, InvalidOid,
                   n, have_error);
        CHECK_ERROR;
        from_char_set_int(&out->bc, value % 2, n, have_error);
        CHECK_ERROR;
        break;
      case DCH_AD:
      case DCH_BC:
      case DCH_ad:
      case DCH_bc:
        from_char_seq_search(&value, &s, adbc_strings,
                   NULL, InvalidOid,
                   n, have_error);
        CHECK_ERROR;
        from_char_set_int(&out->bc, value % 2, n, have_error);
        CHECK_ERROR;
        break;
      case DCH_MONTH:
      case DCH_Month:
      case DCH_month:
        from_char_seq_search(&value, &s, months_full,
                   NULL,
                   collid,
                   n, have_error);
        CHECK_ERROR;
        from_char_set_int(&out->mm, value + 1, n, have_error);
        CHECK_ERROR;
        break;
      case DCH_MON:
      case DCH_Mon:
      case DCH_mon:
        from_char_seq_search(&value, &s, months,
                   NULL,
                   collid,
                   n, have_error);
        CHECK_ERROR;
        from_char_set_int(&out->mm, value + 1, n, have_error);
        CHECK_ERROR;
        break;
      case DCH_MM:
        from_char_parse_int(&out->mm, &s, n, have_error);
        CHECK_ERROR;
        SKIP_THth(s, n->suffix);
        break;
      case DCH_DAY:
      case DCH_Day:
      case DCH_day:
        from_char_seq_search(&value, &s, days,
                   NULL,
                   collid,
                   n, have_error);
        CHECK_ERROR;
        from_char_set_int(&out->d, value, n, have_error);
        CHECK_ERROR;
        out->d++;
        break;
      case DCH_DY:
      case DCH_Dy:
      case DCH_dy:
        from_char_seq_search(&value, &s, days_short,
                   NULL,
                   collid,
                   n, have_error);
        CHECK_ERROR;
        from_char_set_int(&out->d, value, n, have_error);
        CHECK_ERROR;
        out->d++;
        break;
      case DCH_DDD:
        from_char_parse_int(&out->ddd, &s, n, have_error);
        CHECK_ERROR;
        SKIP_THth(s, n->suffix);
        break;
      case DCH_IDDD:
        from_char_parse_int_len(&out->ddd, &s, 3, n, have_error);
        CHECK_ERROR;
        SKIP_THth(s, n->suffix);
        break;
      case DCH_DD:
        from_char_parse_int(&out->dd, &s, n, have_error);
        CHECK_ERROR;
        SKIP_THth(s, n->suffix);
        break;
      case DCH_D:
        from_char_parse_int(&out->d, &s, n, have_error);
        CHECK_ERROR;
        SKIP_THth(s, n->suffix);
        break;
      case DCH_ID:
        from_char_parse_int_len(&out->d, &s, 1, n, have_error);
        CHECK_ERROR;
        /* Shift numbering to match Gregorian where Sunday = 1 */
        if (++out->d > 7)
          out->d = 1;
        SKIP_THth(s, n->suffix);
        break;
      case DCH_WW:
      case DCH_IW:
        from_char_parse_int(&out->ww, &s, n, have_error);
        CHECK_ERROR;
        SKIP_THth(s, n->suffix);
        break;
      case DCH_Q:

        /*
         * We ignore 'Q' when converting to date because it is unclear
         * which date in the quarter to use, and some people specify
         * both quarter and month, so if it was honored it might
         * conflict with the supplied month. That is also why we don't
         * throw an error.
         *
         * We still parse the source string for an integer, but it
         * isn't stored anywhere in 'out'.
         */
        from_char_parse_int((int *) NULL, &s, n, have_error);
        CHECK_ERROR;
        SKIP_THth(s, n->suffix);
        break;
      case DCH_CC:
        from_char_parse_int(&out->cc, &s, n, have_error);
        CHECK_ERROR;
        SKIP_THth(s, n->suffix);
        break;
      case DCH_Y_YYY:
        {
          int      matched,
                years,
                millennia,
                nch;

          matched = sscanf(s, "%d,%03d%n", &millennia, &years, &nch);
          if (matched < 2)
          {
            meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
              "invalid input string for \"Y,YYY\"");
            return;
          }
          years += (millennia * 1000);
          from_char_set_int(&out->year, years, n, have_error);
          CHECK_ERROR;
          out->yysz = 4;
          s += nch;
          SKIP_THth(s, n->suffix);
        }
        break;
      case DCH_YYYY:
      case DCH_IYYY:
        from_char_parse_int(&out->year, &s, n, have_error);
        CHECK_ERROR;
        out->yysz = 4;
        SKIP_THth(s, n->suffix);
        break;
      case DCH_YYY:
      case DCH_IYY:
        len = from_char_parse_int(&out->year, &s, n, have_error);
        CHECK_ERROR;
        if (len < 4)
          out->year = adjust_partial_year_to_2020(out->year);
        out->yysz = 3;
        SKIP_THth(s, n->suffix);
        break;
      case DCH_YY:
      case DCH_IY:
        len = from_char_parse_int(&out->year, &s, n, have_error);
        CHECK_ERROR;
        if (len < 4)
          out->year = adjust_partial_year_to_2020(out->year);
        out->yysz = 2;
        SKIP_THth(s, n->suffix);
        break;
      case DCH_Y:
      case DCH_I:
        len = from_char_parse_int(&out->year, &s, n, have_error);
        CHECK_ERROR;
        if (len < 4)
          out->year = adjust_partial_year_to_2020(out->year);
        out->yysz = 1;
        SKIP_THth(s, n->suffix);
        break;
      case DCH_RM:
      case DCH_rm:
        from_char_seq_search(&value, &s, rm_months_lower,
                   NULL, InvalidOid,
                   n, have_error);
        CHECK_ERROR;
        from_char_set_int(&out->mm, MONTHS_PER_YEAR - value,
                  n, have_error);
        CHECK_ERROR;
        break;
      case DCH_W:
        from_char_parse_int(&out->w, &s, n, have_error);
        CHECK_ERROR;
        SKIP_THth(s, n->suffix);
        break;
      case DCH_J:
        from_char_parse_int(&out->j, &s, n, have_error);
        CHECK_ERROR;
        SKIP_THth(s, n->suffix);
        break;
    }

    /* Ignore all spaces after fields */
    if (!fx_mode)
    {
      extra_skip = 0;
      while (*s != '\0' && isspace((unsigned char) *s))
      {
        s++;
        extra_skip++;
      }
    }
  }

  /*
   * Standard parsing mode doesn't allow unmatched format patterns or
   * trailing characters in the input string.
   */
  if (std)
  {
    if (n->type != NODE_TYPE_END)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "input string is too short for datetime format");
      return;
    }

    while (*s != '\0' && isspace((unsigned char) *s))
      s++;

    if (*s != '\0')
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "trailing characters remain in input string after datetime format");
      return;
    }
  }

on_error:
  return;
}

/*
 * The invariant for DCH cache entry management is that DCHCounter is equal
 * to the maximum age value among the existing entries, and we increment it
 * whenever an access occurs.  If we approach overflow, deal with that by
 * halving all the age values, so that we retain a fairly accurate idea of
 * which entries are oldest.
 */
static inline void
DCH_prevent_counter_overflow(void)
{
  if (DCHCounter >= (INT_MAX - 1))
  {
    for (int i = 0; i < n_DCHCache; i++)
      DCHCache[i]->age >>= 1;
    DCHCounter >>= 1;
  }
}

/*
 * Get mask of date/time/zone components present in format nodes.
 *
 * If 'have_error' is NULL, then errors are thrown, else '*have_error' is set.
 */
static int
DCH_datetime_type(FormatNode *node, bool *have_error)
{
  FormatNode *n;
  int      flags = 0;

  for (n = node; n->type != NODE_TYPE_END; n++)
  {
    if (n->type != NODE_TYPE_ACTION)
      continue;

    switch (n->key->id)
    {
      case DCH_FX:
        break;
      case DCH_A_M:
      case DCH_P_M:
      case DCH_a_m:
      case DCH_p_m:
      case DCH_AM:
      case DCH_PM:
      case DCH_am:
      case DCH_pm:
      case DCH_HH:
      case DCH_HH12:
      case DCH_HH24:
      case DCH_MI:
      case DCH_SS:
      case DCH_MS:    /* millisecond */
      case DCH_US:    /* microsecond */
      case DCH_FF1:
      case DCH_FF2:
      case DCH_FF3:
      case DCH_FF4:
      case DCH_FF5:
      case DCH_FF6:
      case DCH_SSSS:
        flags |= DCH_TIMED;
        break;
      case DCH_tz:
      case DCH_TZ:
      case DCH_OF:
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "formatting field \"%s\" is only supported in to_char",
          n->key->name);
        flags |= DCH_ZONED;
        break;
      case DCH_TZH:
      case DCH_TZM:
        flags |= DCH_ZONED;
        break;
      case DCH_A_D:
      case DCH_B_C:
      case DCH_a_d:
      case DCH_b_c:
      case DCH_AD:
      case DCH_BC:
      case DCH_ad:
      case DCH_bc:
      case DCH_MONTH:
      case DCH_Month:
      case DCH_month:
      case DCH_MON:
      case DCH_Mon:
      case DCH_mon:
      case DCH_MM:
      case DCH_DAY:
      case DCH_Day:
      case DCH_day:
      case DCH_DY:
      case DCH_Dy:
      case DCH_dy:
      case DCH_DDD:
      case DCH_IDDD:
      case DCH_DD:
      case DCH_D:
      case DCH_ID:
      case DCH_WW:
      case DCH_Q:
      case DCH_CC:
      case DCH_Y_YYY:
      case DCH_YYYY:
      case DCH_IYYY:
      case DCH_YYY:
      case DCH_IYY:
      case DCH_YY:
      case DCH_IY:
      case DCH_Y:
      case DCH_I:
      case DCH_RM:
      case DCH_rm:
      case DCH_W:
      case DCH_J:
        flags |= DCH_DATED;
        break;
    }
  }

on_error:
  return flags;
}

/* select a DCHCacheEntry to hold the given format picture */
static DCHCacheEntry *
DCH_cache_getnew(const char *str, bool std)
{
  DCHCacheEntry *ent;

  /* Ensure we can advance DCHCounter below */
  DCH_prevent_counter_overflow();

  /*
   * If cache is full, remove oldest entry (or recycle first not-valid one)
   */
  if (n_DCHCache >= DCH_CACHE_ENTRIES)
  {
    DCHCacheEntry *old = DCHCache[0];

    if (old->valid)
    {
      for (int i = 1; i < DCH_CACHE_ENTRIES; i++)
      {
        ent = DCHCache[i];
        if (!ent->valid)
        {
          old = ent;
          break;
        }
        if (ent->age < old->age)
          old = ent;
      }
    }
    old->valid = false;
    memcpy(old->str, str, DCH_CACHE_SIZE);
    old->str[DCH_CACHE_SIZE] = '\0';
    old->age = (++DCHCounter);
    /* caller is expected to fill format, then set valid */
    return old;
  }
  else
  {
    Assert(DCHCache[n_DCHCache] == NULL);
    DCHCache[n_DCHCache] = ent = (DCHCacheEntry *)
      palloc(sizeof(DCHCacheEntry));
    ent->valid = false;
    memcpy(ent->str, str, DCH_CACHE_SIZE);
    ent->str[DCH_CACHE_SIZE] = '\0';
    ent->std = std;
    ent->age = (++DCHCounter);
    /* caller is expected to fill format, then set valid */
    ++n_DCHCache;
    return ent;
  }
}

/* look for an existing DCHCacheEntry matching the given format picture */
static DCHCacheEntry *
DCH_cache_search(const char *str, bool std)
{
  /* Ensure we can advance DCHCounter below */
  DCH_prevent_counter_overflow();

  for (int i = 0; i < n_DCHCache; i++)
  {
    DCHCacheEntry *ent = DCHCache[i];

    if (ent->valid && strcmp(ent->str, str) == 0 && ent->std == std)
    {
      ent->age = (++DCHCounter);
      return ent;
    }
  }

  return NULL;
}

/* Find or create a DCHCacheEntry for the given format picture */
static DCHCacheEntry *
DCH_cache_fetch(const char *str, bool std)
{
  DCHCacheEntry *ent;

  if ((ent = DCH_cache_search(str, std)) == NULL)
  {
    /*
     * Not in the cache, must run parser and save a new format-picture to
     * the cache.  Do not mark the cache entry valid until parsing
     * succeeds.
     */
    ent = DCH_cache_getnew(str, std);

    parse_format(ent->format, str, DCH_keywords, DCH_suff, DCH_index,
           DCH_FLAG | (std ? STD_FLAG : 0));

    ent->valid = true;
  }
  return ent;
}

/*
 * Format a date/time or interval into a string according to fmt.
 * We parse fmt into a list of FormatNodes.  This is then passed to DCH_to_char
 * for formatting.
 */
static text *
datetime_to_char_body(TmToChar *tmtc, text *fmt, bool is_interval, Oid collid)
{
  FormatNode *format;
  char     *fmt_str,
         *result;
  bool    incache;
  int      fmt_len;
  text     *res;

  /*
   * Convert fmt to C string
   */
  fmt_str = text2cstring(fmt);
  fmt_len = strlen(fmt_str);

  /*
   * Allocate workspace for result as C string
   */
  result = palloc((fmt_len * DCH_MAX_ITEM_SIZ) + 1);
  *result = '\0';

  if (fmt_len > DCH_CACHE_SIZE)
  {
    /*
     * Allocate new memory if format picture is bigger than static cache
     * and do not use cache (call parser always)
     */
    incache = false;

    format = (FormatNode *) palloc((fmt_len + 1) * sizeof(FormatNode));

    parse_format(format, fmt_str, DCH_keywords,
           DCH_suff, DCH_index, DCH_FLAG);
  }
  else
  {
    /*
     * Use cache buffers
     */
    DCHCacheEntry *ent = DCH_cache_fetch(fmt_str, false);

    incache = true;
    format = ent->format;
  }

  /* The real work is here */
  DCH_to_char(format, is_interval, tmtc, result, collid);

  if (!incache)
    pfree(format);

  pfree(fmt_str);

  /* convert C-string result to TEXT format */
  res = cstring2text(result);

  pfree(result);
  return res;
}

/****************************************************************************
 *        Public routines
 ***************************************************************************/

/* -------------------
 * TIMESTAMP to_char()
 * -------------------
 */
/**
 * @brief Output a timestamp as text.
 */
text *
pg_timestamp_to_char(Timestamp dt, text *fmt)
{
  text *res;
  TmToChar tmtc;
  struct pg_tm *tm;
  int thisdate;

  if (VARSIZE_ANY_EXHDR(fmt) <= 0 || TIMESTAMP_NOT_FINITE(dt))
    return NULL;

  ZERO_tmtc(&tmtc);
  tm = tmtcTm(&tmtc);

  if (timestamp2tm(dt, NULL, tm, &tmtcFsec(&tmtc), NULL, NULL) != 0)
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "timestamp out of range");
    return NULL;
  }

  thisdate = date2j(tm->tm_year, tm->tm_mon, tm->tm_mday);
  tm->tm_wday = (thisdate + 1) % 7;
  tm->tm_yday = thisdate - date2j(tm->tm_year, 1, 1) + 1;

  if (!(res = datetime_to_char_body(&tmtc, fmt, false, DEFAULT_COLLATION_OID)))
    return NULL;

  return res;
}

/**
 * @brief Output a timestamptz as text.
 */
text *
pg_timestamptz_to_char(TimestampTz dt, text *fmt)
{
  text *res;
  TmToChar  tmtc;
  int tz;
  struct pg_tm *tm;
  int thisdate;

  if (VARSIZE_ANY_EXHDR(fmt) <= 0 || TIMESTAMP_NOT_FINITE(dt))
    return NULL;

  ZERO_tmtc(&tmtc);
  tm = tmtcTm(&tmtc);

  if (timestamp2tm(dt, &tz, tm, &tmtcFsec(&tmtc), &tmtcTzn(&tmtc), NULL) != 0)
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "timestamp out of range");
    return NULL;
  }

  thisdate = date2j(tm->tm_year, tm->tm_mon, tm->tm_mday);
  tm->tm_wday = (thisdate + 1) % 7;
  tm->tm_yday = thisdate - date2j(tm->tm_year, 1, 1) + 1;

  if (!(res = datetime_to_char_body(&tmtc, fmt, false, DEFAULT_COLLATION_OID)))
    return NULL;

  return res;
}


/* -------------------
 * INTERVAL to_char()
 * -------------------
 */
/**
 * @brief Output an interval as text.
 */
text *
pg_interval_to_char(Interval *it, text *fmt)
{
  text *res;
  TmToChar tmtc;
  struct pg_tm *tm;

  if (VARSIZE_ANY_EXHDR(fmt) <= 0)
    return NULL;

  ZERO_tmtc(&tmtc);
  tm = tmtcTm(&tmtc);

  if (interval2tm(*it, tm, &tmtcFsec(&tmtc)) != 0)
    return NULL;

  /* wday is meaningless, yday approximates the total span in days */
  tm->tm_yday = (tm->tm_year * MONTHS_PER_YEAR + tm->tm_mon) * DAYS_PER_MONTH + tm->tm_mday;

  if (!(res = datetime_to_char_body(&tmtc, fmt, true, DEFAULT_COLLATION_OID)))
    return NULL;

  return res;
}

/* ---------------------
 * TO_TIMESTAMP()
 *
 * Make Timestamp from date_str which is formatted at argument 'fmt'
 * ( to_timestamp is reverse to_char() )
 * ---------------------
 */
/**
 * @brief Input a timestamp from date text.
 */
TimestampTz
pg_to_timestamp(text *date_txt, text *fmt)
{
  Oid collid = DEFAULT_COLLATION_OID;
  TimestampTz  result;
  int      tz;
  struct pg_tm tm;
  fsec_t    fsec;
  int      fprec;

  do_to_timestamp(date_txt, fmt, collid, false,
          &tm, &fsec, &fprec, NULL, NULL);

  /* Use the specified time zone, if any. */
  if (tm.tm_zone)
  {
    int dterr = DecodeTimezone(unconstify(char *, tm.tm_zone), &tz);

    if (dterr)
    {
      DateTimeParseError(dterr, text2cstring(date_txt), "timestamptz");
      return NULL;
    }
  }
  else
    tz = DetermineTimeZoneOffset(&tm, session_timezone);

  if (tm2timestamp(&tm, fsec, &tz, &result) != 0)
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "timestamp out of range");
    return NULL;
  }

  /* Use the specified fractional precision, if any. */
  if (fprec)
    MEOSAdjustTimestampForTypmod(&result, fprec);

  return result;
}

/* ----------
 * TO_DATE
 *  Make Date from date_str which is formatted at argument 'fmt'
 * ----------
 */
/**
 * @brief Input a date from text.
 */
DateADT
pg_to_date(text *date_txt, text *fmt)
{
  DateADT result;
  struct pg_tm tm;
  fsec_t fsec;

  do_to_timestamp(date_txt, fmt, DEFAULT_COLLATION_OID, false,
          &tm, &fsec, NULL, NULL, NULL);

  /* Prevent overflow in Julian-day routines */
  if (!IS_VALID_JULIAN(tm.tm_year, tm.tm_mon, tm.tm_mday))
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
      "date out of range: \"%s\"", text2cstring(date_txt));
    return 0;
  }

  result = date2j(tm.tm_year, tm.tm_mon, tm.tm_mday) - POSTGRES_EPOCH_JDATE;

  /* Now check for just-out-of-range dates */
  if (!IS_VALID_DATE(result))
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
      "date out of range: \"%s\"", text2cstring(date_txt));
    return 0;
  }

  return result;
}

/*
 * do_to_timestamp: shared code for to_timestamp and to_date
 *
 * Parse the 'date_txt' according to 'fmt', return results as a struct pg_tm,
 * fractional seconds, and fractional precision.
 *
 * 'collid' identifies the collation to use, if needed.
 * 'std' specifies standard parsing mode.
 * Bit mask of date/time/zone components found in 'fmt' is returned in 'flags',
 * if that is not NULL.
 * If 'have_error' is NULL, then errors are thrown, else '*have_error' is set.
 *
 * We parse 'fmt' into a list of FormatNodes, which is then passed to
 * DCH_from_char to populate a TmFromChar with the parsed contents of
 * 'date_txt'.
 *
 * The TmFromChar is then analysed and converted into the final results in
 * struct 'tm', 'fsec', and 'fprec'.
 */
static void
do_to_timestamp(text *date_txt, text *fmt, Oid collid, bool std,
        struct pg_tm *tm, fsec_t *fsec, int *fprec,
        uint32 *flags, bool *have_error)
{
  FormatNode *format = NULL;
  TmFromChar  tmfc;
  int      fmt_len;
  char     *date_str;
  int      fmask;
  bool    incache = false;

  Assert(tm != NULL);
  Assert(fsec != NULL);

  date_str = text2cstring(date_txt);

  ZERO_tmfc(&tmfc);
  ZERO_tm(tm);
  *fsec = 0;
  if (fprec)
    *fprec = 0;
  if (flags)
    *flags = 0;
  fmask = 0;          /* bit mask for ValidateDate() */

  fmt_len = VARSIZE_ANY_EXHDR(fmt);

  if (fmt_len)
  {
    char     *fmt_str;

    fmt_str = text2cstring(fmt);

    if (fmt_len > DCH_CACHE_SIZE)
    {
      /*
       * Allocate new memory if format picture is bigger than static
       * cache and do not use cache (call parser always)
       */
      format = (FormatNode *) palloc((fmt_len + 1) * sizeof(FormatNode));

      parse_format(format, fmt_str, DCH_keywords, DCH_suff, DCH_index,
             DCH_FLAG | (std ? STD_FLAG : 0));
    }
    else
    {
      /*
       * Use cache buffers
       */
      DCHCacheEntry *ent = DCH_cache_fetch(fmt_str, std);

      incache = true;
      format = ent->format;
    }

    DCH_from_char(format, date_str, &tmfc, collid, std, have_error);
    CHECK_ERROR;

    pfree(fmt_str);

    if (flags)
      *flags = DCH_datetime_type(format, have_error);

    if (!incache)
    {
      pfree(format);
      format = NULL;
    }

    CHECK_ERROR;
  }

  /*
   * Convert to_date/to_timestamp input fields to standard 'tm'
   */
  if (tmfc.ssss)
  {
    int      x = tmfc.ssss;

    tm->tm_hour = x / SECS_PER_HOUR;
    x %= SECS_PER_HOUR;
    tm->tm_min = x / SECS_PER_MINUTE;
    x %= SECS_PER_MINUTE;
    tm->tm_sec = x;
  }

  if (tmfc.ss)
    tm->tm_sec = tmfc.ss;
  if (tmfc.mi)
    tm->tm_min = tmfc.mi;
  if (tmfc.hh)
    tm->tm_hour = tmfc.hh;

  if (tmfc.clock == CLOCK_12_HOUR)
  {
    if (tm->tm_hour < 1 || tm->tm_hour > HOURS_PER_DAY / 2)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "hour \"%d\" is invalid for the 12-hour clock", tm->tm_hour);
    }

    if (tmfc.pm && tm->tm_hour < HOURS_PER_DAY / 2)
      tm->tm_hour += HOURS_PER_DAY / 2;
    else if (!tmfc.pm && tm->tm_hour == HOURS_PER_DAY / 2)
      tm->tm_hour = 0;
  }

  if (tmfc.year)
  {
    /*
     * If CC and YY (or Y) are provided, use YY as 2 low-order digits for
     * the year in the given century.  Keep in mind that the 21st century
     * AD runs from 2001-2100, not 2000-2099; 6th century BC runs from
     * 600BC to 501BC.
     */
    if (tmfc.cc && tmfc.yysz <= 2)
    {
      if (tmfc.bc)
        tmfc.cc = -tmfc.cc;
      tm->tm_year = tmfc.year % 100;
      if (tm->tm_year)
      {
        if (tmfc.cc >= 0)
          tm->tm_year += (tmfc.cc - 1) * 100;
        else
          tm->tm_year = (tmfc.cc + 1) * 100 - tm->tm_year + 1;
      }
      else
      {
        /* find century year for dates ending in "00" */
        tm->tm_year = tmfc.cc * 100 + ((tmfc.cc >= 0) ? 0 : 1);
      }
    }
    else
    {
      /* If a 4-digit year is provided, we use that and ignore CC. */
      tm->tm_year = tmfc.year;
      if (tmfc.bc)
        tm->tm_year = -tm->tm_year;
      /* correct for our representation of BC years */
      if (tm->tm_year < 0)
        tm->tm_year++;
    }
    fmask |= DTK_M(YEAR);
  }
  else if (tmfc.cc)
  {
    /* use first year of century */
    if (tmfc.bc)
      tmfc.cc = -tmfc.cc;
    if (tmfc.cc >= 0)
      /* +1 because 21st century started in 2001 */
      tm->tm_year = (tmfc.cc - 1) * 100 + 1;
    else
      /* +1 because year == 599 is 600 BC */
      tm->tm_year = tmfc.cc * 100 + 1;
    fmask |= DTK_M(YEAR);
  }

  if (tmfc.j)
  {
    j2date(tmfc.j, &tm->tm_year, &tm->tm_mon, &tm->tm_mday);
    fmask |= DTK_DATE_M;
  }

  if (tmfc.ww)
  {
    if (tmfc.mode == FROM_CHAR_DATE_ISOWEEK)
    {
      /*
       * If tmfc.d is not set, then the date is left at the beginning of
       * the ISO week (Monday).
       */
      if (tmfc.d)
        isoweekdate2date(tmfc.ww, tmfc.d, &tm->tm_year, &tm->tm_mon, &tm->tm_mday);
      else
        isoweek2date(tmfc.ww, &tm->tm_year, &tm->tm_mon, &tm->tm_mday);
      fmask |= DTK_DATE_M;
    }
    else
      tmfc.ddd = (tmfc.ww - 1) * 7 + 1;
  }

  if (tmfc.w)
    tmfc.dd = (tmfc.w - 1) * 7 + 1;
  if (tmfc.dd)
  {
    tm->tm_mday = tmfc.dd;
    fmask |= DTK_M(DAY);
  }
  if (tmfc.mm)
  {
    tm->tm_mon = tmfc.mm;
    fmask |= DTK_M(MONTH);
  }

  if (tmfc.ddd && (tm->tm_mon <= 1 || tm->tm_mday <= 1))
  {
    /*
     * The month and day field have not been set, so we use the
     * day-of-year field to populate them.  Depending on the date mode,
     * this field may be interpreted as a Gregorian day-of-year, or an ISO
     * week date day-of-year.
     */

    if (!tm->tm_year && !tmfc.bc)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "cannot calculate day of year without year information");
    }

    if (tmfc.mode == FROM_CHAR_DATE_ISOWEEK)
    {
      int      j0;    /* zeroth day of the ISO year, in Julian */

      j0 = isoweek2j(tm->tm_year, 1) - 1;

      j2date(j0 + tmfc.ddd, &tm->tm_year, &tm->tm_mon, &tm->tm_mday);
      fmask |= DTK_DATE_M;
    }
    else
    {
      const int  *y;
      int      i;

      static const int ysum[2][13] = {
        {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
      {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}};

      y = ysum[isleap(tm->tm_year)];

      for (i = 1; i <= MONTHS_PER_YEAR; i++)
      {
        if (tmfc.ddd <= y[i])
          break;
      }
      if (tm->tm_mon <= 1)
        tm->tm_mon = i;

      if (tm->tm_mday <= 1)
        tm->tm_mday = tmfc.ddd - y[i - 1];

      fmask |= DTK_M(MONTH) | DTK_M(DAY);
    }
  }

  if (tmfc.ms)
    *fsec += tmfc.ms * 1000;
  if (tmfc.us)
    *fsec += tmfc.us;
  if (fprec)
    *fprec = tmfc.ff;    /* fractional precision, if specified */

  /* Range-check date fields according to bit mask computed above */
  if (fmask != 0)
  {
    /* We already dealt with AD/BC, so pass isjulian = true */
    int      dterr = ValidateDate(fmask, true, false, false, tm);

    if (dterr != 0)
    {
      /*
       * Force the error to be DTERR_FIELD_OVERFLOW even if ValidateDate
       * said DTERR_MD_FIELD_OVERFLOW, because we don't want to print an
       * irrelevant hint about datestyle.
       */
      RETURN_ERROR(DateTimeParseError(DTERR_FIELD_OVERFLOW, date_str, "timestamp"));
    }
  }

  /* Range-check time fields too */
  if (tm->tm_hour < 0 || tm->tm_hour >= HOURS_PER_DAY ||
    tm->tm_min < 0 || tm->tm_min >= MINS_PER_HOUR ||
    tm->tm_sec < 0 || tm->tm_sec >= SECS_PER_MINUTE ||
    *fsec < INT64CONST(0) || *fsec >= USECS_PER_SEC)
  {
    RETURN_ERROR(DateTimeParseError(DTERR_FIELD_OVERFLOW, date_str, "timestamp"));
  }

  /* Save parsed time-zone into tm->tm_zone if it was specified */
  if (tmfc.tzsign)
  {
    #define MAX_TZ_PARSE_LEN 7
    char *tz = palloc(sizeof(char) * MAX_TZ_PARSE_LEN);

    if (tmfc.tzh < 0 || tmfc.tzh > MAX_TZDISP_HOUR ||
      tmfc.tzm < 0 || tmfc.tzm >= MINS_PER_HOUR)
    {
      RETURN_ERROR(DateTimeParseError(DTERR_TZDISP_OVERFLOW, date_str, "timestamp"));
    }
    sprintf(tz, "%c%02d:%02d\0", tmfc.tzsign > 0 ? '+' : '-', tmfc.tzh, tmfc.tzm);

    tm->tm_zone = tz;
  }

on_error:

  if (format && !incache)
    pfree(format);

  pfree(date_str);
}
