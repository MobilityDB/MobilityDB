/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @file
 * @brief Functions for managing the MEOS extension and its global variables
 * and constants
 */

/* C */
#include <math.h>
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
#include <common/pg_prng.h>
/* Proj */
#include <proj.h>
/* GEOS */
#include <geos_c.h>
/* PostGIS */
#include <lwgeom_log.h>
#include <lwgeom_geos.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>

// TODO REMOVE
extern void json_destroy_tofree();

/***************************************************************************
 * Functions for the PostgreSQL pseudo-random number generator
 ***************************************************************************/

/* Global variables */

static MEOS_TLS bool MEOS_PRNG_INITIALIZED = false;
static MEOS_TLS pg_prng_state MEOS_GENERATION_RNG;
static MEOS_TLS pg_prng_state MEOS_AGGREGATION_RNG;

/**
 * @brief Initialize the PostgreSQL pseudo-random number generators
 */
static void
prng_initialize(void)
{
  if (! MEOS_PRNG_INITIALIZED)
  {
    pg_prng_seed(&MEOS_GENERATION_RNG, 0);
    pg_prng_seed(&MEOS_AGGREGATION_RNG, 1);
    MEOS_PRNG_INITIALIZED = true;
  }
  return;
}

#if MEOS
/**
 * @brief Finalize the PostgreSQL pseudo-random number generators
 */
static void
prng_finalize(void)
{
  MEOS_PRNG_INITIALIZED = false;
  return;
}
#endif /* MEOS */

/**
 * @brief Get the random generator used by the data generator
 */
pg_prng_state *
prng_get_generation_rng(void)
{
  if (! MEOS_PRNG_INITIALIZED)
    prng_initialize();
  return &MEOS_GENERATION_RNG;
}

/**
 * @brief Get the random generator used by temporal aggregation
 */
pg_prng_state *
prng_get_aggregation_rng(void)
{
  if (! MEOS_PRNG_INITIALIZED)
    prng_initialize();
  return &MEOS_AGGREGATION_RNG;
}

/***************************************************************************/

/**
 * @brief Generate a uniformly distributed random double in [0, 1).
 */
inline double
meos_random_double(pg_prng_state *rng)
{
  return pg_prng_double(rng);
}

/**
 * @brief Generate an exponentially distributed random value.
 *
 * @param[in] rng Random generator
 * @param[in] mean Mean of the exponential distribution
 * @return Random value
 */
inline double
meos_random_exponential(pg_prng_state *rng, double mean)
{
  double u = pg_prng_double(rng);
  return -mean * log1p(-u);
}

/**
 * @brief Generate a binomially distributed random value.
 * @param[in] rng Random generator
 * @param[in] p Probability of success
 * @param[in] n Number of trials
 * @return Number of successes
 */
inline uint32
meos_random_binomial20_half(pg_prng_state *rng)
{
  uint32 result = 0;
  for (int i = 0; i < 20; i++)
    result += pg_prng_bool(rng);
  return result;
}

/***************************************************************************
 * Functions for the PROJ library
 ***************************************************************************/

/* Global variables keeping Proj context */

static MEOS_TLS PJ_CONTEXT *MEOS_PJ_CONTEXT = NULL;

/**
 * @brief Initialize the PROJ library
 */
static void
proj_initialize(void)
{
  if (! MEOS_PJ_CONTEXT)
    MEOS_PJ_CONTEXT = proj_context_create();
  return;
}

#if MEOS
/**
 * @brief Finalize the PROJ library
 */
static void
proj_finalize(void)
{
  proj_context_destroy(MEOS_PJ_CONTEXT);
  MEOS_PJ_CONTEXT = NULL;
  return;
}
#endif /* MEOS */

/**
 * @brief Get the random generator used by temporal aggregation
 */
PJ_CONTEXT *
proj_get_context(void)
{
  if (! MEOS_PJ_CONTEXT)
    proj_initialize();
  return MEOS_PJ_CONTEXT;
}

/***************************************************************************
 * Functions for the GEOS library
 ***************************************************************************/

#if GEOS

/* Per-thread GEOS context.  Each thread owns its own handle so concurrent
 * callers do not share GEOS state.  MEOS spatial helpers retrieve the
 * handle via geos_get_context() and use the reentrant GEOSXxx_r API. */

