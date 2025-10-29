/*-------------------------------------------------------------------------
 *
 * numeric.c
 *    An exact numeric data type for the Postgres database system
 *
 * Original coding 1998, Jan Wieck.  Heavily revised 2003, Tom Lane.
 *
 * Many of the algorithmic ideas are borrowed from David M. Smith's "FM"
 * multiple-precision math library, most recently published as Algorithm
 * 786: Multiple-Precision Complex Arithmetic and Functions, ACM
 * Transactions on Mathematical Software, Vol. 24, No. 4, December 1998,
 * pages 359-367.
 *
 * Copyright (c) 1998-2025, PostgreSQL Global Development Group
 *
 * IDENTIFICATION
 *    src/backend/utils/adt/numeric.c
 *
 *-------------------------------------------------------------------------
 */

/* C */
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
#include <common/hashfn.h>
#include <common/int.h>
#include <lib/stringinfo.h>
#include <utils/date.h>
#include <utils/datetime.h>
#include <utils/float.h>
#include <utils/numeric.h>
#include <utils/timestamp.h>
#include <utils/jsonb.h>

#include <pgtypes.h>

#if POSTGRESQL_VERSION_NUMBER < 160000
/*
 * Similarly, wrappers around labs()/llabs() matching our int64.
 */
#if SIZEOF_LONG == 8
#define i64abs(i) ((int64) labs(i))
#elif SIZEOF_LONG_LONG == 8
#define i64abs(i) ((int64) llabs(i))
#else
#error "cannot find integer type of the same size as int64_t"
#endif
#endif /* POSTGRESQL_VERSION_NUMBER < 160000 */

/* ----------
 * Local data types
 *
 * Numeric values are represented in a base-NBASE floating point format.
 * Each "digit" ranges from 0 to NBASE-1.  The type NumericDigit is signed
 * and wide enough to store a digit.  We assume that NBASE*NBASE can fit in
 * an int.  Although the purely calculational routines could handle any even
 * NBASE that's less than sqrt(INT_MAX), in practice we are only interested
 * in NBASE a power of ten, so that I/O conversions and decimal rounding
 * are easy.  Also, it's actually more efficient if NBASE is rather less than
 * sqrt(INT_MAX), so that there is "headroom" for mul_var and div_var to
 * postpone processing carries.
 *
 * Values of NBASE other than 10000 are considered of historical interest only
 * and are no longer supported in any sense; no mechanism exists for the client
 * to discover the base, so every client supporting binary mode expects the
 * base-10000 format.  If you plan to change this, also note the numeric
 * abbreviation code, which assumes NBASE=10000.
 * ----------
 */

#if 0
#define NBASE    10
#define HALF_NBASE  5
#define DEC_DIGITS  1      /* decimal digits per NBASE digit */
#define MUL_GUARD_DIGITS  4  /* these are measured in NBASE digits */
#define DIV_GUARD_DIGITS  8
  
typedef signed char NumericDigit;
#endif

#if 0
#define NBASE    100
#define HALF_NBASE  50
#define DEC_DIGITS  2      /* decimal digits per NBASE digit */
#define MUL_GUARD_DIGITS  3  /* these are measured in NBASE digits */
#define DIV_GUARD_DIGITS  6

typedef signed char NumericDigit;
#endif

#if 1
#define NBASE    10000
#define HALF_NBASE  5000
#define DEC_DIGITS  4      /* decimal digits per NBASE digit */
#define MUL_GUARD_DIGITS  2  /* these are measured in NBASE digits */
#define DIV_GUARD_DIGITS  4

typedef int16 NumericDigit;
#endif

#define NBASE_SQR  (NBASE * NBASE)

/*
 * The Numeric type as stored on disk.
 *
 * If the high bits of the first word of a NumericChoice (n_header, or
 * n_short.n_header, or n_long.n_sign_dscale) are NUMERIC_SHORT, then the
 * numeric follows the NumericShort format; if they are NUMERIC_POS or
 * NUMERIC_NEG, it follows the NumericLong format. If they are NUMERIC_SPECIAL,
 * the value is a NaN or Infinity.  We currently always store SPECIAL values
 * using just two bytes (i.e. only n_header), but previous releases used only
 * the NumericLong format, so we might find 4-byte NaNs (though not infinities)
 * on disk if a database has been migrated using pg_upgrade.  In either case,
 * the low-order bits of a special value's header are reserved and currently
 * should always be set to zero.
 *
 * In the NumericShort format, the remaining 14 bits of the header word
 * (n_short.n_header) are allocated as follows: 1 for sign (positive or
 * negative), 6 for dynamic scale, and 7 for weight.  In practice, most
 * commonly-encountered values can be represented this way.
 *
 * In the NumericLong format, the remaining 14 bits of the header word
 * (n_long.n_sign_dscale) represent the display scale; and the weight is
 * stored separately in n_weight.
 *
 * NOTE: by convention, values in the packed form have been stripped of
 * all leading and trailing zero digits (where a "digit" is of base NBASE).
 * In particular, if the value is zero, there will be no digits at all!
 * The weight is arbitrary in that case, but we normally set it to zero.
 */

struct NumericShort
{
  uint16    n_header;    /* Sign + display scale + weight */
  NumericDigit n_data[FLEXIBLE_ARRAY_MEMBER]; /* Digits */
};

struct NumericLong
{
  uint16    n_sign_dscale;  /* Sign + display scale */
  int16    n_weight;    /* Weight of 1st digit  */
  NumericDigit n_data[FLEXIBLE_ARRAY_MEMBER]; /* Digits */
};

union NumericChoice
{
  uint16    n_header;    /* Header word */
  struct NumericLong n_long;  /* Long form (4-byte header) */
  struct NumericShort n_short;  /* Short form (2-byte header) */
};

struct NumericData
{
  int32    vl_len_;    /* varlena header (do not touch directly!) */
  union NumericChoice choice; /* choice of format */
};


/*
 * Interpretation of high bits.
 */

#define NUMERIC_SIGN_MASK  0xC000
#define NUMERIC_POS      0x0000
#define NUMERIC_NEG      0x4000
#define NUMERIC_SHORT    0x8000
#define NUMERIC_SPECIAL    0xC000

#define NUMERIC_FLAGBITS(n) ((n)->choice.n_header & NUMERIC_SIGN_MASK)
#define NUMERIC_IS_SHORT(n)    (NUMERIC_FLAGBITS(n) == NUMERIC_SHORT)
#define NUMERIC_IS_SPECIAL(n)  (NUMERIC_FLAGBITS(n) == NUMERIC_SPECIAL)

#define NUMERIC_HDRSZ  (VARHDRSZ + sizeof(uint16) + sizeof(int16))
#define NUMERIC_HDRSZ_SHORT (VARHDRSZ + sizeof(uint16))

/*
 * If the flag bits are NUMERIC_SHORT or NUMERIC_SPECIAL, we want the short
 * header; otherwise, we want the long one.  Instead of testing against each
 * value, we can just look at the high bit, for a slight efficiency gain.
 */
#define NUMERIC_HEADER_IS_SHORT(n)  (((n)->choice.n_header & 0x8000) != 0)
#define NUMERIC_HEADER_SIZE(n) \
  (VARHDRSZ + sizeof(uint16) + \
   (NUMERIC_HEADER_IS_SHORT(n) ? 0 : sizeof(int16)))

/*
 * Definitions for special values (NaN, positive infinity, negative infinity).
 *
 * The two bits after the NUMERIC_SPECIAL bits are 00 for NaN, 01 for positive
 * infinity, 11 for negative infinity.  (This makes the sign bit match where
 * it is in a short-format value, though we make no use of that at present.)
 * We could mask off the remaining bits before testing the active bits, but
 * currently those bits must be zeroes, so masking would just add cycles.
 */
#define NUMERIC_EXT_SIGN_MASK  0xF000  /* high bits plus NaN/Inf flag bits */
#define NUMERIC_NAN        0xC000
#define NUMERIC_PINF      0xD000
#define NUMERIC_NINF      0xF000
#define NUMERIC_INF_SIGN_MASK  0x2000

#define NUMERIC_EXT_FLAGBITS(n)  ((n)->choice.n_header & NUMERIC_EXT_SIGN_MASK)
#define NUMERIC_IS_NAN(n)    ((n)->choice.n_header == NUMERIC_NAN)
#define NUMERIC_IS_PINF(n)    ((n)->choice.n_header == NUMERIC_PINF)
#define NUMERIC_IS_NINF(n)    ((n)->choice.n_header == NUMERIC_NINF)
#define NUMERIC_IS_INF(n) \
  (((n)->choice.n_header & ~NUMERIC_INF_SIGN_MASK) == NUMERIC_PINF)

/*
 * Short format definitions.
 */

#define NUMERIC_SHORT_SIGN_MASK      0x2000
#define NUMERIC_SHORT_DSCALE_MASK    0x1F80
#define NUMERIC_SHORT_DSCALE_SHIFT    7
#define NUMERIC_SHORT_DSCALE_MAX    \
  (NUMERIC_SHORT_DSCALE_MASK >> NUMERIC_SHORT_DSCALE_SHIFT)
#define NUMERIC_SHORT_WEIGHT_SIGN_MASK  0x0040
#define NUMERIC_SHORT_WEIGHT_MASK    0x003F
#define NUMERIC_SHORT_WEIGHT_MAX    NUMERIC_SHORT_WEIGHT_MASK
#define NUMERIC_SHORT_WEIGHT_MIN    (-(NUMERIC_SHORT_WEIGHT_MASK+1))

/*
 * Extract sign, display scale, weight.  These macros extract field values
 * suitable for the NumericVar format from the Numeric (on-disk) format.
 *
 * Note that we don't trouble to ensure that dscale and weight read as zero
 * for an infinity; however, that doesn't matter since we never convert
 * "special" numerics to NumericVar form.  Only the constants defined below
 * (const_nan, etc) ever represent a non-finite value as a NumericVar.
 */

#define NUMERIC_DSCALE_MASK      0x3FFF
#define NUMERIC_DSCALE_MAX      NUMERIC_DSCALE_MASK

#define NUMERIC_SIGN(n) \
  (NUMERIC_IS_SHORT(n) ? \
    (((n)->choice.n_short.n_header & NUMERIC_SHORT_SIGN_MASK) ? \
     NUMERIC_NEG : NUMERIC_POS) : \
    (NUMERIC_IS_SPECIAL(n) ? \
     NUMERIC_EXT_FLAGBITS(n) : NUMERIC_FLAGBITS(n)))
#define NUMERIC_DSCALE(n)  (NUMERIC_HEADER_IS_SHORT((n)) ? \
  ((n)->choice.n_short.n_header & NUMERIC_SHORT_DSCALE_MASK) \
    >> NUMERIC_SHORT_DSCALE_SHIFT \
  : ((n)->choice.n_long.n_sign_dscale & NUMERIC_DSCALE_MASK))
#define NUMERIC_WEIGHT(n)  (NUMERIC_HEADER_IS_SHORT((n)) ? \
  (((n)->choice.n_short.n_header & NUMERIC_SHORT_WEIGHT_SIGN_MASK ? \
    ~NUMERIC_SHORT_WEIGHT_MASK : 0) \
   | ((n)->choice.n_short.n_header & NUMERIC_SHORT_WEIGHT_MASK)) \
  : ((n)->choice.n_long.n_weight))

/*
 * Maximum weight of a stored Numeric value (based on the use of int16 for the
 * weight in NumericLong).  Note that intermediate values held in NumericVar
 * and NumericSumAccum variables may have much larger weights.
 */
#define NUMERIC_WEIGHT_MAX      PG_INT16_MAX

/* ----------
 * NumericVar is the format we use for arithmetic.  The digit-array part
 * is the same as the NumericData storage format, but the header is more
 * complex.
 *
 * The value represented by a NumericVar is determined by the sign, weight,
 * ndigits, and digits[] array.  If it is a "special" value (NaN or Inf)
 * then only the sign field matters; ndigits should be zero, and the weight
 * and dscale fields are ignored.
 *
 * Note: the first digit of a NumericVar's value is assumed to be multiplied
 * by NBASE ** weight.  Another way to say it is that there are weight+1
 * digits before the decimal point.  It is possible to have weight < 0.
 *
 * buf points at the physical start of the palloc'd digit buffer for the
 * NumericVar.  digits points at the first digit in actual use (the one
 * with the specified weight).  We normally leave an unused digit or two
 * (preset to zeroes) between buf and digits, so that there is room to store
 * a carry out of the top digit without reallocating space.  We just need to
 * decrement digits (and increment weight) to make room for the carry digit.
 * (There is no such extra space in a numeric value stored in the database,
 * only in a NumericVar in memory.)
 *
 * If buf is NULL then the digit buffer isn't actually palloc'd and should
 * not be freed --- see the constants below for an example.
 *
 * dscale, or display scale, is the nominal precision expressed as number
 * of digits after the decimal point (it must always be >= 0 at present).
 * dscale may be more than the number of physically stored fractional digits,
 * implying that we have suppressed storage of significant trailing zeroes.
 * It should never be less than the number of stored digits, since that would
 * imply hiding digits that are present.  NOTE that dscale is always expressed
 * in *decimal* digits, and so it may correspond to a fractional number of
 * base-NBASE digits --- divide by DEC_DIGITS to convert to NBASE digits.
 *
 * rscale, or result scale, is the target precision for a computation.
 * Like dscale it is expressed as number of *decimal* digits after the decimal
 * point, and is always >= 0 at present.
 * Note that rscale is not stored in variables --- it's figured on-the-fly
 * from the dscales of the inputs.
 *
 * While we consistently use "weight" to refer to the base-NBASE weight of
 * a numeric value, it is convenient in some scale-related calculations to
 * make use of the base-10 weight (ie, the approximate log10 of the value).
 * To avoid confusion, such a decimal-units weight is called a "dweight".
 *
 * NB: All the variable-level functions are written in a style that makes it
 * possible to give one and the same variable as argument and destination.
 * This is feasible because the digit buffer is separate from the variable.
 * ----------
 */
typedef struct NumericVar
{
  int      ndigits;    /* # of digits in digits[] - can be 0! */
  int      weight;      /* weight of first digit */
  int      sign;      /* NUMERIC_POS, _NEG, _NAN, _PINF, or _NINF */
  int      dscale;      /* display scale */
  NumericDigit *buf;      /* start of palloc'd space for digits[] */
  NumericDigit *digits;    /* base-NBASE digits */
} NumericVar;

/*
 * We define our own macros for packing and unpacking abbreviated-key
 * representations for numeric values in order to avoid depending on
 * USE_FLOAT8_BYVAL.  The type of abbreviation we use is based only on
 * the size of a datum, not the argument-passing convention for float8.
 *
 * The range of abbreviations for finite values is from +PG_INT64/32_MAX
 * to -PG_INT64/32_MAX.  NaN has the abbreviation PG_INT64/32_MIN, and we
 * define the sort ordering to make that work out properly (see further
 * comments below).  PINF and NINF share the abbreviations of the largest
 * and smallest finite abbreviation classes.
 */
#define NUMERIC_ABBREV_BITS (SIZEOF_DATUM * BITS_PER_BYTE)
#if SIZEOF_DATUM == 8
#define NumericAbbrevGetDatum(X) ((Datum) (X))
#define DatumGetNumericAbbrev(X) ((int64) (X))
#define NUMERIC_ABBREV_NAN     NumericAbbrevGetDatum(PG_INT64_MIN)
#define NUMERIC_ABBREV_PINF     NumericAbbrevGetDatum(-PG_INT64_MAX)
#define NUMERIC_ABBREV_NINF     NumericAbbrevGetDatum(PG_INT64_MAX)
#else
#define NumericAbbrevGetDatum(X) ((Datum) (X))
#define DatumGetNumericAbbrev(X) ((int32) (X))
#define NUMERIC_ABBREV_NAN     NumericAbbrevGetDatum(PG_INT32_MIN)
#define NUMERIC_ABBREV_PINF     NumericAbbrevGetDatum(-PG_INT32_MAX)
#define NUMERIC_ABBREV_NINF     NumericAbbrevGetDatum(PG_INT32_MAX)
#endif


/* ----------
 * Some preinitialized constants
 * ----------
 */
static const NumericDigit const_zero_data[1] = {0};
static const NumericVar const_zero =
{0, 0, NUMERIC_POS, 0, NULL, (NumericDigit *) const_zero_data};

static const NumericDigit const_one_data[1] = {1};
static const NumericVar const_one =
{1, 0, NUMERIC_POS, 0, NULL, (NumericDigit *) const_one_data};

static const NumericVar const_minus_one =
{1, 0, NUMERIC_NEG, 0, NULL, (NumericDigit *) const_one_data};

static const NumericDigit const_two_data[1] = {2};
static const NumericVar const_two =
{1, 0, NUMERIC_POS, 0, NULL, (NumericDigit *) const_two_data};

#if DEC_DIGITS == 4
static const NumericDigit const_zero_point_nine_data[1] = {9000};
#elif DEC_DIGITS == 2
static const NumericDigit const_zero_point_nine_data[1] = {90};
#elif DEC_DIGITS == 1
static const NumericDigit const_zero_point_nine_data[1] = {9};
#endif
static const NumericVar const_zero_point_nine =
{1, -1, NUMERIC_POS, 1, NULL, (NumericDigit *) const_zero_point_nine_data};

#if DEC_DIGITS == 4
static const NumericDigit const_one_point_one_data[2] = {1, 1000};
#elif DEC_DIGITS == 2
static const NumericDigit const_one_point_one_data[2] = {1, 10};
#elif DEC_DIGITS == 1
static const NumericDigit const_one_point_one_data[2] = {1, 1};
#endif
static const NumericVar const_one_point_one =
{2, 0, NUMERIC_POS, 1, NULL, (NumericDigit *) const_one_point_one_data};

static const NumericVar const_nan =
{0, 0, NUMERIC_NAN, 0, NULL, NULL};

static const NumericVar const_pinf =
{0, 0, NUMERIC_PINF, 0, NULL, NULL};

static const NumericVar const_ninf =
{0, 0, NUMERIC_NINF, 0, NULL, NULL};

#if DEC_DIGITS == 4
static const int round_powers[4] = {0, 1000, 100, 10};
#endif


/* ----------
 * Local functions
 * ----------
 */

#ifdef NUMERIC_DEBUG
static void dump_numeric(const char *str, Numeric num);
static void dump_var(const char *str, NumericVar *var);
#else
#define dump_numeric(s,n)
#define dump_var(s,v)
#endif

#define digitbuf_alloc(ndigits)  \
  ((NumericDigit *) palloc((ndigits) * sizeof(NumericDigit)))
#define digitbuf_free(buf)  \
  do { \
     if ((buf) != NULL) \
       pfree(buf); \
  } while (0)

#define init_var(v)    memset(v, 0, sizeof(NumericVar))

#define NUMERIC_DIGITS(num) (NUMERIC_HEADER_IS_SHORT(num) ? \
  (num)->choice.n_short.n_data : (num)->choice.n_long.n_data)
#define NUMERIC_NDIGITS(num) \
  ((VARSIZE(num) - NUMERIC_HEADER_SIZE(num)) / sizeof(NumericDigit))
#define NUMERIC_CAN_BE_SHORT(scale,weight) \
  ((scale) <= NUMERIC_SHORT_DSCALE_MAX && \
  (weight) <= NUMERIC_SHORT_WEIGHT_MAX && \
  (weight) >= NUMERIC_SHORT_WEIGHT_MIN)

static void alloc_var(NumericVar *var, int ndigits);
static void free_var(NumericVar *var);
static void zero_var(NumericVar *var);

static bool set_var_from_str(const char *str, const char *cp,
               NumericVar *dest, const char **endptr);
static bool set_var_from_non_decimal_integer_str(const char *str,
                         const char *cp, int sign,
                         int base, NumericVar *dest,
                         const char **endptr);
static void set_var_from_num(Numeric num, NumericVar *dest);
static void init_var_from_num(Numeric num, NumericVar *dest);
static void set_var_from_var(const NumericVar *value, NumericVar *dest);
static char *get_str_from_var(const NumericVar *var);
static char *get_str_from_var_sci(const NumericVar *var, int rscale);

static Numeric duplicate_numeric(Numeric num);
static Numeric make_result(const NumericVar *var);
static Numeric make_result_opt_error(const NumericVar *var, bool *have_error);

static bool apply_typmod(NumericVar *var, int32 typmod); // MEOS removed last argument void *escontext
static bool apply_typmod_special(Numeric num, int32 typmod); // MEOS removed last argument void *escontext

static bool numericvar_to_int32(const NumericVar *var, int32 *result);
static bool numericvar_to_int64(const NumericVar *var, int64 *result);
static void int64_to_numericvar(int64 val, NumericVar *var);
#ifdef HAVE_INT128
static void int128_to_numericvar(int128 val, NumericVar *var);
#endif
static double numericvar_to_double_no_overflow(const NumericVar *var);

static int  cmp_numerics(Numeric num1, Numeric num2);
static int  cmp_var(const NumericVar *var1, const NumericVar *var2);
static int  cmp_var_common(const NumericDigit *var1digits, int var1ndigits,
               int var1weight, int var1sign,
               const NumericDigit *var2digits, int var2ndigits,
               int var2weight, int var2sign);
static void add_var(const NumericVar *var1, const NumericVar *var2,
          NumericVar *result);
static void sub_var(const NumericVar *var1, const NumericVar *var2,
          NumericVar *result);
static void mul_var(const NumericVar *var1, const NumericVar *var2,
          NumericVar *result,
          int rscale);
static void mul_var_short(const NumericVar *var1, const NumericVar *var2,
              NumericVar *result);
static void div_var(const NumericVar *var1, const NumericVar *var2,
          NumericVar *result, int rscale, bool round, bool exact);
static void div_var_int(const NumericVar *var, int ival, int ival_weight,
            NumericVar *result, int rscale, bool round);
#ifdef HAVE_INT128
static void div_var_int64(const NumericVar *var, int64 ival, int ival_weight,
              NumericVar *result, int rscale, bool round);
#endif
static int select_div_scale(const NumericVar *var1, const NumericVar *var2);
static void mod_var(const NumericVar *var1, const NumericVar *var2,
          NumericVar *result);
static void div_mod_var(const NumericVar *var1, const NumericVar *var2,
            NumericVar *quot, NumericVar *rem);
static void ceil_var(const NumericVar *var, NumericVar *result);
static void floor_var(const NumericVar *var, NumericVar *result);

static void gcd_var(const NumericVar *var1, const NumericVar *var2,
          NumericVar *result);
static void sqrt_var(const NumericVar *arg, NumericVar *result, int rscale);
static void exp_var(const NumericVar *arg, NumericVar *result, int rscale);
static int  estimate_ln_dweight(const NumericVar *var);
static void ln_var(const NumericVar *arg, NumericVar *result, int rscale);
static void log_var(const NumericVar *base, const NumericVar *num,
          NumericVar *result);
static void power_var(const NumericVar *base, const NumericVar *exp,
            NumericVar *result);
static void power_var_int(const NumericVar *base, int exp, int exp_dscale,
              NumericVar *result);
static void power_ten_int(int exp, NumericVar *result);
static int  cmp_abs(const NumericVar *var1, const NumericVar *var2);
static int  cmp_abs_common(const NumericDigit *var1digits, int var1ndigits,
               int var1weight,
               const NumericDigit *var2digits, int var2ndigits,
               int var2weight);
static void add_abs(const NumericVar *var1, const NumericVar *var2,
          NumericVar *result);
static void sub_abs(const NumericVar *var1, const NumericVar *var2,
          NumericVar *result);
static void round_var(NumericVar *var, int rscale);
static void trunc_var(NumericVar *var, int rscale);
static void strip_var(NumericVar *var);
static void compute_bucket(Numeric operand, Numeric bound1, Numeric bound2,
               const NumericVar *count_var,
               NumericVar *result_var);

/*****************************************************************************/

/**
 * @ingroup meos_base_numeric
 * @brief Return a numeric value from its string representation
 * @param[in] str String
 * @param[in] typmod Typmod
 * @note Derived from PostgreSQL function @p numeric_in()
 */
#if MEOS
Numeric
numeric_in(const char *str, int32 typmod)
{
  return pg_numeric_in(str, typmod);
}
#endif 
Numeric
pg_numeric_in(const char *str, int32 typmod)
{
  Numeric res;
  const char *cp;
  const char *numstart;
  int sign;

  /* Skip leading spaces */
  cp = str;
  while (*cp)
  {
    if (!isspace((unsigned char) *cp))
      break;
    cp++;
  }

  /*
   * Process the number's sign. This duplicates logic in set_var_from_str(),
   * but it's worth doing here, since it simplifies the handling of
   * infinities and non-decimal integers.
   */
  numstart = cp;
  sign = NUMERIC_POS;

  if (*cp == '+')
    cp++;
  else if (*cp == '-')
  {
    sign = NUMERIC_NEG;
    cp++;
  }

  /*
   * Check for NaN and infinities.  We recognize the same strings allowed by
   * float8in().
   *
   * Since all other legal inputs have a digit or a decimal point after the
   * sign, we need only check for NaN/infinity if that's not the case.
   */
  if (!isdigit((unsigned char) *cp) && *cp != '.')
  {
    /*
     * The number must be NaN or infinity; anything else can only be a
     * syntax error. Note that NaN mustn't have a sign.
     */
    if (pg_strncasecmp(numstart, "NaN", 3) == 0)
    {
      res = make_result(&const_nan);
      cp = numstart + 3;
    }
    else if (pg_strncasecmp(cp, "Infinity", 8) == 0)
    {
      res = make_result(sign == NUMERIC_POS ? &const_pinf : &const_ninf);
      cp += 8;
    }
    else if (pg_strncasecmp(cp, "inf", 3) == 0)
    {
      res = make_result(sign == NUMERIC_POS ? &const_pinf : &const_ninf);
      cp += 3;
    }
    else
      goto invalid_syntax;

    /*
     * Check for trailing junk; there should be nothing left but spaces.
     *
     * We intentionally do this check before applying the typmod because
     * we would like to throw any trailing-junk syntax error before any
     * semantic error resulting from apply_typmod_special().
     */
    while (*cp)
    {
      if (!isspace((unsigned char) *cp))
        goto invalid_syntax;
      cp++;
    }

    if (!apply_typmod_special(res, typmod))
      return NULL;
  }
  else
  {
    /*
     * We have a normal numeric value, which may be a non-decimal integer
     * or a regular decimal number.
     */
    NumericVar  value;
    int      base;
    bool    have_error;

    init_var(&value);

    /*
     * Determine the number's base by looking for a non-decimal prefix
     * indicator ("0x", "0o", or "0b").
     */
    if (cp[0] == '0')
    {
      switch (cp[1])
      {
        case 'x':
        case 'X':
          base = 16;
          break;
        case 'o':
        case 'O':
          base = 8;
          break;
        case 'b':
        case 'B':
          base = 2;
          break;
        default:
          base = 10;
      }
    }
    else
      base = 10;

    /* Parse the rest of the number and apply the sign */
    if (base == 10)
    {
      if (!set_var_from_str(str, cp, &value, &cp))
        return NULL;
      value.sign = sign;
    }
    else
    {
      if (!set_var_from_non_decimal_integer_str(str, cp + 2, sign, base,
                            &value, &cp))
        return NULL;
    }

    /*
     * Should be nothing left but spaces. As above, throw any typmod error
     * after finishing syntax check.
     */
    while (*cp)
    {
      if (!isspace((unsigned char) *cp))
        goto invalid_syntax;
      cp++;
    }

    if (!apply_typmod(&value, typmod))
      return NULL;

    res = make_result_opt_error(&value, &have_error);

    if (have_error)
    {
      elog(ERROR, "value overflows numeric format");
      return NULL;
    }

    free_var(&value);
  }

  return res;

invalid_syntax:
  elog(ERROR, "invalid input syntax for type %s: \"%s\"", "numeric", str);
  return NULL; // TODO
}

/**
 * @ingroup meos_base_numeric
 * @brief Return the string representation a numeric value
 * @note Derived from PostgreSQL function @p numeric_out()
 */
#if MEOS
char *
numeric_out(Numeric num)
{
  return pg_numeric_out(num);
}
#endif 
char *
pg_numeric_out(Numeric num)
{
  NumericVar  x;
  char *str;

  /*
   * Handle NaN and infinities
   */
  if (NUMERIC_IS_SPECIAL(num))
  {
    if (NUMERIC_IS_PINF(num))
      return pstrdup("Infinity");
    else if (NUMERIC_IS_NINF(num))
      return pstrdup("-Infinity");
    else
      return pstrdup("NaN");
  }

  /*
   * Get the number in the variable format.
   */
  init_var_from_num(num, &x);
  str = get_str_from_var(&x);
  return str;
}

/**
 * @ingroup meos_base_numeric
 * @brief Return a copy of a temporal value
 * @param[in] num Temporal value
 */
Numeric
numeric_copy(Numeric num)
{
  Numeric result = palloc(VARSIZE(num));
  memcpy(result, num, VARSIZE(num));
  return result;
}

/*
 * numeric_is_nan() -
 *
 *  Is Numeric value a NaN?
 */
bool
numeric_is_nan(Numeric num)
{
  return NUMERIC_IS_NAN(num);
}

/*
 * numeric_is_inf() -
 *
 *  Is Numeric value an infinity?
 */
bool
numeric_is_inf(Numeric num)
{
  return NUMERIC_IS_INF(num);
}

/*
 * numeric_is_integral() -
 *
 *  Is Numeric value integral?
 */
static bool
numeric_is_integral(Numeric num)
{
  NumericVar  arg;

  /* Reject NaN, but infinities are considered integral */
  if (NUMERIC_IS_SPECIAL(num))
  {
    if (NUMERIC_IS_NAN(num))
      return false;
    return true;
  }

  /* Integral if there are no digits to the right of the decimal point */
  init_var_from_num(num, &arg);

  return (arg.ndigits == 0 || arg.ndigits <= arg.weight + 1);
}

/*
 * make_numeric_typmod() -
 *
 *  Pack numeric precision and scale values into a typmod.  The upper 16 bits
 *  are used for the precision (though actually not all these bits are needed,
 *  since the maximum allowed precision is 1000).  The lower 16 bits are for
 *  the scale, but since the scale is constrained to the range [-1000, 1000],
 *  we use just the lower 11 of those 16 bits, and leave the remaining 5 bits
 *  unset, for possible future use.
 *
 *  For purely historical reasons VARHDRSZ is then added to the result, thus
 *  the unused space in the upper 16 bits is not all as freely available as it
 *  might seem.  (We can't let the result overflow to a negative int32, as
 *  other parts of the system would interpret that as not-a-valid-typmod.)
 */
static inline int32
make_numeric_typmod(int precision, int scale)
{
  return ((precision << 16) | (scale & 0x7ff)) + VARHDRSZ;
}

/*
 * Because of the offset, valid numeric typmods are at least VARHDRSZ
 */
static inline bool
is_valid_numeric_typmod(int32 typmod)
{
  return typmod >= (int32) VARHDRSZ;
}

/*
 * numeric_typmod_precision() -
 *
 *  Extract the precision from a numeric typmod --- see make_numeric_typmod().
 */
static inline int
numeric_typmod_precision(int32 typmod)
{
  return ((typmod - VARHDRSZ) >> 16) & 0xffff;
}

/*
 * numeric_typmod_scale() -
 *
 *  Extract the scale from a numeric typmod --- see make_numeric_typmod().
 *
 *  Note that the scale may be negative, so we must do sign extension when
 *  unpacking it.  We do this using the bit hack (x^1024)-1024, which sign
 *  extends an 11-bit two's complement number x.
 */
static inline int
numeric_typmod_scale(int32 typmod)
{
  return (((typmod - VARHDRSZ) & 0x7ff) ^ 1024) - 1024;
}

