/*-------------------------------------------------------------------------
 *
 * datetime.c
 *	  Support functions for date/time types.
 *
 * Portions Copyright (c) 1996-2021, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/backend/utils/adt/datetime.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

// MobilityDB
// #include "datatype/timestamp.h"
#include "utils/timestamp_def.h"
#include "utils/datetime.h"
#include "utils/date.h"

static const datetkn *datebsearch(const char *key, const datetkn *base, int nel);

/* Definitions from miscadmin.h */

/* valid DateOrder values */
#define DATEORDER_YMD			0
#define DATEORDER_DMY			1
#define DATEORDER_MDY			2

/* valid DateStyle values */
#define USE_POSTGRES_DATES		0
#define USE_ISO_DATES			1
#define USE_SQL_DATES			2
#define USE_GERMAN_DATES		3
#define USE_XSD_DATES			4

#define MAXTZLEN		10		/* max TZ name len, not counting tr. null */

/* Defined below */

extern char *pg_ultostr_zeropad(char *str, uint32 value, int32 minwidth);
extern char *pg_ultostr(char *str, uint32 value);
static int	DecodeDate(char *str, int fmask, int *tmask, bool *is2digits,
					   struct pg_tm *tm);
static int	DecodeTime(char *str, int fmask, int range,
					   int *tmask, struct pg_tm *tm, fsec_t *fsec);
static int	DecodeNumber(int flen, char *field, bool haveTextMonth,
						 int fmask, int *tmask,
						 struct pg_tm *tm, fsec_t *fsec, bool *is2digits);
static int	DecodeNumberField(int len, char *str,
							  int fmask, int *tmask,
							  struct pg_tm *tm, fsec_t *fsec, bool *is2digits);
static pg_tz *FetchDynamicTimeZone(TimeZoneAbbrevTable *tbl, const datetkn *tp);
static bool DetermineTimeZoneAbbrevOffsetInternal(pg_time_t t,
												  const char *abbr, pg_tz *tzp,
												  int *offset, int *isdst);
static int	DetermineTimeZoneOffsetInternal(struct pg_tm *tm, pg_tz *tzp,
											pg_time_t *tp);

const int	day_tab[2][13] =
{
	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0},
	{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0}
};

const char *const months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
"Jul", "Aug", "Sep", "Oct", "Nov", "Dec", NULL};

const char *const days[] = {"Sunday", "Monday", "Tuesday", "Wednesday",
"Thursday", "Friday", "Saturday", NULL};


/*****************************************************************************
 *	 PRIVATE ROUTINES														 *
 *****************************************************************************/

/*
 * datetktbl holds date/time keywords.
 *
 * Note that this table must be strictly alphabetically ordered to allow an
 * O(ln(N)) search algorithm to be used.
 *
 * The token field must be NUL-terminated; we truncate entries to TOKMAXLEN
 * characters to fit.
 *
 * The static table contains no TZ, DTZ, or DYNTZ entries; rather those
 * are loaded from configuration files and stored in zoneabbrevtbl, whose
 * abbrevs[] field has the same format as the static datetktbl.
 */
static const datetkn datetktbl[] = {
	/* token, type, value */
	{EARLY, RESERV, DTK_EARLY}, /* "-infinity" reserved for "early time" */
	{DA_D, ADBC, AD},			/* "ad" for years > 0 */
	{"allballs", RESERV, DTK_ZULU}, /* 00:00:00 */
	{"am", AMPM, AM},
	{"apr", MONTH, 4},
	{"april", MONTH, 4},
	{"at", IGNORE_DTF, 0},		/* "at" (throwaway) */
	{"aug", MONTH, 8},
	{"august", MONTH, 8},
	{DB_C, ADBC, BC},			/* "bc" for years <= 0 */
	{"d", UNITS, DTK_DAY},		/* "day of month" for ISO input */
	{"dec", MONTH, 12},
	{"december", MONTH, 12},
	{"dow", UNITS, DTK_DOW},	/* day of week */
	{"doy", UNITS, DTK_DOY},	/* day of year */
	{"dst", DTZMOD, SECS_PER_HOUR},
	{EPOCH, RESERV, DTK_EPOCH}, /* "epoch" reserved for system epoch time */
	{"feb", MONTH, 2},
	{"february", MONTH, 2},
	{"fri", DOW, 5},
	{"friday", DOW, 5},
	{"h", UNITS, DTK_HOUR},		/* "hour" */
	{LATE, RESERV, DTK_LATE},	/* "infinity" reserved for "late time" */
	{"isodow", UNITS, DTK_ISODOW},	/* ISO day of week, Sunday == 7 */
	{"isoyear", UNITS, DTK_ISOYEAR},	/* year in terms of the ISO week date */
	{"j", UNITS, DTK_JULIAN},
	{"jan", MONTH, 1},
	{"january", MONTH, 1},
	{"jd", UNITS, DTK_JULIAN},
	{"jul", MONTH, 7},
	{"julian", UNITS, DTK_JULIAN},
	{"july", MONTH, 7},
	{"jun", MONTH, 6},
	{"june", MONTH, 6},
	{"m", UNITS, DTK_MONTH},	/* "month" for ISO input */
	{"mar", MONTH, 3},
	{"march", MONTH, 3},
	{"may", MONTH, 5},
	{"mm", UNITS, DTK_MINUTE},	/* "minute" for ISO input */
	{"mon", DOW, 1},
	{"monday", DOW, 1},
	{"nov", MONTH, 11},
	{"november", MONTH, 11},
	{NOW, RESERV, DTK_NOW},		/* current transaction time */
	{"oct", MONTH, 10},
	{"october", MONTH, 10},
	{"on", IGNORE_DTF, 0},		/* "on" (throwaway) */
	{"pm", AMPM, PM},
	{"s", UNITS, DTK_SECOND},	/* "seconds" for ISO input */
	{"sat", DOW, 6},
	{"saturday", DOW, 6},
	{"sep", MONTH, 9},
	{"sept", MONTH, 9},
	{"september", MONTH, 9},
	{"sun", DOW, 0},
	{"sunday", DOW, 0},
	{"t", ISOTIME, DTK_TIME},	/* Filler for ISO time fields */
	{"thu", DOW, 4},
	{"thur", DOW, 4},
	{"thurs", DOW, 4},
	{"thursday", DOW, 4},
	{TODAY, RESERV, DTK_TODAY}, /* midnight */
	{TOMORROW, RESERV, DTK_TOMORROW},	/* tomorrow midnight */
	{"tue", DOW, 2},
	{"tues", DOW, 2},
	{"tuesday", DOW, 2},
	{"wed", DOW, 3},
	{"wednesday", DOW, 3},
	{"weds", DOW, 3},
	{"y", UNITS, DTK_YEAR},		/* "year" for ISO input */
	{YESTERDAY, RESERV, DTK_YESTERDAY}	/* yesterday midnight */
};

static const int szdatetktbl = sizeof datetktbl / sizeof datetktbl[0];

static TimeZoneAbbrevTable *zoneabbrevtbl = NULL;

/* Caches of recent lookup results in the above tables */

static const datetkn *datecache[MAXDATEFIELDS] = {NULL};

static const datetkn *abbrevcache[MAXDATEFIELDS] = {NULL};

int
date2j(int y, int m, int d)
{
	int			julian;
	int			century;

	if (m > 2)
	{
		m += 1;
		y += 4800;
	}
	else
	{
		m += 13;
		y += 4799;
	}

	century = y / 100;
	julian = y * 365 - 32167;
	julian += y / 4 - century + century / 4;
	julian += 7834 * m / 256 + d;

	return julian;
}								/* date2j() */

void
j2date(int jd, int *year, int *month, int *day)
{
	unsigned int julian;
	unsigned int quad;
	unsigned int extra;
	int			y;

	julian = jd;
	julian += 32044;
	quad = julian / 146097;
	extra = (julian - quad * 146097) * 4 + 3;
	julian += 60 + quad * 3 + extra / 146097;
	quad = julian / 1461;
	julian -= quad * 1461;
	y = julian * 4 / 1461;
	julian = ((y != 0) ? ((julian + 305) % 365) : ((julian + 306) % 366))
		+ 123;
	y += quad * 4;
	*year = y - 4800;
	quad = julian * 2141 / 65536;
	*day = julian - 7834 * quad / 256;
	*month = (quad + 10) % MONTHS_PER_YEAR + 1;
}								/* j2date() */

/*
 * j2day - convert Julian date to day-of-week (0..6 == Sun..Sat)
 *
 * Note: various places use the locution j2day(date - 1) to produce a
 * result according to the convention 0..6 = Mon..Sun.  This is a bit of
 * a crock, but will work as long as the computation here is just a modulo.
 */
int
j2day(int date)
{
	date += 1;
	date %= 7;
	/* Cope if division truncates towards zero, as it probably does */
	if (date < 0)
		date += 7;

	return date;
}								/* j2day() */

/*
 * GetCurrentDateTime()
 *
 * Get the transaction start time ("now()") broken down as a struct pg_tm,
 * converted according to the session timezone setting.
 *
 * This is just a convenience wrapper for GetCurrentTimeUsec, to cover the
 * case where caller doesn't need either fractional seconds or tz offset.
 */
void
GetCurrentDateTime(struct pg_tm *tm)
{
	fsec_t		fsec;

	GetCurrentTimeUsec(tm, &fsec, NULL);
}

/*
 * GetCurrentTimeUsec()
 *
 * Get the transaction start time ("now()") broken down as a struct pg_tm,
 * including fractional seconds and timezone offset.  The time is converted
 * according to the session timezone setting.
 *
 * Callers may pass tzp = NULL if they don't need the offset, but this does
 * not affect the conversion behavior (unlike timestamp2tm()).
 *
 * Internally, we cache the result, since this could be called many times
 * in a transaction, within which now() doesn't change.
 */