static MEOS_TLS GEOSContextHandle_t MEOS_GEOS_CONTEXT = NULL;

/**
 * @brief Initialize the GEOS library
 */
static void
geos_initialize(void)
{
  if (! MEOS_GEOS_CONTEXT)
  {
    MEOS_GEOS_CONTEXT = GEOS_init_r();
    GEOSContext_setNoticeHandler_r(MEOS_GEOS_CONTEXT, lwnotice);
    GEOSContext_setErrorHandler_r(MEOS_GEOS_CONTEXT, lwgeom_geos_error);
  }
  return;
}

#if MEOS
/**
 * @brief Finalize the GEOS library
 */
static void
geos_finalize(void)
{
  if (MEOS_GEOS_CONTEXT)
  {
    GEOS_finish_r(MEOS_GEOS_CONTEXT);
    MEOS_GEOS_CONTEXT = NULL;
  }
  lwgeom_geos_finalize();
  return;
}
#endif /* MEOS */

/**
 * @brief Return the per-thread GEOS context handle
 */
GEOSContextHandle_t
geos_get_context(void)
{
  if (! MEOS_GEOS_CONTEXT)
    geos_initialize();
  return MEOS_GEOS_CONTEXT;
}

#endif /* GEOS */

/*****************************************************************************/
#if MEOS
/*****************************************************************************/

/* Definitions taken from miscadmin.h */

/* valid DateStyle values */
#define USE_POSTGRES_DATES 0
#define USE_ISO_DATES      1
#define USE_SQL_DATES      2
#define USE_GERMAN_DATES   3
#define USE_XSD_DATES      4

/* valid DateOrder values taken */
#define DATEORDER_YMD      0
#define DATEORDER_DMY      1
#define DATEORDER_MDY      2

/*
 * IntervalStyles
 *   INTSTYLE_POSTGRES           Like Postgres < 8.4 when DateStyle = 'iso'
 *   INTSTYLE_POSTGRES_VERBOSE   Like Postgres < 8.4 when DateStyle != 'iso'
 *   INTSTYLE_SQL_STANDARD       SQL standard interval literals
 *   INTSTYLE_ISO_8601           ISO-8601-basic formatted intervals
 */
#define INTSTYLE_POSTGRES          0
#define INTSTYLE_POSTGRES_VERBOSE  1
#define INTSTYLE_SQL_STANDARD      2
#define INTSTYLE_ISO_8601          3

/* Global variables with default definitions taken from globals.c */

int DateStyle = USE_ISO_DATES;
int DateOrder = DATEORDER_MDY;
int IntervalStyle = INTSTYLE_POSTGRES;

/***************************************************************************
 * Definitions taken from pg_regress.h/c
 ***************************************************************************/

/* simple list of strings */
typedef struct _stringlist
{
  char *str;
  struct _stringlist *next;
} _stringlist;

/*
 * Add an item at the end of a stringlist.
 */
static void
add_stringlist_item(_stringlist **listhead, const char *str)
{
  _stringlist *newentry = palloc(sizeof(_stringlist));
  _stringlist *oldentry;

  newentry->str = pstrdup(str);
  newentry->next = NULL;
  if (*listhead == NULL)
    *listhead = newentry;
  else
  {
    for (oldentry = *listhead; oldentry->next; oldentry = oldentry->next)
       /* skip */ ;
    oldentry->next = newentry;
  }
}

/*
 * Free a stringlist.
 */
static void
free_stringlist(_stringlist **listhead)
{
  if (listhead == NULL || *listhead == NULL)
    return;
  if ((*listhead)->next)
    free_stringlist(&((*listhead)->next));
  free((*listhead)->str);
  free(*listhead);
  *listhead = NULL;
}

/*
 * Split a delimited string into a stringlist
 */
static void
split_to_stringlist(const char *s, const char *delim, _stringlist **listhead)
{
  char *sc = pstrdup(s);
  char *token = strtok(sc, delim);

  while (token)
  {
    add_stringlist_item(listhead, token);
    token = strtok(NULL, delim);
  }
  free(sc);
}

/***************************************************************************
 * Definitions derived from variable.c
 ***************************************************************************/

/*
 * DATESTYLE, DATEORDER, INTERVALSTYLE
 */