/*
 * numeric_maximum_size() -
 *
 *  Maximum size of a numeric with given typmod, or -1 if unlimited/unknown.
 */
int32
numeric_maximum_size(int32 typmod)
{
  int      precision;
  int      numeric_digits;

  if (!is_valid_numeric_typmod(typmod))
    return -1;

  /* precision (ie, max # of digits) is in upper bits of typmod */
  precision = numeric_typmod_precision(typmod);

  /*
   * This formula computes the maximum number of NumericDigits we could need
   * in order to store the specified number of decimal digits. Because the
   * weight is stored as a number of NumericDigits rather than a number of
   * decimal digits, it's possible that the first NumericDigit will contain
   * only a single decimal digit.  Thus, the first two decimal digits can
   * require two NumericDigits to store, but it isn't until we reach
   * DEC_DIGITS + 2 decimal digits that we potentially need a third
   * NumericDigit.
   */
  numeric_digits = (precision + 2 * (DEC_DIGITS - 1)) / DEC_DIGITS;

  /*
   * In most cases, the size of a numeric will be smaller than the value
   * computed below, because the varlena header will typically get toasted
   * down to a single byte before being stored on disk, and it may also be
   * possible to use a short numeric header.  But our job here is to compute
   * the worst case.
   */
  return NUMERIC_HDRSZ + (numeric_digits * sizeof(NumericDigit));
}

/*
 * numeric_out_sci() -
 *
 *  Output function for numeric data type in scientific notation.
 */
char *
numeric_out_sci(Numeric num, int scale)
{
  NumericVar  x;
  char     *str;

  /*
   * Handle NaN and infinities
   */
  if (NUMERIC_IS_SPECIAL(num))
  {
    if (NUMERIC_IS_PINF(num))
      return pstrdup("Infinity");
    else if (NUMERIC_IS_NINF(num))
      return pstrdup("-Infinity");
    else
      return pstrdup("NaN");
  }

  init_var_from_num(num, &x);

  str = get_str_from_var_sci(&x, scale);

  return str;
}

/*
 * numeric_normalize() -
 *
 *  Output function for numeric data type, suppressing insignificant trailing
 *  zeroes and then any trailing decimal point.  The intent of this is to
 *  produce strings that are equal if and only if the input numeric values
 *  compare equal.
 */
char *
numeric_normalize(Numeric num)
{
  NumericVar  x;
  char     *str;
  int      last;

  /*
   * Handle NaN and infinities
   */
  if (NUMERIC_IS_SPECIAL(num))
  {
    if (NUMERIC_IS_PINF(num))
      return pstrdup("Infinity");
    else if (NUMERIC_IS_NINF(num))
      return pstrdup("-Infinity");
    else
      return pstrdup("NaN");
  }

  init_var_from_num(num, &x);

  str = get_str_from_var(&x);

  /* If there's no decimal point, there's certainly nothing to remove. */
  if (strchr(str, '.') != NULL)
  {
    /*
     * Back up over trailing fractional zeroes.  Since there is a decimal
     * point, this loop will terminate safely.
     */
    last = strlen(str) - 1;
    while (str[last] == '0')
      last--;

    /* We want to get rid of the decimal point too, if it's now last. */
    if (str[last] == '.')
      last--;

    /* Delete whatever we backed up over. */
    str[last + 1] = '\0';
  }

  return str;
}

/**
 * @ingroup meos_base_numeric
 * @brief Return a numeric value with the given precision and scale
 * @note Derived from PostgreSQL function @p numeric()
 */
#if MEOS
Numeric
numeric(Numeric num, int32 typmod)
{
  return pg_numeric(num, typmod);
}
#endif 
Numeric
pg_numeric(Numeric num, int32 typmod)
{
  Numeric    new;
  int      precision;
  int      scale;
  int      ddigits;
  int      maxdigits;
  int      dscale;
  NumericVar  var;

  /*
   * Handle NaN and infinities: if apply_typmod_special doesn't complain,
   * just return a copy of the input.
   */
  if (NUMERIC_IS_SPECIAL(num))
  {
    (void) apply_typmod_special(num, typmod);
    return duplicate_numeric(num);
  }

  /*
   * If the value isn't a valid type modifier, simply return a copy of the
   * input value
   */
  if (!is_valid_numeric_typmod(typmod))
    return duplicate_numeric(num);

  /*
   * Get the precision and scale out of the typmod value
   */
  precision = numeric_typmod_precision(typmod);
  scale = numeric_typmod_scale(typmod);
  maxdigits = precision - scale;

  /* The target display scale is non-negative */
  dscale = Max(scale, 0);

  /*
   * If the number is certainly in bounds and due to the target scale no
   * rounding could be necessary, just make a copy of the input and modify
   * its scale fields, unless the larger scale forces us to abandon the
   * short representation.  (Note we assume the existing dscale is
   * honest...)
   */
  ddigits = (NUMERIC_WEIGHT(num) + 1) * DEC_DIGITS;
  if (ddigits <= maxdigits && scale >= NUMERIC_DSCALE(num)
    && (NUMERIC_CAN_BE_SHORT(dscale, NUMERIC_WEIGHT(num))
      || !NUMERIC_IS_SHORT(num)))
  {
    new = duplicate_numeric(num);
    if (NUMERIC_IS_SHORT(num))
      new->choice.n_short.n_header =
        (num->choice.n_short.n_header & ~NUMERIC_SHORT_DSCALE_MASK)
        | (dscale << NUMERIC_SHORT_DSCALE_SHIFT);
    else
      new->choice.n_long.n_sign_dscale = NUMERIC_SIGN(new) |
        ((uint16) dscale & NUMERIC_DSCALE_MASK);
    return new;
  }

  /*
   * We really need to fiddle with things - unpack the number into a
   * variable and let apply_typmod() do it.
   */
  init_var(&var);

  set_var_from_num(num, &var);
  (void) apply_typmod(&var, typmod);
  new = make_result(&var);

  free_var(&var);

  return new;
}

/**
 * @brief Return the character string that represents a typmod value
 * @note Derived from PostgreSQL function @p numerictypmodout()
 */
char *
numeric_typmodout(int32 typmod)
{
  char *res = (char *) palloc(64);
  if (is_valid_numeric_typmod(typmod))
    snprintf(res, 64, "(%d,%d)", numeric_typmod_precision(typmod),
      numeric_typmod_scale(typmod));
  else
    *res = '\0';

  return res;
}

/* ----------------------------------------------------------------------
 *
 * Sign manipulation, rounding and the like
 *
 * ----------------------------------------------------------------------
 */

/**
 * @ingroup meos_base_numeric
 * @brief Return the absolute value of a numeric value
 * @note Derived from PostgreSQL function @p numeric_abs()
 */