void
GetCurrentTimeUsec(struct pg_tm *tm, fsec_t *fsec, int *tzp)
{
  // MobilityDB
	// TimestampTz cur_ts = GetCurrentTransactionStartTimestamp();
	TimestampTz cur_ts = GetCurrentTimestamp();

	/*
	 * The cache key must include both current time and current timezone.  By
	 * representing the timezone by just a pointer, we're assuming that
	 * distinct timezone settings could never have the same pointer value.
	 * This is true by virtue of the hashtable used inside pg_tzset();
	 * however, it might need another look if we ever allow entries in that
	 * hash to be recycled.
	 */
	static TimestampTz cache_ts = 0;
	static pg_tz *cache_timezone = NULL;
	static struct pg_tm cache_tm;
	static fsec_t cache_fsec;
	static int	cache_tz;

	if (cur_ts != cache_ts || session_timezone != cache_timezone)
	{
		/*
		 * Make sure cache is marked invalid in case of error after partial
		 * update within timestamp2tm.
		 */
		cache_timezone = NULL;

		/*
		 * Perform the computation, storing results into cache.  We do not
		 * really expect any error here, since current time surely ought to be
		 * within range, but check just for sanity's sake.
		 */
		if (timestamp2tm(cur_ts, &cache_tz, &cache_tm, &cache_fsec,
						 NULL, session_timezone) != 0)
			elog(ERROR, "timestamp out of range");

		/* OK, so mark the cache valid. */
		cache_ts = cur_ts;
		cache_timezone = session_timezone;
	}

	*tm = cache_tm;
	*fsec = cache_fsec;
	if (tzp != NULL)
		*tzp = cache_tz;
}

/*
 * Append seconds and fractional seconds (if any) at *cp.
 *
 * precision is the max number of fraction digits, fillzeros says to
 * pad to two integral-seconds digits.
 *
 * Returns a pointer to the new end of string.  No NUL terminator is put
 * there; callers are responsible for NUL terminating str themselves.
 *
 * Note that any sign is stripped from the input seconds values.
 */
static char *
AppendSeconds(char *cp, int sec, fsec_t fsec, int precision, bool fillzeros)
{
	Assert(precision >= 0);

	if (fillzeros)
		cp = pg_ultostr_zeropad(cp, Abs(sec), 2);
	else
		cp = pg_ultostr(cp, Abs(sec));

	/* fsec_t is just an int32 */
	if (fsec != 0)
	{
		int32		value = Abs(fsec);
		char	   *end = &cp[precision + 1];
		bool		gotnonzero = false;

		*cp++ = '.';

		/*
		 * Append the fractional seconds part.  Note that we don't want any
		 * trailing zeros here, so since we're building the number in reverse
		 * we'll skip appending zeros until we've output a non-zero digit.
		 */
		while (precision--)
		{
			int32		oldval = value;
			int32		remainder;

			value /= 10;
			remainder = oldval - value * 10;

			/* check if we got a non-zero */
			if (remainder)
				gotnonzero = true;

			if (gotnonzero)
				cp[precision] = '0' + remainder;
			else
				end = &cp[precision];
		}

		/*
		 * If we still have a non-zero value then precision must have not been
		 * enough to print the number.  We punt the problem to pg_ltostr(),
		 * which will generate a correct answer in the minimum valid width.
		 */
		if (value)
			return pg_ultostr(cp, Abs(fsec));

		return end;
	}
	else
		return cp;
}

/*
 * Variant of above that's specialized to timestamp case.
 *
 * Returns a pointer to the new end of string.  No NUL terminator is put
 * there; callers are responsible for NUL terminating str themselves.
 */
static char *
AppendTimestampSeconds(char *cp, struct pg_tm *tm, fsec_t fsec)
{
	return AppendSeconds(cp, tm->tm_sec, fsec, MAX_TIMESTAMP_PRECISION, true);
}

/* Fetch a fractional-second value with suitable error checking */
static int
ParseFractionalSecond(char *cp, fsec_t *fsec)
{
	double		frac;

	/* Caller should always pass the start of the fraction part */
	Assert(*cp == '.');
	errno = 0;
	frac = strtod(cp, &cp);
	/* check for parse failure */
	if (*cp != '\0' || errno != 0)
		return DTERR_BAD_FORMAT;
	*fsec = rint(frac * 1000000);
	return 0;
}

/* ParseDateTime()
 *	Break string into tokens based on a date/time context.
 *	Returns 0 if successful, DTERR code if bogus input detected.
 *
 * timestr - the input string
 * workbuf - workspace for field string storage. This must be
 *	 larger than the largest legal input for this datetime type --
 *	 some additional space will be needed to NUL terminate fields.
 * buflen - the size of workbuf
 * field[] - pointers to field strings are returned in this array
 * ftype[] - field type indicators are returned in this array
 * maxfields - dimensions of the above two arrays
 * *numfields - set to the actual number of fields detected
 *
 * The fields extracted from the input are stored as separate,
 * null-terminated strings in the workspace at workbuf. Any text is
 * converted to lower case.
 *
 * Several field types are assigned:
 *	DTK_NUMBER - digits and (possibly) a decimal point
 *	DTK_DATE - digits and two delimiters, or digits and text
 *	DTK_TIME - digits, colon delimiters, and possibly a decimal point
 *	DTK_STRING - text (no digits or punctuation)
 *	DTK_SPECIAL - leading "+" or "-" followed by text
 *	DTK_TZ - leading "+" or "-" followed by digits (also eats ':', '.', '-')
 *
 * Note that some field types can hold unexpected items:
 *	DTK_NUMBER can hold date fields (yy.ddd)
 *	DTK_STRING can hold months (January) and time zones (PST)
 *	DTK_DATE can hold time zone names (America/New_York, GMT-8)
 */
int
ParseDateTime(const char *timestr, char *workbuf, size_t buflen,
			  char **field, int *ftype, int maxfields, int *numfields)
{
	int			nf = 0;
	const char *cp = timestr;
	char	   *bufp = workbuf;
	const char *bufend = workbuf + buflen;

	/*
	 * Set the character pointed-to by "bufptr" to "newchar", and increment
	 * "bufptr". "end" gives the end of the buffer -- we return an error if
	 * there is no space left to append a character to the buffer. Note that
	 * "bufptr" is evaluated twice.
	 */
#define APPEND_CHAR(bufptr, end, newchar)		\
	do											\
	{											\
		if (((bufptr) + 1) >= (end))			\
			return DTERR_BAD_FORMAT;			\
		*(bufptr)++ = newchar;					\
	} while (0)

	/* outer loop through fields */
	while (*cp != '\0')
	{
		/* Ignore spaces between fields */
		if (isspace((unsigned char) *cp))
		{
			cp++;
			continue;
		}

		/* Record start of current field */
		if (nf >= maxfields)
			return DTERR_BAD_FORMAT;
		field[nf] = bufp;

		/* leading digit? then date or time */
		if (isdigit((unsigned char) *cp))
		{
			APPEND_CHAR(bufp, bufend, *cp++);
			while (isdigit((unsigned char) *cp))
				APPEND_CHAR(bufp, bufend, *cp++);

			/* time field? */
			if (*cp == ':')
			{
				ftype[nf] = DTK_TIME;
				APPEND_CHAR(bufp, bufend, *cp++);
				while (isdigit((unsigned char) *cp) ||
					   (*cp == ':') || (*cp == '.'))
					APPEND_CHAR(bufp, bufend, *cp++);
			}
			/* date field? allow embedded text month */
			else if (*cp == '-' || *cp == '/' || *cp == '.')
			{
				/* save delimiting character to use later */
				char		delim = *cp;

				APPEND_CHAR(bufp, bufend, *cp++);
				/* second field is all digits? then no embedded text month */
				if (isdigit((unsigned char) *cp))
				{
					ftype[nf] = ((delim == '.') ? DTK_NUMBER : DTK_DATE);
					while (isdigit((unsigned char) *cp))
						APPEND_CHAR(bufp, bufend, *cp++);

					/*
					 * insist that the delimiters match to get a three-field
					 * date.
					 */
					if (*cp == delim)
					{
						ftype[nf] = DTK_DATE;
						APPEND_CHAR(bufp, bufend, *cp++);
						while (isdigit((unsigned char) *cp) || *cp == delim)
							APPEND_CHAR(bufp, bufend, *cp++);
					}
				}
				else
				{
					ftype[nf] = DTK_DATE;
					while (isalnum((unsigned char) *cp) || *cp == delim)
						APPEND_CHAR(bufp, bufend, pg_tolower((unsigned char) *cp++));
				}
			}

			/*
			 * otherwise, number only and will determine year, month, day, or
			 * concatenated fields later...
			 */
			else
				ftype[nf] = DTK_NUMBER;
		}
		/* Leading decimal point? Then fractional seconds... */
		else if (*cp == '.')
		{
			APPEND_CHAR(bufp, bufend, *cp++);
			while (isdigit((unsigned char) *cp))
				APPEND_CHAR(bufp, bufend, *cp++);

			ftype[nf] = DTK_NUMBER;
		}

		/*
		 * text? then date string, month, day of week, special, or timezone
		 */
		else if (isalpha((unsigned char) *cp))
		{
			bool		is_date;

			ftype[nf] = DTK_STRING;
			APPEND_CHAR(bufp, bufend, pg_tolower((unsigned char) *cp++));
			while (isalpha((unsigned char) *cp))
				APPEND_CHAR(bufp, bufend, pg_tolower((unsigned char) *cp++));

			/*
			 * Dates can have embedded '-', '/', or '.' separators.  It could
			 * also be a timezone name containing embedded '/', '+', '-', '_',
			 * or ':' (but '_' or ':' can't be the first punctuation). If the
			 * next character is a digit or '+', we need to check whether what
			 * we have so far is a recognized non-timezone keyword --- if so,
			 * don't believe that this is the start of a timezone.
			 */
			is_date = false;
			if (*cp == '-' || *cp == '/' || *cp == '.')
				is_date = true;
			else if (*cp == '+' || isdigit((unsigned char) *cp))
			{
				*bufp = '\0';	/* null-terminate current field value */
				/* we need search only the core token table, not TZ names */
				if (datebsearch(field[nf], datetktbl, szdatetktbl) == NULL)
					is_date = true;
			}
			if (is_date)
			{
				ftype[nf] = DTK_DATE;
				do
				{
					APPEND_CHAR(bufp, bufend, pg_tolower((unsigned char) *cp++));
				} while (*cp == '+' || *cp == '-' ||
						 *cp == '/' || *cp == '_' ||
						 *cp == '.' || *cp == ':' ||
						 isalnum((unsigned char) *cp));
			}
		}
		/* sign? then special or numeric timezone */
		else if (*cp == '+' || *cp == '-')
		{
			APPEND_CHAR(bufp, bufend, *cp++);
			/* soak up leading whitespace */
			while (isspace((unsigned char) *cp))
				cp++;
			/* numeric timezone? */
			/* note that "DTK_TZ" could also be a signed float or yyyy-mm */
			if (isdigit((unsigned char) *cp))
			{
				ftype[nf] = DTK_TZ;
				APPEND_CHAR(bufp, bufend, *cp++);
				while (isdigit((unsigned char) *cp) ||
					   *cp == ':' || *cp == '.' || *cp == '-')
					APPEND_CHAR(bufp, bufend, *cp++);
			}
			/* special? */
			else if (isalpha((unsigned char) *cp))
			{
				ftype[nf] = DTK_SPECIAL;
				APPEND_CHAR(bufp, bufend, pg_tolower((unsigned char) *cp++));
				while (isalpha((unsigned char) *cp))
					APPEND_CHAR(bufp, bufend, pg_tolower((unsigned char) *cp++));
			}
			/* otherwise something wrong... */
			else
				return DTERR_BAD_FORMAT;
		}
		/* ignore other punctuation but use as delimiter */
		else if (ispunct((unsigned char) *cp))
		{
			cp++;
			continue;
		}
		/* otherwise, something is not right... */
		else
			return DTERR_BAD_FORMAT;

		/* force in a delimiter after each field */
		*bufp++ = '\0';
		nf++;
	}

	*numfields = nf;

	return 0;
}