#define DATESTYLE_STR_MAXLEN 32
#define INTERVALSTYLE_STR_MAXLEN 32

/**
 * @brief Global constant array containing the datestyle strings
 */
static const char * _DATESTYLE_STRINGS[] =
{
  [USE_POSTGRES_DATES] = "Postgres",
  [USE_ISO_DATES] = "ISO",
  [USE_SQL_DATES] = "SQL",
  [USE_GERMAN_DATES] = "German",
  [USE_XSD_DATES] = "XSD"
};

/**
 * @brief Global constant array containing the dateorder strings
 */
static const char * _DATEORDER_STRINGS[] =
{
  [DATEORDER_YMD] = "YMD",
  [DATEORDER_DMY] = "DMY",
  [DATEORDER_MDY] = "MDY",
};

/**
 * @brief Global constant array containing the intervalstyle string
 */
static const char * _INTERVALSTYLE_STRINGS[] =
{
  [INTSTYLE_POSTGRES] = "postgres",
  [INTSTYLE_POSTGRES_VERBOSE] = "postgres_verbose",
  [INTSTYLE_SQL_STANDARD] = "sql_standard",
  [INTSTYLE_ISO_8601] = "iso_8601"
};

/**
 * @brief Return the string representation of the dateorder
 */
const char *
datestyle_string(int datestyle)
{
  return _DATESTYLE_STRINGS[datestyle];
}

/**
 * @brief Return the string representation of the dateorder
 */
const char *
dateorder_string(int dateorder)
{
  return _DATEORDER_STRINGS[dateorder];
}

/**
 * @brief Return the string representation of the intervalstyle
 */
const char *
intervalstyle_string(int intervalstyle)
{
  return _INTERVALSTYLE_STRINGS[intervalstyle];
}

/***************************************************************************/

/**
 * @brief Check a datestyle string
 */
static bool
check_datestyle(const char **newval, void **extra)
{
  int newDateStyle = DateStyle;
  int newDateOrder = DateOrder;
  bool have_style = false;
  bool have_order = false;
  bool ok = true;
  char *rawstring;
  int *myextra;
  _stringlist *elemlist = NULL;
  _stringlist *l;

  /* Need a modifiable copy of string */
  rawstring = pstrdup(*newval);

  /* Parse string into list of identifiers */
  split_to_stringlist(rawstring, ",", &elemlist);
  if (!(elemlist && elemlist->str && elemlist->str[0]))
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Invalid datestyle value: \"%s\"", *newval);
    return false;
  }

  for (l = elemlist; l; l = l->next)
  {
    char *tok = l->str;
    /* Trim leading white spaces if any */
    while(isspace((unsigned char) *tok)) tok++;

    if (pg_strcasecmp(tok, "ISO") == 0)
    {
      if (have_style && newDateStyle != USE_ISO_DATES)
        ok = false;    /* conflicting styles */
      newDateStyle = USE_ISO_DATES;
      have_style = true;
    }
    else if (pg_strcasecmp(tok, "SQL") == 0)
    {
      if (have_style && newDateStyle != USE_SQL_DATES)
        ok = false;    /* conflicting styles */
      newDateStyle = USE_SQL_DATES;
      have_style = true;
    }
    else if (pg_strncasecmp(tok, "POSTGRES", 8) == 0)
    {
      if (have_style && newDateStyle != USE_POSTGRES_DATES)
        ok = false;    /* conflicting styles */
      newDateStyle = USE_POSTGRES_DATES;
      have_style = true;
    }
    else if (pg_strcasecmp(tok, "GERMAN") == 0)
    {
      if (have_style && newDateStyle != USE_GERMAN_DATES)
        ok = false;    /* conflicting styles */
      newDateStyle = USE_GERMAN_DATES;
      have_style = true;
      /* GERMAN also sets DMY, unless explicitly overridden */
      if (!have_order)
        newDateOrder = DATEORDER_DMY;
    }
    else if (pg_strcasecmp(tok, "YMD") == 0)
    {
      if (have_order && newDateOrder != DATEORDER_YMD)
        ok = false;    /* conflicting orders */
      newDateOrder = DATEORDER_YMD;
      have_order = true;
    }
    else if (pg_strcasecmp(tok, "DMY") == 0 ||
         pg_strncasecmp(tok, "EURO", 4) == 0)
    {
      if (have_order && newDateOrder != DATEORDER_DMY)
        ok = false;    /* conflicting orders */
      newDateOrder = DATEORDER_DMY;
      have_order = true;
    }
    else if (pg_strcasecmp(tok, "MDY") == 0 ||
         pg_strcasecmp(tok, "US") == 0 ||
         pg_strncasecmp(tok, "NONEURO", 7) == 0)
    {
      if (have_order && newDateOrder != DATEORDER_MDY)
        ok = false;    /* conflicting orders */
      newDateOrder = DATEORDER_MDY;
      have_order = true;
    }
    else if (pg_strcasecmp(tok, "DEFAULT") == 0)
    {
      /*
       * We take the default value from PostgreSQL "ISO, MDY" and
       * recursively parse it.
       *
       * We can't simply "return check_datestyle(...)" because we need
       * to handle constructs like "DEFAULT, ISO".
       */
      const char *subval = "ISO, MDY";
      void *subextra = NULL;
      if (!check_datestyle(&subval, &subextra))
      {
        ok = false;
        break;
      }
      myextra = (int *) subextra;
      if (!have_style)
        newDateStyle = myextra[0];
      if (!have_order)
        newDateOrder = myextra[1];
      pfree(subextra);
    }
    else
    {
      meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
        "Unrecognized key word: \"%s\".", tok);
      pfree(rawstring);
      free_stringlist(&elemlist);
      return false;
    }
  }

  pfree(rawstring);
  free_stringlist(&elemlist);

  if (!ok)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Conflicting \"datestyle\" specifications.");
    return false;
  }

  /* Set up the "extra" struct actually used by assign_datestyle */
  myextra = (int *) palloc(sizeof(int) * 2);
  if (!myextra)
    return false;

  myextra[0] = newDateStyle;
  myextra[1] = newDateOrder;
  *extra = (void *) myextra;

  return true;
}