#if MEOS
Numeric
numeric_abs(Numeric num)
{
  return pg_numeric_abs(num);
}
#endif 
Numeric
pg_numeric_abs(Numeric num)
{
  /*
   * Do it the easy way directly on the packed format
   */
  Numeric res = duplicate_numeric(num);

  if (NUMERIC_IS_SHORT(num))
    res->choice.n_short.n_header =
      num->choice.n_short.n_header & ~NUMERIC_SHORT_SIGN_MASK;
  else if (NUMERIC_IS_SPECIAL(num))
  {
    /* This changes -Inf to Inf, and doesn't affect NaN */
    res->choice.n_short.n_header =
      num->choice.n_short.n_header & ~NUMERIC_INF_SIGN_MASK;
  }
  else
    res->choice.n_long.n_sign_dscale = NUMERIC_POS | NUMERIC_DSCALE(num);

  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Return the unary plus of a numeric value
 * @note Derived from PostgreSQL function @p numeric_uplus()
 */
#if MEOS
Numeric
numeric_uplus(Numeric num)
{
  return pg_numeric_uplus(num);
}
#endif 
Numeric
pg_numeric_uplus(Numeric num)
{
  return duplicate_numeric(num);
}

/**
 * @ingroup meos_base_numeric
 * @brief Return the unary minus of a numeric value
 * @note Derived from PostgreSQL function @p numeric_uminus()
 */
#if MEOS
Numeric
numeric_uminus(Numeric num)
{
  return pg_numeric_uminus(num);
}
#endif 
Numeric
pg_numeric_uminus(Numeric num)
{
  /*
   * Do it the easy way directly on the packed format
   */
  Numeric res = duplicate_numeric(num);

  if (NUMERIC_IS_SPECIAL(num))
  {
    /* Flip the sign, if it's Inf or -Inf */
    if (!NUMERIC_IS_NAN(num))
      res->choice.n_short.n_header =
        num->choice.n_short.n_header ^ NUMERIC_INF_SIGN_MASK;
  }

  /*
   * The packed format is known to be totally zero digit trimmed always. So
   * once we've eliminated specials, we can identify a zero by the fact that
   * there are no digits at all. Do nothing to a zero.
   */
  else if (NUMERIC_NDIGITS(num) != 0)
  {
    /* Else, flip the sign */
    if (NUMERIC_IS_SHORT(num))
      res->choice.n_short.n_header =
        num->choice.n_short.n_header ^ NUMERIC_SHORT_SIGN_MASK;
    else if (NUMERIC_SIGN(num) == NUMERIC_POS)
      res->choice.n_long.n_sign_dscale =
        NUMERIC_NEG | NUMERIC_DSCALE(num);
    else
      res->choice.n_long.n_sign_dscale =
        NUMERIC_POS | NUMERIC_DSCALE(num);
  }

  return res;
}

/*
 * numeric_sign_internal() -
 *
 * Returns -1 if the argument is less than 0, 0 if the argument is equal
 * to 0, and 1 if the argument is greater than zero.  Caller must have
 * taken care of the NaN case, but we can handle infinities here.
 */
int
numeric_sign_internal(Numeric num)
{
  if (NUMERIC_IS_SPECIAL(num))
  {
    Assert(!NUMERIC_IS_NAN(num));
    /* Must be Inf or -Inf */
    if (NUMERIC_IS_PINF(num))
      return 1;
    else
      return -1;
  }

  /*
   * The packed format is known to be totally zero digit trimmed always. So
   * once we've eliminated specials, we can identify a zero by the fact that
   * there are no digits at all.
   */
  else if (NUMERIC_NDIGITS(num) == 0)
    return 0;
  else if (NUMERIC_SIGN(num) == NUMERIC_NEG)
    return -1;
  else
    return 1;
}

/**
 * @ingroup meos_base_numeric
 * @brief Return  -1 if the numeric value is less than 0, 0 if it is equal
 * to 0, and 1 if it is greater than zero
 * @note Derived from PostgreSQL function @p numeric_sign()
 */
#if MEOS
Numeric
numeric_sign(Numeric num)
{
  return pg_numeric_sign(num);
}
#endif
Numeric
pg_numeric_sign(Numeric num)
{
  /*
   * Handle NaN (infinities can be handled normally)
   */
  if (NUMERIC_IS_NAN(num))
    return make_result(&const_nan);

  switch (numeric_sign_internal(num))
  {
    case 0:
      return make_result(&const_zero);
    case 1:
      return make_result(&const_one);
    case -1:
      return make_result(&const_minus_one);
  }

  Assert(false);
  return 0;
}

/**
 * @ingroup meos_base_numeric
 * @brief Round a numeric value to have 'scale' digits after the decimal point
 * @details We allow negative 'scale', implying rounding before the decimal
 * point --- Oracle interprets rounding that way.
 * @note Derived from PostgreSQL function @p numeric_round()
 */
#if MEOS
Numeric
numeric_round(Numeric num, int32 scale)
{
  return pg_numeric_round(num, scale);
}
#endif
Numeric
pg_numeric_round(Numeric num, int32 scale)
{
  /*
   * Handle NaN and infinities
   */
  if (NUMERIC_IS_SPECIAL(num))
    return duplicate_numeric(num);

  /*
   * Limit the scale value to avoid possible overflow in calculations.
   *
   * These limits are based on the maximum number of digits a Numeric value
   * can have before and after the decimal point, but we must allow for one
   * extra digit before the decimal point, in case the most significant
   * digit rounds up; we must check if that causes Numeric overflow.
   */
  scale = Max(scale, -(NUMERIC_WEIGHT_MAX + 1) * DEC_DIGITS - 1);
  scale = Min(scale, NUMERIC_DSCALE_MAX);

  /*
   * Unpack the argument and round it at the proper digit position
   */
  NumericVar arg;
  init_var(&arg);
  set_var_from_num(num, &arg);

  round_var(&arg, scale);

  /* We don't allow negative output dscale */
  if (scale < 0)
    arg.dscale = 0;

  /*
   * Return the rounded result
   */
  Numeric res = make_result(&arg);

  free_var(&arg);
  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Truncate a value to have 'scale' digits after the decimal point
 * @details We allow negative 'scale', implying a truncation before the decimal
 *  point --- Oracle interprets truncation that way
 * @note Derived from PostgreSQL function @p numeric_trunc()
 */
#if MEOS
Numeric
numeric_trunc(Numeric num, int32 scale)
{
  return pg_numeric_trunc(num, scale);
}
#endif
Numeric
pg_numeric_trunc(Numeric num, int32 scale)
{
  /*
   * Handle NaN and infinities
   */
  if (NUMERIC_IS_SPECIAL(num))
    return duplicate_numeric(num);

  /*
   * Limit the scale value to avoid possible overflow in calculations.
   *
   * These limits are based on the maximum number of digits a Numeric value
   * can have before and after the decimal point.
   */
  scale = Max(scale, -(NUMERIC_WEIGHT_MAX + 1) * DEC_DIGITS);
  scale = Min(scale, NUMERIC_DSCALE_MAX);

  /*
   * Unpack the argument and truncate it at the proper digit position
   */
  NumericVar arg;
  init_var(&arg);
  set_var_from_num(num, &arg);

  trunc_var(&arg, scale);

  /* We don't allow negative output dscale */
  if (scale < 0)
    arg.dscale = 0;

  /*
   * Return the truncated result
   */
  Numeric res = make_result(&arg);

  free_var(&arg);
  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Return the smallest integer greater than or equal to the argument
 * @note Derived from PostgreSQL function @p numeric_ceil()
 */
#if MEOS
Numeric
numeric_ceil(Numeric num)
{
  return pg_numeric_ceil(num);
}
#endif
Numeric
pg_numeric_ceil(Numeric num)
{
  /*
   * Handle NaN and infinities
   */
  if (NUMERIC_IS_SPECIAL(num))
    return duplicate_numeric(num);

  NumericVar result;
  init_var_from_num(num, &result);
  ceil_var(&result, &result);

  Numeric res = make_result(&result);
  free_var(&result);

  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Return the largest integer equal to or less than a numeric value
 * @note Derived from PostgreSQL function @p numeric_floor()
 */
#if MEOS
Numeric
numeric_floor(Numeric num)
{
  return pg_numeric_floor(num);
}
#endif
Numeric
pg_numeric_floor(Numeric num)
{
  /*
   * Handle NaN and infinities
   */
  if (NUMERIC_IS_SPECIAL(num))
    return duplicate_numeric(num);

  NumericVar result;
  init_var_from_num(num, &result);
  floor_var(&result, &result);

  Numeric res = make_result(&result);
  free_var(&result);

  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Return the number of the bucket in which a numeric value falls in a
 * histogram having count equal-width buckets spanning the range low to high
 * @details Implements the numeric version of the width_bucket() function
 * defined by SQL2003. See also float8_width_bucket().
 *
 * 'bound1' and 'bound2' are the lower and upper bounds of the
 * histogram's range, respectively. 'count' is the number of buckets
 * in the histogram. width_bucket() returns an integer indicating the
 * bucket number that 'operand' belongs to in an equiwidth histogram
 * with the specified characteristics. An operand smaller than the
 * lower bound is assigned to bucket 0. An operand greater than or equal
 * to the upper bound is assigned to an additional bucket (with number
 * count+1). We don't allow "NaN" for any of the numeric inputs, and we
 * don't allow either of the histogram bounds to be +/- infinity.
 * @note Derived from PostgreSQL function @p width_bucket()
 */
int32
numeric_width_bucket(Numeric operand, Numeric bound1, Numeric bound2,
  int32 count)
{
  if (count <= 0)
    elog(ERROR, "count must be greater than zero");

  if (NUMERIC_IS_SPECIAL(operand) || NUMERIC_IS_SPECIAL(bound1) ||
    NUMERIC_IS_SPECIAL(bound2))
  {
    if (NUMERIC_IS_NAN(operand) || NUMERIC_IS_NAN(bound1) ||
        NUMERIC_IS_NAN(bound2))
      elog(ERROR, "operand, lower bound, and upper bound cannot be NaN");
    /* We allow "operand" to be infinite; cmp_numerics will cope */
    if (NUMERIC_IS_INF(bound1) || NUMERIC_IS_INF(bound2))
      elog(ERROR, "lower and upper bounds must be finite");
  }

  NumericVar count_var;
  NumericVar result_var;
  init_var(&result_var);
  init_var(&count_var);

  /* Convert 'count' to a numeric, for ease of use later */
  int64_to_numericvar((int64) count, &count_var);

  switch (cmp_numerics(bound1, bound2))
  {
    case 0:
      elog(ERROR, "lower bound cannot equal upper bound");
      break;

      /* bound1 < bound2 */
    case -1:
      if (cmp_numerics(operand, bound1) < 0)
        set_var_from_var(&const_zero, &result_var);
      else if (cmp_numerics(operand, bound2) >= 0)
        add_var(&count_var, &const_one, &result_var);
      else
        compute_bucket(operand, bound1, bound2, &count_var, &result_var);
      break;

      /* bound1 > bound2 */
    case 1:
      if (cmp_numerics(operand, bound1) > 0)
        set_var_from_var(&const_zero, &result_var);
      else if (cmp_numerics(operand, bound2) <= 0)
        add_var(&count_var, &const_one, &result_var);
      else
        compute_bucket(operand, bound1, bound2, &count_var, &result_var);
      break;
  }

  /* if result exceeds the range of a legal int4, we elog here */
  int32 result;
  if (!numericvar_to_int32(&result_var, &result))
  {
    elog(ERROR, "integer out of range");
    return INT_MAX;
  }

  free_var(&count_var);
  free_var(&result_var);

  return result;
}

/*
 * 'operand' is inside the bucket range, so determine the correct
 * bucket for it to go in. The calculations performed by this function
 * are derived directly from the SQL2003 spec. Note however that we
 * multiply by count before dividing, to avoid unnecessary roundoff error.
 */
static void
compute_bucket(Numeric operand, Numeric bound1, Numeric bound2,
  const NumericVar *count_var, NumericVar *result_var)
{
  NumericVar bound1_var;
  NumericVar bound2_var;
  NumericVar operand_var;

  init_var_from_num(bound1, &bound1_var);
  init_var_from_num(bound2, &bound2_var);
  init_var_from_num(operand, &operand_var);

  /*
   * Per spec, bound1 is inclusive and bound2 is exclusive, and so we have
   * bound1 <= operand < bound2 or bound1 >= operand > bound2.  Either way,
   * the result is ((operand - bound1) * count) / (bound2 - bound1) + 1,
   * where the quotient is computed using floor division (i.e., division to
   * zero decimal places with truncation), which guarantees that the result
   * is in the range [1, count].  Reversing the bounds doesn't affect the
   * computation, because the signs cancel out when dividing.
   */
  sub_var(&operand_var, &bound1_var, &operand_var);
  sub_var(&bound2_var, &bound1_var, &bound2_var);

  mul_var(&operand_var, count_var, &operand_var,
      operand_var.dscale + count_var->dscale);
  div_var(&operand_var, &bound2_var, result_var, 0, false, true);
  add_var(result_var, &const_one, result_var);

  free_var(&bound1_var);
  free_var(&bound2_var);
  free_var(&operand_var);
  return;
}

/* ----------------------------------------------------------------------
 *
 * Comparison functions
 *
 * Note: btree indexes need these routines not to leak memory; therefore,
 * be careful to free working copies of toasted datums.  Most places don't
 * need to be so careful.
 *
 * Sort support:
 *
 * We implement the sortsupport strategy routine in order to get the benefit of
 * abbreviation. The ordinary numeric comparison can be quite slow as a result
 * of palloc/pfree cycles (due to detoasting packed values for alignment);
 * while this could be worked on itself, the abbreviation strategy gives more
 * speedup in many common cases.
 *
 * Two different representations are used for the abbreviated form, one in
 * int32 and one in int64, whichever fits into a by-value Datum.  In both cases
 * the representation is negated relative to the original value, because we use
 * the largest negative value for NaN, which sorts higher than other values. We
 * convert the absolute value of the numeric to a 31-bit or 63-bit positive
 * value, and then negate it if the original number was positive.
 *
 * We abort the abbreviation process if the abbreviation cardinality is below
 * 0.01% of the row count (1 per 10k non-null rows).  The actual break-even
 * point is somewhat below that, perhaps 1 per 30k (at 1 per 100k there's a
 * very small penalty), but we don't want to build up too many abbreviated
 * values before first testing for abort, so we take the slightly pessimistic
 * number.  We make no attempt to estimate the cardinality of the real values,
 * since it plays no part in the cost model here (if the abbreviation is equal,
 * the cost of comparing equal and unequal underlying values is comparable).
 * We discontinue even checking for abort (saving us the hashing overhead) if
 * the estimated cardinality gets to 100k; that would be enough to support many
 * billions of rows while doing no worse than breaking even.
 *
 * ----------------------------------------------------------------------
 */
 
 /**
 * @ingroup meos_base_numeric
 * @brief Return -1, 0, or 1 depending on whether the first numeric value is
 * less than, equal, or greater than the second one
 * @note Derived from PostgreSQL function @p numeric_cmp()
 */
#if MEOS
int
numeric_cmp(Numeric num1, Numeric num2)
{
  return pg_numeric_cmp(num1, num2);
}
#endif
int
pg_numeric_cmp(Numeric num1, Numeric num2)
{
  return cmp_numerics(num1, num2);
}

 /**
 * @ingroup meos_base_numeric
 * @brief Return true if two numeric values are equal
 * @note Derived from PostgreSQL function @p numeric_eq()
 */
#if MEOS
bool
numeric_eq(Numeric num1, Numeric num2)
{
  return pg_numeric_eq(num1, num2);
}
#endif
bool
pg_numeric_eq(Numeric num1, Numeric num2)
{
  return cmp_numerics(num1, num2) == 0;
}

 /**
 * @ingroup meos_base_numeric
 * @brief Return true if two numeric values are not equal
 * @note Derived from PostgreSQL function @p numeric_ne()
 */
#if MEOS
bool
numeric_ne(Numeric num1, Numeric num2)
{
  return pg_numeric_ne(num1, num2);
}
#endif
bool
pg_numeric_ne(Numeric num1, Numeric num2)
{
  return cmp_numerics(num1, num2) != 0;
}

 /**
 * @ingroup meos_base_numeric
 * @brief Return true if a numeric value is greater than another one
 * @note Derived from PostgreSQL function @p numeric_gt()
 */
#if MEOS
bool
numeric_gt(Numeric num1, Numeric num2)
{
  return pg_numeric_gt(num1, num2);
}
#endif
bool
pg_numeric_gt(Numeric num1, Numeric num2)
{
  return cmp_numerics(num1, num2) > 0;
}

 /**
 * @ingroup meos_base_numeric
 * @brief Return true if a numeric value is greater than or equal to another one
 * @note Derived from PostgreSQL function @p numeric_ge()
 */
#if MEOS
bool
numeric_ge(Numeric num1, Numeric num2)
{
  return pg_numeric_ge(num1, num2);
}
#endif
bool
pg_numeric_ge(Numeric num1, Numeric num2)
{
  return cmp_numerics(num1, num2) >= 0;
}

 /**
 * @ingroup meos_base_numeric
 * @brief Return true if a numeric value is less than another one
 * @note Derived from PostgreSQL function @p numeric_lt()
 */
#if MEOS
bool
numeric_lt(Numeric num1, Numeric num2)
{
  return pg_numeric_lt(num1, num2);
}
#endif
bool
pg_numeric_lt(Numeric num1, Numeric num2)
{
  return cmp_numerics(num1, num2) < 0;
}

 /**
 * @ingroup meos_base_numeric
 * @brief Return true if a numeric value is less than or equal to another one
 * @note Derived from PostgreSQL function @p numeric_le()
 */
#if MEOS
bool
numeric_le(Numeric num1, Numeric num2)
{
  return pg_numeric_le(num1, num2);
}
#endif
bool
pg_numeric_le(Numeric num1, Numeric num2)
{
  return cmp_numerics(num1, num2) <= 0;
}

static int
cmp_numerics(Numeric num1, Numeric num2)
{
  int result;

  /*
   * We consider all NANs to be equal and larger than any non-NAN (including
   * Infinity).  This is somewhat arbitrary; the important thing is to have
   * a consistent sort order.
   */
  if (NUMERIC_IS_SPECIAL(num1))
  {
    if (NUMERIC_IS_NAN(num1))
    {
      if (NUMERIC_IS_NAN(num2))
        result = 0;    /* NAN = NAN */
      else
        result = 1;    /* NAN > non-NAN */
    }
    else if (NUMERIC_IS_PINF(num1))
    {
      if (NUMERIC_IS_NAN(num2))
        result = -1;  /* PINF < NAN */
      else if (NUMERIC_IS_PINF(num2))
        result = 0;    /* PINF = PINF */
      else
        result = 1;    /* PINF > anything else */
    }
    else          /* num1 must be NINF */
    {
      if (NUMERIC_IS_NINF(num2))
        result = 0;    /* NINF = NINF */
      else
        result = -1;  /* NINF < anything else */
    }
  }
  else if (NUMERIC_IS_SPECIAL(num2))
  {
    if (NUMERIC_IS_NINF(num2))
      result = 1;      /* normal > NINF */
    else
      result = -1;    /* normal < NAN or PINF */
  }
  else
  {
    result = cmp_var_common(NUMERIC_DIGITS(num1), NUMERIC_NDIGITS(num1),
                NUMERIC_WEIGHT(num1), NUMERIC_SIGN(num1),
                NUMERIC_DIGITS(num2), NUMERIC_NDIGITS(num2),
                NUMERIC_WEIGHT(num2), NUMERIC_SIGN(num2));
  }

  return result;
}

/**
 * @ingroup meos_base_numeric
 * @brief Return the 32-bit hash value of a numeric value
 * @note Derived from PostgreSQL function @p hash_numeric()
 */
uint32
numeric_hash(Numeric num)
{
  Datum    digit_hash;
  Datum    result;
  int      weight;
  int      start_offset;
  int      end_offset;
  int      i;
  int      hash_len;
  NumericDigit *digits;

  /* If it's NaN or infinity, don't try to hash the rest of the fields */
  if (NUMERIC_IS_SPECIAL(num))
    return 0;

  weight = NUMERIC_WEIGHT(num);
  start_offset = 0;
  end_offset = 0;

  /*
   * Omit any leading or trailing zeros from the input to the hash. The
   * numeric implementation *should* guarantee that leading and trailing
   * zeros are suppressed, but we're paranoid. Note that we measure the
   * starting and ending offsets in units of NumericDigits, not bytes.
   */
  digits = NUMERIC_DIGITS(num);
  for (i = 0; i < (int) NUMERIC_NDIGITS(num); i++)
  {
    if (digits[i] != (NumericDigit) 0)
      break;

    start_offset++;

    /*
     * The weight is effectively the # of digits before the decimal point,
     * so decrement it for each leading zero we skip.
     */
    weight--;
  }

  /*
   * If there are no non-zero digits, then the value of the number is zero,
   * regardless of any other fields.
   */
  if ((int) NUMERIC_NDIGITS(num) == start_offset)
    return -1;

  for (i = (int) NUMERIC_NDIGITS(num) - 1; i >= 0; i--)
  {
    if (digits[i] != (NumericDigit) 0)
      break;
    end_offset++;
  }

  /* If we get here, there should be at least one non-zero digit */
  Assert(start_offset + end_offset < NUMERIC_NDIGITS(num));

  /*
   * Note that we don't hash on the Numeric's scale, since two numerics can
   * compare equal but have different scales. We also don't hash on the
   * sign, although we could: since a sign difference implies inequality,
   * this shouldn't affect correctness.
   */
  hash_len = NUMERIC_NDIGITS(num) - start_offset - end_offset;
  digit_hash = hash_any((unsigned char *) (NUMERIC_DIGITS(num) + start_offset),
    hash_len * sizeof(NumericDigit));

  /* Mix in the weight, via XOR */
  result = digit_hash ^ weight;

  return DatumGetInt32(result);
}

/**
 * @ingroup meos_base_numeric
 * @brief Return the 64-bit hash value of a numeric value with a seed
 * @note Derived from PostgreSQL function @p hash_numeric_extended()
 */
uint64
numeric_hash_extended(Numeric num, uint64 seed)
{
  Datum    digit_hash;
  Datum    result;
  int      weight;
  int      start_offset;
  int      end_offset;
  int      i;
  int      hash_len;
  NumericDigit *digits;

  /* If it's NaN or infinity, don't try to hash the rest of the fields */
  if (NUMERIC_IS_SPECIAL(num))
    return seed;

  weight = NUMERIC_WEIGHT(num);
  start_offset = 0;
  end_offset = 0;

  digits = NUMERIC_DIGITS(num);
  for (i = 0; i < (int) NUMERIC_NDIGITS(num); i++)
  {
    if (digits[i] != (NumericDigit) 0)
      break;

    start_offset++;

    weight--;
  }

  if ((int) NUMERIC_NDIGITS(num) == start_offset)
    return (seed - 1);

  for (i = (int) NUMERIC_NDIGITS(num) - 1; i >= 0; i--)
  {
    if (digits[i] != (NumericDigit) 0)
      break;

    end_offset++;
  }

  Assert(start_offset + end_offset < NUMERIC_NDIGITS(num));

  hash_len = NUMERIC_NDIGITS(num) - start_offset - end_offset;
  digit_hash = hash_any_extended((unsigned char *) (NUMERIC_DIGITS(num)
                            + start_offset),
                   hash_len * sizeof(NumericDigit),
                   seed);

  result = DatumGetUInt64(digit_hash) ^ weight;
  return result;
}

/* ----------------------------------------------------------------------
 *
 * Basic arithmetic functions
 *
 * ----------------------------------------------------------------------
 */

/**
 * @ingroup meos_base_numeric
 * @brief Return the addition of two numeric values
 * @note Derived from PostgreSQL function @p numeric_add()
 */
Numeric
numeric_add(Numeric num1, Numeric num2)
{
  return numeric_add_opt_error(num1, num2, NULL);
}

/*
 * numeric_add_opt_error() -
 *
 *  Internal version of numeric_add().  If "*have_error" flag is provided,
 *  on error it's set to true, NULL returned.  This is helpful when caller
 *  need to handle errors by itself.
 */
Numeric
numeric_add_opt_error(Numeric num1, Numeric num2, bool *have_error)
{
  NumericVar  arg1;
  NumericVar  arg2;
  NumericVar  result;
  Numeric    res;

  /*
   * Handle NaN and infinities
   */
  if (NUMERIC_IS_SPECIAL(num1) || NUMERIC_IS_SPECIAL(num2))
  {
    if (NUMERIC_IS_NAN(num1) || NUMERIC_IS_NAN(num2))
      return make_result(&const_nan);
    if (NUMERIC_IS_PINF(num1))
    {
      if (NUMERIC_IS_NINF(num2))
        return make_result(&const_nan); /* Inf + -Inf */
      else
        return make_result(&const_pinf);
    }
    if (NUMERIC_IS_NINF(num1))
    {
      if (NUMERIC_IS_PINF(num2))
        return make_result(&const_nan); /* -Inf + Inf */
      else
        return make_result(&const_ninf);
    }
    /* by here, num1 must be finite, so num2 is not */
    if (NUMERIC_IS_PINF(num2))
      return make_result(&const_pinf);
    Assert(NUMERIC_IS_NINF(num2));
    return make_result(&const_ninf);
  }

  /*
   * Unpack the values, let add_var() compute the result and return it.
   */
  init_var_from_num(num1, &arg1);
  init_var_from_num(num2, &arg2);

  init_var(&result);
  add_var(&arg1, &arg2, &result);

  res = make_result_opt_error(&result, have_error);

  free_var(&result);

  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Return the subtraction of two numeric values
 * @note Derived from PostgreSQL function @p numeric_sub()
 */
Numeric
numeric_minus(Numeric num1, Numeric num2)
{
  return numeric_sub_opt_error(num1, num2, NULL);
}

/*
 * numeric_sub_opt_error() -
 *
 *  Internal version of numeric_sub().  If "*have_error" flag is provided,
 *  on error it's set to true, NULL returned.  This is helpful when caller
 *  need to handle errors by itself.
 */
Numeric
numeric_sub_opt_error(Numeric num1, Numeric num2, bool *have_error)
{
  NumericVar  arg1;
  NumericVar  arg2;
  NumericVar  result;
  Numeric    res;

  /*
   * Handle NaN and infinities
   */
  if (NUMERIC_IS_SPECIAL(num1) || NUMERIC_IS_SPECIAL(num2))
  {
    if (NUMERIC_IS_NAN(num1) || NUMERIC_IS_NAN(num2))
      return make_result(&const_nan);
    if (NUMERIC_IS_PINF(num1))
    {
      if (NUMERIC_IS_PINF(num2))
        return make_result(&const_nan); /* Inf - Inf */
      else
        return make_result(&const_pinf);
    }
    if (NUMERIC_IS_NINF(num1))
    {
      if (NUMERIC_IS_NINF(num2))
        return make_result(&const_nan); /* -Inf - -Inf */
      else
        return make_result(&const_ninf);
    }
    /* by here, num1 must be finite, so num2 is not */
    if (NUMERIC_IS_PINF(num2))
      return make_result(&const_ninf);
    Assert(NUMERIC_IS_NINF(num2));
    return make_result(&const_pinf);
  }

  /*
   * Unpack the values, let sub_var() compute the result and return it.
   */
  init_var_from_num(num1, &arg1);
  init_var_from_num(num2, &arg2);

  init_var(&result);
  sub_var(&arg1, &arg2, &result);

  res = make_result_opt_error(&result, have_error);

  free_var(&result);

  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Return the product of two numeric values
 * @note Derived from PostgreSQL function @p numeric_mul()
 */
#if MEOS
Numeric
numeric_mul(Numeric num1, Numeric num2)
{
  return pg_numeric_mul(num1, num2);
}
#endif
Numeric
pg_numeric_mul(Numeric num1, Numeric num2)
{
  return numeric_mul_opt_error(num1, num2, NULL);
}

/*
 * numeric_mul_opt_error() -
 *
 *  Internal version of numeric_mul().  If "*have_error" flag is provided,
 *  on error it's set to true, NULL returned.  This is helpful when caller
 *  need to handle errors by itself.
 */
Numeric
numeric_mul_opt_error(Numeric num1, Numeric num2, bool *have_error)
{
  NumericVar  arg1;
  NumericVar  arg2;
  NumericVar  result;
  Numeric    res;

  /*
   * Handle NaN and infinities
   */
  if (NUMERIC_IS_SPECIAL(num1) || NUMERIC_IS_SPECIAL(num2))
  {
    if (NUMERIC_IS_NAN(num1) || NUMERIC_IS_NAN(num2))
      return make_result(&const_nan);
    if (NUMERIC_IS_PINF(num1))
    {
      switch (numeric_sign_internal(num2))
      {
        case 0:
          return make_result(&const_nan); /* Inf * 0 */
        case 1:
          return make_result(&const_pinf);
        case -1:
          return make_result(&const_ninf);
      }
      Assert(false);
    }
    if (NUMERIC_IS_NINF(num1))
    {
      switch (numeric_sign_internal(num2))
      {
        case 0:
          return make_result(&const_nan); /* -Inf * 0 */
        case 1:
          return make_result(&const_ninf);
        case -1:
          return make_result(&const_pinf);
      }
      Assert(false);
    }
    /* by here, num1 must be finite, so num2 is not */
    if (NUMERIC_IS_PINF(num2))
    {
      switch (numeric_sign_internal(num1))
      {
        case 0:
          return make_result(&const_nan); /* 0 * Inf */
        case 1:
          return make_result(&const_pinf);
        case -1:
          return make_result(&const_ninf);
      }
      Assert(false);
    }
    Assert(NUMERIC_IS_NINF(num2));
    switch (numeric_sign_internal(num1))
    {
      case 0:
        return make_result(&const_nan); /* 0 * -Inf */
      case 1:
        return make_result(&const_ninf);
      case -1:
        return make_result(&const_pinf);
    }
    Assert(false);
  }

  /*
   * Unpack the values, let mul_var() compute the result and return it.
   * Unlike add_var() and sub_var(), mul_var() will round its result. In the
   * case of numeric_mul(), which is invoked for the * operator on numerics,
   * we request exact representation for the product (rscale = sum(dscale of
   * arg1, dscale of arg2)).  If the exact result has more digits after the
   * decimal point than can be stored in a numeric, we round it.  Rounding
   * after computing the exact result ensures that the final result is
   * correctly rounded (rounding in mul_var() using a truncated product
   * would not guarantee this).
   */
  init_var_from_num(num1, &arg1);
  init_var_from_num(num2, &arg2);

  init_var(&result);
  mul_var(&arg1, &arg2, &result, arg1.dscale + arg2.dscale);

  if (result.dscale > NUMERIC_DSCALE_MAX)
    round_var(&result, NUMERIC_DSCALE_MAX);

  res = make_result_opt_error(&result, have_error);

  free_var(&result);

  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Return the division of one numeric by another
 * @note Derived from PostgreSQL function @p numeric_div()
 */
#if MEOS
Numeric
numeric_div(Numeric num1, Numeric num2)
{
  return pg_numeric_div(num1, num2);
}
#endif
Numeric
pg_numeric_div(Numeric num1, Numeric num2)
{
  return numeric_div_opt_error(num1, num2, NULL);
}

/*
 * numeric_div_opt_error() -
 *
 *  Internal version of numeric_div().  If "*have_error" flag is provided,
 *  on error it's set to true, NULL returned.  This is helpful when caller
 *  need to handle errors by itself.
 */
Numeric
numeric_div_opt_error(Numeric num1, Numeric num2, bool *have_error)
{
  NumericVar  arg1;
  NumericVar  arg2;
  NumericVar  result;
  Numeric    res;
  int      rscale;

  if (have_error)
    *have_error = false;

  /*
   * Handle NaN and infinities
   */
  if (NUMERIC_IS_SPECIAL(num1) || NUMERIC_IS_SPECIAL(num2))
  {
    if (NUMERIC_IS_NAN(num1) || NUMERIC_IS_NAN(num2))
      return make_result(&const_nan);
    if (NUMERIC_IS_PINF(num1))
    {
      if (NUMERIC_IS_SPECIAL(num2))
        return make_result(&const_nan); /* Inf / [-]Inf */
      switch (numeric_sign_internal(num2))
      {
        case 0:
          if (have_error)
            *have_error = true;
          elog(ERROR, "division by zero");
          return NULL;
        case 1:
          return make_result(&const_pinf);
        case -1:
          return make_result(&const_ninf);
      }
      Assert(false);
    }
    if (NUMERIC_IS_NINF(num1))
    {
      if (NUMERIC_IS_SPECIAL(num2))
        return make_result(&const_nan); /* -Inf / [-]Inf */
      switch (numeric_sign_internal(num2))
      {
        case 0:
          if (have_error)
            *have_error = true;
          elog(ERROR, "division by zero");
          return NULL;
        case 1:
          return make_result(&const_ninf);
        case -1:
          return make_result(&const_pinf);
      }
      Assert(false);
    }
    /* by here, num1 must be finite, so num2 is not */

    /*
     * POSIX would have us return zero or minus zero if num1 is zero, and
     * otherwise throw an underflow error.  But the numeric type doesn't
     * really do underflow, so let's just return zero.
     */
    return make_result(&const_zero);
  }

  /*
   * Unpack the arguments
   */
  init_var_from_num(num1, &arg1);
  init_var_from_num(num2, &arg2);

  init_var(&result);

  /*
   * Select scale for division result
   */
  rscale = select_div_scale(&arg1, &arg2);

  /*
   * If "have_error" is provided, check for division by zero here
   */
  if (have_error && (arg2.ndigits == 0 || arg2.digits[0] == 0))
  {
    *have_error = true;
    return NULL;
  }

  /*
   * Do the divide and return the result
   */
  div_var(&arg1, &arg2, &result, rscale, true, true);

  res = make_result_opt_error(&result, have_error);

  free_var(&result);

  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Return the division of one numeric by another, truncating the result
 * to an integer
 * @note Derived from PostgreSQL function @p numeric_div_trunc()
 */
#if MEOS
Numeric
numeric_div_trunc(Numeric num1, Numeric num2)
{
  return pg_numeric_div_trunc(num1, num2);
}
#endif
Numeric
pg_numeric_div_trunc(Numeric num1, Numeric num2)
{
  NumericVar  arg1;
  NumericVar  arg2;
  NumericVar  result;
  Numeric    res;

  /*
   * Handle NaN and infinities
   */
  if (NUMERIC_IS_SPECIAL(num1) || NUMERIC_IS_SPECIAL(num2))
  {
    if (NUMERIC_IS_NAN(num1) || NUMERIC_IS_NAN(num2))
      return make_result(&const_nan);
    if (NUMERIC_IS_PINF(num1))
    {
      if (NUMERIC_IS_SPECIAL(num2))
        return make_result(&const_nan); /* Inf / [-]Inf */
      switch (numeric_sign_internal(num2))
      {
        case 0:
          elog(ERROR, "division by zero");
          return NULL;
        case 1:
          return make_result(&const_pinf);
        case -1:
          return make_result(&const_ninf);
      }
      Assert(false);
    }
    if (NUMERIC_IS_NINF(num1))
    {
      if (NUMERIC_IS_SPECIAL(num2))
        return make_result(&const_nan); /* -Inf / [-]Inf */
      switch (numeric_sign_internal(num2))
      {
        case 0:
          elog(ERROR, "division by zero");
          return NULL;
        case 1:
          return make_result(&const_ninf);
        case -1:
          return make_result(&const_pinf);
      }
      Assert(false);
    }
    /* by here, num1 must be finite, so num2 is not */

    /*
     * POSIX would have us return zero or minus zero if num1 is zero, and
     * otherwise throw an underflow error.  But the numeric type doesn't
     * really do underflow, so let's just return zero.
     */
    return make_result(&const_zero);
  }

  /*
   * Unpack the arguments
   */
  init_var_from_num(num1, &arg1);
  init_var_from_num(num2, &arg2);

  init_var(&result);

  /*
   * Do the divide and return the result
   */
  div_var(&arg1, &arg2, &result, 0, false, true);

  res = make_result(&result);

  free_var(&result);

  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Return modulo of two numeric values
 * @note Derived from PostgreSQL function @p numeric_mod()
 */
#if MEOS
Numeric
numeric_mod(Numeric num1, Numeric num2)
{
  return pg_numeric_mod(num1, num2);
}
#endif
Numeric
pg_numeric_mod(Numeric num1, Numeric num2)
{
  return numeric_mod_opt_error(num1, num2, NULL);
}

/*
 * numeric_mod_opt_error() -
 *
 *  Internal version of numeric_mod().  If "*have_error" flag is provided,
 *  on error it's set to true, NULL returned.  This is helpful when caller
 *  need to handle errors by itself.
 */
Numeric
numeric_mod_opt_error(Numeric num1, Numeric num2, bool *have_error)
{
  Numeric    res;
  NumericVar  arg1;
  NumericVar  arg2;
  NumericVar  result;

  if (have_error)
    *have_error = false;

  /*
   * Handle NaN and infinities.  We follow POSIX fmod() on this, except that
   * POSIX treats x-is-infinite and y-is-zero identically, raising EDOM and
   * returning NaN.  We choose to throw error only for y-is-zero.
   */
  if (NUMERIC_IS_SPECIAL(num1) || NUMERIC_IS_SPECIAL(num2))
  {
    if (NUMERIC_IS_NAN(num1) || NUMERIC_IS_NAN(num2))
      return make_result(&const_nan);
    if (NUMERIC_IS_INF(num1))
    {
      if (numeric_sign_internal(num2) == 0)
      {
        if (have_error)
          *have_error = true;
        elog(ERROR, "division by zero");
        return NULL;
      }
      /* Inf % any nonzero = NaN */
      return make_result(&const_nan);
    }
    /* num2 must be [-]Inf; result is num1 regardless of sign of num2 */
    return duplicate_numeric(num1);
  }

  init_var_from_num(num1, &arg1);
  init_var_from_num(num2, &arg2);

  init_var(&result);

  /*
   * If "have_error" is provided, check for division by zero here
   */
  if (have_error && (arg2.ndigits == 0 || arg2.digits[0] == 0))
  {
    *have_error = true;
    return NULL;
  }

  mod_var(&arg1, &arg2, &result);

  res = make_result_opt_error(&result, NULL);

  free_var(&result);

  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Return a numeric value incremented by one
 * @note Derived from PostgreSQL function @p numeric_inc()
 */
#if MEOS
Numeric
numeric_inc(Numeric num)
{
  return pg_numeric_inc(num);
}
#endif
Numeric
pg_numeric_inc(Numeric num)
{
  NumericVar  arg;
  Numeric    res;

  /*
   * Handle NaN and infinities
   */
  if (NUMERIC_IS_SPECIAL(num))
    return duplicate_numeric(num);

  /*
   * Compute the result and return it
   */
  init_var_from_num(num, &arg);

  add_var(&arg, &const_one, &arg);

  res = make_result(&arg);

  free_var(&arg);

  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Return the smaller of two numeric values
 * @note Derived from PostgreSQL function @p numeric_smaller()
 */
#if MEOS
Numeric
numeric_smaller(Numeric num1, Numeric num2)
{
  return pg_numeric_smaller(num1, num2);
}
#endif
Numeric
pg_numeric_smaller(Numeric num1, Numeric num2)
{
  /*
   * Use cmp_numerics so that this will agree with the comparison operators,
   * particularly as regards comparisons involving NaN.
   */
  if (cmp_numerics(num1, num2) < 0)
    return numeric_copy(num1);
  else
    return numeric_copy(num2);
}

/**
 * @ingroup meos_base_numeric
 * @brief Return the larger of two numeric values
 * @note Derived from PostgreSQL function @p numeric_larger()
 */
#if MEOS
Numeric
numeric_larger(Numeric num1, Numeric num2)
{
  return pg_numeric_larger(num1, num2);
}
#endif
Numeric
pg_numeric_larger(Numeric num1, Numeric num2)
{
  /*
   * Use cmp_numerics so that this will agree with the comparison operators,
   * particularly as regards comparisons involving NaN.
   */
  if (cmp_numerics(num1, num2) > 0)
    return numeric_copy(num1);
  else
    return numeric_copy(num2);
}

/* ----------------------------------------------------------------------
 *
 * Advanced math functions
 *
 * ----------------------------------------------------------------------
 */

/**
 * @ingroup meos_base_numeric
 * @brief Return the greatest common divisor of two numeric values
 * @note Derived from PostgreSQL function @p numeric_gcd()
 */
#if MEOS
Numeric
numeric_gcd(Numeric num1, Numeric num2)
{
  return pg_numeric_gcd(num1, num2);
}
#endif
Numeric
pg_numeric_gcd(Numeric num1, Numeric num2)
{
  NumericVar  arg1;
  NumericVar  arg2;
  NumericVar  result;
  Numeric    res;

  /*
   * Handle NaN and infinities: we consider the result to be NaN in all such
   * cases.
   */
  if (NUMERIC_IS_SPECIAL(num1) || NUMERIC_IS_SPECIAL(num2))
    return make_result(&const_nan);

  /*
   * Unpack the arguments
   */
  init_var_from_num(num1, &arg1);
  init_var_from_num(num2, &arg2);

  init_var(&result);

  /*
   * Find the GCD and return the result
   */
  gcd_var(&arg1, &arg2, &result);

  res = make_result(&result);

  free_var(&result);

  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Return the least common multiple of two numeric values
 * @note Derived from PostgreSQL function @p numeric_lcm()
 */
#if MEOS
Numeric
numeric_lcm(Numeric num1, Numeric num2)
{
  return pg_numeric_lcm(num1, num2);
}
#endif
Numeric
pg_numeric_lcm(Numeric num1, Numeric num2)
{
  NumericVar  arg1;
  NumericVar  arg2;
  NumericVar  result;
  Numeric    res;

  /*
   * Handle NaN and infinities: we consider the result to be NaN in all such
   * cases.
   */
  if (NUMERIC_IS_SPECIAL(num1) || NUMERIC_IS_SPECIAL(num2))
    return make_result(&const_nan);

  /*
   * Unpack the arguments
   */
  init_var_from_num(num1, &arg1);
  init_var_from_num(num2, &arg2);

  init_var(&result);

  /*
   * Compute the result using lcm(x, y) = abs(x / gcd(x, y) * y), returning
   * zero if either input is zero.
   *
   * Note that the division is guaranteed to be exact, returning an integer
   * result, so the LCM is an integral multiple of both x and y.  A display
   * scale of Min(x.dscale, y.dscale) would be sufficient to represent it,
   * but as with other numeric functions, we choose to return a result whose
   * display scale is no smaller than either input.
   */
  if (arg1.ndigits == 0 || arg2.ndigits == 0)
    set_var_from_var(&const_zero, &result);
  else
  {
    gcd_var(&arg1, &arg2, &result);
    div_var(&arg1, &result, &result, 0, false, true);
    mul_var(&arg2, &result, &result, arg2.dscale);
    result.sign = NUMERIC_POS;
  }

  result.dscale = Max(arg1.dscale, arg2.dscale);

  res = make_result(&result);

  free_var(&result);

  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Return the factorial of a numeric value
 * @note Derived from PostgreSQL function @p numeric_fac()
 */
#if MEOS
Numeric
numeric_fac(int64 num)
{
  return pg_numeric_fac(num);
}
#endif
Numeric
pg_numeric_fac(int64 num)
{
  Numeric res;
  NumericVar fact;
  NumericVar result;

  if (num < 0)
  {
    elog(ERROR, "factorial of a negative number is undefined");
    return NULL;
  }
  if (num <= 1)
  {
    res = make_result(&const_one);
    return res;
  }
  /* Fail immediately if the result would overflow */
  if (num > 32177)
  {
    elog(ERROR, "value overflows numeric format");
    return NULL;
  }

  init_var(&fact);
  init_var(&result);

  int64_to_numericvar(num, &result);

  for (num = num - 1; num > 1; num--)
  {
    int64_to_numericvar(num, &fact);
    mul_var(&result, &fact, &result, 0);
  }

  res = make_result(&result);

  free_var(&fact);
  free_var(&result);

  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Return the square root of a numeric
 * @note Derived from PostgreSQL function @p numeric_sqrt()
 */
#if MEOS
Numeric
numeric_sqrt(Numeric num)
{
  return pg_numeric_sqrt(num);
}
#endif
Numeric
pg_numeric_sqrt(Numeric num)
{
  Numeric    res;
  NumericVar  arg;
  NumericVar  result;
  int      sweight;
  int      rscale;

  /*
   * Handle NaN and infinities
   */
  if (NUMERIC_IS_SPECIAL(num))
  {
    /* error should match that in sqrt_var() */
    if (NUMERIC_IS_NINF(num))
    {
      elog(ERROR, "cannot take square root of a negative number");
      return NULL;
    }
    /* For NAN or PINF, just duplicate the input */
    return duplicate_numeric(num);
  }

  /*
   * Unpack the argument and determine the result scale.  We choose a scale
   * to give at least NUMERIC_MIN_SIG_DIGITS significant digits; but in any
   * case not less than the input's dscale.
   */
  init_var_from_num(num, &arg);

  init_var(&result);

  /*
   * Assume the input was normalized, so arg.weight is accurate.  The result
   * then has at least sweight = floor(arg.weight * DEC_DIGITS / 2 + 1)
   * digits before the decimal point.  When DEC_DIGITS is even, we can save
   * a few cycles, since the division is exact and there is no need to round
   * towards negative infinity.
   */
#if DEC_DIGITS == ((DEC_DIGITS / 2) * 2)
  sweight = arg.weight * DEC_DIGITS / 2 + 1;
#else
  if (arg.weight >= 0)
    sweight = arg.weight * DEC_DIGITS / 2 + 1;
  else
    sweight = 1 - (1 - arg.weight * DEC_DIGITS) / 2;
#endif

  rscale = NUMERIC_MIN_SIG_DIGITS - sweight;
  rscale = Max(rscale, arg.dscale);
  rscale = Max(rscale, NUMERIC_MIN_DISPLAY_SCALE);
  rscale = Min(rscale, NUMERIC_MAX_DISPLAY_SCALE);

  /*
   * Let sqrt_var() do the calculation and return the result.
   */
  sqrt_var(&arg, &result, rscale);

  res = make_result(&result);

  free_var(&result);

  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Return e raised to the power of a numeric value
 * @note Derived from PostgreSQL function @p numeric_exp()
 */
#if MEOS
Numeric
numeric_exp(Numeric num)
{
  return pg_numeric_exp(num);
}
#endif
Numeric
pg_numeric_exp(Numeric num)
{
  Numeric    res;
  NumericVar  arg;
  NumericVar  result;
  int      rscale;
  double    val;

  /*
   * Handle NaN and infinities
   */
  if (NUMERIC_IS_SPECIAL(num))
  {
    /* Per POSIX, exp(-Inf) is zero */
    if (NUMERIC_IS_NINF(num))
      return make_result(&const_zero);
    /* For NAN or PINF, just duplicate the input */
    return duplicate_numeric(num);
  }

  /*
   * Unpack the argument and determine the result scale.  We choose a scale
   * to give at least NUMERIC_MIN_SIG_DIGITS significant digits; but in any
   * case not less than the input's dscale.
   */
  init_var_from_num(num, &arg);

  init_var(&result);

  /* convert input to float8, ignoring overflow */
  val = numericvar_to_double_no_overflow(&arg);

  /*
   * log10(result) = num * log10(e), so this is approximately the decimal
   * weight of the result:
   */
  val *= 0.434294481903252;

  /* limit to something that won't cause integer overflow */
  val = Max(val, -NUMERIC_MAX_RESULT_SCALE);
  val = Min(val, NUMERIC_MAX_RESULT_SCALE);

  rscale = NUMERIC_MIN_SIG_DIGITS - (int) val;
  rscale = Max(rscale, arg.dscale);
  rscale = Max(rscale, NUMERIC_MIN_DISPLAY_SCALE);
  rscale = Min(rscale, NUMERIC_MAX_DISPLAY_SCALE);

  /*
   * Let exp_var() do the calculation and return the result.
   */
  exp_var(&arg, &result, rscale);

  res = make_result(&result);

  free_var(&result);

  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Return the natural logarithm of a numeric value
 * @note Derived from PostgreSQL function @p numeric_ln()
 */
#if MEOS
Numeric
numeric_ln(Numeric num)
{
  return pg_numeric_ln(num);
}
#endif
Numeric
pg_numeric_ln(Numeric num)
{
  Numeric    res;
  NumericVar  arg;
  NumericVar  result;
  int      ln_dweight;
  int      rscale;

  /*
   * Handle NaN and infinities
   */
  if (NUMERIC_IS_SPECIAL(num))
  {
    if (NUMERIC_IS_NINF(num))
    {
      elog(ERROR, "cannot take logarithm of a negative number");
      return NULL;
    }
    /* For NAN or PINF, just duplicate the input */
    return duplicate_numeric(num);
  }

  init_var_from_num(num, &arg);
  init_var(&result);

  /* Estimated dweight of logarithm */
  ln_dweight = estimate_ln_dweight(&arg);

  rscale = NUMERIC_MIN_SIG_DIGITS - ln_dweight;
  rscale = Max(rscale, arg.dscale);
  rscale = Max(rscale, NUMERIC_MIN_DISPLAY_SCALE);
  rscale = Min(rscale, NUMERIC_MAX_DISPLAY_SCALE);

  ln_var(&arg, &result, rscale);

  res = make_result(&result);

  free_var(&result);

  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Return the logarithm of x in a given base
 * @note Derived from PostgreSQL function @p numeric_log()
 */
#if MEOS
Numeric
numeric_log(Numeric num1, Numeric num2)
{
  return pg_numeric_log(num1, num2);
}
#endif
Numeric
pg_numeric_log(Numeric num1, Numeric num2)
{
  
  Numeric    res;
  NumericVar  arg1;
  NumericVar  arg2;
  NumericVar  result;

  /*
   * Handle NaN and infinities
   */
  if (NUMERIC_IS_SPECIAL(num1) || NUMERIC_IS_SPECIAL(num2))
  {
    int      sign1,
          sign2;

    if (NUMERIC_IS_NAN(num1) || NUMERIC_IS_NAN(num2))
      return make_result(&const_nan);
    /* fail on negative inputs including -Inf, as log_var would */
    sign1 = numeric_sign_internal(num1);
    sign2 = numeric_sign_internal(num2);
    if (sign1 < 0 || sign2 < 0)
    {
      elog(ERROR, "cannot take logarithm of a negative number");
      return NULL;
    }
    /* fail on zero inputs, as log_var would */
    if (sign1 == 0 || sign2 == 0)
    {
      elog(ERROR, "cannot take logarithm of zero");
      return NULL;
    }
    if (NUMERIC_IS_PINF(num1))
    {
      /* log(Inf, Inf) reduces to Inf/Inf, so it's NaN */
      if (NUMERIC_IS_PINF(num2))
        return make_result(&const_nan);
      /* log(Inf, finite-positive) is zero (we don't throw underflow) */
      return make_result(&const_zero);
    }
    Assert(NUMERIC_IS_PINF(num2));
    /* log(finite-positive, Inf) is Inf */
    return make_result(&const_pinf);
  }

  /*
   * Initialize things
   */
  init_var_from_num(num1, &arg1);
  init_var_from_num(num2, &arg2);
  init_var(&result);

  /*
   * Call log_var() to compute and return the result; note it handles scale
   * selection itself.
   */
  log_var(&arg1, &arg2, &result);

  res = make_result(&result);

  free_var(&result);

  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Raise x to the power of y
 * @note Derived from PostgreSQL function @p numeric_power()
 */
Numeric
numeric_pow(Numeric num1, Numeric num2)
{
  NumericVar arg1;
  NumericVar arg2;
  int sign1, sign2;

  /*
   * Handle NaN and infinities
   */
  if (NUMERIC_IS_SPECIAL(num1) || NUMERIC_IS_SPECIAL(num2))
  {
    /*
     * We follow the POSIX spec for pow(3), which says that NaN ^ 0 = 1,
     * and 1 ^ NaN = 1, while all other cases with NaN inputs yield NaN
     * (with no error).
     */
    if (NUMERIC_IS_NAN(num1))
    {
      if (!NUMERIC_IS_SPECIAL(num2))
      {
        init_var_from_num(num2, &arg2);
        if (cmp_var(&arg2, &const_zero) == 0)
          return make_result(&const_one);
      }
      return make_result(&const_nan);
    }
    if (NUMERIC_IS_NAN(num2))
    {
      if (!NUMERIC_IS_SPECIAL(num1))
      {
        init_var_from_num(num1, &arg1);
        if (cmp_var(&arg1, &const_one) == 0)
          return make_result(&const_one);
      }
      return make_result(&const_nan);
    }
    /* At least one input is infinite, but error rules still apply */
    sign1 = numeric_sign_internal(num1);
    sign2 = numeric_sign_internal(num2);
    if (sign1 == 0 && sign2 < 0)
    {
      elog(ERROR, "zero raised to a negative power is undefined");
      return NULL;
    }
    if (sign1 < 0 && !numeric_is_integral(num2))
    {
      elog(ERROR, 
        "a negative number raised to a non-integer power yields a complex result");
      return NULL;
    }

    /*
     * POSIX gives this series of rules for pow(3) with infinite inputs:
     *
     * For any value of y, if x is +1, 1.0 shall be returned.
     */
    if (!NUMERIC_IS_SPECIAL(num1))
    {
      init_var_from_num(num1, &arg1);
      if (cmp_var(&arg1, &const_one) == 0)
        return make_result(&const_one);
    }

    /*
     * For any value of x, if y is [-]0, 1.0 shall be returned.
     */
    if (sign2 == 0)
      return make_result(&const_one);

    /*
     * For any odd integer value of y > 0, if x is [-]0, [-]0 shall be
     * returned.  For y > 0 and not an odd integer, if x is [-]0, +0 shall
     * be returned.  (Since we don't deal in minus zero, we need not
     * distinguish these two cases.)
     */
    if (sign1 == 0 && sign2 > 0)
      return make_result(&const_zero);

    /*
     * If x is -1, and y is [-]Inf, 1.0 shall be returned.
     *
     * For |x| < 1, if y is -Inf, +Inf shall be returned.
     *
     * For |x| > 1, if y is -Inf, +0 shall be returned.
     *
     * For |x| < 1, if y is +Inf, +0 shall be returned.
     *
     * For |x| > 1, if y is +Inf, +Inf shall be returned.
     */
    if (NUMERIC_IS_INF(num2))
    {
      bool    abs_x_gt_one;

      if (NUMERIC_IS_SPECIAL(num1))
        abs_x_gt_one = true;  /* x is either Inf or -Inf */
      else
      {
        init_var_from_num(num1, &arg1);
        if (cmp_var(&arg1, &const_minus_one) == 0)
          return make_result(&const_one);
        arg1.sign = NUMERIC_POS;  /* now arg1 = abs(x) */
        abs_x_gt_one = (cmp_var(&arg1, &const_one) > 0);
      }
      if (abs_x_gt_one == (sign2 > 0))
        return make_result(&const_pinf);
      else
        return make_result(&const_zero);
    }

    /*
     * For y < 0, if x is +Inf, +0 shall be returned.
     *
     * For y > 0, if x is +Inf, +Inf shall be returned.
     */
    if (NUMERIC_IS_PINF(num1))
    {
      if (sign2 > 0)
        return make_result(&const_pinf);
      else
        return make_result(&const_zero);
    }

    Assert(NUMERIC_IS_NINF(num1));

    /*
     * For y an odd integer < 0, if x is -Inf, -0 shall be returned.  For
     * y < 0 and not an odd integer, if x is -Inf, +0 shall be returned.
     * (Again, we need not distinguish these two cases.)
     */
    if (sign2 < 0)
      return make_result(&const_zero);

    /*
     * For y an odd integer > 0, if x is -Inf, -Inf shall be returned. For
     * y > 0 and not an odd integer, if x is -Inf, +Inf shall be returned.
     */
    init_var_from_num(num2, &arg2);
    if (arg2.ndigits > 0 && arg2.ndigits == arg2.weight + 1 &&
      (arg2.digits[arg2.ndigits - 1] & 1))
      return make_result(&const_ninf);
    else
      return make_result(&const_pinf);
  }

  /*
   * The SQL spec requires that we emit a particular SQLSTATE error code for
   * certain error conditions.  Specifically, we don't return a
   * divide-by-zero error code for 0 ^ -1.  Raising a negative number to a
   * non-integer power must produce the same error code, but that case is
   * handled in power_var().
   */
  sign1 = numeric_sign_internal(num1);
  sign2 = numeric_sign_internal(num2);

  if (sign1 == 0 && sign2 < 0)
  {
    elog(ERROR, "zero raised to a negative power is undefined");
      return NULL;
    }

  /*
   * Initialize things
   */
  NumericVar res;
  init_var(&res);
  init_var_from_num(num1, &arg1);
  init_var_from_num(num2, &arg2);

  /*
   * Call power_var() to compute and return the result; note it handles
   * scale selection itself.
   */
  power_var(&arg1, &arg2, &res);

  Numeric result = make_result(&res);
  free_var(&res);

  return result;
}

/**
 * @ingroup meos_base_numeric
 * @brief Returns the scale, i.e., the count of decimal digits in the
 * fractional part
 * @note Derived from PostgreSQL function @p numeric_scale()
 */
#if MEOS
int
numeric_scale(Numeric num)
{
  return pg_numeric_scale(num);
}
#endif
int
pg_numeric_scale(Numeric num)
{
  if (NUMERIC_IS_SPECIAL(num))
    return INT_MAX;
  return NUMERIC_DSCALE(num);
}

/*
 * Calculate minimum scale for value.
 */
static int
get_min_scale(NumericVar *var)
{
  int      min_scale;
  int      last_digit_pos;

  /*
   * Ordinarily, the input value will be "stripped" so that the last
   * NumericDigit is nonzero.  But we don't want to get into an infinite
   * loop if it isn't, so explicitly find the last nonzero digit.
   */
  last_digit_pos = var->ndigits - 1;
  while (last_digit_pos >= 0 &&
       var->digits[last_digit_pos] == 0)
    last_digit_pos--;

  if (last_digit_pos >= 0)
  {
    /* compute min_scale assuming that last ndigit has no zeroes */
    min_scale = (last_digit_pos - var->weight) * DEC_DIGITS;

    /*
     * We could get a negative result if there are no digits after the
     * decimal point.  In this case the min_scale must be zero.
     */
    if (min_scale > 0)
    {
      /*
       * Reduce min_scale if trailing digit(s) in last NumericDigit are
       * zero.
       */
      NumericDigit last_digit = var->digits[last_digit_pos];

      while (last_digit % 10 == 0)
      {
        min_scale--;
        last_digit /= 10;
      }
    }
    else
      min_scale = 0;
  }
  else
    min_scale = 0;      /* result if input is zero */

  return min_scale;
}

/**
 * @ingroup meos_base_numeric
 * @brief Returns minimum scale required to represent the numeric value without loss
 * @note Derived from PostgreSQL function @p numeric_min_scale()
 */
#if MEOS
int
numeric_min_scale(Numeric num)
{
  return pg_numeric_min_scale(num);
}
#endif
int
pg_numeric_min_scale(Numeric num)
{
  NumericVar  arg;
  int      min_scale;

  if (NUMERIC_IS_SPECIAL(num))
    return INT_MAX;

  init_var_from_num(num, &arg);
  min_scale = get_min_scale(&arg);
  free_var(&arg);

  return min_scale;
}

/**
 * @ingroup meos_base_numeric
 * @brief Reduce scale of numeric value to represent it without loss
 * @note Derived from PostgreSQL function @p numeric_trim_scale()
 */
#if MEOS
Numeric
numeric_trim_scale(Numeric num)
{
  return pg_numeric_trim_scale(num);
}
#endif
Numeric
pg_numeric_trim_scale(Numeric num)
{
  Numeric    res;
  NumericVar  result;

  if (NUMERIC_IS_SPECIAL(num))
    return duplicate_numeric(num);

  init_var_from_num(num, &result);
  result.dscale = get_min_scale(&result);
  res = make_result(&result);
  free_var(&result);

  return res;
}

/* ----------------------------------------------------------------------
 *
 * Type conversion functions
 *
 * ----------------------------------------------------------------------
 */

/**
 * @ingroup meos_base_numeric
 * @brief Transform an int64 number into a numeric value
 * @note Existing PostgreSQL function
 */
Numeric
int64_to_numeric(int64 num)
{
  Numeric    res;
  NumericVar  result;

  init_var(&result);

  int64_to_numericvar(num, &result);

  res = make_result(&result);

  free_var(&result);

  return res;
}

/*
 * Convert val1/(10**log10val2) to numeric.  This is much faster than normal
 * numeric division.
 */
Numeric
int64_div_fast_to_numeric(int64 val1, int log10val2)
{
  Numeric    res;
  NumericVar  result;
  int      rscale;
  int      w;
  int      m;

  init_var(&result);

  /* result scale */
  rscale = log10val2 < 0 ? 0 : log10val2;

  /* how much to decrease the weight by */
  w = log10val2 / DEC_DIGITS;
  /* how much is left to divide by */
  m = log10val2 % DEC_DIGITS;
  if (m < 0)
  {
    m += DEC_DIGITS;
    w--;
  }

  /*
   * If there is anything left to divide by (10^m with 0 < m < DEC_DIGITS),
   * multiply the dividend by 10^(DEC_DIGITS - m), and shift the weight by
   * one more.
   */
  if (m > 0)
  {
#if DEC_DIGITS == 4
    static const int pow10[] = {1, 10, 100, 1000};
#elif DEC_DIGITS == 2
    static const int pow10[] = {1, 10};
#elif DEC_DIGITS == 1
    static const int pow10[] = {1};
#else
#error unsupported NBASE
#endif
    int64    factor = pow10[DEC_DIGITS - m];
    int64    new_val1;

    StaticAssertDecl(lengthof(pow10) == DEC_DIGITS, "mismatch with DEC_DIGITS");

    if (unlikely(pg_mul_s64_overflow(val1, factor, &new_val1)))
    {
#ifdef HAVE_INT128
      /* do the multiplication using 128-bit integers */
      int128    tmp;

      tmp = (int128) val1 * (int128) factor;

      int128_to_numericvar(tmp, &result);
#else
      /* do the multiplication using numerics */
      NumericVar  tmp;

      init_var(&tmp);

      int64_to_numericvar(val1, &result);
      int64_to_numericvar(factor, &tmp);
      mul_var(&result, &tmp, &result, 0);

      free_var(&tmp);
#endif
    }
    else
      int64_to_numericvar(new_val1, &result);

    w++;
  }
  else
    int64_to_numericvar(val1, &result);

  result.weight -= w;
  result.dscale = rscale;

  res = make_result(&result);
  free_var(&result);
  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Transform an int4 value into a numeric value 
 * @note Derived from PostgreSQL function @p numeric_int4()
 */
int32
numeric_to_int32(Numeric num)
{
  return numeric_int4_opt_error(num, NULL);
}

int32
numeric_int4_opt_error(Numeric num, bool *have_error)
{
  NumericVar  x;
  int32    result;

  if (have_error)
    *have_error = false;

  if (NUMERIC_IS_SPECIAL(num))
  {
    if (have_error)
    {
      *have_error = true;
      return 0;
    }
    else
    {
      if (NUMERIC_IS_NAN(num))
        elog(ERROR, "cannot convert NaN to %s", "integer");
      else
        elog(ERROR, "cannot convert infinity to %s", "integer");
      return INT_MAX;
    }
  }

  /* Convert to variable format, then convert to int4 */
  init_var_from_num(num, &x);

  if (!numericvar_to_int32(&x, &result))
  {
    if (have_error)
    {
      *have_error = true;
      return 0;
    }
    else
    {
      elog(ERROR, "integer out of range");
      return INT_MAX;
    }
  }

  return result;
}

/*
 * Given a NumericVar, convert it to an int32. If the NumericVar
 * exceeds the range of an int32, false is returned, otherwise true is returned.
 * The input NumericVar is *not* free'd.
 */
static bool
numericvar_to_int32(const NumericVar *var, int32 *result)
{
  int64    val;

  if (!numericvar_to_int64(var, &val))
    return false;

  if (unlikely(val < PG_INT32_MIN) || unlikely(val > PG_INT32_MAX))
    return false;

  /* Down-convert to int4 */
  *result = (int32) val;

  return true;
}


int64
numeric_int8_opt_error(Numeric num, bool *have_error)
{
  NumericVar  x;
  int64    result;

  if (have_error)
    *have_error = false;

  if (NUMERIC_IS_SPECIAL(num))
  {
    if (have_error)
    {
      *have_error = true;
      return 0;
    }
    else
    {
      if (NUMERIC_IS_NAN(num))
        elog(ERROR, "cannot convert NaN to %s", "bigint");
      else
        elog(ERROR, "cannot convert infinity to %s", "bigint");
      return LONG_MAX;
    }
  }

  /* Convert to variable format, then convert to int8 */
  init_var_from_num(num, &x);

  if (!numericvar_to_int64(&x, &result))
  {
    if (have_error)
    {
      *have_error = true;
      return 0;
    }
    else
    {
      elog(ERROR, "bigint out of range");
      return LONG_MAX;
    }
  }

  return result;
}

/**
 * @ingroup meos_base_numeric
 * @brief Transform a numeric value into an int64 number
 * @note Derived from PostgreSQL function @p numeric_int8()
 */
int64
numeric_to_int64(Numeric num)
{
  return numeric_int8_opt_error(num, NULL);
}

/**
 * @ingroup meos_base_numeric
 * @brief Transform an int2 value into a numeric value
 * @note Derived from PostgreSQL function @p numeric_int8()
 */
Numeric
int16_to_numeric(int16 num)
{
  return int64_to_numeric(num);
}

/**
 * @ingroup meos_base_numeric
 * @brief Transform a numeric value into an int2 value
 * @note Derived from PostgreSQL function @p numeric_int2()
 */
int16
numeric_to_int16(Numeric num)
{
  NumericVar  x;
  int64    val;
  int16    result;

  if (NUMERIC_IS_SPECIAL(num))
  {
    if (NUMERIC_IS_NAN(num))
      elog(ERROR, "cannot convert NaN to %s", "smallint");
    else
      elog(ERROR, "cannot convert infinity to %s", "smallint");
    return INT16_MAX;
  }

  /* Convert to variable format and thence to int8 */
  init_var_from_num(num, &x);

  if (!numericvar_to_int64(&x, &val))
  {
    elog(ERROR, "smallint out of range");
    return INT16_MAX;
  }

  if (unlikely(val < PG_INT16_MIN) || unlikely(val > PG_INT16_MAX))
  {
    elog(ERROR, "smallint out of range");
    return INT16_MAX;
  }

  /* Down-convert to int2 */
  result = (int16) val;

  return result;
}

/**
 * @ingroup meos_base_numeric
 * @brief Transform a float8 value into a numeric value
 * @note Derived from PostgreSQL function @p numeric_float8()
 */
Numeric
float8_to_numeric(float8 num)
{
  Numeric    res;
  NumericVar  result;
  char    buf[DBL_DIG + 100];
  const char *endptr;

  if (isnan(num))
    return make_result(&const_nan);

  if (isinf(num))
  {
    if (num < 0)
      return make_result(&const_ninf);
    else
      return make_result(&const_pinf);
  }

  snprintf(buf, sizeof(buf), "%.*g", DBL_DIG, num);

  init_var(&result);

  /* Assume we need not worry about leading/trailing spaces */
  (void) set_var_from_str(buf, buf, &result, &endptr);

  res = make_result(&result);

  free_var(&result);

  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Transform a numeric value into a float8 value
 * @note Derived from PostgreSQL function @p numeric_float8()
 */
float8
numeric_to_float8(Numeric num)
{
  if (NUMERIC_IS_SPECIAL(num))
  {
    if (NUMERIC_IS_PINF(num))
      return get_float8_infinity();
    else if (NUMERIC_IS_NINF(num))
      return -get_float8_infinity();
    else
      return get_float8_nan();
  }

  char *tmp = pg_numeric_out(num);
  double result = float8_in(tmp);

  pfree(tmp);
  return result;
}

/*
 * Convert numeric to float8; if out of range, return +/- HUGE_VAL
 *
 * (internal helper function, not directly callable from SQL)
 */
float8 
numeric_float8_no_overflow_internal(Numeric num)
{
  double    val;
  if (NUMERIC_IS_SPECIAL(num))
  {
    if (NUMERIC_IS_PINF(num))
      val = HUGE_VAL;
    else if (NUMERIC_IS_NINF(num))
      val = -HUGE_VAL;
    else
      val = get_float8_nan();
  }
  else
  {
    NumericVar  x;

    init_var_from_num(num, &x);
    val = numericvar_to_double_no_overflow(&x);
  }

  return val;
}

/**
 * @ingroup meos_base_numeric
 * @brief Transform a float4 value into a numeric value
 * @note Derived from PostgreSQL function @p float4_numeric()
 */
Numeric
float4_to_numeric(float4 num)
{
  Numeric    res;
  NumericVar  result;
  char    buf[FLT_DIG + 100];
  const char *endptr;

  if (isnan(num))
    return make_result(&const_nan);

  if (isinf(num))
  {
    if (num < 0)
      return make_result(&const_ninf);
    else
      return make_result(&const_pinf);
  }

  snprintf(buf, sizeof(buf), "%.*g", FLT_DIG, num);

  init_var(&result);

  /* Assume we need not worry about leading/trailing spaces */
  (void) set_var_from_str(buf, buf, &result, &endptr);

  res = make_result(&result);

  free_var(&result);

  return res;
}

/**
 * @ingroup meos_base_numeric
 * @brief Transform a numeric value into a float4 value
 * @note Derived from PostgreSQL function @p numeric_float4()
 */
float4
numeric_to_float4(Numeric num)
{
  if (NUMERIC_IS_SPECIAL(num))
  {
    if (NUMERIC_IS_PINF(num))
      return get_float4_infinity();
    else if (NUMERIC_IS_NINF(num))
      return -get_float4_infinity();
    else
      return get_float4_nan();
  }

  char *tmp = pg_numeric_out(num);
  float4 result = float4_in(tmp);
  pfree(tmp);
  return result;
}

/* ----------------------------------------------------------------------
 *
 * Local functions follow
 *
 * In general, these do not support "special" (NaN or infinity) inputs;
 * callers should handle those possibilities first.
 * (There are one or two exceptions, noted in their header comments.)
 *
 * ----------------------------------------------------------------------
 */

/*
 * alloc_var() -
 *
 *  Allocate a digit buffer of ndigits digits (plus a spare digit for rounding)
 */
static void
alloc_var(NumericVar *var, int ndigits)
{
  digitbuf_free(var->buf);
  var->buf = digitbuf_alloc(ndigits + 1);
  var->buf[0] = 0;      /* spare digit for rounding */
  var->digits = var->buf + 1;
  var->ndigits = ndigits;
}

/*
 * free_var() -
 *
 *  Return the digit buffer of a variable to the free pool
 */
static void
free_var(NumericVar *var)
{
  digitbuf_free(var->buf);
  var->buf = NULL;
  var->digits = NULL;
  var->sign = NUMERIC_NAN;
}

/*
 * zero_var() -
 *
 *  Set a variable to ZERO.
 *  Note: its dscale is not touched.
 */
static void
zero_var(NumericVar *var)
{
  digitbuf_free(var->buf);
  var->buf = NULL;
  var->digits = NULL;
  var->ndigits = 0;
  var->weight = 0;      /* by convention; doesn't really matter */
  var->sign = NUMERIC_POS;  /* anything but NAN... */
}

/*
 * set_var_from_str()
 *
 *  Parse a string and put the number into a variable
 *
 * This function does not handle leading or trailing spaces.  It returns
 * the end+1 position parsed into *endptr, so that caller can check for
 * trailing spaces/garbage if deemed necessary.
 *
 * cp is the place to actually start parsing; str is what to use in error
 * reports.  (Typically cp would be the same except advanced over spaces.)
 *
 * Returns true on success, false on failure (if escontext points to an
 * ErrorSaveContext; otherwise errors are thrown).
 */
static bool
set_var_from_str(const char *str, const char *cp,
         NumericVar *dest, const char **endptr)
{
  bool    have_dp = false;
  int      i;
  unsigned char *decdigits;
  int      sign = NUMERIC_POS;
  int      dweight = -1;
  int      ddigits;
  int      dscale = 0;
  int      weight;
  int      ndigits;
  int      offset;
  NumericDigit *digits;

  /*
   * We first parse the string to extract decimal digits and determine the
   * correct decimal weight.  Then convert to NBASE representation.
   */
  switch (*cp)
  {
    case '+':
      sign = NUMERIC_POS;
      cp++;
      break;

    case '-':
      sign = NUMERIC_NEG;
      cp++;
      break;
  }

  if (*cp == '.')
  {
    have_dp = true;
    cp++;
  }

  if (!isdigit((unsigned char) *cp))
    goto invalid_syntax;

  decdigits = (unsigned char *) palloc(strlen(cp) + DEC_DIGITS * 2);

  /* leading padding for digit alignment later */
  memset(decdigits, 0, DEC_DIGITS);
  i = DEC_DIGITS;

  while (*cp)
  {
    if (isdigit((unsigned char) *cp))
    {
      decdigits[i++] = *cp++ - '0';
      if (!have_dp)
        dweight++;
      else
        dscale++;
    }
    else if (*cp == '.')
    {
      if (have_dp)
        goto invalid_syntax;
      have_dp = true;
      cp++;
      /* decimal point must not be followed by underscore */
      if (*cp == '_')
        goto invalid_syntax;
    }
    else if (*cp == '_')
    {
      /* underscore must be followed by more digits */
      cp++;
      if (!isdigit((unsigned char) *cp))
        goto invalid_syntax;
    }
    else
      break;
  }

  ddigits = i - DEC_DIGITS;
  /* trailing padding for digit alignment later */
  memset(decdigits + i, 0, DEC_DIGITS - 1);

  /* Handle exponent, if any */
  if (*cp == 'e' || *cp == 'E')
  {
    int64    exponent = 0;
    bool    neg = false;

    /*
     * At this point, dweight and dscale can't be more than about
     * INT_MAX/2 due to the MaxAllocSize limit on string length, so
     * constraining the exponent similarly should be enough to prevent
     * integer overflow in this function.  If the value is too large to
     * fit in storage format, make_result() will complain about it later;
     * for consistency use the same elog errcode/text as make_result().
     */

    /* exponent sign */
    cp++;
    if (*cp == '+')
      cp++;
    else if (*cp == '-')
    {
      neg = true;
      cp++;
    }

    /* exponent digits */
    if (!isdigit((unsigned char) *cp))
      goto invalid_syntax;

    while (*cp)
    {
      if (isdigit((unsigned char) *cp))
      {
        exponent = exponent * 10 + (*cp++ - '0');
        if (exponent > PG_INT32_MAX / 2)
          goto out_of_range;
      }
      else if (*cp == '_')
      {
        /* underscore must be followed by more digits */
        cp++;
        if (!isdigit((unsigned char) *cp))
          goto invalid_syntax;
      }
      else
        break;
    }

    if (neg)
      exponent = -exponent;

    dweight += (int) exponent;
    dscale -= (int) exponent;
    if (dscale < 0)
      dscale = 0;
  }

  /*
   * Okay, convert pure-decimal representation to base NBASE.  First we need
   * to determine the converted weight and ndigits.  offset is the number of
   * decimal zeroes to insert before the first given digit to have a
   * correctly aligned first NBASE digit.
   */
  if (dweight >= 0)
    weight = (dweight + 1 + DEC_DIGITS - 1) / DEC_DIGITS - 1;
  else
    weight = -((-dweight - 1) / DEC_DIGITS + 1);
  offset = (weight + 1) * DEC_DIGITS - (dweight + 1);
  ndigits = (ddigits + offset + DEC_DIGITS - 1) / DEC_DIGITS;

  alloc_var(dest, ndigits);
  dest->sign = sign;
  dest->weight = weight;
  dest->dscale = dscale;

  i = DEC_DIGITS - offset;
  digits = dest->digits;

  while (ndigits-- > 0)
  {
#if DEC_DIGITS == 4
    *digits++ = ((decdigits[i] * 10 + decdigits[i + 1]) * 10 +
           decdigits[i + 2]) * 10 + decdigits[i + 3];
#elif DEC_DIGITS == 2
    *digits++ = decdigits[i] * 10 + decdigits[i + 1];
#elif DEC_DIGITS == 1
    *digits++ = decdigits[i];
#else
#error unsupported NBASE
#endif
    i += DEC_DIGITS;
  }

  pfree(decdigits);

  /* Strip any leading/trailing zeroes, and normalize weight if zero */
  strip_var(dest);

  /* Return end+1 position for caller */
  *endptr = cp;

  return true;

out_of_range:
  elog(ERROR, "value overflows numeric format");
  return false;

invalid_syntax:
  elog(ERROR, "invalid input syntax for type %s: \"%s\"", "numeric", str);
  return false;
}

/*
 * Return the numeric value of a single hex digit.
 */
static inline int
xdigit_value(char dig)
{
  return dig >= '0' && dig <= '9' ? dig - '0' :
    dig >= 'a' && dig <= 'f' ? dig - 'a' + 10 :
    dig >= 'A' && dig <= 'F' ? dig - 'A' + 10 : -1;
}

/*
 * set_var_from_non_decimal_integer_str()
 *
 *  Parse a string containing a non-decimal integer
 *
 * This function does not handle leading or trailing spaces.  It returns
 * the end+1 position parsed into *endptr, so that caller can check for
 * trailing spaces/garbage if deemed necessary.
 *
 * cp is the place to actually start parsing; str is what to use in error
 * reports.  The number's sign and base prefix indicator (e.g., "0x") are
 * assumed to have already been parsed, so cp should point to the number's
 * first digit in the base specified.
 *
 * base is expected to be 2, 8 or 16.
 *
 * Returns true on success, false on failure (if escontext points to an
 * ErrorSaveContext; otherwise errors are thrown).
 */
static bool
set_var_from_non_decimal_integer_str(const char *str, const char *cp, int sign,
                   int base, NumericVar *dest,
                   const char **endptr)
{
  const char *firstdigit = cp;
  int64    tmp;
  int64    mul;
  NumericVar  tmp_var;

  init_var(&tmp_var);

  zero_var(dest);

  /*
   * Process input digits in groups that fit in int64.  Here "tmp" is the
   * value of the digits in the group, and "mul" is base^n, where n is the
   * number of digits in the group.  Thus tmp < mul, and we must start a new
   * group when mul * base threatens to overflow PG_INT64_MAX.
   */
  tmp = 0;
  mul = 1;

  if (base == 16)
  {
    while (*cp)
    {
      if (isxdigit((unsigned char) *cp))
      {
        if (mul > PG_INT64_MAX / 16)
        {
          /* Add the contribution from this group of digits */
          int64_to_numericvar(mul, &tmp_var);
          mul_var(dest, &tmp_var, dest, 0);
          int64_to_numericvar(tmp, &tmp_var);
          add_var(dest, &tmp_var, dest);

          /* Result will overflow if weight overflows int16 */
          if (dest->weight > NUMERIC_WEIGHT_MAX)
            goto out_of_range;

          /* Begin a new group */
          tmp = 0;
          mul = 1;
        }

        tmp = tmp * 16 + xdigit_value(*cp++);
        mul = mul * 16;
      }
      else if (*cp == '_')
      {
        /* Underscore must be followed by more digits */
        cp++;
        if (!isxdigit((unsigned char) *cp))
          goto invalid_syntax;
      }
      else
        break;
    }
  }
  else if (base == 8)
  {
    while (*cp)
    {
      if (*cp >= '0' && *cp <= '7')
      {
        if (mul > PG_INT64_MAX / 8)
        {
          /* Add the contribution from this group of digits */
          int64_to_numericvar(mul, &tmp_var);
          mul_var(dest, &tmp_var, dest, 0);
          int64_to_numericvar(tmp, &tmp_var);
          add_var(dest, &tmp_var, dest);

          /* Result will overflow if weight overflows int16 */
          if (dest->weight > NUMERIC_WEIGHT_MAX)
            goto out_of_range;

          /* Begin a new group */
          tmp = 0;
          mul = 1;
        }

        tmp = tmp * 8 + (*cp++ - '0');
        mul = mul * 8;
      }
      else if (*cp == '_')
      {
        /* Underscore must be followed by more digits */
        cp++;
        if (*cp < '0' || *cp > '7')
          goto invalid_syntax;
      }
      else
        break;
    }
  }
  else if (base == 2)
  {
    while (*cp)
    {
      if (*cp >= '0' && *cp <= '1')
      {
        if (mul > PG_INT64_MAX / 2)
        {
          /* Add the contribution from this group of digits */
          int64_to_numericvar(mul, &tmp_var);
          mul_var(dest, &tmp_var, dest, 0);
          int64_to_numericvar(tmp, &tmp_var);
          add_var(dest, &tmp_var, dest);

          /* Result will overflow if weight overflows int16 */
          if (dest->weight > NUMERIC_WEIGHT_MAX)
            goto out_of_range;

          /* Begin a new group */
          tmp = 0;
          mul = 1;
        }

        tmp = tmp * 2 + (*cp++ - '0');
        mul = mul * 2;
      }
      else if (*cp == '_')
      {
        /* Underscore must be followed by more digits */
        cp++;
        if (*cp < '0' || *cp > '1')
          goto invalid_syntax;
      }
      else
        break;
    }
  }
  else
    /* Should never happen; treat as invalid input */
    goto invalid_syntax;

  /* Check that we got at least one digit */
  if (unlikely(cp == firstdigit))
    goto invalid_syntax;

  /* Add the contribution from the final group of digits */
  int64_to_numericvar(mul, &tmp_var);
  mul_var(dest, &tmp_var, dest, 0);
  int64_to_numericvar(tmp, &tmp_var);
  add_var(dest, &tmp_var, dest);

  if (dest->weight > NUMERIC_WEIGHT_MAX)
    goto out_of_range;

  dest->sign = sign;

  free_var(&tmp_var);

  /* Return end+1 position for caller */
  *endptr = cp;

  return true;

out_of_range:
  elog(ERROR, "value overflows numeric format");
  return false;

invalid_syntax:
  elog(ERROR, "invalid input syntax for type %s: \"%s\"", "numeric", str);
  return false;
}

/*
 * set_var_from_num() -
 *
 *  Convert the packed db format into a variable
 */
static void
set_var_from_num(Numeric num, NumericVar *dest)
{
  int      ndigits;

  ndigits = NUMERIC_NDIGITS(num);

  alloc_var(dest, ndigits);

  dest->weight = NUMERIC_WEIGHT(num);
  dest->sign = NUMERIC_SIGN(num);
  dest->dscale = NUMERIC_DSCALE(num);

  memcpy(dest->digits, NUMERIC_DIGITS(num), ndigits * sizeof(NumericDigit));
}

/*
 * init_var_from_num() -
 *
 *  Initialize a variable from packed db format. The digits array is not
 *  copied, which saves some cycles when the resulting var is not modified.
 *  Also, there's no need to call free_var(), as long as you don't assign any
 *  other value to it (with set_var_* functions, or by using the var as the
 *  destination of a function like add_var())
 *
 *  CAUTION: Do not modify the digits buffer of a var initialized with this
 *  function, e.g by calling round_var() or trunc_var(), as the changes will
 *  propagate to the original Numeric! It's OK to use it as the destination
 *  argument of one of the calculational functions, though.
 */
static void
init_var_from_num(Numeric num, NumericVar *dest)
{
  dest->ndigits = NUMERIC_NDIGITS(num);
  dest->weight = NUMERIC_WEIGHT(num);
  dest->sign = NUMERIC_SIGN(num);
  dest->dscale = NUMERIC_DSCALE(num);
  dest->digits = NUMERIC_DIGITS(num);
  dest->buf = NULL;      /* digits array is not palloc'd */
}

/*
 * set_var_from_var() -
 *
 *  Copy one variable into another
 */
static void
set_var_from_var(const NumericVar *value, NumericVar *dest)
{
  NumericDigit *newbuf;

  newbuf = digitbuf_alloc(value->ndigits + 1);
  newbuf[0] = 0;        /* spare digit for rounding */
  if (value->ndigits > 0)    /* else value->digits might be null */
    memcpy(newbuf + 1, value->digits,
         value->ndigits * sizeof(NumericDigit));

  digitbuf_free(dest->buf);

  memmove(dest, value, sizeof(NumericVar));
  dest->buf = newbuf;
  dest->digits = newbuf + 1;
}

/*
 * get_str_from_var() -
 *
 *  Convert a var to text representation (guts of numeric_out).
 *  The var is displayed to the number of digits indicated by its dscale.
 *  Returns a palloc'd string.
 */
static char *
get_str_from_var(const NumericVar *var)
{
  int      dscale;
  char     *str;
  char     *cp;
  char     *endcp;
  int      i;
  int      d;
  NumericDigit dig;

#if DEC_DIGITS > 1
  NumericDigit d1;
#endif

  dscale = var->dscale;

  /*
   * Allocate space for the result.
   *
   * i is set to the # of decimal digits before decimal point. dscale is the
   * # of decimal digits we will print after decimal point. We may generate
   * as many as DEC_DIGITS-1 excess digits at the end, and in addition we
   * need room for sign, decimal point, null terminator.
   */
  i = (var->weight + 1) * DEC_DIGITS;
  if (i <= 0)
    i = 1;

  str = palloc(i + dscale + DEC_DIGITS + 2);
  cp = str;

  /*
   * Output a dash for negative values
   */
  if (var->sign == NUMERIC_NEG)
    *cp++ = '-';

  /*
   * Output all digits before the decimal point
   */
  if (var->weight < 0)
  {
    d = var->weight + 1;
    *cp++ = '0';
  }
  else
  {
    for (d = 0; d <= var->weight; d++)
    {
      dig = (d < var->ndigits) ? var->digits[d] : 0;
      /* In the first digit, suppress extra leading decimal zeroes */
#if DEC_DIGITS == 4
      {
        bool    putit = (d > 0);

        d1 = dig / 1000;
        dig -= d1 * 1000;
        putit |= (d1 > 0);
        if (putit)
          *cp++ = d1 + '0';
        d1 = dig / 100;
        dig -= d1 * 100;
        putit |= (d1 > 0);
        if (putit)
          *cp++ = d1 + '0';
        d1 = dig / 10;
        dig -= d1 * 10;
        putit |= (d1 > 0);
        if (putit)
          *cp++ = d1 + '0';
        *cp++ = dig + '0';
      }
#elif DEC_DIGITS == 2
      d1 = dig / 10;
      dig -= d1 * 10;
      if (d1 > 0 || d > 0)
        *cp++ = d1 + '0';
      *cp++ = dig + '0';
#elif DEC_DIGITS == 1
      *cp++ = dig + '0';
#else
#error unsupported NBASE
#endif
    }
  }

  /*
   * If requested, output a decimal point and all the digits that follow it.
   * We initially put out a multiple of DEC_DIGITS digits, then truncate if
   * needed.
   */
  if (dscale > 0)
  {
    *cp++ = '.';
    endcp = cp + dscale;
    for (i = 0; i < dscale; d++, i += DEC_DIGITS)
    {
      dig = (d >= 0 && d < var->ndigits) ? var->digits[d] : 0;
#if DEC_DIGITS == 4
      d1 = dig / 1000;
      dig -= d1 * 1000;
      *cp++ = d1 + '0';
      d1 = dig / 100;
      dig -= d1 * 100;
      *cp++ = d1 + '0';
      d1 = dig / 10;
      dig -= d1 * 10;
      *cp++ = d1 + '0';
      *cp++ = dig + '0';
#elif DEC_DIGITS == 2
      d1 = dig / 10;
      dig -= d1 * 10;
      *cp++ = d1 + '0';
      *cp++ = dig + '0';
#elif DEC_DIGITS == 1
      *cp++ = dig + '0';
#else
#error unsupported NBASE
#endif
    }
    cp = endcp;
  }

  /*
   * terminate the string and return it
   */
  *cp = '\0';
  return str;
}

/*
 * get_str_from_var_sci() -
 *
 *  Convert a var to a normalised scientific notation text representation.
 *  This function does the heavy lifting for numeric_out_sci().
 *
 *  This notation has the general form a * 10^b, where a is known as the
 *  "significand" and b is known as the "exponent".
 *
 *  Because we can't do superscript in ASCII (and because we want to copy
 *  printf's behaviour) we display the exponent using E notation, with a
 *  minimum of two exponent digits.
 *
 *  For example, the value 1234 could be output as 1.2e+03.
 *
 *  We assume that the exponent can fit into an int32.
 *
 *  rscale is the number of decimal digits desired after the decimal point in
 *  the output, negative values will be treated as meaning zero.
 *
 *  Returns a palloc'd string.
 */
static char *
get_str_from_var_sci(const NumericVar *var, int rscale)
{
  int32    exponent;
  NumericVar  tmp_var;
  size_t    len;
  char     *str;
  char     *sig_out;

  if (rscale < 0)
    rscale = 0;

  /*
   * Determine the exponent of this number in normalised form.
   *
   * This is the exponent required to represent the number with only one
   * significant digit before the decimal place.
   */
  if (var->ndigits > 0)
  {
    exponent = (var->weight + 1) * DEC_DIGITS;

    /*
     * Compensate for leading decimal zeroes in the first numeric digit by
     * decrementing the exponent.
     */
    exponent -= DEC_DIGITS - (int) log10(var->digits[0]);
  }
  else
  {
    /*
     * If var has no digits, then it must be zero.
     *
     * Zero doesn't technically have a meaningful exponent in normalised
     * notation, but we just display the exponent as zero for consistency
     * of output.
     */
    exponent = 0;
  }

  /*
   * Divide var by 10^exponent to get the significand, rounding to rscale
   * decimal digits in the process.
   */
  init_var(&tmp_var);

  power_ten_int(exponent, &tmp_var);
  div_var(var, &tmp_var, &tmp_var, rscale, true, true);
  sig_out = get_str_from_var(&tmp_var);

  free_var(&tmp_var);

  /*
   * Allocate space for the result.
   *
   * In addition to the significand, we need room for the exponent
   * decoration ("e"), the sign of the exponent, up to 10 digits for the
   * exponent itself, and of course the null terminator.
   */
  len = strlen(sig_out) + 13;
  str = palloc(len);
  snprintf(str, len, "%se%+03d", sig_out, exponent);

  pfree(sig_out);

  return str;
}

/*
 * duplicate_numeric() - copy a packed-format Numeric
 *
 * This will handle NaN and Infinity cases.
 */
static Numeric
duplicate_numeric(Numeric num)
{
  Numeric    res;

  res = (Numeric) palloc(VARSIZE(num));
  memcpy(res, num, VARSIZE(num));
  return res;
}

/*
 * make_result_opt_error() -
 *
 *  Create the packed db numeric format in palloc()'d memory from
 *  a variable.  This will handle NaN and Infinity cases.
 *
 *  If "have_error" isn't NULL, on overflow *have_error is set to true and
 *  NULL is returned.  This is helpful when caller needs to handle errors.
 */
static Numeric
make_result_opt_error(const NumericVar *var, bool *have_error)
{
  Numeric    result;
  NumericDigit *digits = var->digits;
  int      weight = var->weight;
  int      sign = var->sign;
  int      n;
  Size    len;

  if (have_error)
    *have_error = false;

  if ((sign & NUMERIC_SIGN_MASK) == NUMERIC_SPECIAL)
  {
    /*
     * Verify valid special value.  This could be just an Assert, perhaps,
     * but it seems worthwhile to expend a few cycles to ensure that we
     * never write any nonzero reserved bits to disk.
     */
    if (!(sign == NUMERIC_NAN || sign == NUMERIC_PINF || sign == NUMERIC_NINF))
    {
      elog(ERROR, "invalid numeric sign value 0x%x", sign);
      return NULL;
    }

    result = (Numeric) palloc(NUMERIC_HDRSZ);

    SET_VARSIZE(result, NUMERIC_HDRSZ_SHORT);
    result->choice.n_header = sign;
    /* the header word is all we need */

    dump_numeric("make_result()", result);
    return result;
  }

  n = var->ndigits;

  /* truncate leading zeroes */
  while (n > 0 && *digits == 0)
  {
    digits++;
    weight--;
    n--;
  }
  /* truncate trailing zeroes */
  while (n > 0 && digits[n - 1] == 0)
    n--;

  /* If zero result, force to weight=0 and positive sign */
  if (n == 0)
  {
    weight = 0;
    sign = NUMERIC_POS;
  }

  /* Build the result */
  if (NUMERIC_CAN_BE_SHORT(var->dscale, weight))
  {
    len = NUMERIC_HDRSZ_SHORT + n * sizeof(NumericDigit);
    result = (Numeric) palloc(len);
    SET_VARSIZE(result, len);
    result->choice.n_short.n_header =
      (sign == NUMERIC_NEG ? (NUMERIC_SHORT | NUMERIC_SHORT_SIGN_MASK)
       : NUMERIC_SHORT)
      | (var->dscale << NUMERIC_SHORT_DSCALE_SHIFT)
      | (weight < 0 ? NUMERIC_SHORT_WEIGHT_SIGN_MASK : 0)
      | (weight & NUMERIC_SHORT_WEIGHT_MASK);
  }
  else
  {
    len = NUMERIC_HDRSZ + n * sizeof(NumericDigit);
    result = (Numeric) palloc(len);
    SET_VARSIZE(result, len);
    result->choice.n_long.n_sign_dscale =
      sign | (var->dscale & NUMERIC_DSCALE_MASK);
    result->choice.n_long.n_weight = weight;
  }

  Assert(NUMERIC_NDIGITS(result) == n);
  if (n > 0)
    memcpy(NUMERIC_DIGITS(result), digits, n * sizeof(NumericDigit));

  /* Check for overflow of int16 fields */
  if (NUMERIC_WEIGHT(result) != weight ||
    NUMERIC_DSCALE(result) != var->dscale)
  {
    if (have_error)
    {
      *have_error = true;
      return NULL;
    }
    else
    {
      elog(ERROR, "value overflows numeric format");
      return NULL;
    }
  }

  dump_numeric("make_result()", result);
  return result;
}

/*
 * make_result() -
 *
 *  An interface to make_result_opt_error() without "have_error" argument.
 */
static Numeric
make_result(const NumericVar *var)
{
  return make_result_opt_error(var, NULL);
}

/*
 * apply_typmod() -
 *
 *  Do bounds checking and rounding according to the specified typmod.
 *  Note that this is only applied to normal finite values.
 *
 * Returns true on success, false on failure (if escontext points to an
 * ErrorSaveContext; otherwise errors are thrown).
 */
static bool
apply_typmod(NumericVar *var, int32 typmod)
{
  int      precision;
  int      scale;
  int      maxdigits;
  int      ddigits;
  int      i;

  /* Do nothing if we have an invalid typmod */
  if (!is_valid_numeric_typmod(typmod))
    return true;

  precision = numeric_typmod_precision(typmod);
  scale = numeric_typmod_scale(typmod);
  maxdigits = precision - scale;

  /* Round to target scale (and set var->dscale) */
  round_var(var, scale);

  /* but don't allow var->dscale to be negative */
  if (var->dscale < 0)
    var->dscale = 0;

  /*
   * Check for overflow - note we can't do this before rounding, because
   * rounding could raise the weight.  Also note that the var's weight could
   * be inflated by leading zeroes, which will be stripped before storage
   * but perhaps might not have been yet. In any case, we must recognize a
   * true zero, whose weight doesn't mean anything.
   */
  ddigits = (var->weight + 1) * DEC_DIGITS;
  if (ddigits > maxdigits)
  {
    /* Determine true weight; and check for all-zero result */
    for (i = 0; i < var->ndigits; i++)
    {
      NumericDigit dig = var->digits[i];

      if (dig)
      {
        /* Adjust for any high-order decimal zero digits */
#if DEC_DIGITS == 4
        if (dig < 10)
          ddigits -= 3;
        else if (dig < 100)
          ddigits -= 2;
        else if (dig < 1000)
          ddigits -= 1;
#elif DEC_DIGITS == 2
        if (dig < 10)
          ddigits -= 1;
#elif DEC_DIGITS == 1
        /* no adjustment */
#else
#error unsupported NBASE
#endif
        if (ddigits > maxdigits)
        {
          elog(ERROR, "numeric field overflow");
          return false;
        }
        break;
      }
      ddigits -= DEC_DIGITS;
    }
  }

  return true;
}

/*
 * apply_typmod_special() -
 *
 *  Do bounds checking according to the specified typmod, for an Inf or NaN.
 *  For convenience of most callers, the value is presented in packed form.
 *
 * Returns true on success, false on failure (if escontext points to an
 * ErrorSaveContext; otherwise errors are thrown).
 */
static bool
apply_typmod_special(Numeric num, int32 typmod)
{
  Assert(NUMERIC_IS_SPECIAL(num));  /* caller error if not */

  /*
   * NaN is allowed regardless of the typmod; that's rather dubious perhaps,
   * but it's a longstanding behavior.  Inf is rejected if we have any
   * typmod restriction, since an infinity shouldn't be claimed to fit in
   * any finite number of digits.
   */
  if (NUMERIC_IS_NAN(num))
    return true;

  /* Do nothing if we have a default typmod (-1) */
  if (!is_valid_numeric_typmod(typmod))
    return true;

  elog(ERROR, "numeric field overflow");
  return false;
}

/*
 * Convert numeric to int8, rounding if needed.
 *
 * If overflow, return false (no error is raised).  Return true if okay.
 */
static bool
numericvar_to_int64(const NumericVar *var, int64 *result)
{
  NumericDigit *digits;
  int      ndigits;
  int      weight;
  int      i;
  int64    val;
  bool    neg;
  NumericVar  rounded;

  /* Round to nearest integer */
  init_var(&rounded);
  set_var_from_var(var, &rounded);
  round_var(&rounded, 0);

  /* Check for zero input */
  strip_var(&rounded);
  ndigits = rounded.ndigits;
  if (ndigits == 0)
  {
    *result = 0;
    free_var(&rounded);
    return true;
  }

  /*
   * For input like 10000000000, we must treat stripped digits as real. So
   * the loop assumes there are weight+1 digits before the decimal point.
   */
  weight = rounded.weight;
  Assert(weight >= 0 && ndigits <= weight + 1);

  /*
   * Construct the result. To avoid issues with converting a value
   * corresponding to INT64_MIN (which can't be represented as a positive 64
   * bit two's complement integer), accumulate value as a negative number.
   */
  digits = rounded.digits;
  neg = (rounded.sign == NUMERIC_NEG);
  val = -digits[0];
  for (i = 1; i <= weight; i++)
  {
    if (unlikely(pg_mul_s64_overflow(val, NBASE, &val)))
    {
      free_var(&rounded);
      return false;
    }

    if (i < ndigits)
    {
      if (unlikely(pg_sub_s64_overflow(val, digits[i], &val)))
      {
        free_var(&rounded);
        return false;
      }
    }
  }

  free_var(&rounded);

  if (!neg)
  {
    if (unlikely(val == PG_INT64_MIN))
      return false;
    val = -val;
  }
  *result = val;

  return true;
}

/*
 * Convert int8 value to numeric.
 */
static void
int64_to_numericvar(int64 val, NumericVar *var)
{
  uint64 uval, newuval;
  NumericDigit *ptr;
  int ndigits;

  /* int64 can require at most 19 decimal digits; add one for safety */
  alloc_var(var, 20 / DEC_DIGITS);
  if (val < 0)
  {
    var->sign = NUMERIC_NEG;
    // uval = pg_abs_s64(val);
    uval = labs(val);
  }
  else
  {
    var->sign = NUMERIC_POS;
    uval = val;
  }
  var->dscale = 0;
  if (val == 0)
  {
    var->ndigits = 0;
    var->weight = 0;
    return;
  }
  ptr = var->digits + var->ndigits;
  ndigits = 0;
  do
  {
    ptr--;
    ndigits++;
    newuval = uval / NBASE;
    *ptr = uval - newuval * NBASE;
    uval = newuval;
  } while (uval);
  var->digits = ptr;
  var->ndigits = ndigits;
  var->weight = ndigits - 1;
}

#ifdef HAVE_INT128
/*
 * Convert 128 bit integer to numeric.
 */
static void
int128_to_numericvar(int128 val, NumericVar *var)
{
  uint128    uval,
        newuval;
  NumericDigit *ptr;
  int      ndigits;

  /* int128 can require at most 39 decimal digits; add one for safety */
  alloc_var(var, 40 / DEC_DIGITS);
  if (val < 0)
  {
    var->sign = NUMERIC_NEG;
    uval = -val;
  }
  else
  {
    var->sign = NUMERIC_POS;
    uval = val;
  }
  var->dscale = 0;
  if (val == 0)
  {
    var->ndigits = 0;
    var->weight = 0;
    return;
  }
  ptr = var->digits + var->ndigits;
  ndigits = 0;
  do
  {
    ptr--;
    ndigits++;
    newuval = uval / NBASE;
    *ptr = uval - newuval * NBASE;
    uval = newuval;
  } while (uval);
  var->digits = ptr;
  var->ndigits = ndigits;
  var->weight = ndigits - 1;
}
#endif

/*
 * Convert a NumericVar to float8; if out of range, return +/- HUGE_VAL
 */
static double
numericvar_to_double_no_overflow(const NumericVar *var)
{
  char     *tmp;
  double    val;
  char     *endptr;

  tmp = get_str_from_var(var);

  /* unlike float8in, we ignore ERANGE from strtod */
  val = strtod(tmp, &endptr);
  if (*endptr != '\0')
  {
    /* shouldn't happen ... */
    elog(ERROR,
      "invalid input syntax for type %s: \"%s\"", "double precision", tmp);
    return get_float8_infinity();
  }

  pfree(tmp);

  return val;
}


/*
 * cmp_var() -
 *
 *  Compare two values on variable level.  We assume zeroes have been
 *  truncated to no digits.
 */
static int
cmp_var(const NumericVar *var1, const NumericVar *var2)
{
  return cmp_var_common(var1->digits, var1->ndigits,
              var1->weight, var1->sign,
              var2->digits, var2->ndigits,
              var2->weight, var2->sign);
}

/*
 * cmp_var_common() -
 *
 *  Main routine of cmp_var(). This function can be used by both
 *  NumericVar and Numeric.
 */
static int
cmp_var_common(const NumericDigit *var1digits, int var1ndigits,
         int var1weight, int var1sign,
         const NumericDigit *var2digits, int var2ndigits,
         int var2weight, int var2sign)
{
  if (var1ndigits == 0)
  {
    if (var2ndigits == 0)
      return 0;
    if (var2sign == NUMERIC_NEG)
      return 1;
    return -1;
  }
  if (var2ndigits == 0)
  {
    if (var1sign == NUMERIC_POS)
      return 1;
    return -1;
  }

  if (var1sign == NUMERIC_POS)
  {
    if (var2sign == NUMERIC_NEG)
      return 1;
    return cmp_abs_common(var1digits, var1ndigits, var1weight,
                var2digits, var2ndigits, var2weight);
  }

  if (var2sign == NUMERIC_POS)
    return -1;

  return cmp_abs_common(var2digits, var2ndigits, var2weight,
              var1digits, var1ndigits, var1weight);
}

/*
 * add_var() -
 *
 *  Full version of add functionality on variable level (handling signs).
 *  result might point to one of the operands too without danger.
 */
static void
add_var(const NumericVar *var1, const NumericVar *var2, NumericVar *result)
{
  /*
   * Decide on the signs of the two variables what to do
   */
  if (var1->sign == NUMERIC_POS)
  {
    if (var2->sign == NUMERIC_POS)
    {
      /*
       * Both are positive result = +(ABS(var1) + ABS(var2))
       */
      add_abs(var1, var2, result);
      result->sign = NUMERIC_POS;
    }
    else
    {
      /*
       * var1 is positive, var2 is negative Must compare absolute values
       */
      switch (cmp_abs(var1, var2))
      {
        case 0:
          /* ----------
           * ABS(var1) == ABS(var2)
           * result = ZERO
           * ----------
           */
          zero_var(result);
          result->dscale = Max(var1->dscale, var2->dscale);
          break;

        case 1:
          /* ----------
           * ABS(var1) > ABS(var2)
           * result = +(ABS(var1) - ABS(var2))
           * ----------
           */
          sub_abs(var1, var2, result);
          result->sign = NUMERIC_POS;
          break;

        case -1:
          /* ----------
           * ABS(var1) < ABS(var2)
           * result = -(ABS(var2) - ABS(var1))
           * ----------
           */
          sub_abs(var2, var1, result);
          result->sign = NUMERIC_NEG;
          break;
      }
    }
  }
  else
  {
    if (var2->sign == NUMERIC_POS)
    {
      /* ----------
       * var1 is negative, var2 is positive
       * Must compare absolute values
       * ----------
       */
      switch (cmp_abs(var1, var2))
      {
        case 0:
          /* ----------
           * ABS(var1) == ABS(var2)
           * result = ZERO
           * ----------
           */
          zero_var(result);
          result->dscale = Max(var1->dscale, var2->dscale);
          break;

        case 1:
          /* ----------
           * ABS(var1) > ABS(var2)
           * result = -(ABS(var1) - ABS(var2))
           * ----------
           */
          sub_abs(var1, var2, result);
          result->sign = NUMERIC_NEG;
          break;

        case -1:
          /* ----------
           * ABS(var1) < ABS(var2)
           * result = +(ABS(var2) - ABS(var1))
           * ----------
           */
          sub_abs(var2, var1, result);
          result->sign = NUMERIC_POS;
          break;
      }
    }
    else
    {
      /* ----------
       * Both are negative
       * result = -(ABS(var1) + ABS(var2))
       * ----------
       */
      add_abs(var1, var2, result);
      result->sign = NUMERIC_NEG;
    }
  }
}

/*
 * sub_var() -
 *
 *  Full version of sub functionality on variable level (handling signs).
 *  result might point to one of the operands too without danger.
 */
static void
sub_var(const NumericVar *var1, const NumericVar *var2, NumericVar *result)
{
  /*
   * Decide on the signs of the two variables what to do
   */
  if (var1->sign == NUMERIC_POS)
  {
    if (var2->sign == NUMERIC_NEG)
    {
      /* ----------
       * var1 is positive, var2 is negative
       * result = +(ABS(var1) + ABS(var2))
       * ----------
       */
      add_abs(var1, var2, result);
      result->sign = NUMERIC_POS;
    }
    else
    {
      /* ----------
       * Both are positive
       * Must compare absolute values
       * ----------
       */
      switch (cmp_abs(var1, var2))
      {
        case 0:
          /* ----------
           * ABS(var1) == ABS(var2)
           * result = ZERO
           * ----------
           */
          zero_var(result);
          result->dscale = Max(var1->dscale, var2->dscale);
          break;

        case 1:
          /* ----------
           * ABS(var1) > ABS(var2)
           * result = +(ABS(var1) - ABS(var2))
           * ----------
           */
          sub_abs(var1, var2, result);
          result->sign = NUMERIC_POS;
          break;

        case -1:
          /* ----------
           * ABS(var1) < ABS(var2)
           * result = -(ABS(var2) - ABS(var1))
           * ----------
           */
          sub_abs(var2, var1, result);
          result->sign = NUMERIC_NEG;
          break;
      }
    }
  }
  else
  {
    if (var2->sign == NUMERIC_NEG)
    {
      /* ----------
       * Both are negative
       * Must compare absolute values
       * ----------
       */
      switch (cmp_abs(var1, var2))
      {
        case 0:
          /* ----------
           * ABS(var1) == ABS(var2)
           * result = ZERO
           * ----------
           */
          zero_var(result);
          result->dscale = Max(var1->dscale, var2->dscale);
          break;

        case 1:
          /* ----------
           * ABS(var1) > ABS(var2)
           * result = -(ABS(var1) - ABS(var2))
           * ----------
           */
          sub_abs(var1, var2, result);
          result->sign = NUMERIC_NEG;
          break;

        case -1:
          /* ----------
           * ABS(var1) < ABS(var2)
           * result = +(ABS(var2) - ABS(var1))
           * ----------
           */
          sub_abs(var2, var1, result);
          result->sign = NUMERIC_POS;
          break;
      }
    }
    else
    {
      /* ----------
       * var1 is negative, var2 is positive
       * result = -(ABS(var1) + ABS(var2))
       * ----------
       */
      add_abs(var1, var2, result);
      result->sign = NUMERIC_NEG;
    }
  }
}

/*
 * mul_var() -
 *
 *  Multiplication on variable level. Product of var1 * var2 is stored
 *  in result.  Result is rounded to no more than rscale fractional digits.
 */
static void
mul_var(const NumericVar *var1, const NumericVar *var2, NumericVar *result,
    int rscale)
{
  int      res_ndigits;
  int      res_ndigitpairs;
  int      res_sign;
  int      res_weight;
  int      pair_offset;
  int      maxdigits;
  int      maxdigitpairs;
  uint64     *dig,
         *dig_i1_off;
  uint64    maxdig;
  uint64    carry;
  uint64    newdig;
  int      var1ndigits;
  int      var2ndigits;
  int      var1ndigitpairs;
  int      var2ndigitpairs;
  NumericDigit *var1digits;
  NumericDigit *var2digits;
  uint32    var1digitpair;
  uint32     *var2digitpairs;
  NumericDigit *res_digits;
  int      i,
        i1,
        i2,
        i2limit;

  /*
   * Arrange for var1 to be the shorter of the two numbers.  This improves
   * performance because the inner multiplication loop is much simpler than
   * the outer loop, so it's better to have a smaller number of iterations
   * of the outer loop.  This also reduces the number of times that the
   * accumulator array needs to be normalized.
   */
  if (var1->ndigits > var2->ndigits)
  {
    const NumericVar *tmp = var1;

    var1 = var2;
    var2 = tmp;
  }

  /* copy these values into local vars for speed in inner loop */
  var1ndigits = var1->ndigits;
  var2ndigits = var2->ndigits;
  var1digits = var1->digits;
  var2digits = var2->digits;

  if (var1ndigits == 0)
  {
    /* one or both inputs is zero; so is result */
    zero_var(result);
    result->dscale = rscale;
    return;
  }

  /*
   * If var1 has 1-6 digits and the exact result was requested, delegate to
   * mul_var_short() which uses a faster direct multiplication algorithm.
   */
  if (var1ndigits <= 6 && rscale == var1->dscale + var2->dscale)
  {
    mul_var_short(var1, var2, result);
    return;
  }

  /* Determine result sign */
  if (var1->sign == var2->sign)
    res_sign = NUMERIC_POS;
  else
    res_sign = NUMERIC_NEG;

  /*
   * Determine the number of result digits to compute and the (maximum
   * possible) result weight.  If the exact result would have more than
   * rscale fractional digits, truncate the computation with
   * MUL_GUARD_DIGITS guard digits, i.e., ignore input digits that would
   * only contribute to the right of that.  (This will give the exact
   * rounded-to-rscale answer unless carries out of the ignored positions
   * would have propagated through more than MUL_GUARD_DIGITS digits.)
   *
   * Note: an exact computation could not produce more than var1ndigits +
   * var2ndigits digits, but we allocate at least one extra output digit in
   * case rscale-driven rounding produces a carry out of the highest exact
   * digit.
   *
   * The computation itself is done using base-NBASE^2 arithmetic, so we
   * actually process the input digits in pairs, producing a base-NBASE^2
   * intermediate result.  This significantly improves performance, since
   * schoolbook multiplication is O(N^2) in the number of input digits, and
   * working in base NBASE^2 effectively halves "N".
   *
   * Note: in a truncated computation, we must compute at least one extra
   * output digit to ensure that all the guard digits are fully computed.
   */
  /* digit pairs in each input */
  var1ndigitpairs = (var1ndigits + 1) / 2;
  var2ndigitpairs = (var2ndigits + 1) / 2;

  /* digits in exact result */
  res_ndigits = var1ndigits + var2ndigits;

  /* digit pairs in exact result with at least one extra output digit */
  res_ndigitpairs = res_ndigits / 2 + 1;

  /* pair offset to align result to end of dig[] */
  pair_offset = res_ndigitpairs - var1ndigitpairs - var2ndigitpairs + 1;

  /* maximum possible result weight (odd-length inputs shifted up below) */
  res_weight = var1->weight + var2->weight + 1 + 2 * res_ndigitpairs -
    res_ndigits - (var1ndigits & 1) - (var2ndigits & 1);

  /* rscale-based truncation with at least one extra output digit */
  maxdigits = res_weight + 1 + (rscale + DEC_DIGITS - 1) / DEC_DIGITS +
    MUL_GUARD_DIGITS;
  maxdigitpairs = maxdigits / 2 + 1;

  res_ndigitpairs = Min(res_ndigitpairs, maxdigitpairs);
  res_ndigits = 2 * res_ndigitpairs;

  /*
   * In the computation below, digit pair i1 of var1 and digit pair i2 of
   * var2 are multiplied and added to digit i1+i2+pair_offset of dig[]. Thus
   * input digit pairs with index >= res_ndigitpairs - pair_offset don't
   * contribute to the result, and can be ignored.
   */
  if (res_ndigitpairs <= pair_offset)
  {
    /* All input digits will be ignored; so result is zero */
    zero_var(result);
    result->dscale = rscale;
    return;
  }
  var1ndigitpairs = Min(var1ndigitpairs, res_ndigitpairs - pair_offset);
  var2ndigitpairs = Min(var2ndigitpairs, res_ndigitpairs - pair_offset);

  /*
   * We do the arithmetic in an array "dig[]" of unsigned 64-bit integers.
   * Since PG_UINT64_MAX is much larger than NBASE^4, this gives us a lot of
   * headroom to avoid normalizing carries immediately.
   *
   * maxdig tracks the maximum possible value of any dig[] entry; when this
   * threatens to exceed PG_UINT64_MAX, we take the time to propagate
   * carries.  Furthermore, we need to ensure that overflow doesn't occur
   * during the carry propagation passes either.  The carry values could be
   * as much as PG_UINT64_MAX / NBASE^2, so really we must normalize when
   * digits threaten to exceed PG_UINT64_MAX - PG_UINT64_MAX / NBASE^2.
   *
   * To avoid overflow in maxdig itself, it actually represents the maximum
   * possible value divided by NBASE^2-1, i.e., at the top of the loop it is
   * known that no dig[] entry exceeds maxdig * (NBASE^2-1).
   *
   * The conversion of var1 to base NBASE^2 is done on the fly, as each new
   * digit is required.  The digits of var2 are converted upfront, and
   * stored at the end of dig[].  To avoid loss of precision, the input
   * digits are aligned with the start of digit pair array, effectively
   * shifting them up (multiplying by NBASE) if the inputs have an odd
   * number of NBASE digits.
   */
  dig = (uint64 *) palloc(res_ndigitpairs * sizeof(uint64) +
              var2ndigitpairs * sizeof(uint32));

  /* convert var2 to base NBASE^2, shifting up if its length is odd */
  var2digitpairs = (uint32 *) (dig + res_ndigitpairs);

  for (i2 = 0; i2 < var2ndigitpairs - 1; i2++)
    var2digitpairs[i2] = var2digits[2 * i2] * NBASE + var2digits[2 * i2 + 1];

  if (2 * i2 + 1 < var2ndigits)
    var2digitpairs[i2] = var2digits[2 * i2] * NBASE + var2digits[2 * i2 + 1];
  else
    var2digitpairs[i2] = var2digits[2 * i2] * NBASE;

  /*
   * Start by multiplying var2 by the least significant contributing digit
   * pair from var1, storing the results at the end of dig[], and filling
   * the leading digits with zeros.
   *
   * The loop here is the same as the inner loop below, except that we set
   * the results in dig[], rather than adding to them.  This is the
   * performance bottleneck for multiplication, so we want to keep it simple
   * enough so that it can be auto-vectorized.  Accordingly, process the
   * digits left-to-right even though schoolbook multiplication would
   * suggest right-to-left.  Since we aren't propagating carries in this
   * loop, the order does not matter.
   */
  i1 = var1ndigitpairs - 1;
  if (2 * i1 + 1 < var1ndigits)
    var1digitpair = var1digits[2 * i1] * NBASE + var1digits[2 * i1 + 1];
  else
    var1digitpair = var1digits[2 * i1] * NBASE;
  maxdig = var1digitpair;

  i2limit = Min(var2ndigitpairs, res_ndigitpairs - i1 - pair_offset);
  dig_i1_off = &dig[i1 + pair_offset];

  memset(dig, 0, (i1 + pair_offset) * sizeof(uint64));
  for (i2 = 0; i2 < i2limit; i2++)
    dig_i1_off[i2] = (uint64) var1digitpair * var2digitpairs[i2];

  /*
   * Next, multiply var2 by the remaining digit pairs from var1, adding the
   * results to dig[] at the appropriate offsets, and normalizing whenever
   * there is a risk of any dig[] entry overflowing.
   */
  for (i1 = i1 - 1; i1 >= 0; i1--)
  {
    var1digitpair = var1digits[2 * i1] * NBASE + var1digits[2 * i1 + 1];
    if (var1digitpair == 0)
      continue;

    /* Time to normalize? */
    maxdig += var1digitpair;
    if (maxdig > (PG_UINT64_MAX - PG_UINT64_MAX / NBASE_SQR) / (NBASE_SQR - 1))
    {
      /* Yes, do it (to base NBASE^2) */
      carry = 0;
      for (i = res_ndigitpairs - 1; i >= 0; i--)
      {
        newdig = dig[i] + carry;
        if (newdig >= NBASE_SQR)
        {
          carry = newdig / NBASE_SQR;
          newdig -= carry * NBASE_SQR;
        }
        else
          carry = 0;
        dig[i] = newdig;
      }
      Assert(carry == 0);
      /* Reset maxdig to indicate new worst-case */
      maxdig = 1 + var1digitpair;
    }

    /* Multiply and add */
    i2limit = Min(var2ndigitpairs, res_ndigitpairs - i1 - pair_offset);
    dig_i1_off = &dig[i1 + pair_offset];

    for (i2 = 0; i2 < i2limit; i2++)
      dig_i1_off[i2] += (uint64) var1digitpair * var2digitpairs[i2];
  }

  /*
   * Now we do a final carry propagation pass to normalize back to base
   * NBASE^2, and construct the base-NBASE result digits.  Note that this is
   * still done at full precision w/guard digits.
   */
  alloc_var(result, res_ndigits);
  res_digits = result->digits;
  carry = 0;
  for (i = res_ndigitpairs - 1; i >= 0; i--)
  {
    newdig = dig[i] + carry;
    if (newdig >= NBASE_SQR)
    {
      carry = newdig / NBASE_SQR;
      newdig -= carry * NBASE_SQR;
    }
    else
      carry = 0;
    res_digits[2 * i + 1] = (NumericDigit) ((uint32) newdig % NBASE);
    res_digits[2 * i] = (NumericDigit) ((uint32) newdig / NBASE);
  }
  Assert(carry == 0);

  pfree(dig);

  /*
   * Finally, round the result to the requested precision.
   */
  result->weight = res_weight;
  result->sign = res_sign;

  /* Round to target rscale (and set result->dscale) */
  round_var(result, rscale);

  /* Strip leading and trailing zeroes */
  strip_var(result);
}

/*
 * mul_var_short() -
 *
 *  Special-case multiplication function used when var1 has 1-6 digits, var2
 *  has at least as many digits as var1, and the exact product var1 * var2 is
 *  requested.
 */
static void
mul_var_short(const NumericVar *var1, const NumericVar *var2,
        NumericVar *result)
{
  int      var1ndigits = var1->ndigits;
  int      var2ndigits = var2->ndigits;
  NumericDigit *var1digits = var1->digits;
  NumericDigit *var2digits = var2->digits;
  int      res_sign;
  int      res_weight;
  int      res_ndigits;
  NumericDigit *res_buf;
  NumericDigit *res_digits;
  uint32    carry = 0;
  uint32    term;

  /* Check preconditions */
  Assert(var1ndigits >= 1);
  Assert(var1ndigits <= 6);
  Assert(var2ndigits >= var1ndigits);

  /*
   * Determine the result sign, weight, and number of digits to calculate.
   * The weight figured here is correct if the product has no leading zero
   * digits; otherwise strip_var() will fix things up.  Note that, unlike
   * mul_var(), we do not need to allocate an extra output digit, because we
   * are not rounding here.
   */
  if (var1->sign == var2->sign)
    res_sign = NUMERIC_POS;
  else
    res_sign = NUMERIC_NEG;
  res_weight = var1->weight + var2->weight + 1;
  res_ndigits = var1ndigits + var2ndigits;

  /* Allocate result digit array */
  res_buf = digitbuf_alloc(res_ndigits + 1);
  res_buf[0] = 0;        /* spare digit for later rounding */
  res_digits = res_buf + 1;

  /*
   * Compute the result digits in reverse, in one pass, propagating the
   * carry up as we go.  The i'th result digit consists of the sum of the
   * products var1digits[i1] * var2digits[i2] for which i = i1 + i2 + 1.
   */
#define PRODSUM1(v1,i1,v2,i2) ((v1)[(i1)] * (v2)[(i2)])
#define PRODSUM2(v1,i1,v2,i2) (PRODSUM1(v1,i1,v2,i2) + (v1)[(i1)+1] * (v2)[(i2)-1])
#define PRODSUM3(v1,i1,v2,i2) (PRODSUM2(v1,i1,v2,i2) + (v1)[(i1)+2] * (v2)[(i2)-2])
#define PRODSUM4(v1,i1,v2,i2) (PRODSUM3(v1,i1,v2,i2) + (v1)[(i1)+3] * (v2)[(i2)-3])
#define PRODSUM5(v1,i1,v2,i2) (PRODSUM4(v1,i1,v2,i2) + (v1)[(i1)+4] * (v2)[(i2)-4])
#define PRODSUM6(v1,i1,v2,i2) (PRODSUM5(v1,i1,v2,i2) + (v1)[(i1)+5] * (v2)[(i2)-5])

  switch (var1ndigits)
  {
    case 1:
      /* ---------
       * 1-digit case:
       *    var1ndigits = 1
       *    var2ndigits >= 1
       *    res_ndigits = var2ndigits + 1
       * ----------
       */
      for (int i = var2ndigits - 1; i >= 0; i--)
      {
        term = PRODSUM1(var1digits, 0, var2digits, i) + carry;
        res_digits[i + 1] = (NumericDigit) (term % NBASE);
        carry = term / NBASE;
      }
      res_digits[0] = (NumericDigit) carry;
      break;

    case 2:
      /* ---------
       * 2-digit case:
       *    var1ndigits = 2
       *    var2ndigits >= 2
       *    res_ndigits = var2ndigits + 2
       * ----------
       */
      /* last result digit and carry */
      term = PRODSUM1(var1digits, 1, var2digits, var2ndigits - 1);
      res_digits[res_ndigits - 1] = (NumericDigit) (term % NBASE);
      carry = term / NBASE;

      /* remaining digits, except for the first two */
      for (int i = var2ndigits - 1; i >= 1; i--)
      {
        term = PRODSUM2(var1digits, 0, var2digits, i) + carry;
        res_digits[i + 1] = (NumericDigit) (term % NBASE);
        carry = term / NBASE;
      }
      break;

    case 3:
      /* ---------
       * 3-digit case:
       *    var1ndigits = 3
       *    var2ndigits >= 3
       *    res_ndigits = var2ndigits + 3
       * ----------
       */
      /* last two result digits */
      term = PRODSUM1(var1digits, 2, var2digits, var2ndigits - 1);
      res_digits[res_ndigits - 1] = (NumericDigit) (term % NBASE);
      carry = term / NBASE;

      term = PRODSUM2(var1digits, 1, var2digits, var2ndigits - 1) + carry;
      res_digits[res_ndigits - 2] = (NumericDigit) (term % NBASE);
      carry = term / NBASE;

      /* remaining digits, except for the first three */
      for (int i = var2ndigits - 1; i >= 2; i--)
      {
        term = PRODSUM3(var1digits, 0, var2digits, i) + carry;
        res_digits[i + 1] = (NumericDigit) (term % NBASE);
        carry = term / NBASE;
      }
      break;

    case 4:
      /* ---------
       * 4-digit case:
       *    var1ndigits = 4
       *    var2ndigits >= 4
       *    res_ndigits = var2ndigits + 4
       * ----------
       */
      /* last three result digits */
      term = PRODSUM1(var1digits, 3, var2digits, var2ndigits - 1);
      res_digits[res_ndigits - 1] = (NumericDigit) (term % NBASE);
      carry = term / NBASE;

      term = PRODSUM2(var1digits, 2, var2digits, var2ndigits - 1) + carry;
      res_digits[res_ndigits - 2] = (NumericDigit) (term % NBASE);
      carry = term / NBASE;

      term = PRODSUM3(var1digits, 1, var2digits, var2ndigits - 1) + carry;
      res_digits[res_ndigits - 3] = (NumericDigit) (term % NBASE);
      carry = term / NBASE;

      /* remaining digits, except for the first four */
      for (int i = var2ndigits - 1; i >= 3; i--)
      {
        term = PRODSUM4(var1digits, 0, var2digits, i) + carry;
        res_digits[i + 1] = (NumericDigit) (term % NBASE);
        carry = term / NBASE;
      }
      break;

    case 5:
      /* ---------
       * 5-digit case:
       *    var1ndigits = 5
       *    var2ndigits >= 5
       *    res_ndigits = var2ndigits + 5
       * ----------
       */
      /* last four result digits */
      term = PRODSUM1(var1digits, 4, var2digits, var2ndigits - 1);
      res_digits[res_ndigits - 1] = (NumericDigit) (term % NBASE);
      carry = term / NBASE;

      term = PRODSUM2(var1digits, 3, var2digits, var2ndigits - 1) + carry;
      res_digits[res_ndigits - 2] = (NumericDigit) (term % NBASE);
      carry = term / NBASE;

      term = PRODSUM3(var1digits, 2, var2digits, var2ndigits - 1) + carry;
      res_digits[res_ndigits - 3] = (NumericDigit) (term % NBASE);
      carry = term / NBASE;

      term = PRODSUM4(var1digits, 1, var2digits, var2ndigits - 1) + carry;
      res_digits[res_ndigits - 4] = (NumericDigit) (term % NBASE);
      carry = term / NBASE;

      /* remaining digits, except for the first five */
      for (int i = var2ndigits - 1; i >= 4; i--)
      {
        term = PRODSUM5(var1digits, 0, var2digits, i) + carry;
        res_digits[i + 1] = (NumericDigit) (term % NBASE);
        carry = term / NBASE;
      }
      break;

    case 6:
      /* ---------
       * 6-digit case:
       *    var1ndigits = 6
       *    var2ndigits >= 6
       *    res_ndigits = var2ndigits + 6
       * ----------
       */
      /* last five result digits */
      term = PRODSUM1(var1digits, 5, var2digits, var2ndigits - 1);
      res_digits[res_ndigits - 1] = (NumericDigit) (term % NBASE);
      carry = term / NBASE;

      term = PRODSUM2(var1digits, 4, var2digits, var2ndigits - 1) + carry;
      res_digits[res_ndigits - 2] = (NumericDigit) (term % NBASE);
      carry = term / NBASE;

      term = PRODSUM3(var1digits, 3, var2digits, var2ndigits - 1) + carry;
      res_digits[res_ndigits - 3] = (NumericDigit) (term % NBASE);
      carry = term / NBASE;

      term = PRODSUM4(var1digits, 2, var2digits, var2ndigits - 1) + carry;
      res_digits[res_ndigits - 4] = (NumericDigit) (term % NBASE);
      carry = term / NBASE;

      term = PRODSUM5(var1digits, 1, var2digits, var2ndigits - 1) + carry;
      res_digits[res_ndigits - 5] = (NumericDigit) (term % NBASE);
      carry = term / NBASE;

      /* remaining digits, except for the first six */
      for (int i = var2ndigits - 1; i >= 5; i--)
      {
        term = PRODSUM6(var1digits, 0, var2digits, i) + carry;
        res_digits[i + 1] = (NumericDigit) (term % NBASE);
        carry = term / NBASE;
      }
      break;
  }

  /*
   * Finally, for var1ndigits > 1, compute the remaining var1ndigits most
   * significant result digits.
   */
  switch (var1ndigits)
  {
    case 6:
      term = PRODSUM5(var1digits, 0, var2digits, 4) + carry;
      res_digits[5] = (NumericDigit) (term % NBASE);
      carry = term / NBASE;
      /* FALLTHROUGH */
    case 5:
      term = PRODSUM4(var1digits, 0, var2digits, 3) + carry;
      res_digits[4] = (NumericDigit) (term % NBASE);
      carry = term / NBASE;
      /* FALLTHROUGH */
    case 4:
      term = PRODSUM3(var1digits, 0, var2digits, 2) + carry;
      res_digits[3] = (NumericDigit) (term % NBASE);
      carry = term / NBASE;
      /* FALLTHROUGH */
    case 3:
      term = PRODSUM2(var1digits, 0, var2digits, 1) + carry;
      res_digits[2] = (NumericDigit) (term % NBASE);
      carry = term / NBASE;
      /* FALLTHROUGH */
    case 2:
      term = PRODSUM1(var1digits, 0, var2digits, 0) + carry;
      res_digits[1] = (NumericDigit) (term % NBASE);
      res_digits[0] = (NumericDigit) (term / NBASE);
      break;
  }

  /* Store the product in result */
  digitbuf_free(result->buf);
  result->ndigits = res_ndigits;
  result->buf = res_buf;
  result->digits = res_digits;
  result->weight = res_weight;
  result->sign = res_sign;
  result->dscale = var1->dscale + var2->dscale;

  /* Strip leading and trailing zeroes */
  strip_var(result);
}

/*
 * div_var() -
 *
 *  Compute the quotient var1 / var2 to rscale fractional digits.
 *
 *  If "round" is true, the result is rounded at the rscale'th digit; if
 *  false, it is truncated (towards zero) at that digit.
 *
 *  If "exact" is true, the exact result is computed to the specified rscale;
 *  if false, successive quotient digits are approximated up to rscale plus
 *  DIV_GUARD_DIGITS extra digits, ignoring all contributions from digits to
 *  the right of that, before rounding or truncating to the specified rscale.
 *  This can be significantly faster, and usually gives the same result as the
 *  exact computation, but it may occasionally be off by one in the final
 *  digit, if contributions from the ignored digits would have propagated
 *  through the guard digits.  This is good enough for the transcendental
 *  functions, where small errors are acceptable.
 */
static void
div_var(const NumericVar *var1, const NumericVar *var2, NumericVar *result,
    int rscale, bool round, bool exact)
{
  int      var1ndigits = var1->ndigits;
  int      var2ndigits = var2->ndigits;
  int      res_sign;
  int      res_weight;
  int      res_ndigits;
  int      var1ndigitpairs;
  int      var2ndigitpairs;
  int      res_ndigitpairs;
  int      div_ndigitpairs;
  int64     *dividend;
  int32     *divisor;
  double    fdivisor,
        fdivisorinverse,
        fdividend,
        fquotient;
  int64    maxdiv;
  int      qi;
  int32    qdigit;
  int64    carry;
  int64    newdig;
  int64     *remainder;
  NumericDigit *res_digits;
  int      i;

  /*
   * First of all division by zero check; we must not be handed an
   * unnormalized divisor.
   */
  if (var2ndigits == 0 || var2->digits[0] == 0)
  {
    elog(ERROR, "division by zero");
    return;
  }

  /*
   * If the divisor has just one or two digits, delegate to div_var_int(),
   * which uses fast short division.
   *
   * Similarly, on platforms with 128-bit integer support, delegate to
   * div_var_int64() for divisors with three or four digits.
   */
  if (var2ndigits <= 2)
  {
    int      idivisor;
    int      idivisor_weight;

    idivisor = var2->digits[0];
    idivisor_weight = var2->weight;
    if (var2ndigits == 2)
    {
      idivisor = idivisor * NBASE + var2->digits[1];
      idivisor_weight--;
    }
    if (var2->sign == NUMERIC_NEG)
      idivisor = -idivisor;

    div_var_int(var1, idivisor, idivisor_weight, result, rscale, round);
    return;
  }
#ifdef HAVE_INT128
  if (var2ndigits <= 4)
  {
    int64    idivisor;
    int      idivisor_weight;

    idivisor = var2->digits[0];
    idivisor_weight = var2->weight;
    for (i = 1; i < var2ndigits; i++)
    {
      idivisor = idivisor * NBASE + var2->digits[i];
      idivisor_weight--;
    }
    if (var2->sign == NUMERIC_NEG)
      idivisor = -idivisor;

    div_var_int64(var1, idivisor, idivisor_weight, result, rscale, round);
    return;
  }
#endif

  /*
   * Otherwise, perform full long division.
   */

  /* Result zero check */
  if (var1ndigits == 0)
  {
    zero_var(result);
    result->dscale = rscale;
    return;
  }

  /*
   * The approximate computation can be significantly faster than the exact
   * one, since the working dividend is var2ndigitpairs base-NBASE^2 digits
   * shorter below.  However, that comes with the tradeoff of computing
   * DIV_GUARD_DIGITS extra base-NBASE result digits.  Ignoring all other
   * overheads, that suggests that, in theory, the approximate computation
   * will only be faster than the exact one when var2ndigits is greater than
   * 2 * (DIV_GUARD_DIGITS + 1), independent of the size of var1.
   *
   * Thus, we're better off doing an exact computation when var2 is shorter
   * than this.  Empirically, it has been found that the exact threshold is
   * a little higher, due to other overheads in the outer division loop.
   */
  if (var2ndigits <= 2 * (DIV_GUARD_DIGITS + 2))
    exact = true;

  /*
   * Determine the result sign, weight and number of digits to calculate.
   * The weight figured here is correct if the emitted quotient has no
   * leading zero digits; otherwise strip_var() will fix things up.
   */
  if (var1->sign == var2->sign)
    res_sign = NUMERIC_POS;
  else
    res_sign = NUMERIC_NEG;
  res_weight = var1->weight - var2->weight + 1;
  /* The number of accurate result digits we need to produce: */
  res_ndigits = res_weight + 1 + (rscale + DEC_DIGITS - 1) / DEC_DIGITS;
  /* ... but always at least 1 */
  res_ndigits = Max(res_ndigits, 1);
  /* If rounding needed, figure one more digit to ensure correct result */
  if (round)
    res_ndigits++;
  /* Add guard digits for roundoff error when producing approx result */
  if (!exact)
    res_ndigits += DIV_GUARD_DIGITS;

  /*
   * The computation itself is done using base-NBASE^2 arithmetic, so we
   * actually process the input digits in pairs, producing a base-NBASE^2
   * intermediate result.  This significantly improves performance, since
   * the computation is O(N^2) in the number of input digits, and working in
   * base NBASE^2 effectively halves "N".
   */
  var1ndigitpairs = (var1ndigits + 1) / 2;
  var2ndigitpairs = (var2ndigits + 1) / 2;
  res_ndigitpairs = (res_ndigits + 1) / 2;
  res_ndigits = 2 * res_ndigitpairs;

  /*
   * We do the arithmetic in an array "dividend[]" of signed 64-bit
   * integers.  Since PG_INT64_MAX is much larger than NBASE^4, this gives
   * us a lot of headroom to avoid normalizing carries immediately.
   *
   * When performing an exact computation, the working dividend requires
   * res_ndigitpairs + var2ndigitpairs digits.  If var1 is larger than that,
   * the extra digits do not contribute to the result, and are ignored.
   *
   * When performing an approximate computation, the working dividend only
   * requires res_ndigitpairs digits (which includes the extra guard
   * digits).  All input digits beyond that are ignored.
   */
  if (exact)
  {
    div_ndigitpairs = res_ndigitpairs + var2ndigitpairs;
    var1ndigitpairs = Min(var1ndigitpairs, div_ndigitpairs);
  }
  else
  {
    div_ndigitpairs = res_ndigitpairs;
    var1ndigitpairs = Min(var1ndigitpairs, div_ndigitpairs);
    var2ndigitpairs = Min(var2ndigitpairs, div_ndigitpairs);
  }

  /*
   * Allocate room for the working dividend (div_ndigitpairs 64-bit digits)
   * plus the divisor (var2ndigitpairs 32-bit base-NBASE^2 digits).
   *
   * For convenience, we allocate one extra dividend digit, which is set to
   * zero and not counted in div_ndigitpairs, so that the main loop below
   * can safely read and write the (qi+1)'th digit in the approximate case.
   */
  dividend = (int64 *) palloc((div_ndigitpairs + 1) * sizeof(int64) +
                var2ndigitpairs * sizeof(int32));
  divisor = (int32 *) (dividend + div_ndigitpairs + 1);

  /* load var1 into dividend[0 .. var1ndigitpairs-1], zeroing the rest */
  for (i = 0; i < var1ndigitpairs - 1; i++)
    dividend[i] = var1->digits[2 * i] * NBASE + var1->digits[2 * i + 1];

  if (2 * i + 1 < var1ndigits)
    dividend[i] = var1->digits[2 * i] * NBASE + var1->digits[2 * i + 1];
  else
    dividend[i] = var1->digits[2 * i] * NBASE;

  memset(dividend + i + 1, 0, (div_ndigitpairs - i) * sizeof(int64));

  /* load var2 into divisor[0 .. var2ndigitpairs-1] */
  for (i = 0; i < var2ndigitpairs - 1; i++)
    divisor[i] = var2->digits[2 * i] * NBASE + var2->digits[2 * i + 1];

  if (2 * i + 1 < var2ndigits)
    divisor[i] = var2->digits[2 * i] * NBASE + var2->digits[2 * i + 1];
  else
    divisor[i] = var2->digits[2 * i] * NBASE;

  /*
   * We estimate each quotient digit using floating-point arithmetic, taking
   * the first 2 base-NBASE^2 digits of the (current) dividend and divisor.
   * This must be float to avoid overflow.
   *
   * Since the floating-point dividend and divisor use 4 base-NBASE input
   * digits, they include roughly 40-53 bits of information from their
   * respective inputs (assuming NBASE is 10000), which fits well in IEEE
   * double-precision variables.  The relative error in the floating-point
   * quotient digit will then be less than around 2/NBASE^3, so the
   * estimated base-NBASE^2 quotient digit will typically be correct, and
   * should not be off by more than one from the correct value.
   */
  fdivisor = (double) divisor[0] * NBASE_SQR;
  if (var2ndigitpairs > 1)
    fdivisor += (double) divisor[1];
  fdivisorinverse = 1.0 / fdivisor;

  /*
   * maxdiv tracks the maximum possible absolute value of any dividend[]
   * entry; when this threatens to exceed PG_INT64_MAX, we take the time to
   * propagate carries.  Furthermore, we need to ensure that overflow
   * doesn't occur during the carry propagation passes either.  The carry
   * values may have an absolute value as high as PG_INT64_MAX/NBASE^2 + 1,
   * so really we must normalize when digits threaten to exceed PG_INT64_MAX
   * - PG_INT64_MAX/NBASE^2 - 1.
   *
   * To avoid overflow in maxdiv itself, it represents the max absolute
   * value divided by NBASE^2-1, i.e., at the top of the loop it is known
   * that no dividend[] entry has an absolute value exceeding maxdiv *
   * (NBASE^2-1).
   *
   * Actually, though, that holds good only for dividend[] entries after
   * dividend[qi]; the adjustment done at the bottom of the loop may cause
   * dividend[qi + 1] to exceed the maxdiv limit, so that dividend[qi] in
   * the next iteration is beyond the limit.  This does not cause problems,
   * as explained below.
   */
  maxdiv = 1;

  /*
   * Outer loop computes next quotient digit, which goes in dividend[qi].
   */
  for (qi = 0; qi < res_ndigitpairs; qi++)
  {
    /* Approximate the current dividend value */
    fdividend = (double) dividend[qi] * NBASE_SQR;
    fdividend += (double) dividend[qi + 1];

    /* Compute the (approximate) quotient digit */
    fquotient = fdividend * fdivisorinverse;
    qdigit = (fquotient >= 0.0) ? ((int32) fquotient) :
      (((int32) fquotient) - 1);  /* truncate towards -infinity */

    if (qdigit != 0)
    {
      /* Do we need to normalize now? */
      maxdiv += i64abs(qdigit);
      if (maxdiv > (PG_INT64_MAX - PG_INT64_MAX / NBASE_SQR - 1) / (NBASE_SQR - 1))
      {
        /*
         * Yes, do it.  Note that if var2ndigitpairs is much smaller
         * than div_ndigitpairs, we can save a significant amount of
         * effort here by noting that we only need to normalise those
         * dividend[] entries touched where prior iterations
         * subtracted multiples of the divisor.
         */
        carry = 0;
        for (i = Min(qi + var2ndigitpairs - 2, div_ndigitpairs - 1); i > qi; i--)
        {
          newdig = dividend[i] + carry;
          if (newdig < 0)
          {
            carry = -((-newdig - 1) / NBASE_SQR) - 1;
            newdig -= carry * NBASE_SQR;
          }
          else if (newdig >= NBASE_SQR)
          {
            carry = newdig / NBASE_SQR;
            newdig -= carry * NBASE_SQR;
          }
          else
            carry = 0;
          dividend[i] = newdig;
        }
        dividend[qi] += carry;

        /*
         * All the dividend[] digits except possibly dividend[qi] are
         * now in the range 0..NBASE^2-1.  We do not need to consider
         * dividend[qi] in the maxdiv value anymore, so we can reset
         * maxdiv to 1.
         */
        maxdiv = 1;

        /*
         * Recompute the quotient digit since new info may have
         * propagated into the top two dividend digits.
         */
        fdividend = (double) dividend[qi] * NBASE_SQR;
        fdividend += (double) dividend[qi + 1];
        fquotient = fdividend * fdivisorinverse;
        qdigit = (fquotient >= 0.0) ? ((int32) fquotient) :
          (((int32) fquotient) - 1);  /* truncate towards -infinity */

        maxdiv += i64abs(qdigit);
      }

      /*
       * Subtract off the appropriate multiple of the divisor.
       *
       * The digits beyond dividend[qi] cannot overflow, because we know
       * they will fall within the maxdiv limit.  As for dividend[qi]
       * itself, note that qdigit is approximately trunc(dividend[qi] /
       * divisor[0]), which would make the new value simply dividend[qi]
       * mod divisor[0].  The lower-order terms in qdigit can change
       * this result by not more than about twice PG_INT64_MAX/NBASE^2,
       * so overflow is impossible.
       *
       * This inner loop is the performance bottleneck for division, so
       * code it in the same way as the inner loop of mul_var() so that
       * it can be auto-vectorized.
       */
      if (qdigit != 0)
      {
        int      istop = Min(var2ndigitpairs, div_ndigitpairs - qi);
        int64     *dividend_qi = &dividend[qi];

        for (i = 0; i < istop; i++)
          dividend_qi[i] -= (int64) qdigit * divisor[i];
      }
    }

    /*
     * The dividend digit we are about to replace might still be nonzero.
     * Fold it into the next digit position.
     *
     * There is no risk of overflow here, although proving that requires
     * some care.  Much as with the argument for dividend[qi] not
     * overflowing, if we consider the first two terms in the numerator
     * and denominator of qdigit, we can see that the final value of
     * dividend[qi + 1] will be approximately a remainder mod
     * (divisor[0]*NBASE^2 + divisor[1]).  Accounting for the lower-order
     * terms is a bit complicated but ends up adding not much more than
     * PG_INT64_MAX/NBASE^2 to the possible range.  Thus, dividend[qi + 1]
     * cannot overflow here, and in its role as dividend[qi] in the next
     * loop iteration, it can't be large enough to cause overflow in the
     * carry propagation step (if any), either.
     *
     * But having said that: dividend[qi] can be more than
     * PG_INT64_MAX/NBASE^2, as noted above, which means that the product
     * dividend[qi] * NBASE^2 *can* overflow.  When that happens, adding
     * it to dividend[qi + 1] will always cause a canceling overflow so
     * that the end result is correct.  We could avoid the intermediate
     * overflow by doing the multiplication and addition using unsigned
     * int64 arithmetic, which is modulo 2^64, but so far there appears no
     * need.
     */
    dividend[qi + 1] += dividend[qi] * NBASE_SQR;

    dividend[qi] = qdigit;
  }

  /*
   * If an exact result was requested, use the remainder to correct the
   * approximate quotient.  The remainder is in dividend[], immediately
   * after the quotient digits.  Note, however, that although the remainder
   * starts at dividend[qi = res_ndigitpairs], the first digit is the result
   * of folding two remainder digits into one above, and the remainder
   * currently only occupies var2ndigitpairs - 1 digits (the last digit of
   * the working dividend was untouched by the computation above).  Thus we
   * expand the remainder down by one base-NBASE^2 digit when we normalize
   * it, so that it completely fills the last var2ndigitpairs digits of the
   * dividend array.
   */
  if (exact)
  {
    /* Normalize the remainder, expanding it down by one digit */
    remainder = &dividend[qi];
    carry = 0;
    for (i = var2ndigitpairs - 2; i >= 0; i--)
    {
      newdig = remainder[i] + carry;
      if (newdig < 0)
      {
        carry = -((-newdig - 1) / NBASE_SQR) - 1;
        newdig -= carry * NBASE_SQR;
      }
      else if (newdig >= NBASE_SQR)
      {
        carry = newdig / NBASE_SQR;
        newdig -= carry * NBASE_SQR;
      }
      else
        carry = 0;
      remainder[i + 1] = newdig;
    }
    remainder[0] = carry;

    if (remainder[0] < 0)
    {
      /*
       * The remainder is negative, so the approximate quotient is too
       * large.  Correct by reducing the quotient by one and adding the
       * divisor to the remainder until the remainder is positive.  We
       * expect the quotient to be off by at most one, which has been
       * borne out in all testing, but not conclusively proven, so we
       * allow for larger corrections, just in case.
       */
      do
      {
        /* Add the divisor to the remainder */
        carry = 0;
        for (i = var2ndigitpairs - 1; i > 0; i--)
        {
          newdig = remainder[i] + divisor[i] + carry;
          if (newdig >= NBASE_SQR)
          {
            remainder[i] = newdig - NBASE_SQR;
            carry = 1;
          }
          else
          {
            remainder[i] = newdig;
            carry = 0;
          }
        }
        remainder[0] += divisor[0] + carry;

        /* Subtract 1 from the quotient (propagating carries later) */
        dividend[qi - 1]--;

      } while (remainder[0] < 0);
    }
    else
    {
      /*
       * The remainder is nonnegative.  If it's greater than or equal to
       * the divisor, then the approximate quotient is too small and
       * must be corrected.  As above, we don't expect to have to apply
       * more than one correction, but allow for it just in case.
       */
      while (true)
      {
        bool    less = false;

        /* Is remainder < divisor? */
        for (i = 0; i < var2ndigitpairs; i++)
        {
          if (remainder[i] < divisor[i])
          {
            less = true;
            break;
          }
          if (remainder[i] > divisor[i])
            break;  /* remainder > divisor */
        }
        if (less)
          break;    /* quotient is correct */

        /* Subtract the divisor from the remainder */
        carry = 0;
        for (i = var2ndigitpairs - 1; i > 0; i--)
        {
          newdig = remainder[i] - divisor[i] + carry;
          if (newdig < 0)
          {
            remainder[i] = newdig + NBASE_SQR;
            carry = -1;
          }
          else
          {
            remainder[i] = newdig;
            carry = 0;
          }
        }
        remainder[0] = remainder[0] - divisor[0] + carry;

        /* Add 1 to the quotient (propagating carries later) */
        dividend[qi - 1]++;
      }
    }
  }

  /*
   * Because the quotient digits were estimates that might have been off by
   * one (and we didn't bother propagating carries when adjusting the
   * quotient above), some quotient digits might be out of range, so do a
   * final carry propagation pass to normalize back to base NBASE^2, and
   * construct the base-NBASE result digits.  Note that this is still done
   * at full precision w/guard digits.
   */
  alloc_var(result, res_ndigits);
  res_digits = result->digits;
  carry = 0;
  for (i = res_ndigitpairs - 1; i >= 0; i--)
  {
    newdig = dividend[i] + carry;
    if (newdig < 0)
    {
      carry = -((-newdig - 1) / NBASE_SQR) - 1;
      newdig -= carry * NBASE_SQR;
    }
    else if (newdig >= NBASE_SQR)
    {
      carry = newdig / NBASE_SQR;
      newdig -= carry * NBASE_SQR;
    }
    else
      carry = 0;
    res_digits[2 * i + 1] = (NumericDigit) ((uint32) newdig % NBASE);
    res_digits[2 * i] = (NumericDigit) ((uint32) newdig / NBASE);
  }
  Assert(carry == 0);

  pfree(dividend);

  /*
   * Finally, round or truncate the result to the requested precision.
   */
  result->weight = res_weight;
  result->sign = res_sign;

  /* Round or truncate to target rscale (and set result->dscale) */
  if (round)
    round_var(result, rscale);
  else
    trunc_var(result, rscale);

  /* Strip leading and trailing zeroes */
  strip_var(result);
}

/*
 * div_var_int() -
 *
 *  Divide a numeric variable by a 32-bit integer with the specified weight.
 *  The quotient var / (ival * NBASE^ival_weight) is stored in result.
 */
static void
div_var_int(const NumericVar *var, int ival, int ival_weight,
      NumericVar *result, int rscale, bool round)
{
  NumericDigit *var_digits = var->digits;
  int      var_ndigits = var->ndigits;
  int      res_sign;
  int      res_weight;
  int      res_ndigits;
  NumericDigit *res_buf;
  NumericDigit *res_digits;
  uint32    divisor;
  int      i;

  /* Guard against division by zero */
  if (ival == 0)
  {
    elog(ERROR, "division by zero");
    return;
  }

  /* Result zero check */
  if (var_ndigits == 0)
  {
    zero_var(result);
    result->dscale = rscale;
    return;
  }

  /*
   * Determine the result sign, weight and number of digits to calculate.
   * The weight figured here is correct if the emitted quotient has no
   * leading zero digits; otherwise strip_var() will fix things up.
   */
  if (var->sign == NUMERIC_POS)
    res_sign = ival > 0 ? NUMERIC_POS : NUMERIC_NEG;
  else
    res_sign = ival > 0 ? NUMERIC_NEG : NUMERIC_POS;
  res_weight = var->weight - ival_weight;
  /* The number of accurate result digits we need to produce: */
  res_ndigits = res_weight + 1 + (rscale + DEC_DIGITS - 1) / DEC_DIGITS;
  /* ... but always at least 1 */
  res_ndigits = Max(res_ndigits, 1);
  /* If rounding needed, figure one more digit to ensure correct result */
  if (round)
    res_ndigits++;

  res_buf = digitbuf_alloc(res_ndigits + 1);
  res_buf[0] = 0;        /* spare digit for later rounding */
  res_digits = res_buf + 1;

  /*
   * Now compute the quotient digits.  This is the short division algorithm
   * described in Knuth volume 2, section 4.3.1 exercise 16, except that we
   * allow the divisor to exceed the internal base.
   *
   * In this algorithm, the carry from one digit to the next is at most
   * divisor - 1.  Therefore, while processing the next digit, carry may
   * become as large as divisor * NBASE - 1, and so it requires a 64-bit
   * integer if this exceeds UINT_MAX.
   */
  divisor = abs(ival);

  if (divisor <= UINT_MAX / NBASE)
  {
    /* carry cannot overflow 32 bits */
    uint32    carry = 0;

    for (i = 0; i < res_ndigits; i++)
    {
      carry = carry * NBASE + (i < var_ndigits ? var_digits[i] : 0);
      res_digits[i] = (NumericDigit) (carry / divisor);
      carry = carry % divisor;
    }
  }
  else
  {
    /* carry may exceed 32 bits */
    uint64    carry = 0;

    for (i = 0; i < res_ndigits; i++)
    {
      carry = carry * NBASE + (i < var_ndigits ? var_digits[i] : 0);
      res_digits[i] = (NumericDigit) (carry / divisor);
      carry = carry % divisor;
    }
  }

  /* Store the quotient in result */
  digitbuf_free(result->buf);
  result->ndigits = res_ndigits;
  result->buf = res_buf;
  result->digits = res_digits;
  result->weight = res_weight;
  result->sign = res_sign;

  /* Round or truncate to target rscale (and set result->dscale) */
  if (round)
    round_var(result, rscale);
  else
    trunc_var(result, rscale);

  /* Strip leading/trailing zeroes */
  strip_var(result);
}

#ifdef HAVE_INT128
/*
 * div_var_int64() -
 *
 *  Divide a numeric variable by a 64-bit integer with the specified weight.
 *  The quotient var / (ival * NBASE^ival_weight) is stored in result.
 *
 *  This duplicates the logic in div_var_int(), so any changes made there
 *  should be made here too.
 */
static void
div_var_int64(const NumericVar *var, int64 ival, int ival_weight,
        NumericVar *result, int rscale, bool round)
{
  NumericDigit *var_digits = var->digits;
  int      var_ndigits = var->ndigits;
  int      res_sign;
  int      res_weight;
  int      res_ndigits;
  NumericDigit *res_buf;
  NumericDigit *res_digits;
  uint64    divisor;
  int      i;

  /* Guard against division by zero */
  if (ival == 0)
  {
    elog(ERROR, "division by zero");
    return;
  }

  /* Result zero check */
  if (var_ndigits == 0)
  {
    zero_var(result);
    result->dscale = rscale;
    return;
  }

  /*
   * Determine the result sign, weight and number of digits to calculate.
   * The weight figured here is correct if the emitted quotient has no
   * leading zero digits; otherwise strip_var() will fix things up.
   */
  if (var->sign == NUMERIC_POS)
    res_sign = ival > 0 ? NUMERIC_POS : NUMERIC_NEG;
  else
    res_sign = ival > 0 ? NUMERIC_NEG : NUMERIC_POS;
  res_weight = var->weight - ival_weight;
  /* The number of accurate result digits we need to produce: */
  res_ndigits = res_weight + 1 + (rscale + DEC_DIGITS - 1) / DEC_DIGITS;
  /* ... but always at least 1 */
  res_ndigits = Max(res_ndigits, 1);
  /* If rounding needed, figure one more digit to ensure correct result */
  if (round)
    res_ndigits++;

  res_buf = digitbuf_alloc(res_ndigits + 1);
  res_buf[0] = 0;        /* spare digit for later rounding */
  res_digits = res_buf + 1;

  /*
   * Now compute the quotient digits.  This is the short division algorithm
   * described in Knuth volume 2, section 4.3.1 exercise 16, except that we
   * allow the divisor to exceed the internal base.
   *
   * In this algorithm, the carry from one digit to the next is at most
   * divisor - 1.  Therefore, while processing the next digit, carry may
   * become as large as divisor * NBASE - 1, and so it requires a 128-bit
   * integer if this exceeds PG_UINT64_MAX.
   */
  divisor = i64abs(ival);

  if (divisor <= PG_UINT64_MAX / NBASE)
  {
    /* carry cannot overflow 64 bits */
    uint64    carry = 0;

    for (i = 0; i < res_ndigits; i++)
    {
      carry = carry * NBASE + (i < var_ndigits ? var_digits[i] : 0);
      res_digits[i] = (NumericDigit) (carry / divisor);
      carry = carry % divisor;
    }
  }
  else
  {
    /* carry may exceed 64 bits */
    uint128    carry = 0;

    for (i = 0; i < res_ndigits; i++)
    {
      carry = carry * NBASE + (i < var_ndigits ? var_digits[i] : 0);
      res_digits[i] = (NumericDigit) (carry / divisor);
      carry = carry % divisor;
    }
  }

  /* Store the quotient in result */
  digitbuf_free(result->buf);
  result->ndigits = res_ndigits;
  result->buf = res_buf;
  result->digits = res_digits;
  result->weight = res_weight;
  result->sign = res_sign;

  /* Round or truncate to target rscale (and set result->dscale) */
  if (round)
    round_var(result, rscale);
  else
    trunc_var(result, rscale);

  /* Strip leading/trailing zeroes */
  strip_var(result);
}
#endif

/*
 * Default scale selection for division
 *
 * Returns the appropriate result scale for the division result.
 */
static int
select_div_scale(const NumericVar *var1, const NumericVar *var2)
{
  int      weight1,
        weight2,
        qweight,
        i;
  NumericDigit firstdigit1,
        firstdigit2;
  int      rscale;

  /*
   * The result scale of a division isn't specified in any SQL standard. For
   * PostgreSQL we select a result scale that will give at least
   * NUMERIC_MIN_SIG_DIGITS significant digits, so that numeric gives a
   * result no less accurate than float8; but use a scale not less than
   * either input's display scale.
   */

  /* Get the actual (normalized) weight and first digit of each input */

  weight1 = 0;        /* values to use if var1 is zero */
  firstdigit1 = 0;
  for (i = 0; i < var1->ndigits; i++)
  {
    firstdigit1 = var1->digits[i];
    if (firstdigit1 != 0)
    {
      weight1 = var1->weight - i;
      break;
    }
  }

  weight2 = 0;        /* values to use if var2 is zero */
  firstdigit2 = 0;
  for (i = 0; i < var2->ndigits; i++)
  {
    firstdigit2 = var2->digits[i];
    if (firstdigit2 != 0)
    {
      weight2 = var2->weight - i;
      break;
    }
  }

  /*
   * Estimate weight of quotient.  If the two first digits are equal, we
   * can't be sure, but assume that var1 is less than var2.
   */
  qweight = weight1 - weight2;
  if (firstdigit1 <= firstdigit2)
    qweight--;

  /* Select result scale */
  rscale = NUMERIC_MIN_SIG_DIGITS - qweight * DEC_DIGITS;
  rscale = Max(rscale, var1->dscale);
  rscale = Max(rscale, var2->dscale);
  rscale = Max(rscale, NUMERIC_MIN_DISPLAY_SCALE);
  rscale = Min(rscale, NUMERIC_MAX_DISPLAY_SCALE);

  return rscale;
}

/*
 * mod_var() -
 *
 *  Calculate the modulo of two numerics at variable level
 */
static void
mod_var(const NumericVar *var1, const NumericVar *var2, NumericVar *result)
{
  NumericVar  tmp;

  init_var(&tmp);

  /* ---------
   * We do this using the equation
   *    mod(x,y) = x - trunc(x/y)*y
   * div_var can be persuaded to give us trunc(x/y) directly.
   * ----------
   */
  div_var(var1, var2, &tmp, 0, false, true);

  mul_var(var2, &tmp, &tmp, var2->dscale);

  sub_var(var1, &tmp, result);

  free_var(&tmp);
}

/*
 * div_mod_var() -
 *
 *  Calculate the truncated integer quotient and numeric remainder of two
 *  numeric variables.  The remainder is precise to var2's dscale.
 */
static void
div_mod_var(const NumericVar *var1, const NumericVar *var2,
      NumericVar *quot, NumericVar *rem)
{
  NumericVar  q;
  NumericVar  r;

  init_var(&q);
  init_var(&r);

  /*
   * Use div_var() with exact = false to get an initial estimate for the
   * integer quotient (truncated towards zero).  This might be slightly
   * inaccurate, but we correct it below.
   */
  div_var(var1, var2, &q, 0, false, false);

  /* Compute initial estimate of remainder using the quotient estimate. */
  mul_var(var2, &q, &r, var2->dscale);
  sub_var(var1, &r, &r);

  /*
   * Adjust the results if necessary --- the remainder should have the same
   * sign as var1, and its absolute value should be less than the absolute
   * value of var2.
   */
  while (r.ndigits != 0 && r.sign != var1->sign)
  {
    /* The absolute value of the quotient is too large */
    if (var1->sign == var2->sign)
    {
      sub_var(&q, &const_one, &q);
      add_var(&r, var2, &r);
    }
    else
    {
      add_var(&q, &const_one, &q);
      sub_var(&r, var2, &r);
    }
  }

  while (cmp_abs(&r, var2) >= 0)
  {
    /* The absolute value of the quotient is too small */
    if (var1->sign == var2->sign)
    {
      add_var(&q, &const_one, &q);
      sub_var(&r, var2, &r);
    }
    else
    {
      sub_var(&q, &const_one, &q);
      add_var(&r, var2, &r);
    }
  }

  set_var_from_var(&q, quot);
  set_var_from_var(&r, rem);

  free_var(&q);
  free_var(&r);
}

/*
 * ceil_var() -
 *
 *  Return the smallest integer greater than or equal to the argument
 *  on variable level
 */
static void
ceil_var(const NumericVar *var, NumericVar *result)
{
  NumericVar  tmp;

  init_var(&tmp);
  set_var_from_var(var, &tmp);

  trunc_var(&tmp, 0);

  if (var->sign == NUMERIC_POS && cmp_var(var, &tmp) != 0)
    add_var(&tmp, &const_one, &tmp);

  set_var_from_var(&tmp, result);
  free_var(&tmp);
}

/*
 * floor_var() -
 *
 *  Return the largest integer equal to or less than the argument
 *  on variable level
 */
static void
floor_var(const NumericVar *var, NumericVar *result)
{
  NumericVar  tmp;

  init_var(&tmp);
  set_var_from_var(var, &tmp);

  trunc_var(&tmp, 0);

  if (var->sign == NUMERIC_NEG && cmp_var(var, &tmp) != 0)
    sub_var(&tmp, &const_one, &tmp);

  set_var_from_var(&tmp, result);
  free_var(&tmp);
}

/*
 * gcd_var() -
 *
 *  Calculate the greatest common divisor of two numerics at variable level
 */
static void
gcd_var(const NumericVar *var1, const NumericVar *var2, NumericVar *result)
{
  int      res_dscale;
  int      cmp;
  NumericVar  tmp_arg;
  NumericVar  mod;

  res_dscale = Max(var1->dscale, var2->dscale);

  /*
   * Arrange for var1 to be the number with the greater absolute value.
   *
   * This would happen automatically in the loop below, but avoids an
   * expensive modulo operation.
   */
  cmp = cmp_abs(var1, var2);
  if (cmp < 0)
  {
    const NumericVar *tmp = var1;

    var1 = var2;
    var2 = tmp;
  }

  /*
   * Also avoid the taking the modulo if the inputs have the same absolute
   * value, or if the smaller input is zero.
   */
  if (cmp == 0 || var2->ndigits == 0)
  {
    set_var_from_var(var1, result);
    result->sign = NUMERIC_POS;
    result->dscale = res_dscale;
    return;
  }

  init_var(&tmp_arg);
  init_var(&mod);

  /* Use the Euclidean algorithm to find the GCD */
  set_var_from_var(var1, &tmp_arg);
  set_var_from_var(var2, result);

  for (;;)
  {
    /* this loop can take a while, so allow it to be interrupted */
    // CHECK_FOR_INTERRUPTS();

    mod_var(&tmp_arg, result, &mod);
    if (mod.ndigits == 0)
      break;
    set_var_from_var(result, &tmp_arg);
    set_var_from_var(&mod, result);
  }
  result->sign = NUMERIC_POS;
  result->dscale = res_dscale;

  free_var(&tmp_arg);
  free_var(&mod);
}

/*
 * sqrt_var() -
 *
 *  Compute the square root of x using the Karatsuba Square Root algorithm.
 *  NOTE: we allow rscale < 0 here, implying rounding before the decimal
 *  point.
 */
static void
sqrt_var(const NumericVar *arg, NumericVar *result, int rscale)
{
  int      stat;
  int      res_weight;
  int      res_ndigits;
  int      src_ndigits;
  int      step;
  int      ndigits[32];
  int      blen;
  int64    arg_int64;
  int      src_idx;
  int64    s_int64;
  int64    r_int64;
  NumericVar  s_var;
  NumericVar  r_var;
  NumericVar  a0_var;
  NumericVar  a1_var;
  NumericVar  q_var;
  NumericVar  u_var;

  stat = cmp_var(arg, &const_zero);
  if (stat == 0)
  {
    zero_var(result);
    result->dscale = rscale;
    return;
  }

  /*
   * SQL2003 defines sqrt() in terms of power, so we need to emit the right
   * SQLSTATE error code if the operand is negative.
   */
  if (stat < 0)
  {
    elog(ERROR, "cannot take square root of a negative number");
    return;
  }

  init_var(&s_var);
  init_var(&r_var);
  init_var(&a0_var);
  init_var(&a1_var);
  init_var(&q_var);
  init_var(&u_var);

  /*
   * The result weight is half the input weight, rounded towards minus
   * infinity --- res_weight = floor(arg->weight / 2).
   */
  if (arg->weight >= 0)
    res_weight = arg->weight / 2;
  else
    res_weight = -((-arg->weight - 1) / 2 + 1);

  /*
   * Number of NBASE digits to compute.  To ensure correct rounding, compute
   * at least 1 extra decimal digit.  We explicitly allow rscale to be
   * negative here, but must always compute at least 1 NBASE digit.  Thus
   * res_ndigits = res_weight + 1 + ceil((rscale + 1) / DEC_DIGITS) or 1.
   */
  if (rscale + 1 >= 0)
    res_ndigits = res_weight + 1 + (rscale + DEC_DIGITS) / DEC_DIGITS;
  else
    res_ndigits = res_weight + 1 - (-rscale - 1) / DEC_DIGITS;
  res_ndigits = Max(res_ndigits, 1);

  /*
   * Number of source NBASE digits logically required to produce a result
   * with this precision --- every digit before the decimal point, plus 2
   * for each result digit after the decimal point (or minus 2 for each
   * result digit we round before the decimal point).
   */
  src_ndigits = arg->weight + 1 + (res_ndigits - res_weight - 1) * 2;
  src_ndigits = Max(src_ndigits, 1);

  /* ----------
   * From this point on, we treat the input and the result as integers and
   * compute the integer square root and remainder using the Karatsuba
   * Square Root algorithm, which may be written recursively as follows:
   *
   *  SqrtRem(n = a3*b^3 + a2*b^2 + a1*b + a0):
   *    [ for some base b, and coefficients a0,a1,a2,a3 chosen so that
   *      0 <= a0,a1,a2 < b and a3 >= b/4 ]
   *    Let (s,r) = SqrtRem(a3*b + a2)
   *    Let (q,u) = DivRem(r*b + a1, 2*s)
   *    Let s = s*b + q
   *    Let r = u*b + a0 - q^2
   *    If r < 0 Then
   *      Let r = r + s
   *      Let s = s - 1
   *      Let r = r + s
   *    Return (s,r)
   *
   * See "Karatsuba Square Root", Paul Zimmermann, INRIA Research Report
   * RR-3805, November 1999.  At the time of writing this was available
   * on the net at <https://hal.inria.fr/inria-00072854>.
   *
   * The way to read the assumption "n = a3*b^3 + a2*b^2 + a1*b + a0" is
   * "choose a base b such that n requires at least four base-b digits to
   * express; then those digits are a3,a2,a1,a0, with a3 possibly larger
   * than b".  For optimal performance, b should have approximately a
   * quarter the number of digits in the input, so that the outer square
   * root computes roughly twice as many digits as the inner one.  For
   * simplicity, we choose b = NBASE^blen, an integer power of NBASE.
   *
   * We implement the algorithm iteratively rather than recursively, to
   * allow the working variables to be reused.  With this approach, each
   * digit of the input is read precisely once --- src_idx tracks the number
   * of input digits used so far.
   *
   * The array ndigits[] holds the number of NBASE digits of the input that
   * will have been used at the end of each iteration, which roughly doubles
   * each time.  Note that the array elements are stored in reverse order,
   * so if the final iteration requires src_ndigits = 37 input digits, the
   * array will contain [37,19,11,7,5,3], and we would start by computing
   * the square root of the 3 most significant NBASE digits.
   *
   * In each iteration, we choose blen to be the largest integer for which
   * the input number has a3 >= b/4, when written in the form above.  In
   * general, this means blen = src_ndigits / 4 (truncated), but if
   * src_ndigits is a multiple of 4, that might lead to the coefficient a3
   * being less than b/4 (if the first input digit is less than NBASE/4), in
   * which case we choose blen = src_ndigits / 4 - 1.  The number of digits
   * in the inner square root is then src_ndigits - 2*blen.  So, for
   * example, if we have src_ndigits = 26 initially, the array ndigits[]
   * will be either [26,14,8,4] or [26,14,8,6,4], depending on the size of
   * the first input digit.
   *
   * Additionally, we can put an upper bound on the number of steps required
   * as follows --- suppose that the number of source digits is an n-bit
   * number in the range [2^(n-1), 2^n-1], then blen will be in the range
   * [2^(n-3)-1, 2^(n-2)-1] and the number of digits in the inner square
   * root will be in the range [2^(n-2), 2^(n-1)+1].  In the next step, blen
   * will be in the range [2^(n-4)-1, 2^(n-3)] and the number of digits in
   * the next inner square root will be in the range [2^(n-3), 2^(n-2)+1].
   * This pattern repeats, and in the worst case the array ndigits[] will
   * contain [2^n-1, 2^(n-1)+1, 2^(n-2)+1, ... 9, 5, 3], and the computation
   * will require n steps.  Therefore, since all digit array sizes are
   * signed 32-bit integers, the number of steps required is guaranteed to
   * be less than 32.
   * ----------
   */
  step = 0;
  while ((ndigits[step] = src_ndigits) > 4)
  {
    /* Choose b so that a3 >= b/4, as described above */
    blen = src_ndigits / 4;
    if (blen * 4 == src_ndigits && arg->digits[0] < NBASE / 4)
      blen--;

    /* Number of digits in the next step (inner square root) */
    src_ndigits -= 2 * blen;
    step++;
  }

  /*
   * First iteration (innermost square root and remainder):
   *
   * Here src_ndigits <= 4, and the input fits in an int64.  Its square root
   * has at most 9 decimal digits, so estimate it using double precision
   * arithmetic, which will in fact almost certainly return the correct
   * result with no further correction required.
   */
  arg_int64 = arg->digits[0];
  for (src_idx = 1; src_idx < src_ndigits; src_idx++)
  {
    arg_int64 *= NBASE;
    if (src_idx < arg->ndigits)
      arg_int64 += arg->digits[src_idx];
  }

  s_int64 = (int64) sqrt((double) arg_int64);
  r_int64 = arg_int64 - s_int64 * s_int64;

  /*
   * Use Newton's method to correct the result, if necessary.
   *
   * This uses integer division with truncation to compute the truncated
   * integer square root by iterating using the formula x -> (x + n/x) / 2.
   * This is known to converge to isqrt(n), unless n+1 is a perfect square.
   * If n+1 is a perfect square, the sequence will oscillate between the two
   * values isqrt(n) and isqrt(n)+1, so we can be assured of convergence by
   * checking the remainder.
   */
  while (r_int64 < 0 || r_int64 > 2 * s_int64)
  {
    s_int64 = (s_int64 + arg_int64 / s_int64) / 2;
    r_int64 = arg_int64 - s_int64 * s_int64;
  }

  /*
   * Iterations with src_ndigits <= 8:
   *
   * The next 1 or 2 iterations compute larger (outer) square roots with
   * src_ndigits <= 8, so the result still fits in an int64 (even though the
   * input no longer does) and we can continue to compute using int64
   * variables to avoid more expensive numeric computations.
   *
   * It is fairly easy to see that there is no risk of the intermediate
   * values below overflowing 64-bit integers.  In the worst case, the
   * previous iteration will have computed a 3-digit square root (of a
   * 6-digit input less than NBASE^6 / 4), so at the start of this
   * iteration, s will be less than NBASE^3 / 2 = 10^12 / 2, and r will be
   * less than 10^12.  In this case, blen will be 1, so numer will be less
   * than 10^17, and denom will be less than 10^12 (and hence u will also be
   * less than 10^12).  Finally, since q^2 = u*b + a0 - r, we can also be
   * sure that q^2 < 10^17.  Therefore all these quantities fit comfortably
   * in 64-bit integers.
   */
  step--;
  while (step >= 0 && (src_ndigits = ndigits[step]) <= 8)
  {
    int      b;
    int      a0;
    int      a1;
    int      i;
    int64    numer;
    int64    denom;
    int64    q;
    int64    u;

    blen = (src_ndigits - src_idx) / 2;

    /* Extract a1 and a0, and compute b */
    a0 = 0;
    a1 = 0;
    b = 1;

    for (i = 0; i < blen; i++, src_idx++)
    {
      b *= NBASE;
      a1 *= NBASE;
      if (src_idx < arg->ndigits)
        a1 += arg->digits[src_idx];
    }

    for (i = 0; i < blen; i++, src_idx++)
    {
      a0 *= NBASE;
      if (src_idx < arg->ndigits)
        a0 += arg->digits[src_idx];
    }

    /* Compute (q,u) = DivRem(r*b + a1, 2*s) */
    numer = r_int64 * b + a1;
    denom = 2 * s_int64;
    q = numer / denom;
    u = numer - q * denom;

    /* Compute s = s*b + q and r = u*b + a0 - q^2 */
    s_int64 = s_int64 * b + q;
    r_int64 = u * b + a0 - q * q;

    if (r_int64 < 0)
    {
      /* s is too large by 1; set r += s, s--, r += s */
      r_int64 += s_int64;
      s_int64--;
      r_int64 += s_int64;
    }

    Assert(src_idx == src_ndigits); /* All input digits consumed */
    step--;
  }

  /*
   * On platforms with 128-bit integer support, we can further delay the
   * need to use numeric variables.
   */
#ifdef HAVE_INT128
  if (step >= 0)
  {
    int128    s_int128;
    int128    r_int128;

    s_int128 = s_int64;
    r_int128 = r_int64;

    /*
     * Iterations with src_ndigits <= 16:
     *
     * The result fits in an int128 (even though the input doesn't) so we
     * use int128 variables to avoid more expensive numeric computations.
     */
    while (step >= 0 && (src_ndigits = ndigits[step]) <= 16)
    {
      int64    b;
      int64    a0;
      int64    a1;
      int64    i;
      int128    numer;
      int128    denom;
      int128    q;
      int128    u;

      blen = (src_ndigits - src_idx) / 2;

      /* Extract a1 and a0, and compute b */
      a0 = 0;
      a1 = 0;
      b = 1;

      for (i = 0; i < blen; i++, src_idx++)
      {
        b *= NBASE;
        a1 *= NBASE;
        if (src_idx < arg->ndigits)
          a1 += arg->digits[src_idx];
      }

      for (i = 0; i < blen; i++, src_idx++)
      {
        a0 *= NBASE;
        if (src_idx < arg->ndigits)
          a0 += arg->digits[src_idx];
      }

      /* Compute (q,u) = DivRem(r*b + a1, 2*s) */
      numer = r_int128 * b + a1;
      denom = 2 * s_int128;
      q = numer / denom;
      u = numer - q * denom;

      /* Compute s = s*b + q and r = u*b + a0 - q^2 */
      s_int128 = s_int128 * b + q;
      r_int128 = u * b + a0 - q * q;

      if (r_int128 < 0)
      {
        /* s is too large by 1; set r += s, s--, r += s */
        r_int128 += s_int128;
        s_int128--;
        r_int128 += s_int128;
      }

      Assert(src_idx == src_ndigits); /* All input digits consumed */
      step--;
    }

    /*
     * All remaining iterations require numeric variables.  Convert the
     * integer values to NumericVar and continue.  Note that in the final
     * iteration we don't need the remainder, so we can save a few cycles
     * there by not fully computing it.
     */
    int128_to_numericvar(s_int128, &s_var);
    if (step >= 0)
      int128_to_numericvar(r_int128, &r_var);
  }
  else
  {
    int64_to_numericvar(s_int64, &s_var);
    /* step < 0, so we certainly don't need r */
  }
#else              /* !HAVE_INT128 */
  int64_to_numericvar(s_int64, &s_var);
  if (step >= 0)
    int64_to_numericvar(r_int64, &r_var);
#endif              /* HAVE_INT128 */

  /*
   * The remaining iterations with src_ndigits > 8 (or 16, if have int128)
   * use numeric variables.
   */
  while (step >= 0)
  {
    int      tmp_len;

    src_ndigits = ndigits[step];
    blen = (src_ndigits - src_idx) / 2;

    /* Extract a1 and a0 */
    if (src_idx < arg->ndigits)
    {
      tmp_len = Min(blen, arg->ndigits - src_idx);
      alloc_var(&a1_var, tmp_len);
      memcpy(a1_var.digits, arg->digits + src_idx,
           tmp_len * sizeof(NumericDigit));
      a1_var.weight = blen - 1;
      a1_var.sign = NUMERIC_POS;
      a1_var.dscale = 0;
      strip_var(&a1_var);
    }
    else
    {
      zero_var(&a1_var);
      a1_var.dscale = 0;
    }
    src_idx += blen;

    if (src_idx < arg->ndigits)
    {
      tmp_len = Min(blen, arg->ndigits - src_idx);
      alloc_var(&a0_var, tmp_len);
      memcpy(a0_var.digits, arg->digits + src_idx,
           tmp_len * sizeof(NumericDigit));
      a0_var.weight = blen - 1;
      a0_var.sign = NUMERIC_POS;
      a0_var.dscale = 0;
      strip_var(&a0_var);
    }
    else
    {
      zero_var(&a0_var);
      a0_var.dscale = 0;
    }
    src_idx += blen;

    /* Compute (q,u) = DivRem(r*b + a1, 2*s) */
    set_var_from_var(&r_var, &q_var);
    q_var.weight += blen;
    add_var(&q_var, &a1_var, &q_var);
    add_var(&s_var, &s_var, &u_var);
    div_mod_var(&q_var, &u_var, &q_var, &u_var);

    /* Compute s = s*b + q */
    s_var.weight += blen;
    add_var(&s_var, &q_var, &s_var);

    /*
     * Compute r = u*b + a0 - q^2.
     *
     * In the final iteration, we don't actually need r; we just need to
     * know whether it is negative, so that we know whether to adjust s.
     * So instead of the final subtraction we can just compare.
     */
    u_var.weight += blen;
    add_var(&u_var, &a0_var, &u_var);
    mul_var(&q_var, &q_var, &q_var, 0);

    if (step > 0)
    {
      /* Need r for later iterations */
      sub_var(&u_var, &q_var, &r_var);
      if (r_var.sign == NUMERIC_NEG)
      {
        /* s is too large by 1; set r += s, s--, r += s */
        add_var(&r_var, &s_var, &r_var);
        sub_var(&s_var, &const_one, &s_var);
        add_var(&r_var, &s_var, &r_var);
      }
    }
    else
    {
      /* Don't need r anymore, except to test if s is too large by 1 */
      if (cmp_var(&u_var, &q_var) < 0)
        sub_var(&s_var, &const_one, &s_var);
    }

    Assert(src_idx == src_ndigits); /* All input digits consumed */
    step--;
  }

  /*
   * Construct the final result, rounding it to the requested precision.
   */
  set_var_from_var(&s_var, result);
  result->weight = res_weight;
  result->sign = NUMERIC_POS;

  /* Round to target rscale (and set result->dscale) */
  round_var(result, rscale);

  /* Strip leading and trailing zeroes */
  strip_var(result);

  free_var(&s_var);
  free_var(&r_var);
  free_var(&a0_var);
  free_var(&a1_var);
  free_var(&q_var);
  free_var(&u_var);
}

/*
 * exp_var() -
 *
 *  Raise e to the power of x, computed to rscale fractional digits
 */
static void
exp_var(const NumericVar *arg, NumericVar *result, int rscale)
{
  NumericVar  x;
  NumericVar  elem;
  int      ni;
  double    val;
  int      dweight;
  int      ndiv2;
  int      sig_digits;
  int      local_rscale;

  init_var(&x);
  init_var(&elem);

  set_var_from_var(arg, &x);

  /*
   * Estimate the dweight of the result using floating point arithmetic, so
   * that we can choose an appropriate local rscale for the calculation.
   */
  val = numericvar_to_double_no_overflow(&x);

  /* Guard against overflow/underflow */
  /* If you change this limit, see also power_var()'s limit */
  if (fabs(val) >= NUMERIC_MAX_RESULT_SCALE * 3)
  {
    if (val > 0)
    {
      elog(ERROR, "value overflows numeric format");
      return;
    }
    zero_var(result);
    result->dscale = rscale;
    return;
  }

  /* decimal weight = log10(e^x) = x * log10(e) */
  dweight = (int) (val * 0.434294481903252);

  /*
   * Reduce x to the range -0.01 <= x <= 0.01 (approximately) by dividing by
   * 2^ndiv2, to improve the convergence rate of the Taylor series.
   *
   * Note that the overflow check above ensures that fabs(x) < 6000, which
   * means that ndiv2 <= 20 here.
   */
  if (fabs(val) > 0.01)
  {
    ndiv2 = 1;
    val /= 2;

    while (fabs(val) > 0.01)
    {
      ndiv2++;
      val /= 2;
    }

    local_rscale = x.dscale + ndiv2;
    div_var_int(&x, 1 << ndiv2, 0, &x, local_rscale, true);
  }
  else
    ndiv2 = 0;

  /*
   * Set the scale for the Taylor series expansion.  The final result has
   * (dweight + rscale + 1) significant digits.  In addition, we have to
   * raise the Taylor series result to the power 2^ndiv2, which introduces
   * an error of up to around log10(2^ndiv2) digits, so work with this many
   * extra digits of precision (plus a few more for good measure).
   */
  sig_digits = 1 + dweight + rscale + (int) (ndiv2 * 0.301029995663981);
  sig_digits = Max(sig_digits, 0) + 8;

  local_rscale = sig_digits - 1;

  /*
   * Use the Taylor series
   *
   * exp(x) = 1 + x + x^2/2! + x^3/3! + ...
   *
   * Given the limited range of x, this should converge reasonably quickly.
   * We run the series until the terms fall below the local_rscale limit.
   */
  add_var(&const_one, &x, result);

  mul_var(&x, &x, &elem, local_rscale);
  ni = 2;
  div_var_int(&elem, ni, 0, &elem, local_rscale, true);

  while (elem.ndigits != 0)
  {
    add_var(result, &elem, result);

    mul_var(&elem, &x, &elem, local_rscale);
    ni++;
    div_var_int(&elem, ni, 0, &elem, local_rscale, true);
  }

  /*
   * Compensate for the argument range reduction.  Since the weight of the
   * result doubles with each multiplication, we can reduce the local rscale
   * as we proceed.
   */
  while (ndiv2-- > 0)
  {
    local_rscale = sig_digits - result->weight * 2 * DEC_DIGITS;
    local_rscale = Max(local_rscale, NUMERIC_MIN_DISPLAY_SCALE);
    mul_var(result, result, result, local_rscale);
  }

  /* Round to requested rscale */
  round_var(result, rscale);

  free_var(&x);
  free_var(&elem);
}

/*
 * Estimate the dweight of the most significant decimal digit of the natural
 * logarithm of a number.
 *
 * Essentially, we're approximating log10(abs(ln(var))).  This is used to
 * determine the appropriate rscale when computing natural logarithms.
 *
 * Note: many callers call this before range-checking the input.  Therefore,
 * we must be robust against values that are invalid to apply ln() to.
 * We don't wish to throw an error here, so just return zero in such cases.
 */
static int
estimate_ln_dweight(const NumericVar *var)
{
  int      ln_dweight;

  /* Caller should fail on ln(negative), but for the moment return zero */
  if (var->sign != NUMERIC_POS)
    return 0;

  if (cmp_var(var, &const_zero_point_nine) >= 0 &&
    cmp_var(var, &const_one_point_one) <= 0)
  {
    /*
     * 0.9 <= var <= 1.1
     *
     * ln(var) has a negative weight (possibly very large).  To get a
     * reasonably accurate result, estimate it using ln(1+x) ~= x.
     */
    NumericVar  x;

    init_var(&x);
    sub_var(var, &const_one, &x);

    if (x.ndigits > 0)
    {
      /* Use weight of most significant decimal digit of x */
      ln_dweight = x.weight * DEC_DIGITS + (int) log10(x.digits[0]);
    }
    else
    {
      /* x = 0.  Since ln(1) = 0 exactly, we don't need extra digits */
      ln_dweight = 0;
    }

    free_var(&x);
  }
  else
  {
    /*
     * Estimate the logarithm using the first couple of digits from the
     * input number.  This will give an accurate result whenever the input
     * is not too close to 1.
     */
    if (var->ndigits > 0)
    {
      int      digits;
      int      dweight;
      double    ln_var;

      digits = var->digits[0];
      dweight = var->weight * DEC_DIGITS;

      if (var->ndigits > 1)
      {
        digits = digits * NBASE + var->digits[1];
        dweight -= DEC_DIGITS;
      }

      /*----------
       * We have var ~= digits * 10^dweight
       * so ln(var) ~= ln(digits) + dweight * ln(10)
       *----------
       */
      ln_var = log((double) digits) + dweight * 2.302585092994046;
      ln_dweight = (int) log10(fabs(ln_var));
    }
    else
    {
      /* Caller should fail on ln(0), but for the moment return zero */
      ln_dweight = 0;
    }
  }

  return ln_dweight;
}

/*
 * ln_var() -
 *
 *  Compute the natural log of x
 */
static void
ln_var(const NumericVar *arg, NumericVar *result, int rscale)
{
  NumericVar  x;
  NumericVar  xx;
  int      ni;
  NumericVar  elem;
  NumericVar  fact;
  int      nsqrt;
  int      local_rscale;
  int      cmp;

  cmp = cmp_var(arg, &const_zero);
  if (cmp == 0)
  {
    elog(ERROR, "cannot take logarithm of zero");
    return;
  }
  else if (cmp < 0)
  {
    elog(ERROR, "cannot take logarithm of a negative number");
    return;
  }

  init_var(&x);
  init_var(&xx);
  init_var(&elem);
  init_var(&fact);

  set_var_from_var(arg, &x);
  set_var_from_var(&const_two, &fact);

  /*
   * Reduce input into range 0.9 < x < 1.1 with repeated sqrt() operations.
   *
   * The final logarithm will have up to around rscale+6 significant digits.
   * Each sqrt() will roughly halve the weight of x, so adjust the local
   * rscale as we work so that we keep this many significant digits at each
   * step (plus a few more for good measure).
   *
   * Note that we allow local_rscale < 0 during this input reduction
   * process, which implies rounding before the decimal point.  sqrt_var()
   * explicitly supports this, and it significantly reduces the work
   * required to reduce very large inputs to the required range.  Once the
   * input reduction is complete, x.weight will be 0 and its display scale
   * will be non-negative again.
   */
  nsqrt = 0;
  while (cmp_var(&x, &const_zero_point_nine) <= 0)
  {
    local_rscale = rscale - x.weight * DEC_DIGITS / 2 + 8;
    sqrt_var(&x, &x, local_rscale);
    mul_var(&fact, &const_two, &fact, 0);
    nsqrt++;
  }
  while (cmp_var(&x, &const_one_point_one) >= 0)
  {
    local_rscale = rscale - x.weight * DEC_DIGITS / 2 + 8;
    sqrt_var(&x, &x, local_rscale);
    mul_var(&fact, &const_two, &fact, 0);
    nsqrt++;
  }

  /*
   * We use the Taylor series for 0.5 * ln((1+z)/(1-z)),
   *
   * z + z^3/3 + z^5/5 + ...
   *
   * where z = (x-1)/(x+1) is in the range (approximately) -0.053 .. 0.048
   * due to the above range-reduction of x.
   *
   * The convergence of this is not as fast as one would like, but is
   * tolerable given that z is small.
   *
   * The Taylor series result will be multiplied by 2^(nsqrt+1), which has a
   * decimal weight of (nsqrt+1) * log10(2), so work with this many extra
   * digits of precision (plus a few more for good measure).
   */
  local_rscale = rscale + (int) ((nsqrt + 1) * 0.301029995663981) + 8;

  sub_var(&x, &const_one, result);
  add_var(&x, &const_one, &elem);
  div_var(result, &elem, result, local_rscale, true, false);
  set_var_from_var(result, &xx);
  mul_var(result, result, &x, local_rscale);

  ni = 1;

  for (;;)
  {
    ni += 2;
    mul_var(&xx, &x, &xx, local_rscale);
    div_var_int(&xx, ni, 0, &elem, local_rscale, true);

    if (elem.ndigits == 0)
      break;

    add_var(result, &elem, result);

    if (elem.weight < (result->weight - local_rscale * 2 / DEC_DIGITS))
      break;
  }

  /* Compensate for argument range reduction, round to requested rscale */
  mul_var(result, &fact, result, rscale);

  free_var(&x);
  free_var(&xx);
  free_var(&elem);
  free_var(&fact);
}

/*
 * log_var() -
 *
 *  Compute the logarithm of num in a given base.
 *
 *  Note: this routine chooses dscale of the result.
 */
static void
log_var(const NumericVar *base, const NumericVar *num, NumericVar *result)
{
  NumericVar  ln_base;
  NumericVar  ln_num;
  int      ln_base_dweight;
  int      ln_num_dweight;
  int      result_dweight;
  int      rscale;
  int      ln_base_rscale;
  int      ln_num_rscale;

  init_var(&ln_base);
  init_var(&ln_num);

  /* Estimated dweights of ln(base), ln(num) and the final result */
  ln_base_dweight = estimate_ln_dweight(base);
  ln_num_dweight = estimate_ln_dweight(num);
  result_dweight = ln_num_dweight - ln_base_dweight;

  /*
   * Select the scale of the result so that it will have at least
   * NUMERIC_MIN_SIG_DIGITS significant digits and is not less than either
   * input's display scale.
   */
  rscale = NUMERIC_MIN_SIG_DIGITS - result_dweight;
  rscale = Max(rscale, base->dscale);
  rscale = Max(rscale, num->dscale);
  rscale = Max(rscale, NUMERIC_MIN_DISPLAY_SCALE);
  rscale = Min(rscale, NUMERIC_MAX_DISPLAY_SCALE);

  /*
   * Set the scales for ln(base) and ln(num) so that they each have more
   * significant digits than the final result.
   */
  ln_base_rscale = rscale + result_dweight - ln_base_dweight + 8;
  ln_base_rscale = Max(ln_base_rscale, NUMERIC_MIN_DISPLAY_SCALE);

  ln_num_rscale = rscale + result_dweight - ln_num_dweight + 8;
  ln_num_rscale = Max(ln_num_rscale, NUMERIC_MIN_DISPLAY_SCALE);

  /* Form natural logarithms */
  ln_var(base, &ln_base, ln_base_rscale);
  ln_var(num, &ln_num, ln_num_rscale);

  /* Divide and round to the required scale */
  div_var(&ln_num, &ln_base, result, rscale, true, false);

  free_var(&ln_num);
  free_var(&ln_base);
}

/*
 * power_var() -
 *
 *  Raise base to the power of exp
 *
 *  Note: this routine chooses dscale of the result.
 */
static void
power_var(const NumericVar *base, const NumericVar *exp, NumericVar *result)
{
  int      res_sign;
  NumericVar  abs_base;
  NumericVar  ln_base;
  NumericVar  ln_num;
  int      ln_dweight;
  int      rscale;
  int      sig_digits;
  int      local_rscale;
  double    val;

  /* If exp can be represented as an integer, use power_var_int */
  if (exp->ndigits == 0 || exp->ndigits <= exp->weight + 1)
  {
    /* exact integer, but does it fit in int? */
    int64    expval64;

    if (numericvar_to_int64(exp, &expval64))
    {
      if (expval64 >= PG_INT32_MIN && expval64 <= PG_INT32_MAX)
      {
        /* Okay, use power_var_int */
        power_var_int(base, (int) expval64, exp->dscale, result);
        return;
      }
    }
  }

  /*
   * This avoids log(0) for cases of 0 raised to a non-integer.  0 ^ 0 is
   * handled by power_var_int().
   */
  if (cmp_var(base, &const_zero) == 0)
  {
    set_var_from_var(&const_zero, result);
    result->dscale = NUMERIC_MIN_SIG_DIGITS;  /* no need to round */
    return;
  }

  init_var(&abs_base);
  init_var(&ln_base);
  init_var(&ln_num);

  /*
   * If base is negative, insist that exp be an integer.  The result is then
   * positive if exp is even and negative if exp is odd.
   */
  if (base->sign == NUMERIC_NEG)
  {
    /*
     * Check that exp is an integer.  This error code is defined by the
     * SQL standard, and matches other errors in numeric_power().
     */
    if (exp->ndigits > 0 && exp->ndigits > exp->weight + 1)
    {
      elog(ERROR, 
        "a negative number raised to a non-integer power yields a complex result");
      return;
    }

    /* Test if exp is odd or even */
    if (exp->ndigits > 0 && exp->ndigits == exp->weight + 1 &&
      (exp->digits[exp->ndigits - 1] & 1))
      res_sign = NUMERIC_NEG;
    else
      res_sign = NUMERIC_POS;

    /* Then work with abs(base) below */
    set_var_from_var(base, &abs_base);
    abs_base.sign = NUMERIC_POS;
    base = &abs_base;
  }
  else
    res_sign = NUMERIC_POS;

  /*----------
   * Decide on the scale for the ln() calculation.  For this we need an
   * estimate of the weight of the result, which we obtain by doing an
   * initial low-precision calculation of exp * ln(base).
   *
   * We want result = e ^ (exp * ln(base))
   * so result dweight = log10(result) = exp * ln(base) * log10(e)
   *
   * We also perform a crude overflow test here so that we can exit early if
   * the full-precision result is sure to overflow, and to guard against
   * integer overflow when determining the scale for the real calculation.
   * exp_var() supports inputs up to NUMERIC_MAX_RESULT_SCALE * 3, so the
   * result will overflow if exp * ln(base) >= NUMERIC_MAX_RESULT_SCALE * 3.
   * Since the values here are only approximations, we apply a small fuzz
   * factor to this overflow test and let exp_var() determine the exact
   * overflow threshold so that it is consistent for all inputs.
   *----------
   */
  ln_dweight = estimate_ln_dweight(base);

  /*
   * Set the scale for the low-precision calculation, computing ln(base) to
   * around 8 significant digits.  Note that ln_dweight may be as small as
   * -NUMERIC_DSCALE_MAX, so the scale may exceed NUMERIC_MAX_DISPLAY_SCALE
   * here.
   */
  local_rscale = 8 - ln_dweight;
  local_rscale = Max(local_rscale, NUMERIC_MIN_DISPLAY_SCALE);

  ln_var(base, &ln_base, local_rscale);

  mul_var(&ln_base, exp, &ln_num, local_rscale);

  val = numericvar_to_double_no_overflow(&ln_num);

  /* initial overflow/underflow test with fuzz factor */
  if (fabs(val) > NUMERIC_MAX_RESULT_SCALE * 3.01)
  {
    if (val > 0)
    {
      elog(ERROR, "value overflows numeric format");
      return;
    }
    zero_var(result);
    result->dscale = NUMERIC_MAX_DISPLAY_SCALE;
    return;
  }

  val *= 0.434294481903252;  /* approximate decimal result weight */

  /* choose the result scale */
  rscale = NUMERIC_MIN_SIG_DIGITS - (int) val;
  rscale = Max(rscale, base->dscale);
  rscale = Max(rscale, exp->dscale);
  rscale = Max(rscale, NUMERIC_MIN_DISPLAY_SCALE);
  rscale = Min(rscale, NUMERIC_MAX_DISPLAY_SCALE);

  /* significant digits required in the result */
  sig_digits = rscale + (int) val;
  sig_digits = Max(sig_digits, 0);

  /* set the scale for the real exp * ln(base) calculation */
  local_rscale = sig_digits - ln_dweight + 8;
  local_rscale = Max(local_rscale, NUMERIC_MIN_DISPLAY_SCALE);

  /* and do the real calculation */

  ln_var(base, &ln_base, local_rscale);

  mul_var(&ln_base, exp, &ln_num, local_rscale);

  exp_var(&ln_num, result, rscale);

  if (res_sign == NUMERIC_NEG && result->ndigits > 0)
    result->sign = NUMERIC_NEG;

  free_var(&ln_num);
  free_var(&ln_base);
  free_var(&abs_base);
}

/*
 * power_var_int() -
 *
 *  Raise base to the power of exp, where exp is an integer.
 *
 *  Note: this routine chooses dscale of the result.
 */
static void
power_var_int(const NumericVar *base, int exp, int exp_dscale,
        NumericVar *result)
{
  double    f;
  int      p;
  int      i;
  int      rscale;
  int      sig_digits;
  unsigned int mask;
  bool    neg;
  NumericVar  base_prod;
  int      local_rscale;

  /*
   * Choose the result scale.  For this we need an estimate of the decimal
   * weight of the result, which we obtain by approximating using double
   * precision arithmetic.
   *
   * We also perform crude overflow/underflow tests here so that we can exit
   * early if the result is sure to overflow/underflow, and to guard against
   * integer overflow when choosing the result scale.
   */
  if (base->ndigits != 0)
  {
    /*----------
     * Choose f (double) and p (int) such that base ~= f * 10^p.
     * Then log10(result) = log10(base^exp) ~= exp * (log10(f) + p).
     *----------
     */
    f = base->digits[0];
    p = base->weight * DEC_DIGITS;

    for (i = 1; i < base->ndigits && i * DEC_DIGITS < 16; i++)
    {
      f = f * NBASE + base->digits[i];
      p -= DEC_DIGITS;
    }

    f = exp * (log10(f) + p);  /* approximate decimal result weight */
  }
  else
    f = 0;          /* result is 0 or 1 (weight 0), or error */

  /* overflow/underflow tests with fuzz factors */
  if (f > (NUMERIC_WEIGHT_MAX + 1) * DEC_DIGITS)
  {
    elog(ERROR, "value overflows numeric format");
    return;
  }
  if (f + 1 < -NUMERIC_MAX_DISPLAY_SCALE)
  {
    zero_var(result);
    result->dscale = NUMERIC_MAX_DISPLAY_SCALE;
    return;
  }

  /*
   * Choose the result scale in the same way as power_var(), so it has at
   * least NUMERIC_MIN_SIG_DIGITS significant digits and is not less than
   * either input's display scale.
   */
  rscale = NUMERIC_MIN_SIG_DIGITS - (int) f;
  rscale = Max(rscale, base->dscale);
  rscale = Max(rscale, exp_dscale);
  rscale = Max(rscale, NUMERIC_MIN_DISPLAY_SCALE);
  rscale = Min(rscale, NUMERIC_MAX_DISPLAY_SCALE);

  /* Handle some common special cases, as well as corner cases */
  switch (exp)
  {
    case 0:

      /*
       * While 0 ^ 0 can be either 1 or indeterminate (error), we treat
       * it as 1 because most programming languages do this. SQL:2003
       * also requires a return value of 1.
       * https://en.wikipedia.org/wiki/Exponentiation#Zero_to_the_zero_power
       */
      set_var_from_var(&const_one, result);
      result->dscale = rscale;  /* no need to round */
      return;
    case 1:
      set_var_from_var(base, result);
      round_var(result, rscale);
      return;
    case -1:
      div_var(&const_one, base, result, rscale, true, true);
      return;
    case 2:
      mul_var(base, base, result, rscale);
      return;
    default:
      break;
  }

  /* Handle the special case where the base is zero */
  if (base->ndigits == 0)
  {
    if (exp < 0)
    {
      elog(ERROR, "division by zero");
      return;
    }
    zero_var(result);
    result->dscale = rscale;
    return;
  }

  /*
   * The general case repeatedly multiplies base according to the bit
   * pattern of exp.
   *
   * The local rscale used for each multiplication is varied to keep a fixed
   * number of significant digits, sufficient to give the required result
   * scale.
   */

  /*
   * Approximate number of significant digits in the result.  Note that the
   * underflow test above, together with the choice of rscale, ensures that
   * this approximation is necessarily > 0.
   */
  sig_digits = 1 + rscale + (int) f;

  /*
   * The multiplications to produce the result may introduce an error of up
   * to around log10(abs(exp)) digits, so work with this many extra digits
   * of precision (plus a few more for good measure).
   */
  sig_digits += (int) log(fabs((double) exp)) + 8;

  /*
   * Now we can proceed with the multiplications.
   */
  neg = (exp < 0);
  // mask = pg_abs_s32(exp);
  mask = abs(exp);

  init_var(&base_prod);
  set_var_from_var(base, &base_prod);

  if (mask & 1)
    set_var_from_var(base, result);
  else
    set_var_from_var(&const_one, result);

  while ((mask >>= 1) > 0)
  {
    /*
     * Do the multiplications using rscales large enough to hold the
     * results to the required number of significant digits, but don't
     * waste time by exceeding the scales of the numbers themselves.
     */
    local_rscale = sig_digits - 2 * base_prod.weight * DEC_DIGITS;
    local_rscale = Min(local_rscale, 2 * base_prod.dscale);
    local_rscale = Max(local_rscale, NUMERIC_MIN_DISPLAY_SCALE);

    mul_var(&base_prod, &base_prod, &base_prod, local_rscale);

    if (mask & 1)
    {
      local_rscale = sig_digits -
        (base_prod.weight + result->weight) * DEC_DIGITS;
      local_rscale = Min(local_rscale,
                 base_prod.dscale + result->dscale);
      local_rscale = Max(local_rscale, NUMERIC_MIN_DISPLAY_SCALE);

      mul_var(&base_prod, result, result, local_rscale);
    }

    /*
     * When abs(base) > 1, the number of digits to the left of the decimal
     * point in base_prod doubles at each iteration, so if exp is large we
     * could easily spend large amounts of time and memory space doing the
     * multiplications.  But once the weight exceeds what will fit in
     * int16, the final result is guaranteed to overflow (or underflow, if
     * exp < 0), so we can give up before wasting too many cycles.
     */
    if (base_prod.weight > NUMERIC_WEIGHT_MAX ||
      result->weight > NUMERIC_WEIGHT_MAX)
    {
      /* overflow, unless neg, in which case result should be 0 */
      if (!neg)
      {
        elog(ERROR, "value overflows numeric format");
        return;
      }
      zero_var(result);
      neg = false;
      break;
    }
  }

  free_var(&base_prod);

  /* Compensate for input sign, and round to requested rscale */
  if (neg)
    div_var(&const_one, result, result, rscale, true, false);
  else
    round_var(result, rscale);
}

/*
 * power_ten_int() -
 *
 *  Raise ten to the power of exp, where exp is an integer.  Note that unlike
 *  power_var_int(), this does no overflow/underflow checking or rounding.
 */
static void
power_ten_int(int exp, NumericVar *result)
{
  /* Construct the result directly, starting from 10^0 = 1 */
  set_var_from_var(&const_one, result);

  /* Scale needed to represent the result exactly */
  result->dscale = exp < 0 ? -exp : 0;

  /* Base-NBASE weight of result and remaining exponent */
  if (exp >= 0)
    result->weight = exp / DEC_DIGITS;
  else
    result->weight = (exp + 1) / DEC_DIGITS - 1;

  exp -= result->weight * DEC_DIGITS;

  /* Final adjustment of the result's single NBASE digit */
  while (exp-- > 0)
    result->digits[0] *= 10;
}

/* ----------------------------------------------------------------------
 *
 * Following are the lowest level functions that operate unsigned
 * on the variable level
 *
 * ----------------------------------------------------------------------
 */

/* ----------
 * cmp_abs() -
 *
 *  Compare the absolute values of var1 and var2
 *  Returns:  -1 for ABS(var1) < ABS(var2)
 *        0  for ABS(var1) == ABS(var2)
 *        1  for ABS(var1) > ABS(var2)
 * ----------
 */
static int
cmp_abs(const NumericVar *var1, const NumericVar *var2)
{
  return cmp_abs_common(var1->digits, var1->ndigits, var1->weight,
              var2->digits, var2->ndigits, var2->weight);
}

/* ----------
 * cmp_abs_common() -
 *
 *  Main routine of cmp_abs(). This function can be used by both
 *  NumericVar and Numeric.
 * ----------
 */
static int
cmp_abs_common(const NumericDigit *var1digits, int var1ndigits, int var1weight,
         const NumericDigit *var2digits, int var2ndigits, int var2weight)
{
  int      i1 = 0;
  int      i2 = 0;

  /* Check any digits before the first common digit */

  while (var1weight > var2weight && i1 < var1ndigits)
  {
    if (var1digits[i1++] != 0)
      return 1;
    var1weight--;
  }
  while (var2weight > var1weight && i2 < var2ndigits)
  {
    if (var2digits[i2++] != 0)
      return -1;
    var2weight--;
  }

  /* At this point, either w1 == w2 or we've run out of digits */

  if (var1weight == var2weight)
  {
    while (i1 < var1ndigits && i2 < var2ndigits)
    {
      int      stat = var1digits[i1++] - var2digits[i2++];

      if (stat)
      {
        if (stat > 0)
          return 1;
        return -1;
      }
    }
  }

  /*
   * At this point, we've run out of digits on one side or the other; so any
   * remaining nonzero digits imply that side is larger
   */
  while (i1 < var1ndigits)
  {
    if (var1digits[i1++] != 0)
      return 1;
  }
  while (i2 < var2ndigits)
  {
    if (var2digits[i2++] != 0)
      return -1;
  }

  return 0;
}

/*
 * add_abs() -
 *
 *  Add the absolute values of two variables into result.
 *  result might point to one of the operands without danger.
 */
static void
add_abs(const NumericVar *var1, const NumericVar *var2, NumericVar *result)
{
  NumericDigit *res_buf;
  NumericDigit *res_digits;
  int      res_ndigits;
  int      res_weight;
  int      res_rscale,
        rscale1,
        rscale2;
  int      res_dscale;
  int      i,
        i1,
        i2;
  int      carry = 0;

  /* copy these values into local vars for speed in inner loop */
  int      var1ndigits = var1->ndigits;
  int      var2ndigits = var2->ndigits;
  NumericDigit *var1digits = var1->digits;
  NumericDigit *var2digits = var2->digits;

  res_weight = Max(var1->weight, var2->weight) + 1;

  res_dscale = Max(var1->dscale, var2->dscale);

  /* Note: here we are figuring rscale in base-NBASE digits */
  rscale1 = var1->ndigits - var1->weight - 1;
  rscale2 = var2->ndigits - var2->weight - 1;
  res_rscale = Max(rscale1, rscale2);

  res_ndigits = res_rscale + res_weight + 1;
  if (res_ndigits <= 0)
    res_ndigits = 1;

  res_buf = digitbuf_alloc(res_ndigits + 1);
  res_buf[0] = 0;        /* spare digit for later rounding */
  res_digits = res_buf + 1;

  i1 = res_rscale + var1->weight + 1;
  i2 = res_rscale + var2->weight + 1;
  for (i = res_ndigits - 1; i >= 0; i--)
  {
    i1--;
    i2--;
    if (i1 >= 0 && i1 < var1ndigits)
      carry += var1digits[i1];
    if (i2 >= 0 && i2 < var2ndigits)
      carry += var2digits[i2];

    if (carry >= NBASE)
    {
      res_digits[i] = carry - NBASE;
      carry = 1;
    }
    else
    {
      res_digits[i] = carry;
      carry = 0;
    }
  }

  Assert(carry == 0);      /* else we failed to allow for carry out */

  digitbuf_free(result->buf);
  result->ndigits = res_ndigits;
  result->buf = res_buf;
  result->digits = res_digits;
  result->weight = res_weight;
  result->dscale = res_dscale;

  /* Remove leading/trailing zeroes */
  strip_var(result);
}

/*
 * sub_abs()
 *
 *  Subtract the absolute value of var2 from the absolute value of var1
 *  and store in result. result might point to one of the operands
 *  without danger.
 *
 *  ABS(var1) MUST BE GREATER OR EQUAL ABS(var2) !!!
 */
static void
sub_abs(const NumericVar *var1, const NumericVar *var2, NumericVar *result)
{
  NumericDigit *res_buf;
  NumericDigit *res_digits;
  int      res_ndigits;
  int      res_weight;
  int      res_rscale,
        rscale1,
        rscale2;
  int      res_dscale;
  int      i,
        i1,
        i2;
  int      borrow = 0;

  /* copy these values into local vars for speed in inner loop */
  int      var1ndigits = var1->ndigits;
  int      var2ndigits = var2->ndigits;
  NumericDigit *var1digits = var1->digits;
  NumericDigit *var2digits = var2->digits;

  res_weight = var1->weight;

  res_dscale = Max(var1->dscale, var2->dscale);

  /* Note: here we are figuring rscale in base-NBASE digits */
  rscale1 = var1->ndigits - var1->weight - 1;
  rscale2 = var2->ndigits - var2->weight - 1;
  res_rscale = Max(rscale1, rscale2);

  res_ndigits = res_rscale + res_weight + 1;
  if (res_ndigits <= 0)
    res_ndigits = 1;

  res_buf = digitbuf_alloc(res_ndigits + 1);
  res_buf[0] = 0;        /* spare digit for later rounding */
  res_digits = res_buf + 1;

  i1 = res_rscale + var1->weight + 1;
  i2 = res_rscale + var2->weight + 1;
  for (i = res_ndigits - 1; i >= 0; i--)
  {
    i1--;
    i2--;
    if (i1 >= 0 && i1 < var1ndigits)
      borrow += var1digits[i1];
    if (i2 >= 0 && i2 < var2ndigits)
      borrow -= var2digits[i2];

    if (borrow < 0)
    {
      res_digits[i] = borrow + NBASE;
      borrow = -1;
    }
    else
    {
      res_digits[i] = borrow;
      borrow = 0;
    }
  }

  Assert(borrow == 0);    /* else caller gave us var1 < var2 */

  digitbuf_free(result->buf);
  result->ndigits = res_ndigits;
  result->buf = res_buf;
  result->digits = res_digits;
  result->weight = res_weight;
  result->dscale = res_dscale;

  /* Remove leading/trailing zeroes */
  strip_var(result);
}

/*
 * round_var
 *
 * Round the value of a variable to no more than rscale decimal digits
 * after the decimal point.  NOTE: we allow rscale < 0 here, implying
 * rounding before the decimal point.
 */
static void
round_var(NumericVar *var, int rscale)
{
  NumericDigit *digits = var->digits;
  int      di;
  int      ndigits;
  int      carry;

  var->dscale = rscale;

  /* decimal digits wanted */
  di = (var->weight + 1) * DEC_DIGITS + rscale;

  /*
   * If di = 0, the value loses all digits, but could round up to 1 if its
   * first extra digit is >= 5.  If di < 0 the result must be 0.
   */
  if (di < 0)
  {
    var->ndigits = 0;
    var->weight = 0;
    var->sign = NUMERIC_POS;
  }
  else
  {
    /* NBASE digits wanted */
    ndigits = (di + DEC_DIGITS - 1) / DEC_DIGITS;

    /* 0, or number of decimal digits to keep in last NBASE digit */
    di %= DEC_DIGITS;

    if (ndigits < var->ndigits ||
      (ndigits == var->ndigits && di > 0))
    {
      var->ndigits = ndigits;

#if DEC_DIGITS == 1
      /* di must be zero */
      carry = (digits[ndigits] >= HALF_NBASE) ? 1 : 0;
#else
      if (di == 0)
        carry = (digits[ndigits] >= HALF_NBASE) ? 1 : 0;
      else
      {
        /* Must round within last NBASE digit */
        int      extra,
              pow10;

#if DEC_DIGITS == 4
        pow10 = round_powers[di];
#elif DEC_DIGITS == 2
        pow10 = 10;
#else
#error unsupported NBASE
#endif
        extra = digits[--ndigits] % pow10;
        digits[ndigits] -= extra;
        carry = 0;
        if (extra >= pow10 / 2)
        {
          pow10 += digits[ndigits];
          if (pow10 >= NBASE)
          {
            pow10 -= NBASE;
            carry = 1;
          }
          digits[ndigits] = pow10;
        }
      }
#endif

      /* Propagate carry if needed */
      while (carry)
      {
        carry += digits[--ndigits];
        if (carry >= NBASE)
        {
          digits[ndigits] = carry - NBASE;
          carry = 1;
        }
        else
        {
          digits[ndigits] = carry;
          carry = 0;
        }
      }

      if (ndigits < 0)
      {
        Assert(ndigits == -1);  /* better not have added > 1 digit */
        Assert(var->digits > var->buf);
        var->digits--;
        var->ndigits++;
        var->weight++;
      }
    }
  }
}

/*
 * trunc_var
 *
 * Truncate (towards zero) the value of a variable at rscale decimal digits
 * after the decimal point.  NOTE: we allow rscale < 0 here, implying
 * truncation before the decimal point.
 */
static void
trunc_var(NumericVar *var, int rscale)
{
  int      di;
  int      ndigits;

  var->dscale = rscale;

  /* decimal digits wanted */
  di = (var->weight + 1) * DEC_DIGITS + rscale;

  /*
   * If di <= 0, the value loses all digits.
   */
  if (di <= 0)
  {
    var->ndigits = 0;
    var->weight = 0;
    var->sign = NUMERIC_POS;
  }
  else
  {
    /* NBASE digits wanted */
    ndigits = (di + DEC_DIGITS - 1) / DEC_DIGITS;

    if (ndigits <= var->ndigits)
    {
      var->ndigits = ndigits;

#if DEC_DIGITS == 1
      /* no within-digit stuff to worry about */
#else
      /* 0, or number of decimal digits to keep in last NBASE digit */
      di %= DEC_DIGITS;

      if (di > 0)
      {
        /* Must truncate within last NBASE digit */
        NumericDigit *digits = var->digits;
        int      extra,
              pow10;

#if DEC_DIGITS == 4
        pow10 = round_powers[di];
#elif DEC_DIGITS == 2
        pow10 = 10;
#else
#error unsupported NBASE
#endif
        extra = digits[--ndigits] % pow10;
        digits[ndigits] -= extra;
      }
#endif
    }
  }
}

/*
 * strip_var
 *
 * Strip any leading and trailing zeroes from a numeric variable
 */
static void
strip_var(NumericVar *var)
{
  NumericDigit *digits = var->digits;
  int      ndigits = var->ndigits;

  /* Strip leading zeroes */
  while (ndigits > 0 && *digits == 0)
  {
    digits++;
    var->weight--;
    ndigits--;
  }

  /* Strip trailing zeroes */
  while (ndigits > 0 && digits[ndigits - 1] == 0)
    ndigits--;

  /* If it's zero, normalize the sign and weight */
  if (ndigits == 0)
  {
    var->sign = NUMERIC_POS;
    var->weight = 0;
  }

  var->digits = digits;
  var->ndigits = ndigits;
}

/*****************************************************************************/