/*
 * strtoint --- just like strtol, but returns int not long
 * MobilityDB: Function copied from string.c
 */
int
strtoint(const char *pg_restrict str, char **pg_restrict endptr, int base)
{
	long		val;

	val = strtol(str, endptr, base);
	if (val != (int) val)
		errno = ERANGE;
	return (int) val;
}

/* DecodeDateTime()
 * Interpret previously parsed fields for general date and time.
 * Return 0 if full date, 1 if only time, and negative DTERR code if problems.
 * (Currently, all callers treat 1 as an error return too.)
 *
 *		External format(s):
 *				"<weekday> <month>-<day>-<year> <hour>:<minute>:<second>"
 *				"Fri Feb-7-1997 15:23:27"
 *				"Feb-7-1997 15:23:27"
 *				"2-7-1997 15:23:27"
 *				"1997-2-7 15:23:27"
 *				"1997.038 15:23:27"		(day of year 1-366)
 *		Also supports input in compact time:
 *				"970207 152327"
 *				"97038 152327"
 *				"20011225T040506.789-07"
 *
 * Use the system-provided functions to get the current time zone
 * if not specified in the input string.
 *
 * If the date is outside the range of pg_time_t (in practice that could only
 * happen if pg_time_t is just 32 bits), then assume UTC time zone - thomas
 * 1997-05-27
 */