/**
 * @ingroup meos_setup
 * @brief Set the DateStyle
 */
bool
meos_set_datestyle(const char *newval, void *extra)
{
  if (! check_datestyle(&newval, &extra))
    return false;

  int *myextra = (int *) extra;
  DateStyle = myextra[0];
  DateOrder = myextra[1];
  return true;
}

/**
 * @ingroup meos_setup
 * @brief Get the DateStyle
 */
char *
meos_get_datestyle(void)
{
  char *result = palloc(DATESTYLE_STR_MAXLEN);
  if (! result)
    return NULL;
  snprintf(result, DATESTYLE_STR_MAXLEN, "%s, %s", datestyle_string(DateStyle),
    dateorder_string(DateStyle));
  return result;
}

/***************************************************************************/

/**
 * @brief Check an IntervalStyle string
 */
static bool
check_intervalstyle(const char *newval, int *extra)
{
  int newIntervalStyle = IntervalStyle;

  if (pg_strcasecmp(newval, "postgres") == 0)
    newIntervalStyle = INTSTYLE_POSTGRES;
  else if (pg_strcasecmp(newval, "postgres_verbose") == 0)
    newIntervalStyle = INTSTYLE_POSTGRES_VERBOSE;
  else if (pg_strncasecmp(newval, "sql_standard", 8) == 0)
    newIntervalStyle = INTSTYLE_SQL_STANDARD;
  else if (pg_strcasecmp(newval, "iso_8601") == 0)
    newIntervalStyle = INTSTYLE_ISO_8601;
  else
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Unrecognized key word: \"%s\".", newval);
    return false;
  }

  *extra = newIntervalStyle;

  return true;
}

/**
 * @ingroup meos_setup
 * @brief Set the IntervalStyle
 */
bool
meos_set_intervalstyle(const char *newval, int extra)
{
  if (! check_intervalstyle(newval, &extra))
    return false;

  IntervalStyle = extra;
  return true;
}

/**
 * @ingroup meos_setup
 * @brief Get the IntervalStyle
 */
char *
meos_get_intervalstyle(void)
{
  char *result = palloc(INTERVALSTYLE_STR_MAXLEN);
  if (! result)
    return NULL;
  snprintf(result, INTERVALSTYLE_STR_MAXLEN, "%s",
    intervalstyle_string(IntervalStyle));
  return result;
}

