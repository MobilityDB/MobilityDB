/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 */

/* C */
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
/* Proj */
#include <proj.h>
/* MEOS */
#include <meos.h>
// #include <meos_internal.h>

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

/* Global variables keeping Proj context */

PJ_CONTEXT *_PJ_CONTEXT;

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
void
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
  if ((*listhead)->next != NULL)
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

/**
 * @brief Global array containing the datestyle strings
 */
char * _DATESTYLE_STRINGS[] =
{
  [USE_POSTGRES_DATES] = "Postgres",
  [USE_ISO_DATES] = "ISO",
  [USE_SQL_DATES] = "SQL",
  [USE_GERMAN_DATES] = "German",
  [USE_XSD_DATES] = "XSD"
};

/**
 * @brief Global array containing the dateorder strings
 */
char * _DATEORDER_STRINGS[] =
{
  [DATEORDER_YMD] = "YMD",
  [DATEORDER_DMY] = "DMY",
  [DATEORDER_MDY] = "MDY",
};

/**
 * @brief Global array containing the intervalstyle string
 */
char * _INTERVALSTYLE_STRINGS[] =
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
check_datestyle(char **newval, void **extra)
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
      "Invalid datestyle value: \"%s\"", newval);
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
      char *subval = "ISO, MDY";
      void *subextra = NULL;
      if (!check_datestyle(&subval, &subextra))
      {
        pfree(subval);
        ok = false;
        break;
      }
      myextra = (int *) subextra;
      if (!have_style)
        newDateStyle = myextra[0];
      if (!have_order)
        newDateOrder = myextra[1];
      pfree(subval);
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
  myextra = (int *) palloc(2 * sizeof(int));
  if (!myextra)
    return false;

  myextra[0] = newDateStyle;
  myextra[1] = newDateOrder;
  *extra = (void *) myextra;

  return true;
}

/**
 * @brief Set the DateStyle
 */
bool
meos_set_datestyle(char *newval, void *extra)
{
  if (! check_datestyle(&newval, &extra))
    return false;

  int *myextra = (int *) extra;
  DateStyle = myextra[0];
  DateOrder = myextra[1];
  return true;
}

/**
 * @brief Get the DateStyle
 */
char *
meos_get_datestyle(void)
{
  char *result = palloc(32);
  if (! result)
    return NULL;
  sprintf(result, "%s, %s", datestyle_string(DateStyle),
    dateorder_string(DateStyle));
  return result;
}

/***************************************************************************/

/**
 * @brief Check an IntervalStyle string
 */
static bool
check_intervalstyle(char *newval, int *extra)
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
 * @brief Set the IntervalStyle
 */
bool
meos_set_intervalstyle(char *newval, int extra)
{
  if (! check_intervalstyle(newval, &extra))
    return false;

  IntervalStyle = extra;
  return true;
}

/**
 * @brief Get the IntervalStyle
 */
char *
meos_get_intervalstyle(void)
{
  char *result = palloc(32);
  if (! result)
    return NULL;
  sprintf(result, "%s", intervalstyle_string(IntervalStyle));
  return result;
}

/*****************************************************************************/

/*
 * Initialize MEOS library
 */
void
meos_initialize(const char *tz_str, error_handler_fn err_handler)
{
  meos_initialize_error_handler(err_handler);
  meos_initialize_timezone(tz_str);
  /* Initialize PROJ */
  _PJ_CONTEXT = proj_context_create();
  return;
}

/*
 * Free the timezone cache
 */
void
meos_finalize(void)
{
  meos_finalize_timezone();
  /* Finalize PROJ */
  proj_context_destroy(_PJ_CONTEXT);
  return;
}

/*****************************************************************************/
#endif /* MEOS */