int
DecodeDateTime(char **field, int *ftype, int nf,
			   int *dtype, struct pg_tm *tm, fsec_t *fsec, int *tzp)
{
	int			fmask = 0,
				tmask,
				type;
	int			ptype = 0;		/* "prefix type" for ISO y2001m02d04 format */
	int			i;
	int			val;
	int			dterr;
	int			mer = HR24;
	bool		haveTextMonth = false;
	bool		isjulian = false;
	bool		is2digits = false;
	bool		bc = false;
	pg_tz	   *namedTz = NULL;
	pg_tz	   *abbrevTz = NULL;
	pg_tz	   *valtz;
	char	   *abbrev = NULL;
	struct pg_tm cur_tm;

	/*
	 * We'll insist on at least all of the date fields, but initialize the
	 * remaining fields in case they are not set later...
	 */
	*dtype = DTK_DATE;
	tm->tm_hour = 0;
	tm->tm_min = 0;
	tm->tm_sec = 0;
	*fsec = 0;
	/* don't know daylight savings time status apriori */
	tm->tm_isdst = -1;
	if (tzp != NULL)
		*tzp = 0;

	for (i = 0; i < nf; i++)
	{
		switch (ftype[i])
		{
			case DTK_DATE:

				/*
				 * Integral julian day with attached time zone? All other
				 * forms with JD will be separated into distinct fields, so we
				 * handle just this case here.
				 */
				if (ptype == DTK_JULIAN)
				{
					char	   *cp;
					int			val;

					if (tzp == NULL)
						return DTERR_BAD_FORMAT;

					errno = 0;
					val = strtoint(field[i], &cp, 10);
					if (errno == ERANGE || val < 0)
						return DTERR_FIELD_OVERFLOW;

					j2date(val, &tm->tm_year, &tm->tm_mon, &tm->tm_mday);
					isjulian = true;

					/* Get the time zone from the end of the string */
					dterr = DecodeTimezone(cp, tzp);
					if (dterr)
						return dterr;

					tmask = DTK_DATE_M | DTK_TIME_M | DTK_M(TZ);
					ptype = 0;
					break;
				}

				/*
				 * Already have a date? Then this might be a time zone name
				 * with embedded punctuation (e.g. "America/New_York") or a
				 * run-together time with trailing time zone (e.g. hhmmss-zz).
				 * - thomas 2001-12-25
				 *
				 * We consider it a time zone if we already have month & day.
				 * This is to allow the form "mmm dd hhmmss tz year", which
				 * we've historically accepted.
				 */
				else if (ptype != 0 ||
						 ((fmask & (DTK_M(MONTH) | DTK_M(DAY))) ==
						  (DTK_M(MONTH) | DTK_M(DAY))))
				{
					/* No time zone accepted? Then quit... */
					if (tzp == NULL)
						return DTERR_BAD_FORMAT;

					if (isdigit((unsigned char) *field[i]) || ptype != 0)
					{
						char	   *cp;

						if (ptype != 0)
						{
							/* Sanity check; should not fail this test */
							if (ptype != DTK_TIME)
								return DTERR_BAD_FORMAT;
							ptype = 0;
						}

						/*
						 * Starts with a digit but we already have a time
						 * field? Then we are in trouble with a date and time
						 * already...
						 */
						if ((fmask & DTK_TIME_M) == DTK_TIME_M)
							return DTERR_BAD_FORMAT;

						if ((cp = strchr(field[i], '-')) == NULL)
							return DTERR_BAD_FORMAT;

						/* Get the time zone from the end of the string */
						dterr = DecodeTimezone(cp, tzp);
						if (dterr)
							return dterr;
						*cp = '\0';

						/*
						 * Then read the rest of the field as a concatenated
						 * time
						 */
						dterr = DecodeNumberField(strlen(field[i]), field[i],
												  fmask,
												  &tmask, tm,
												  fsec, &is2digits);
						if (dterr < 0)
							return dterr;

						/*
						 * modify tmask after returning from
						 * DecodeNumberField()
						 */
						tmask |= DTK_M(TZ);
					}
					else
					{
						namedTz = pg_tzset(field[i]);
						if (!namedTz)
						{
							/*
							 * We should return an error code instead of
							 * ereport'ing directly, but then there is no way
							 * to report the bad time zone name.
							 */
							elog(ERROR, "time zone \"%s\" not recognized", field[i]);
						}
						/* we'll apply the zone setting below */
						tmask = DTK_M(TZ);
					}
				}
				else
				{
					dterr = DecodeDate(field[i], fmask,
									   &tmask, &is2digits, tm);
					if (dterr)
						return dterr;
				}
				break;

			case DTK_TIME:

				/*
				 * This might be an ISO time following a "t" field.
				 */
				if (ptype != 0)
				{
					/* Sanity check; should not fail this test */
					if (ptype != DTK_TIME)
						return DTERR_BAD_FORMAT;
					ptype = 0;
				}
				dterr = DecodeTime(field[i], fmask, INTERVAL_FULL_RANGE,
								   &tmask, tm, fsec);
				if (dterr)
					return dterr;

				/* check for time overflow */
				if (time_overflows(tm->tm_hour, tm->tm_min, tm->tm_sec,
								   *fsec))
					return DTERR_FIELD_OVERFLOW;
				break;

			case DTK_TZ:
				{
					int			tz;

					if (tzp == NULL)
						return DTERR_BAD_FORMAT;

					dterr = DecodeTimezone(field[i], &tz);
					if (dterr)
						return dterr;
					*tzp = tz;
					tmask = DTK_M(TZ);
				}
				break;

			case DTK_NUMBER:

				/*
				 * Was this an "ISO date" with embedded field labels? An
				 * example is "y2001m02d04" - thomas 2001-02-04
				 */
				if (ptype != 0)
				{
					char	   *cp;
					int			val;

					errno = 0;
					val = strtoint(field[i], &cp, 10);
					if (errno == ERANGE)
						return DTERR_FIELD_OVERFLOW;

					/*
					 * only a few kinds are allowed to have an embedded
					 * decimal
					 */
					if (*cp == '.')
						switch (ptype)
						{
							case DTK_JULIAN:
							case DTK_TIME:
							case DTK_SECOND:
								break;
							default:
								return DTERR_BAD_FORMAT;
								break;
						}
					else if (*cp != '\0')
						return DTERR_BAD_FORMAT;

					switch (ptype)
					{
						case DTK_YEAR:
							tm->tm_year = val;
							tmask = DTK_M(YEAR);
							break;

						case DTK_MONTH:

							/*
							 * already have a month and hour? then assume
							 * minutes
							 */
							if ((fmask & DTK_M(MONTH)) != 0 &&
								(fmask & DTK_M(HOUR)) != 0)
							{
								tm->tm_min = val;
								tmask = DTK_M(MINUTE);
							}
							else
							{
								tm->tm_mon = val;
								tmask = DTK_M(MONTH);
							}
							break;

						case DTK_DAY:
							tm->tm_mday = val;
							tmask = DTK_M(DAY);
							break;

						case DTK_HOUR:
							tm->tm_hour = val;
							tmask = DTK_M(HOUR);
							break;

						case DTK_MINUTE:
							tm->tm_min = val;
							tmask = DTK_M(MINUTE);
							break;

						case DTK_SECOND:
							tm->tm_sec = val;
							tmask = DTK_M(SECOND);
							if (*cp == '.')
							{
								dterr = ParseFractionalSecond(cp, fsec);
								if (dterr)
									return dterr;
								tmask = DTK_ALL_SECS_M;
							}
							break;

						case DTK_TZ:
							tmask = DTK_M(TZ);
							dterr = DecodeTimezone(field[i], tzp);
							if (dterr)
								return dterr;
							break;

						case DTK_JULIAN:
							/* previous field was a label for "julian date" */
							if (val < 0)
								return DTERR_FIELD_OVERFLOW;
							tmask = DTK_DATE_M;
							j2date(val, &tm->tm_year, &tm->tm_mon, &tm->tm_mday);
							isjulian = true;

							/* fractional Julian Day? */
							if (*cp == '.')
							{
								double		time;

								errno = 0;
								time = strtod(cp, &cp);
								if (*cp != '\0' || errno != 0)
									return DTERR_BAD_FORMAT;
								time *= USECS_PER_DAY;
								dt2time(time,
										&tm->tm_hour, &tm->tm_min,
										&tm->tm_sec, fsec);
								tmask |= DTK_TIME_M;
							}
							break;

						case DTK_TIME:
							/* previous field was "t" for ISO time */
							dterr = DecodeNumberField(strlen(field[i]), field[i],
													  (fmask | DTK_DATE_M),
													  &tmask, tm,
													  fsec, &is2digits);
							if (dterr < 0)
								return dterr;
							if (tmask != DTK_TIME_M)
								return DTERR_BAD_FORMAT;
							break;

						default:
							return DTERR_BAD_FORMAT;
							break;
					}

					ptype = 0;
					*dtype = DTK_DATE;
				}
				else
				{
					char	   *cp;
					int			flen;

					flen = strlen(field[i]);
					cp = strchr(field[i], '.');

					/* Embedded decimal and no date yet? */
					if (cp != NULL && !(fmask & DTK_DATE_M))
					{
						dterr = DecodeDate(field[i], fmask,
										   &tmask, &is2digits, tm);
						if (dterr)
							return dterr;
					}
					/* embedded decimal and several digits before? */
					else if (cp != NULL && flen - strlen(cp) > 2)
					{
						/*
						 * Interpret as a concatenated date or time Set the
						 * type field to allow decoding other fields later.
						 * Example: 20011223 or 040506
						 */
						dterr = DecodeNumberField(flen, field[i], fmask,
												  &tmask, tm,
												  fsec, &is2digits);
						if (dterr < 0)
							return dterr;
					}

					/*
					 * Is this a YMD or HMS specification, or a year number?
					 * YMD and HMS are required to be six digits or more, so
					 * if it is 5 digits, it is a year.  If it is six or more
					 * digits, we assume it is YMD or HMS unless no date and
					 * no time values have been specified.  This forces 6+
					 * digit years to be at the end of the string, or to use
					 * the ISO date specification.
					 */
					else if (flen >= 6 && (!(fmask & DTK_DATE_M) ||
										   !(fmask & DTK_TIME_M)))
					{
						dterr = DecodeNumberField(flen, field[i], fmask,
												  &tmask, tm,
												  fsec, &is2digits);
						if (dterr < 0)
							return dterr;
					}
					/* otherwise it is a single date/time field... */
					else
					{
						dterr = DecodeNumber(flen, field[i],
											 haveTextMonth, fmask,
											 &tmask, tm,
											 fsec, &is2digits);
						if (dterr)
							return dterr;
					}
				}
				break;

			case DTK_STRING:
			case DTK_SPECIAL:
				/* timezone abbrevs take precedence over built-in tokens */
				type = DecodeTimezoneAbbrev(i, field[i], &val, &valtz);
				if (type == UNKNOWN_FIELD)
					type = DecodeSpecial(i, field[i], &val);
				if (type == IGNORE_DTF)
					continue;

				tmask = DTK_M(type);
				switch (type)
				{
					case RESERV:
						switch (val)
						{
							case DTK_NOW:
								tmask = (DTK_DATE_M | DTK_TIME_M | DTK_M(TZ));
								*dtype = DTK_DATE;
								GetCurrentTimeUsec(tm, fsec, tzp);
								break;

							case DTK_YESTERDAY:
								tmask = DTK_DATE_M;
								*dtype = DTK_DATE;
								GetCurrentDateTime(&cur_tm);
								j2date(date2j(cur_tm.tm_year, cur_tm.tm_mon, cur_tm.tm_mday) - 1,
									   &tm->tm_year, &tm->tm_mon, &tm->tm_mday);
								break;

							case DTK_TODAY:
								tmask = DTK_DATE_M;
								*dtype = DTK_DATE;
								GetCurrentDateTime(&cur_tm);
								tm->tm_year = cur_tm.tm_year;
								tm->tm_mon = cur_tm.tm_mon;
								tm->tm_mday = cur_tm.tm_mday;
								break;

							case DTK_TOMORROW:
								tmask = DTK_DATE_M;
								*dtype = DTK_DATE;
								GetCurrentDateTime(&cur_tm);
								j2date(date2j(cur_tm.tm_year, cur_tm.tm_mon, cur_tm.tm_mday) + 1,
									   &tm->tm_year, &tm->tm_mon, &tm->tm_mday);
								break;

							case DTK_ZULU:
								tmask = (DTK_TIME_M | DTK_M(TZ));
								*dtype = DTK_DATE;
								tm->tm_hour = 0;
								tm->tm_min = 0;
								tm->tm_sec = 0;
								if (tzp != NULL)
									*tzp = 0;
								break;

							default:
								*dtype = val;
						}

						break;

					case MONTH:

						/*
						 * already have a (numeric) month? then see if we can
						 * substitute...
						 */
						if ((fmask & DTK_M(MONTH)) && !haveTextMonth &&
							!(fmask & DTK_M(DAY)) && tm->tm_mon >= 1 &&
							tm->tm_mon <= 31)
						{
							tm->tm_mday = tm->tm_mon;
							tmask = DTK_M(DAY);
						}
						haveTextMonth = true;
						tm->tm_mon = val;
						break;

					case DTZMOD:

						/*
						 * daylight savings time modifier (solves "MET DST"
						 * syntax)
						 */
						tmask |= DTK_M(DTZ);
						tm->tm_isdst = 1;
						if (tzp == NULL)
							return DTERR_BAD_FORMAT;
						*tzp -= val;
						break;

					case DTZ:

						/*
						 * set mask for TZ here _or_ check for DTZ later when
						 * getting default timezone
						 */
						tmask |= DTK_M(TZ);
						tm->tm_isdst = 1;
						if (tzp == NULL)
							return DTERR_BAD_FORMAT;
						*tzp = -val;
						break;

					case TZ:
						tm->tm_isdst = 0;
						if (tzp == NULL)
							return DTERR_BAD_FORMAT;
						*tzp = -val;
						break;

					case DYNTZ:
						tmask |= DTK_M(TZ);
						if (tzp == NULL)
							return DTERR_BAD_FORMAT;
						/* we'll determine the actual offset later */
						abbrevTz = valtz;
						abbrev = field[i];
						break;

					case AMPM:
						mer = val;
						break;

					case ADBC:
						bc = (val == BC);
						break;

					case DOW:
						tm->tm_wday = val;
						break;

					case UNITS:
						tmask = 0;
						ptype = val;
						break;

					case ISOTIME:

						/*
						 * This is a filler field "t" indicating that the next
						 * field is time. Try to verify that this is sensible.
						 */
						tmask = 0;

						/* No preceding date? Then quit... */
						if ((fmask & DTK_DATE_M) != DTK_DATE_M)
							return DTERR_BAD_FORMAT;

						/***
						 * We will need one of the following fields:
						 *	DTK_NUMBER should be hhmmss.fff
						 *	DTK_TIME should be hh:mm:ss.fff
						 *	DTK_DATE should be hhmmss-zz
						 ***/
						if (i >= nf - 1 ||
							(ftype[i + 1] != DTK_NUMBER &&
							 ftype[i + 1] != DTK_TIME &&
							 ftype[i + 1] != DTK_DATE))
							return DTERR_BAD_FORMAT;

						ptype = val;
						break;

					case UNKNOWN_FIELD:

						/*
						 * Before giving up and declaring error, check to see
						 * if it is an all-alpha timezone name.
						 */
						namedTz = pg_tzset(field[i]);
						if (!namedTz)
							return DTERR_BAD_FORMAT;
						/* we'll apply the zone setting below */
						tmask = DTK_M(TZ);
						break;

					default:
						return DTERR_BAD_FORMAT;
				}
				break;

			default:
				return DTERR_BAD_FORMAT;
		}

		if (tmask & fmask)
			return DTERR_BAD_FORMAT;
		fmask |= tmask;
	}							/* end loop over fields */

	/* do final checking/adjustment of Y/M/D fields */
	dterr = ValidateDate(fmask, isjulian, is2digits, bc, tm);
	if (dterr)
		return dterr;

	/* handle AM/PM */
	if (mer != HR24 && tm->tm_hour > HOURS_PER_DAY / 2)
		return DTERR_FIELD_OVERFLOW;
	if (mer == AM && tm->tm_hour == HOURS_PER_DAY / 2)
		tm->tm_hour = 0;
	else if (mer == PM && tm->tm_hour != HOURS_PER_DAY / 2)
		tm->tm_hour += HOURS_PER_DAY / 2;

	/* do additional checking for full date specs... */
	if (*dtype == DTK_DATE)
	{
		if ((fmask & DTK_DATE_M) != DTK_DATE_M)
		{
			if ((fmask & DTK_TIME_M) == DTK_TIME_M)
				return 1;
			return DTERR_BAD_FORMAT;
		}

		/*
		 * If we had a full timezone spec, compute the offset (we could not do
		 * it before, because we need the date to resolve DST status).
		 */
		if (namedTz != NULL)
		{
			/* daylight savings time modifier disallowed with full TZ */
			if (fmask & DTK_M(DTZMOD))
				return DTERR_BAD_FORMAT;

			*tzp = DetermineTimeZoneOffset(tm, namedTz);
		}

		/*
		 * Likewise, if we had a dynamic timezone abbreviation, resolve it
		 * now.
		 */
		if (abbrevTz != NULL)
		{
			/* daylight savings time modifier disallowed with dynamic TZ */
			if (fmask & DTK_M(DTZMOD))
				return DTERR_BAD_FORMAT;

			*tzp = DetermineTimeZoneAbbrevOffset(tm, abbrev, abbrevTz);
		}

		/* timezone not specified? then use session timezone */
		if (tzp != NULL && !(fmask & DTK_M(TZ)))
		{
			/*
			 * daylight savings time modifier but no standard timezone? then
			 * error
			 */
			if (fmask & DTK_M(DTZMOD))
				return DTERR_BAD_FORMAT;

			*tzp = DetermineTimeZoneOffset(tm, session_timezone);
		}
	}

	return 0;
}