/*****************************************************************************/

/*****************************************************************************
 * liblwgeom message handlers
 *****************************************************************************/

/* Bound for a formatted liblwgeom message, matching liblwgeom's own
 * LW_MSG_MAXLEN, which is file-scope in lwutil.c and so not shared. */
#define MEOS_LW_MSG_MAXLEN 256

/**
 * @brief Report a liblwgeom error through the MEOS error channel
 * @details liblwgeom reports through a handler the embedder installs, and a
 * PostgreSQL backend installs one in @c mobilitydb_init that raises. A
 * standalone MEOS program installs none, so liblwgeom keeps its own default,
 * which writes the message to @c stderr and ends the process — the message
 * reaches the caller with no error code, and a host that meant to catch it
 * never gets the chance.
 * @note The error reaches the caller the way every other MEOS error does: the
 * installed error handler decides what follows. The default one ends the
 * process on @p ERROR, so a standalone program behaves as liblwgeom's own
 * default does. A host that installed the no-exit handler regains control with
 * @c meos_errno() set, which is what that handler exists to promise — a foreign
 * thread in a JVM (JNR-FFI on Spark or JMEOS) keeps its process.
 */
static void
meos_lwerror_handler(const char *fmt, va_list ap)
{
  char msg[MEOS_LW_MSG_MAXLEN + 1];
  vsnprintf(msg, sizeof(msg), fmt, ap);
  msg[MEOS_LW_MSG_MAXLEN] = '\0';
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "%s", msg);
}

/**
 * @brief Report a liblwgeom notice through the MEOS error channel
 * @details A notice is advisory, so this handler returns as liblwgeom expects.
 */
static void
meos_lwnotice_handler(const char *fmt, va_list ap)
{
  char msg[MEOS_LW_MSG_MAXLEN + 1];
  vsnprintf(msg, sizeof(msg), fmt, ap);
  msg[MEOS_LW_MSG_MAXLEN] = '\0';
  meos_error(WARNING, MEOS_ERR_INVALID_ARG_VALUE, "%s", msg);
}

/*****************************************************************************/

extern void init_database_collation(void);

/**
 * @ingroup meos_setup
 * @brief Initialize MEOS library
 */
void
meos_initialize(void)
{
  /* Install the default (libc) allocator before anything else can allocate */
  meos_initialize_allocator(NULL, NULL, NULL);
  meos_initialize_error_handler(NULL);
  /* Route liblwgeom's messages into the MEOS error channel, symmetric with the
   * PG backend's mobilitydb_init. The allocators stay liblwgeom's own: a
   * backend passes palloc, a standalone program keeps malloc. */
  lwgeom_set_handlers(NULL, NULL, NULL, meos_lwerror_handler,
    meos_lwnotice_handler);
  meos_initialize_timezone(NULL);
  /* Initialize collation */
  meos_initialize_collation();
  /* Initialize PROJ */
  proj_initialize();
  /* Initialize GEOS */
#if GEOS
  geos_initialize();
#endif
  /* Initialize the PostgreSQL pseudo-random number generators */
  prng_initialize();
#if POINTCLOUD
  /* Install the bundled libpc.a handlers so standalone MEOS programs
   * touching a pgPointCloud schema-aware path do not dereference a
   * NULL handler (symmetric with the PG backend's mobilitydb_init). */
  meos_initialize_pointcloud();
#endif
  return;
}

/**
 * @ingroup meos_setup
 * @brief Free the timezone cache
 */
void
meos_finalize(void)
{
  meos_finalize_timezone();
  /* Finalize PROJ SRS cache */
  meos_finalize_projsrs();
  /* Finalize collation */
  meos_finalize_collation();
#if JSON
  /* Finalize the list keeping the items to be freed after a JSON parsing */
  json_destroy_tofree();
#endif
#if NPOINT
  /* Finalize Ways cache */
  meos_finalize_ways();
#endif
  /* Finalize PROJ */
  proj_finalize();
  /* Finalize GEOS */
#if GEOS
  geos_finalize();
#endif
  /* Finalize the PostgreSQL pseudo-random number generators */
  prng_finalize();
  return;
}

/*****************************************************************************/
#endif /* MEOS */
/*****************************************************************************/
