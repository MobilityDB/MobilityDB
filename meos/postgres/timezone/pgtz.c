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
#include "postgres.h"

#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h> /* MEOS */
#include <common/hashfn.h> /* MEOS */
#include <sys/stat.h> /* MEOS */
// #include "datatype/timestamp.h" /* MEOS */
#include "utils/timestamp_def.h"
#include "pgtz.h"

/**
 * Structure to represent the timezone cache hash table, which extends
 * the `ENTRY` structure used by hsearch
 * https://man7.org/linux/man-pages/man3/hsearch.3.html
 * with the additional field `status` required by `simplehash`
 */
typedef struct
{
  char *key;     /**< timezone name (hashtable key) */
  void *data;    /**< pointer to the timezone structure */
  char status;   /**< hash status */
} tzentry;

static uint32 hash_string_pointer(const char *s);
#define SH_PREFIX tzcache
#define SH_ELEMENT_TYPE tzentry
#define SH_KEY_TYPE const char *
#define SH_KEY key
#define SH_HASH_KEY(tb, key) hash_string_pointer(key)
#define SH_EQUAL(tb, a, b) (strcmp(a, b) == 0)
#define SH_SCOPE static inline
#define SH_RAW_ALLOCATOR palloc0
#define SH_DEFINE
#define SH_DECLARE
#include <lib/simplehash.h>

/* Size of the timezone hash table */
#define TZCACHE_INITIAL_SIZE 32

/* Function in findtimezone.c */
extern const char *select_default_timezone(const char *share_path);

/* Current session timezone (controlled by TimeZone GUC) */
pg_tz *session_timezone = NULL;

/* Current log timezone (controlled by log_timezone GUC) */
// pg_tz *log_timezone = NULL; /* MEOS */

static bool scan_directory_ci(const char *dirname,
                const char *fname, int fnamelen,
                char *canonname, int canonnamelen);

/*
 * Helper function borrowed from PostgreSQL file `filemap.c`.
 */
static uint32
hash_string_pointer(const char *s)
{
  unsigned char *ss = (unsigned char *) s;
  return hash_bytes(ss, strlen(s));
}

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
  // strlcpy(fullname, pg_TZDIR(), sizeof(fullname)); /* MEOS */
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
    int result;

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
    int fnamelen;

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
    // strlcpy(canonname, fullname + orignamelen + 1, TZ_STRLEN_MAX + 1); /* MEOS */
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

  // dirdesc = AllocateDir(dirname); /* MEOS */
  dirdesc = opendir(dirname);

  // while ((direntry = ReadDirExtended(dirdesc, dirname, LOG)) != NULL) /* MEOS */
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
      // strlcpy(canonname, direntry->d_name, canonnamelen); /* MEOS */
      strncpy(canonname, direntry->d_name, canonnamelen);
      found = true;
      break;
    }
  }

  // FreeDir(dirdesc); /* MEOS */
  closedir(dirdesc);

  return found;
}

/*
 * We keep loaded timezones in a hashtable so we don't have to
 * load and parse the TZ definition file every time one is selected.
 * Because we want timezone names to be found case-insensitively,
 * the hash key is the uppercased name of the zone.
 * MEOS: We use a fixed size hash table instead of a dynamic hash table
 * as in the original PG code.
 */
/* MEOS */
// typedef struct {...} pg_tz_cache;

static tzcache_hash *timezone_cache = NULL;

static bool
init_timezone_hashtable(void)
{
  /* MEOS: Create the timezone hash table */
  timezone_cache = tzcache_create(TZCACHE_INITIAL_SIZE, NULL);

  if (!timezone_cache)
    return false;

  return true;
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
  // pg_tz_cache *tzp; /* MEOS */
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

  /* MEOS: Look for timezone in the cache */
  tzentry *entry = tzcache_lookup(timezone_cache, uppername);
  if (entry)
    /* Timezone found in cache, nothing more to do */
    return (pg_tz *) entry->data;

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

  /* MEOS: Fill the struct to be added to the hash table */
  bool found;
  entry = tzcache_insert(timezone_cache, uppername, &found);
  if (! found)
  {
    entry->key = strdup(uppername);
    entry->data = tz;
    return (pg_tz *) entry->data;
  }

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
  // char    tzname[128]; /* MEOS */
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
 * Initialize timezone cache
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
meos_initialize(const char *tz_str)
{
  if (tz_str == NULL || strlen(tz_str) == 0)
    /* fetch local timezone */
    tz_str = select_default_timezone(NULL);
  if (tz_str == NULL)
    meos_timezone_initialize("GMT");
  else
    meos_timezone_initialize(tz_str);
  return;
}

/*
 * Free the timezone cache
 */
void
meos_finalize(void)
{
  if (session_timezone)
    tzcache_destroy(timezone_cache);
  return;
}