/* DetermineTimeZoneOffset()
 *
 * Given a struct pg_tm in which tm_year, tm_mon, tm_mday, tm_hour, tm_min,
 * and tm_sec fields are set, and a zic-style time zone definition, determine
 * the applicable GMT offset and daylight-savings status at that time.
 * Set the struct pg_tm's tm_isdst field accordingly, and return the GMT
 * offset as the function result.
 *
 * Note: if the date is out of the range we can deal with, we return zero
 * as the GMT offset and set tm_isdst = 0.  We don't throw an error here,
 * though probably some higher-level code will.
 */
int
DetermineTimeZoneOffset(struct pg_tm *tm, pg_tz *tzp)
{
	pg_time_t	t;

	return DetermineTimeZoneOffsetInternal(tm, tzp, &t);
}

/* DetermineTimeZoneOffsetInternal()
 *
 * As above, but also return the actual UTC time imputed to the date/time
 * into *tp.
 *
 * In event of an out-of-range date, we punt by returning zero into *tp.
 * This is okay for the immediate callers but is a good reason for not
 * exposing this worker function globally.
 *
 * Note: it might seem that we should use mktime() for this, but bitter
 * experience teaches otherwise.  This code is much faster than most versions
 * of mktime(), anyway.
 */
static int
DetermineTimeZoneOffsetInternal(struct pg_tm *tm, pg_tz *tzp, pg_time_t *tp)
{
	int			date,
				sec;
	pg_time_t	day,
				mytime,
				prevtime,
				boundary,
				beforetime,
				aftertime;
	long int	before_gmtoff,
				after_gmtoff;
	int			before_isdst,
				after_isdst;
	int			res;

	/*
	 * First, generate the pg_time_t value corresponding to the given
	 * y/m/d/h/m/s taken as GMT time.  If this overflows, punt and decide the
	 * timezone is GMT.  (For a valid Julian date, integer overflow should be
	 * impossible with 64-bit pg_time_t, but let's check for safety.)
	 */
	if (!IS_VALID_JULIAN(tm->tm_year, tm->tm_mon, tm->tm_mday))
		goto overflow;
	date = date2j(tm->tm_year, tm->tm_mon, tm->tm_mday) - UNIX_EPOCH_JDATE;

	day = ((pg_time_t) date) * SECS_PER_DAY;
	if (day / SECS_PER_DAY != date)
		goto overflow;
	sec = tm->tm_sec + (tm->tm_min + tm->tm_hour * MINS_PER_HOUR) * SECS_PER_MINUTE;
	mytime = day + sec;
	/* since sec >= 0, overflow could only be from +day to -mytime */
	if (mytime < 0 && day > 0)
		goto overflow;

	/*
	 * Find the DST time boundary just before or following the target time. We
	 * assume that all zones have GMT offsets less than 24 hours, and that DST
	 * boundaries can't be closer together than 48 hours, so backing up 24
	 * hours and finding the "next" boundary will work.
	 */
	prevtime = mytime - SECS_PER_DAY;
	if (mytime < 0 && prevtime > 0)
		goto overflow;

	res = pg_next_dst_boundary(&prevtime,
							   &before_gmtoff, &before_isdst,
							   &boundary,
							   &after_gmtoff, &after_isdst,
							   tzp);
	if (res < 0)
		goto overflow;			/* failure? */

	if (res == 0)
	{
		/* Non-DST zone, life is simple */
		tm->tm_isdst = before_isdst;
		*tp = mytime - before_gmtoff;
		return -(int) before_gmtoff;
	}

	/*
	 * Form the candidate pg_time_t values with local-time adjustment
	 */
	beforetime = mytime - before_gmtoff;
	if ((before_gmtoff > 0 &&
		 mytime < 0 && beforetime > 0) ||
		(before_gmtoff <= 0 &&
		 mytime > 0 && beforetime < 0))
		goto overflow;
	aftertime = mytime - after_gmtoff;
	if ((after_gmtoff > 0 &&
		 mytime < 0 && aftertime > 0) ||
		(after_gmtoff <= 0 &&
		 mytime > 0 && aftertime < 0))
		goto overflow;

	/*
	 * If both before or both after the boundary time, we know what to do. The
	 * boundary time itself is considered to be after the transition, which
	 * means we can accept aftertime == boundary in the second case.
	 */
	if (beforetime < boundary && aftertime < boundary)
	{
		tm->tm_isdst = before_isdst;
		*tp = beforetime;
		return -(int) before_gmtoff;
	}
	if (beforetime > boundary && aftertime >= boundary)
	{
		tm->tm_isdst = after_isdst;
		*tp = aftertime;
		return -(int) after_gmtoff;
	}

	/*
	 * It's an invalid or ambiguous time due to timezone transition.  In a
	 * spring-forward transition, prefer the "before" interpretation; in a
	 * fall-back transition, prefer "after".  (We used to define and implement
	 * this test as "prefer the standard-time interpretation", but that rule
	 * does not help to resolve the behavior when both times are reported as
	 * standard time; which does happen, eg Europe/Moscow in Oct 2014.  Also,
	 * in some zones such as Europe/Dublin, there is widespread confusion
	 * about which time offset is "standard" time, so it's fortunate that our
	 * behavior doesn't depend on that.)
	 */
	if (beforetime > aftertime)
	{
		tm->tm_isdst = before_isdst;
		*tp = beforetime;
		return -(int) before_gmtoff;
	}
	tm->tm_isdst = after_isdst;
	*tp = aftertime;
	return -(int) after_gmtoff;

overflow:
	/* Given date is out of range, so assume UTC */
	tm->tm_isdst = 0;
	*tp = 0;
	return 0;
}

/* DetermineTimeZoneAbbrevOffset()
 *
 * Determine the GMT offset and DST flag to be attributed to a dynamic
 * time zone abbreviation, that is one whose meaning has changed over time.
 * *tm contains the local time at which the meaning should be determined,
 * and tm->tm_isdst receives the DST flag.
 *
 * This differs from the behavior of DetermineTimeZoneOffset() in that a
 * standard-time or daylight-time abbreviation forces use of the corresponding
 * GMT offset even when the zone was then in DS or standard time respectively.
 * (However, that happens only if we can match the given abbreviation to some
 * abbreviation that appears in the IANA timezone data.  Otherwise, we fall
 * back to doing DetermineTimeZoneOffset().)
 */
int
DetermineTimeZoneAbbrevOffset(struct pg_tm *tm, const char *abbr, pg_tz *tzp)
{
	pg_time_t	t;
	int			zone_offset;
	int			abbr_offset;
	int			abbr_isdst;

	/*
	 * Compute the UTC time we want to probe at.  (In event of overflow, we'll
	 * probe at the epoch, which is a bit random but probably doesn't matter.)
	 */
	zone_offset = DetermineTimeZoneOffsetInternal(tm, tzp, &t);

	/*
	 * Try to match the abbreviation to something in the zone definition.
	 */
	if (DetermineTimeZoneAbbrevOffsetInternal(t, abbr, tzp,
											  &abbr_offset, &abbr_isdst))
	{
		/* Success, so use the abbrev-specific answers. */
		tm->tm_isdst = abbr_isdst;
		return abbr_offset;
	}

	/*
	 * No match, so use the answers we already got from
	 * DetermineTimeZoneOffsetInternal.
	 */
	return zone_offset;
}

/* DetermineTimeZoneAbbrevOffsetInternal()
 *
 * Workhorse for above two functions: work from a pg_time_t probe instant.
 * On success, return GMT offset and DST status into *offset and *isdst.
 */
static bool
DetermineTimeZoneAbbrevOffsetInternal(pg_time_t t, const char *abbr, pg_tz *tzp,
									  int *offset, int *isdst)
{
	char		upabbr[TZ_STRLEN_MAX + 1];
	unsigned char *p;
	long int	gmtoff;

	/* We need to force the abbrev to upper case */
	// strlcpy(upabbr, abbr, sizeof(upabbr)); /* MobilityDB */
	strncpy(upabbr, abbr, sizeof(upabbr));
	for (p = (unsigned char *) upabbr; *p; p++)
		*p = pg_toupper(*p);

	/* Look up the abbrev's meaning at this time in this zone */
	if (pg_interpret_timezone_abbrev(upabbr,
									 &t,
									 &gmtoff,
									 isdst,
									 tzp))
	{
		/* Change sign to agree with DetermineTimeZoneOffset() */
		*offset = (int) -gmtoff;
		return true;
	}
	return false;
}

/* DecodeDate()
 * Decode date string which includes delimiters.
 * Return 0 if okay, a DTERR code if not.
 *
 *	str: field to be parsed
 *	fmask: bitmask for field types already seen
 *	*tmask: receives bitmask for fields found here
 *	*is2digits: set to true if we find 2-digit year
 *	*tm: field values are stored into appropriate members of this struct
 */
static int
DecodeDate(char *str, int fmask __attribute__((unused)), int *tmask, bool *is2digits,
		   struct pg_tm *tm)
{
	fsec_t		fsec;
	int			nf = 0;
	int			i,
				len;
	int			dterr;
	bool		haveTextMonth = false;
	int			type,
				val,
				dmask = 0;
	char	   *field[MAXDATEFIELDS];

	*tmask = 0;

	/* parse this string... */
	while (*str != '\0' && nf < MAXDATEFIELDS)
	{
		/* skip field separators */
		while (*str != '\0' && !isalnum((unsigned char) *str))
			str++;

		if (*str == '\0')
			return DTERR_BAD_FORMAT;	/* end of string after separator */

		field[nf] = str;
		if (isdigit((unsigned char) *str))
		{
			while (isdigit((unsigned char) *str))
				str++;
		}
		else if (isalpha((unsigned char) *str))
		{
			while (isalpha((unsigned char) *str))
				str++;
		}

		/* Just get rid of any non-digit, non-alpha characters... */
		if (*str != '\0')
			*str++ = '\0';
		nf++;
	}

	/* look first for text fields, since that will be unambiguous month */
	for (i = 0; i < nf; i++)
	{
		if (isalpha((unsigned char) *field[i]))
		{
			type = DecodeSpecial(i, field[i], &val);
			if (type == IGNORE_DTF)
				continue;

			dmask = DTK_M(type);
			switch (type)
			{
				case MONTH:
					tm->tm_mon = val;
					haveTextMonth = true;
					break;

				default:
					return DTERR_BAD_FORMAT;
			}
			if (fmask & dmask)
				return DTERR_BAD_FORMAT;

			fmask |= dmask;
			*tmask |= dmask;

			/* mark this field as being completed */
			field[i] = NULL;
		}
	}

