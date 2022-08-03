/*-------------------------------------------------------------------------
 *
 * pgtz.c
 *    Timezone Library Integration Functions
 *
 * Portions Copyright (c) 1996-2021, PostgreSQL Global Development Group
 *
 * IDENTIFICATION
 *    src/timezone/pgtz.c
 *
 *-------------------------------------------------------------------------
 */
#define _GNU_SOURCE
#include "postgres.h"

#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h> /* MobilityDB */
#include <sys/stat.h> /* MobilityDB */
#include <search.h> /* MobilityDB */
// #include "datatype/timestamp.h" /* MobilityDB */
#include "utils/timestamp_def.h"
#include "pgtz.h"

/* Function in findtimezone.c */
extern const char *select_default_timezone(const char *share_path);

/* Size of the timezone hash table manipulated by the POSIX hsearch() functions */
#define TZ_HTABLE_MAXSIZE 32

/* Current session timezone (controlled by TimeZone GUC) */
pg_tz *session_timezone = NULL;

/* Current log timezone (controlled by log_timezone GUC) */
// pg_tz *log_timezone = NULL;

static bool scan_directory_ci(const char *dirname,
                const char *fname, int fnamelen,
                char *canonname, int canonnamelen);

/*
 * Return full pathname of timezone data directory
 */
static const char *
pg_TZDIR(void)
{
#ifndef SYSTEMTZDIR
  /* normal case: timezone stuff is under our share dir */
  static bool done_tzdir = false;
  static char tzdir[MAXPGPATH];

  if (done_tzdir)
    return tzdir;

  get_share_path(my_exec_path, tzdir);
  strlcpy(tzdir + strlen(tzdir), "/timezone", MAXPGPATH - strlen(tzdir));

  done_tzdir = true;
  return tzdir;
#else
  /* we're configured to use system's timezone database */
  return SYSTEMTZDIR;
#endif
}


/*
 * Given a timezone name, open() the timezone data file.  Return the
 * file descriptor if successful, -1 if not.
 *
 * The input name is searched for case-insensitively (we assume that the
 * timezone database does not contain case-equivalent names).
 *
 * If "canonname" is not NULL, then on success the canonical spelling of the
 * given name is stored there (the buffer must be > TZ_STRLEN_MAX bytes!).
 */
int
pg_open_tzfile(const char *name, char *canonname)
{
  const char *fname;
  char    fullname[MAXPGPATH];
  int      fullnamelen;
  int      orignamelen;

  /* Initialize fullname with base name of tzdata directory */
  // strlcpy(fullname, pg_TZDIR(), sizeof(fullname)); /* MobilityDB */
  strncpy(fullname, pg_TZDIR(), sizeof(fullname));
  orignamelen = fullnamelen = strlen(fullname);

  if (fullnamelen + 1 + strlen(name) >= MAXPGPATH)
    return -1;        /* not gonna fit */

  /*
   * If the caller doesn't need the canonical spelling, first just try to
   * open the name as-is.  This can be expected to succeed if the given name
   * is already case-correct, or if the filesystem is case-insensitive; and
   * we don't need to distinguish those situations if we aren't tasked with
   * reporting the canonical spelling.
   */
  if (canonname == NULL)
  {
    int      result;

    fullname[fullnamelen] = '/';
    /* test above ensured this will fit: */
    strcpy(fullname + fullnamelen + 1, name);
    result = open(fullname, O_RDONLY | PG_BINARY, 0);
    if (result >= 0)
      return result;
    /* If that didn't work, fall through to do it the hard way */
    fullname[fullnamelen] = '\0';
  }

  /*
   * Loop to split the given name into directory levels; for each level,
   * search using scan_directory_ci().
   */
  fname = name;
  for (;;)
  {
    const char *slashptr;
    int      fnamelen;

    slashptr = strchr(fname, '/');
    if (slashptr)
      fnamelen = slashptr - fname;
    else
      fnamelen = strlen(fname);
    if (!scan_directory_ci(fullname, fname, fnamelen,
                 fullname + fullnamelen + 1,
                 MAXPGPATH - fullnamelen - 1))
      return -1;
    fullname[fullnamelen++] = '/';
    fullnamelen += strlen(fullname + fullnamelen);
    if (slashptr)
      fname = slashptr + 1;
    else
      break;
  }

  if (canonname)
    // strlcpy(canonname, fullname + orignamelen + 1, TZ_STRLEN_MAX + 1); /* MobilityDB */
    strncpy(canonname, fullname + orignamelen + 1, TZ_STRLEN_MAX + 1);

  return open(fullname, O_RDONLY | PG_BINARY, 0);
}