	/* now pick up remaining numeric fields */
	for (i = 0; i < nf; i++)
	{
		if (field[i] == NULL)
			continue;

		if ((len = strlen(field[i])) <= 0)
			return DTERR_BAD_FORMAT;

		dterr = DecodeNumber(len, field[i], haveTextMonth, fmask,
							 &dmask, tm,
							 &fsec, is2digits);
		if (dterr)
			return dterr;

		if (fmask & dmask)
			return DTERR_BAD_FORMAT;

		fmask |= dmask;
		*tmask |= dmask;
	}

	if ((fmask & ~(DTK_M(DOY) | DTK_M(TZ))) != DTK_DATE_M)
		return DTERR_BAD_FORMAT;

	/* validation of the field values must wait until ValidateDate() */

	return 0;
}

/* ValidateDate()
 * Check valid year/month/day values, handle BC and DOY cases
 * Return 0 if okay, a DTERR code if not.
 */
int
ValidateDate(int fmask, bool isjulian, bool is2digits, bool bc,
			 struct pg_tm *tm)
{
	if (fmask & DTK_M(YEAR))
	{
		if (isjulian)
		{
			/* tm_year is correct and should not be touched */
		}
		else if (bc)
		{
			/* there is no year zero in AD/BC notation */
			if (tm->tm_year <= 0)
				return DTERR_FIELD_OVERFLOW;
			/* internally, we represent 1 BC as year zero, 2 BC as -1, etc */
			tm->tm_year = -(tm->tm_year - 1);
		}
		else if (is2digits)
		{
			/* process 1 or 2-digit input as 1970-2069 AD, allow '0' and '00' */
			if (tm->tm_year < 0)	/* just paranoia */
				return DTERR_FIELD_OVERFLOW;
			if (tm->tm_year < 70)
				tm->tm_year += 2000;
			else if (tm->tm_year < 100)
				tm->tm_year += 1900;
		}
		else
		{
			/* there is no year zero in AD/BC notation */
			if (tm->tm_year <= 0)
				return DTERR_FIELD_OVERFLOW;
		}
	}

	/* now that we have correct year, decode DOY */
	if (fmask & DTK_M(DOY))
	{
		j2date(date2j(tm->tm_year, 1, 1) + tm->tm_yday - 1,
			   &tm->tm_year, &tm->tm_mon, &tm->tm_mday);
	}

	/* check for valid month */
	if (fmask & DTK_M(MONTH))
	{
		if (tm->tm_mon < 1 || tm->tm_mon > MONTHS_PER_YEAR)
			return DTERR_MD_FIELD_OVERFLOW;
	}

	/* minimal check for valid day */
	if (fmask & DTK_M(DAY))
	{
		if (tm->tm_mday < 1 || tm->tm_mday > 31)
			return DTERR_MD_FIELD_OVERFLOW;
	}

	if ((fmask & DTK_DATE_M) == DTK_DATE_M)
	{
		/*
		 * Check for valid day of month, now that we know for sure the month
		 * and year.  Note we don't use MD_FIELD_OVERFLOW here, since it seems
		 * unlikely that "Feb 29" is a YMD-order error.
		 */
		if (tm->tm_mday > day_tab[isleap(tm->tm_year)][tm->tm_mon - 1])
			return DTERR_FIELD_OVERFLOW;
	}

	return 0;
}

/* DecodeTime()
 * Decode time string which includes delimiters.
 * Return 0 if okay, a DTERR code if not.
 *
 * Only check the lower limit on hours, since this same code can be
 * used to represent time spans.
 */
static int
DecodeTime(char *str, int fmask __attribute__((unused)), int range,
		   int *tmask, struct pg_tm *tm, fsec_t *fsec)
{
	char	   *cp;
	int			dterr;

	*tmask = DTK_TIME_M;

	errno = 0;
	tm->tm_hour = strtoint(str, &cp, 10);
	if (errno == ERANGE)
		return DTERR_FIELD_OVERFLOW;
	if (*cp != ':')
		return DTERR_BAD_FORMAT;
	errno = 0;
	tm->tm_min = strtoint(cp + 1, &cp, 10);
	if (errno == ERANGE)
		return DTERR_FIELD_OVERFLOW;
	if (*cp == '\0')
	{
		tm->tm_sec = 0;
		*fsec = 0;
		/* If it's a MINUTE TO SECOND interval, take 2 fields as being mm:ss */
		if (range == (INTERVAL_MASK(MINUTE) | INTERVAL_MASK(SECOND)))
		{
			tm->tm_sec = tm->tm_min;
			tm->tm_min = tm->tm_hour;
			tm->tm_hour = 0;
		}
	}
	else if (*cp == '.')
	{
		/* always assume mm:ss.sss is MINUTE TO SECOND */
		dterr = ParseFractionalSecond(cp, fsec);
		if (dterr)
			return dterr;
		tm->tm_sec = tm->tm_min;
		tm->tm_min = tm->tm_hour;
		tm->tm_hour = 0;
	}
	else if (*cp == ':')
	{
		errno = 0;
		tm->tm_sec = strtoint(cp + 1, &cp, 10);
		if (errno == ERANGE)
			return DTERR_FIELD_OVERFLOW;
		if (*cp == '\0')
			*fsec = 0;
		else if (*cp == '.')
		{
			dterr = ParseFractionalSecond(cp, fsec);
			if (dterr)
				return dterr;
		}
		else
			return DTERR_BAD_FORMAT;
	}
	else
		return DTERR_BAD_FORMAT;

	/* do a sanity check */
	if (tm->tm_hour < 0 || tm->tm_min < 0 || tm->tm_min > MINS_PER_HOUR - 1 ||
		tm->tm_sec < 0 || tm->tm_sec > SECS_PER_MINUTE ||
		*fsec < INT64CONST(0) ||
		*fsec > USECS_PER_SEC)
		return DTERR_FIELD_OVERFLOW;

	return 0;
}

/* DecodeNumber()
 * Interpret plain numeric field as a date value in context.
 * Return 0 if okay, a DTERR code if not.
 */
static int
DecodeNumber(int flen, char *str, bool haveTextMonth, int fmask,
			 int *tmask, struct pg_tm *tm, fsec_t *fsec, bool *is2digits)
{
	int			val;
	char	   *cp;
	int			dterr;

	*tmask = 0;

	errno = 0;
	val = strtoint(str, &cp, 10);
	if (errno == ERANGE)
		return DTERR_FIELD_OVERFLOW;
	if (cp == str)
		return DTERR_BAD_FORMAT;

	if (*cp == '.')
	{
		/*
		 * More than two digits before decimal point? Then could be a date or
		 * a run-together time: 2001.360 20011225 040506.789
		 */
		if (cp - str > 2)
		{
			dterr = DecodeNumberField(flen, str,
									  (fmask | DTK_DATE_M),
									  tmask, tm,
									  fsec, is2digits);
			if (dterr < 0)
				return dterr;
			return 0;
		}

		dterr = ParseFractionalSecond(cp, fsec);
		if (dterr)
			return dterr;
	}
	else if (*cp != '\0')
		return DTERR_BAD_FORMAT;

	/* Special case for day of year */
	if (flen == 3 && (fmask & DTK_DATE_M) == DTK_M(YEAR) && val >= 1 &&
		val <= 366)
	{
		*tmask = (DTK_M(DOY) | DTK_M(MONTH) | DTK_M(DAY));
		tm->tm_yday = val;
		/* tm_mon and tm_mday can't actually be set yet ... */
		return 0;
	}

	/* Switch based on what we have so far */
	switch (fmask & DTK_DATE_M)
	{
		case 0:

			/*
			 * Nothing so far; make a decision about what we think the input
			 * is.  There used to be lots of heuristics here, but the
			 * consensus now is to be paranoid.  It *must* be either
			 * YYYY-MM-DD (with a more-than-two-digit year field), or the
			 * field order defined by DateOrder.
			 */
			if (flen >= 3 || DateOrder == DATEORDER_YMD)
			{
				*tmask = DTK_M(YEAR);
				tm->tm_year = val;
			}
			else if (DateOrder == DATEORDER_DMY)
			{
				*tmask = DTK_M(DAY);
				tm->tm_mday = val;
			}
			else
			{
				*tmask = DTK_M(MONTH);
				tm->tm_mon = val;
			}
			break;

		case (DTK_M(YEAR)):
			/* Must be at second field of YY-MM-DD */
			*tmask = DTK_M(MONTH);
			tm->tm_mon = val;
			break;

		case (DTK_M(MONTH)):
			if (haveTextMonth)
			{
				/*
				 * We are at the first numeric field of a date that included a
				 * textual month name.  We want to support the variants
				 * MON-DD-YYYY, DD-MON-YYYY, and YYYY-MON-DD as unambiguous
				 * inputs.  We will also accept MON-DD-YY or DD-MON-YY in
				 * either DMY or MDY modes, as well as YY-MON-DD in YMD mode.
				 */
				if (flen >= 3 || DateOrder == DATEORDER_YMD)
				{
					*tmask = DTK_M(YEAR);
					tm->tm_year = val;
				}
				else
				{
					*tmask = DTK_M(DAY);
					tm->tm_mday = val;
				}
			}
			else
			{
				/* Must be at second field of MM-DD-YY */
				*tmask = DTK_M(DAY);
				tm->tm_mday = val;
			}
			break;

		case (DTK_M(YEAR) | DTK_M(MONTH)):
			if (haveTextMonth)
			{
				/* Need to accept DD-MON-YYYY even in YMD mode */
				if (flen >= 3 && *is2digits)
				{
					/* Guess that first numeric field is day was wrong */
					*tmask = DTK_M(DAY);	/* YEAR is already set */
					tm->tm_mday = tm->tm_year;
					tm->tm_year = val;
					*is2digits = false;
				}
				else
				{
					*tmask = DTK_M(DAY);
					tm->tm_mday = val;
				}
			}
			else
			{
				/* Must be at third field of YY-MM-DD */
				*tmask = DTK_M(DAY);
				tm->tm_mday = val;
			}
			break;

		case (DTK_M(DAY)):
			/* Must be at second field of DD-MM-YY */
			*tmask = DTK_M(MONTH);
			tm->tm_mon = val;
			break;

		case (DTK_M(MONTH) | DTK_M(DAY)):
			/* Must be at third field of DD-MM-YY or MM-DD-YY */
			*tmask = DTK_M(YEAR);
			tm->tm_year = val;
			break;

		case (DTK_M(YEAR) | DTK_M(MONTH) | DTK_M(DAY)):
			/* we have all the date, so it must be a time field */
			dterr = DecodeNumberField(flen, str, fmask,
									  tmask, tm,
									  fsec, is2digits);
			if (dterr < 0)
				return dterr;
			return 0;

		default:
			/* Anything else is bogus input */
			return DTERR_BAD_FORMAT;
	}

	/*
	 * When processing a year field, mark it for adjustment if it's only one
	 * or two digits.
	 */
	if (*tmask == DTK_M(YEAR))
		*is2digits = (flen <= 2);

	return 0;
}

/* DecodeNumberField()
 * Interpret numeric string as a concatenated date or time field.
 * Return a DTK token (>= 0) if successful, a DTERR code (< 0) if not.
 *
 * Use the context of previously decoded fields to help with
 * the interpretation.
 */
static int
DecodeNumberField(int len, char *str, int fmask,
				  int *tmask, struct pg_tm *tm, fsec_t *fsec, bool *is2digits)
{
	char	   *cp;

	/*
	 * Have a decimal point? Then this is a date or something with a seconds
	 * field...
	 */
	if ((cp = strchr(str, '.')) != NULL)
	{
		/*
		 * Can we use ParseFractionalSecond here?  Not clear whether trailing
		 * junk should be rejected ...
		 */
		double		frac;

		errno = 0;
		frac = strtod(cp, NULL);
		if (errno != 0)
			return DTERR_BAD_FORMAT;
		*fsec = rint(frac * 1000000);
		/* Now truncate off the fraction for further processing */
		*cp = '\0';
		len = strlen(str);
	}
	/* No decimal point and no complete date yet? */
	else if ((fmask & DTK_DATE_M) != DTK_DATE_M)
	{
		if (len >= 6)
		{
			*tmask = DTK_DATE_M;

			/*
			 * Start from end and consider first 2 as Day, next 2 as Month,
			 * and the rest as Year.
			 */
			tm->tm_mday = atoi(str + (len - 2));
			*(str + (len - 2)) = '\0';
			tm->tm_mon = atoi(str + (len - 4));
			*(str + (len - 4)) = '\0';
			tm->tm_year = atoi(str);
			if ((len - 4) == 2)
				*is2digits = true;

			return DTK_DATE;
		}
	}

	/* not all time fields are specified? */
	if ((fmask & DTK_TIME_M) != DTK_TIME_M)
	{
		/* hhmmss */
		if (len == 6)
		{
			*tmask = DTK_TIME_M;
			tm->tm_sec = atoi(str + 4);
			*(str + 4) = '\0';
			tm->tm_min = atoi(str + 2);
			*(str + 2) = '\0';
			tm->tm_hour = atoi(str);

			return DTK_TIME;
		}
		/* hhmm? */
		else if (len == 4)
		{
			*tmask = DTK_TIME_M;
			tm->tm_sec = 0;
			tm->tm_min = atoi(str + 2);
			*(str + 2) = '\0';
			tm->tm_hour = atoi(str);

			return DTK_TIME;
		}
	}

	return DTERR_BAD_FORMAT;
}

/* DecodeTimezone()
 * Interpret string as a numeric timezone.
 *
 * Return 0 if okay (and set *tzp), a DTERR code if not okay.
 */
int
DecodeTimezone(char *str, int *tzp)
{
	int			tz;
	int			hr,
				min,
				sec = 0;
	char	   *cp;

	/* leading character must be "+" or "-" */
	if (*str != '+' && *str != '-')
		return DTERR_BAD_FORMAT;

	errno = 0;
	hr = strtoint(str + 1, &cp, 10);
	if (errno == ERANGE)
		return DTERR_TZDISP_OVERFLOW;

	/* explicit delimiter? */
	if (*cp == ':')
	{
		errno = 0;
		min = strtoint(cp + 1, &cp, 10);
		if (errno == ERANGE)
			return DTERR_TZDISP_OVERFLOW;
		if (*cp == ':')
		{
			errno = 0;
			sec = strtoint(cp + 1, &cp, 10);
			if (errno == ERANGE)
				return DTERR_TZDISP_OVERFLOW;
		}
	}
	/* otherwise, might have run things together... */
	else if (*cp == '\0' && strlen(str) > 3)
	{
		min = hr % 100;
		hr = hr / 100;
		/* we could, but don't, support a run-together hhmmss format */
	}
	else
		min = 0;

	/* Range-check the values; see notes in datatype/timestamp.h */
	if (hr < 0 || hr > MAX_TZDISP_HOUR)
		return DTERR_TZDISP_OVERFLOW;
	if (min < 0 || min >= MINS_PER_HOUR)
		return DTERR_TZDISP_OVERFLOW;
	if (sec < 0 || sec >= SECS_PER_MINUTE)
		return DTERR_TZDISP_OVERFLOW;

	tz = (hr * MINS_PER_HOUR + min) * SECS_PER_MINUTE + sec;
	if (*str == '-')
		tz = -tz;

	*tzp = -tz;

	if (*cp != '\0')
		return DTERR_BAD_FORMAT;

	return 0;
}

/* DecodeTimezoneAbbrev()
 * Interpret string as a timezone abbreviation, if possible.
 *
 * Returns an abbreviation type (TZ, DTZ, or DYNTZ), or UNKNOWN_FIELD if
 * string is not any known abbreviation.  On success, set *offset and *tz to
 * represent the UTC offset (for TZ or DTZ) or underlying zone (for DYNTZ).
 * Note that full timezone names (such as America/New_York) are not handled
 * here, mostly for historical reasons.
 *
 * Given string must be lowercased already.
 *
 * Implement a cache lookup since it is likely that dates
 *	will be related in format.
 */
int
DecodeTimezoneAbbrev(int field, char *lowtoken,
					 int *offset, pg_tz **tz)
{
	int			type;
	const datetkn *tp;

	tp = abbrevcache[field];
	/* use strncmp so that we match truncated tokens */
	if (tp == NULL || strncmp(lowtoken, tp->token, TOKMAXLEN) != 0)
	{
		if (zoneabbrevtbl)
			tp = datebsearch(lowtoken, zoneabbrevtbl->abbrevs,
							 zoneabbrevtbl->numabbrevs);
		else
			tp = NULL;
	}
	if (tp == NULL)
	{
		type = UNKNOWN_FIELD;
		*offset = 0;
		*tz = NULL;
	}
	else
	{
		abbrevcache[field] = tp;
		type = tp->type;
		if (type == DYNTZ)
		{
			*offset = 0;
			*tz = FetchDynamicTimeZone(zoneabbrevtbl, tp);
		}
		else
		{
			*offset = tp->value;
			*tz = NULL;
		}
	}

	return type;
}

/* DecodeSpecial()
 * Decode text string using lookup table.
 *
 * Recognizes the keywords listed in datetktbl.
 * Note: at one time this would also recognize timezone abbreviations,
 * but no more; use DecodeTimezoneAbbrev for that.
 *
 * Given string must be lowercased already.
 *
 * Implement a cache lookup since it is likely that dates
 *	will be related in format.
 */
int
DecodeSpecial(int field, char *lowtoken, int *val)
{
	int			type;
	const datetkn *tp;

	tp = datecache[field];
	/* use strncmp so that we match truncated tokens */
	if (tp == NULL || strncmp(lowtoken, tp->token, TOKMAXLEN) != 0)
	{
		tp = datebsearch(lowtoken, datetktbl, szdatetktbl);
	}
	if (tp == NULL)
	{
		type = UNKNOWN_FIELD;
		*val = 0;
	}
	else
	{
		datecache[field] = tp;
		type = tp->type;
		*val = tp->value;
	}

	return type;
}

/* datebsearch()
 * Binary search -- from Knuth (6.2.1) Algorithm B.  Special case like this
 * is WAY faster than the generic bsearch().
 */
static const datetkn *
datebsearch(const char *key, const datetkn *base, int nel)
{
	if (nel > 0)
	{
		const datetkn *last = base + nel - 1,
				   *position;
		int			result;

		while (last >= base)
		{
			position = base + ((last - base) >> 1);
			/* precheck the first character for a bit of extra speed */
			result = (int) key[0] - (int) position->token[0];
			if (result == 0)
			{
				/* use strncmp so that we match truncated tokens */
				result = strncmp(key, position->token, TOKMAXLEN);
				if (result == 0)
					return position;
			}
			if (result < 0)
				last = position - 1;
			else
				base = position + 1;
		}
	}
	return NULL;
}

/* EncodeTimezone()
 *		Copies representation of a numeric timezone offset to str.
 *
 * Returns a pointer to the new end of string.  No NUL terminator is put
 * there; callers are responsible for NUL terminating str themselves.
 */
static char *
EncodeTimezone(char *str, int tz, int style)
{
	int			hour,
				min,
				sec;

	sec = abs(tz);
	min = sec / SECS_PER_MINUTE;
	sec -= min * SECS_PER_MINUTE;
	hour = min / MINS_PER_HOUR;
	min -= hour * MINS_PER_HOUR;

	/* TZ is negated compared to sign we wish to display ... */
	*str++ = (tz <= 0 ? '+' : '-');

	if (sec != 0)
	{
		str = pg_ultostr_zeropad(str, hour, 2);
		*str++ = ':';
		str = pg_ultostr_zeropad(str, min, 2);
		*str++ = ':';
		str = pg_ultostr_zeropad(str, sec, 2);
	}
	else if (min != 0 || style == USE_XSD_DATES)
	{
		str = pg_ultostr_zeropad(str, hour, 2);
		*str++ = ':';
		str = pg_ultostr_zeropad(str, min, 2);
	}
	else
		str = pg_ultostr_zeropad(str, hour, 2);
	return str;
}

/*
 * Convert reserved date values to string.
 */
void
EncodeSpecialDate(DateADT dt, char *str)
{
	if (DATE_IS_NOBEGIN(dt))
		strcpy(str, EARLY);
	else if (DATE_IS_NOEND(dt))
		strcpy(str, LATE);
	else						/* shouldn't happen */
		elog(ERROR, "invalid argument for EncodeSpecialDate");
}