/* Function adapted from file fd.c */
/*
 * Alternate version of ReadDir that allows caller to specify the elevel
 * for any error report (whether it's reporting an initial failure of
 * AllocateDir or a subsequent directory read failure).
 *
 * If elevel < ERROR, returns NULL after any error.  With the normal coding
 * pattern, this will result in falling out of the loop immediately as
 * though the directory contained no (more) entries.
 */
struct dirent *
ReadDir(DIR *dir, const char *dirname)
{
  struct dirent *dent;

  /* Give a generic message for AllocateDir failure, if caller didn't */
  if (dir == NULL)
  {
    elog(ERROR, "could not open directory \"%s\": %m", dirname);
    return NULL;
  }

  errno = 0;
  if ((dent = readdir(dir)) != NULL)
    return dent;

  if (errno)
    elog(ERROR, "could not read directory \"%s\": %m", dirname);
  return NULL;
}

/*
 * Scan specified directory for a case-insensitive match to fname
 * (of length fnamelen --- fname may not be null terminated!).  If found,
 * copy the actual filename into canonname and return true.
 */
static bool
scan_directory_ci(const char *dirname, const char *fname, int fnamelen,
  char *canonname, int canonnamelen)
{
  bool found = false;
  DIR *dirdesc;
  struct dirent *direntry;

  // dirdesc = AllocateDir(dirname); /* MobilityDB */
  dirdesc = opendir(dirname);

  // while ((direntry = ReadDirExtended(dirdesc, dirname, LOG)) != NULL) /* MobilityDB */
  while ((direntry = ReadDir(dirdesc, dirname)) != NULL)
  {
    /*
     * Ignore . and .., plus any other "hidden" files.  This is a security
     * measure to prevent access to files outside the timezone directory.
     */
    if (direntry->d_name[0] == '.')
      continue;

    if (strlen(direntry->d_name) == (size_t) fnamelen &&
      pg_strncasecmp(direntry->d_name, fname, fnamelen) == 0)
    {
      /* Found our match */
      // strlcpy(canonname, direntry->d_name, canonnamelen); /* MobilityDB */
      strncpy(canonname, direntry->d_name, canonnamelen);
      found = true;
      break;
    }
  }

  // FreeDir(dirdesc); /* MobilityDB */
  closedir(dirdesc);

  return found;
}

/*
 * We keep loaded timezones in a hashtable so we don't have to
 * load and parse the TZ definition file every time one is selected.
 * Because we want timezone names to be found case-insensitively,
 * the hash key is the uppercased name of the zone.
 * MobilityDB: We use a fixed size hash table instead of a dynamic hash table
 * as in the original PG code.
 */

static struct hsearch_data *timezone_cache = NULL;

static bool
init_timezone_hashtable(void)
{
  timezone_cache = palloc0(sizeof(struct hsearch_data));
  if (!timezone_cache)
    return false;

#ifdef NO_HSEARCH_R
  if (hcreate(TZ_HTABLE_MAXSIZE))
#else
  if (hcreate_r(TZ_HTABLE_MAXSIZE, timezone_cache))
#endif /* NO_HSEARCH_R */
    return true;

  return false;
}

/*
 * Load a timezone from file or from cache.
 * Does not verify that the timezone is acceptable!
 *
 * "GMT" is always interpreted as the tzparse() definition, without attempting
 * to load a definition from the filesystem.  This has a number of benefits:
 * 1. It's guaranteed to succeed, so we don't have the failure mode wherein
 * the bootstrap default timezone setting doesn't work (as could happen if
 * the OS attempts to supply a leap-second-aware version of "GMT").
 * 2. Because we aren't accessing the filesystem, we can safely initialize
 * the "GMT" zone definition before my_exec_path is known.
 * 3. It's quick enough that we don't waste much time when the bootstrap
 * default timezone setting is later overridden from postgresql.conf.
 */