/* EncodeDateOnly()
 * Encode date as local time.
 */
void
EncodeDateOnly(struct pg_tm *tm, int style, char *str)
{
	Assert(tm->tm_mon >= 1 && tm->tm_mon <= MONTHS_PER_YEAR);

	switch (style)
	{
		case USE_ISO_DATES:
		case USE_XSD_DATES:
			/* compatible with ISO date formats */
			str = pg_ultostr_zeropad(str,
									 (tm->tm_year > 0) ? tm->tm_year : -(tm->tm_year - 1), 4);
			*str++ = '-';
			str = pg_ultostr_zeropad(str, tm->tm_mon, 2);
			*str++ = '-';
			str = pg_ultostr_zeropad(str, tm->tm_mday, 2);
			break;

		case USE_SQL_DATES:
			/* compatible with Oracle/Ingres date formats */
			if (DateOrder == DATEORDER_DMY)
			{
				str = pg_ultostr_zeropad(str, tm->tm_mday, 2);
				*str++ = '/';
				str = pg_ultostr_zeropad(str, tm->tm_mon, 2);
			}
			else
			{
				str = pg_ultostr_zeropad(str, tm->tm_mon, 2);
				*str++ = '/';
				str = pg_ultostr_zeropad(str, tm->tm_mday, 2);
			}
			*str++ = '/';
			str = pg_ultostr_zeropad(str,
									 (tm->tm_year > 0) ? tm->tm_year : -(tm->tm_year - 1), 4);
			break;

		case USE_GERMAN_DATES:
			/* German-style date format */
			str = pg_ultostr_zeropad(str, tm->tm_mday, 2);
			*str++ = '.';
			str = pg_ultostr_zeropad(str, tm->tm_mon, 2);
			*str++ = '.';
			str = pg_ultostr_zeropad(str,
									 (tm->tm_year > 0) ? tm->tm_year : -(tm->tm_year - 1), 4);
			break;

		case USE_POSTGRES_DATES:
		default:
			/* traditional date-only style for Postgres */
			if (DateOrder == DATEORDER_DMY)
			{
				str = pg_ultostr_zeropad(str, tm->tm_mday, 2);
				*str++ = '-';
				str = pg_ultostr_zeropad(str, tm->tm_mon, 2);
			}
			else
			{
				str = pg_ultostr_zeropad(str, tm->tm_mon, 2);
				*str++ = '-';
				str = pg_ultostr_zeropad(str, tm->tm_mday, 2);
			}
			*str++ = '-';
			str = pg_ultostr_zeropad(str,
									 (tm->tm_year > 0) ? tm->tm_year : -(tm->tm_year - 1), 4);
			break;
	}

	if (tm->tm_year <= 0)
	{
		memcpy(str, " BC", 3);	/* Don't copy NUL */
		str += 3;
	}
	*str = '\0';
}

/* EncodeDateTime()
 * Encode date and time interpreted as local time.
 *
 * tm and fsec are the value to encode, print_tz determines whether to include
 * a time zone (the difference between timestamp and timestamptz types), tz is
 * the numeric time zone offset, tzn is the textual time zone, which if
 * specified will be used instead of tz by some styles, style is the date
 * style, str is where to write the output.
 *
 * Supported date styles:
 *	Postgres - day mon hh:mm:ss yyyy tz
 *	SQL - mm/dd/yyyy hh:mm:ss.ss tz
 *	ISO - yyyy-mm-dd hh:mm:ss+/-tz
 *	German - dd.mm.yyyy hh:mm:ss tz
 *	XSD - yyyy-mm-ddThh:mm:ss.ss+/-tz
 */
void
EncodeDateTime(struct pg_tm *tm, fsec_t fsec, bool print_tz, int tz, const char *tzn, int style, char *str)
{
	int			day;

	Assert(tm->tm_mon >= 1 && tm->tm_mon <= MONTHS_PER_YEAR);

	/*
	 * Negative tm_isdst means we have no valid time zone translation.
	 */
	if (tm->tm_isdst < 0)
		print_tz = false;

	switch (style)
	{
		case USE_ISO_DATES:
		case USE_XSD_DATES:
			/* Compatible with ISO-8601 date formats */
			str = pg_ultostr_zeropad(str,
									 (tm->tm_year > 0) ? tm->tm_year : -(tm->tm_year - 1), 4);
			*str++ = '-';
			str = pg_ultostr_zeropad(str, tm->tm_mon, 2);
			*str++ = '-';
			str = pg_ultostr_zeropad(str, tm->tm_mday, 2);
			*str++ = (style == USE_ISO_DATES) ? ' ' : 'T';
			str = pg_ultostr_zeropad(str, tm->tm_hour, 2);
			*str++ = ':';
			str = pg_ultostr_zeropad(str, tm->tm_min, 2);
			*str++ = ':';
			str = AppendTimestampSeconds(str, tm, fsec);
			if (print_tz)
				str = EncodeTimezone(str, tz, style);
			break;

		case USE_SQL_DATES:
			/* Compatible with Oracle/Ingres date formats */
			if (DateOrder == DATEORDER_DMY)
			{
				str = pg_ultostr_zeropad(str, tm->tm_mday, 2);
				*str++ = '/';
				str = pg_ultostr_zeropad(str, tm->tm_mon, 2);
			}
			else
			{
				str = pg_ultostr_zeropad(str, tm->tm_mon, 2);
				*str++ = '/';
				str = pg_ultostr_zeropad(str, tm->tm_mday, 2);
			}
			*str++ = '/';
			str = pg_ultostr_zeropad(str,
									 (tm->tm_year > 0) ? tm->tm_year : -(tm->tm_year - 1), 4);
			*str++ = ' ';
			str = pg_ultostr_zeropad(str, tm->tm_hour, 2);
			*str++ = ':';
			str = pg_ultostr_zeropad(str, tm->tm_min, 2);
			*str++ = ':';
			str = AppendTimestampSeconds(str, tm, fsec);

			/*
			 * Note: the uses of %.*s in this function would be risky if the
			 * timezone names ever contain non-ASCII characters, since we are
			 * not being careful to do encoding-aware clipping.  However, all
			 * TZ abbreviations in the IANA database are plain ASCII.
			 */
			if (print_tz)
			{
				if (tzn)
				{
					sprintf(str, " %.*s", MAXTZLEN, tzn);
					str += strlen(str);
				}
				else
					str = EncodeTimezone(str, tz, style);
			}
			break;

		case USE_GERMAN_DATES:
			/* German variant on European style */
			str = pg_ultostr_zeropad(str, tm->tm_mday, 2);
			*str++ = '.';
			str = pg_ultostr_zeropad(str, tm->tm_mon, 2);
			*str++ = '.';
			str = pg_ultostr_zeropad(str,
									 (tm->tm_year > 0) ? tm->tm_year : -(tm->tm_year - 1), 4);
			*str++ = ' ';
			str = pg_ultostr_zeropad(str, tm->tm_hour, 2);
			*str++ = ':';
			str = pg_ultostr_zeropad(str, tm->tm_min, 2);
			*str++ = ':';
			str = AppendTimestampSeconds(str, tm, fsec);

			if (print_tz)
			{
				if (tzn)
				{
					sprintf(str, " %.*s", MAXTZLEN, tzn);
					str += strlen(str);
				}
				else
					str = EncodeTimezone(str, tz, style);
			}
			break;

		case USE_POSTGRES_DATES:
		default:
			/* Backward-compatible with traditional Postgres abstime dates */
			day = date2j(tm->tm_year, tm->tm_mon, tm->tm_mday);
			tm->tm_wday = j2day(day);
			memcpy(str, days[tm->tm_wday], 3);
			str += 3;
			*str++ = ' ';
			if (DateOrder == DATEORDER_DMY)
			{
				str = pg_ultostr_zeropad(str, tm->tm_mday, 2);
				*str++ = ' ';
				memcpy(str, months[tm->tm_mon - 1], 3);
				str += 3;
			}
			else
			{
				memcpy(str, months[tm->tm_mon - 1], 3);
				str += 3;
				*str++ = ' ';
				str = pg_ultostr_zeropad(str, tm->tm_mday, 2);
			}
			*str++ = ' ';
			str = pg_ultostr_zeropad(str, tm->tm_hour, 2);
			*str++ = ':';
			str = pg_ultostr_zeropad(str, tm->tm_min, 2);
			*str++ = ':';
			str = AppendTimestampSeconds(str, tm, fsec);
			*str++ = ' ';
			str = pg_ultostr_zeropad(str,
									 (tm->tm_year > 0) ? tm->tm_year : -(tm->tm_year - 1), 4);

			if (print_tz)
			{
				if (tzn)
				{
					sprintf(str, " %.*s", MAXTZLEN, tzn);
					str += strlen(str);
				}
				else
				{
					/*
					 * We have a time zone, but no string version. Use the
					 * numeric form, but be sure to include a leading space to
					 * avoid formatting something which would be rejected by
					 * the date/time parser later. - thomas 2001-10-19
					 */
					*str++ = ' ';
					str = EncodeTimezone(str, tz, style);
				}
			}
			break;
	}

	if (tm->tm_year <= 0)
	{
		memcpy(str, " BC", 3);	/* Don't copy NUL */
		str += 3;
	}
	*str = '\0';
}

/*
 * Helper subroutine to locate pg_tz timezone for a dynamic abbreviation.
 */
static pg_tz *
FetchDynamicTimeZone(TimeZoneAbbrevTable *tbl, const datetkn *tp)
{
	DynamicZoneAbbrev *dtza;

	/* Just some sanity checks to prevent indexing off into nowhere */
	Assert(tp->type == DYNTZ);
	Assert(tp->value > 0 && tp->value < tbl->tblsize);

	dtza = (DynamicZoneAbbrev *) ((char *) tbl + tp->value);

	/* Look up the underlying zone if we haven't already */
	if (dtza->tz == NULL)
	{
		dtza->tz = pg_tzset(dtza->zone);

		/*
		 * Ideally we'd let the caller ereport instead of doing it here, but
		 * then there is no way to report the bad time zone name.
		 */
		if (dtza->tz == NULL)
			elog(ERROR, "time zone \"%s\" not recognized", dtza->zone);
	}
	return dtza->tz;
}