pg_tz *
pg_tzset(const char *name)
{
  // pg_tz_cache *tzp; /* MobilityDB */
  ENTRY e;
  ENTRY *ep = &e;
  struct state tzstate;
  char uppername[TZ_STRLEN_MAX + 1];
  char canonname[TZ_STRLEN_MAX + 1];
  char *p;

  if (strlen(name) > TZ_STRLEN_MAX)
    return NULL;      /* not going to fit */

  if (!timezone_cache)
    if (!init_timezone_hashtable())
      return NULL;

  /*
   * Upcase the given name to perform a case-insensitive hashtable search.
   * (We could alternatively downcase it, but we prefer upcase so that we
   * can get consistently upcased results from tzparse() in case the name is
   * a POSIX-style timezone spec.)
   */
  p = uppername;
  while (*name)
    *p++ = pg_toupper((unsigned char) *name++);
  *p = '\0';

  /* Look for timezone in the cache */
  e.key = uppername;
#ifdef NO_HSEARCH_R
  ep = hsearch(e, FIND);
  if (ep != NULL)
#else
  if (hsearch_r(e, FIND, &ep, timezone_cache))
#endif /* NO_HSEARCH_R */
    return (pg_tz *) ep->data;

  /*
   * "GMT" is always sent to tzparse(), as per discussion above.
   */
  if (strcmp(uppername, "GMT") == 0)
  {
    if (!tzparse(uppername, &tzstate, true))
    {
      /* This really, really should not happen ... */
      elog(ERROR, "could not initialize GMT time zone");
    }
    /* Use uppercase name as canonical */
    strcpy(canonname, uppername);
  }
  else if (tzload(uppername, canonname, &tzstate, true) != 0)
  {
    if (uppername[0] == ':' || !tzparse(uppername, &tzstate, false))
    {
      /* Unknown timezone. Fail our call instead of loading GMT! */
      return NULL;
    }
    /* For POSIX timezone specs, use uppercase name as canonical */
    strcpy(canonname, uppername);
  }

  /* Save timezone in the cache */
  pg_tz *tz = palloc(sizeof(pg_tz));
  strcpy(tz->TZname, canonname);
  memcpy(&tz->state, &tzstate, sizeof(tzstate));

  e.key = strdup(uppername);
  e.data = tz;
#ifdef NO_HSEARCH_R
  ep = hsearch(e, ENTER);
  if (ep != NULL)
#else
  if (hsearch_r(e, ENTER, &ep, timezone_cache))
#endif /* NO_HSEARCH_R */
    return (pg_tz *) ep->data;

  return NULL;
}

/*
 * Load a fixed-GMT-offset timezone.
 * This is used for SQL-spec SET TIME ZONE INTERVAL 'foo' cases.
 * It's otherwise equivalent to pg_tzset().
 *
 * The GMT offset is specified in seconds, positive values meaning west of
 * Greenwich (ie, POSIX not ISO sign convention).  However, we use ISO
 * sign convention in the displayable abbreviation for the zone.
 *
 * Caution: this can fail (return NULL) if the specified offset is outside
 * the range allowed by the zic library.
 */
pg_tz *
pg_tzset_offset(long gmtoffset)
{
  long    absoffset = (gmtoffset < 0) ? -gmtoffset : gmtoffset;
  char    offsetstr[64];
  // char    tzname[128]; /* MobilityDB */
  char    tzname[256];

  snprintf(offsetstr, sizeof(offsetstr),
       "%02ld", absoffset / SECS_PER_HOUR);
  absoffset %= SECS_PER_HOUR;
  if (absoffset != 0)
  {
    snprintf(offsetstr + strlen(offsetstr),
         sizeof(offsetstr) - strlen(offsetstr),
         ":%02ld", absoffset / SECS_PER_MINUTE);
    absoffset %= SECS_PER_MINUTE;
    if (absoffset != 0)
      snprintf(offsetstr + strlen(offsetstr),
           sizeof(offsetstr) - strlen(offsetstr),
           ":%02ld", absoffset);
  }
  if (gmtoffset > 0)
    snprintf(tzname, sizeof(tzname), "<-%s>+%s",
         offsetstr, offsetstr);
  else
    snprintf(tzname, sizeof(tzname), "<+%s>-%s",
         offsetstr, offsetstr);

  return pg_tzset(tzname);
}

/*
 * Initialize timezone library
 */
void
meos_timezone_initialize(const char *name)
{
  session_timezone = pg_tzset(name);
  if (! session_timezone)
    elog(ERROR, "Failed to initialize local timezone");
  return;
}

/*
 * Initialize timezone library
 */
void
meos_initialize(void)
{
  const char *tz_str = select_default_timezone(NULL);
  if (tz_str == NULL)
    meos_timezone_initialize("GMT");
  else
    meos_timezone_initialize(tz_str);
  return;
}

/*
 * Initialize timezone library
 */
void
meos_finish(void)
{
  if (session_timezone)
#ifdef NO_HSEARCH_R
    hdestroy();
#else
    hdestroy_r(timezone_cache);
#endif
  return;
}


