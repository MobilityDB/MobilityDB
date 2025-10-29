/*-------------------------------------------------------------------------
 *
 * pg_conversion.c
 *    routines to support manipulation of the pg_conversion relation
 *
 * Portions Copyright (c) 1996-2025, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *    src/backend/catalog/pg_conversion.c
 *
 *-------------------------------------------------------------------------
 */

#include <string.h>

#include "postgres.h"
#include "catalog/pg_conversion.h"
#include "utils/builtins.h"
#include "utils/mb/conversion_procs.h"
#include "utils/mb/pg_wchar.h"

// #include "access/htup_details.h"
// #include "access/table.h"
// #include "catalog/catalog.h"
// #include "catalog/dependency.h"
// #include "catalog/indexing.h"
// #include "catalog/objectaccess.h"
// #include "catalog/pg_conversion.h"
// #include "catalog/pg_namespace.h"
// #include "catalog/pg_proc.h"
// #include "utils/mb/pg_wchar.h"
// #include "utils/builtins.h"
// #include "utils/catcache.h"
// #include "utils/rel.h"
// #include "utils/syscache.h"

/*****************************************************************************
 * Definitions for reading the pg_conversion.csv file
 *****************************************************************************/

/* Maximum length in characters of a header record in the input CSV file */
#define MAX_LENGTH_HEADER 1024
/* Maximum length in characters of a geometry in the input data */
#define MAX_LENGTH_CONV_RECORD 156
/* Location of the pg_conversion.csv file */

char *PG_CONVERSION_CSV = "/usr/local/share/pg_conversion.csv";

/**
 * @brief Set the location of the PG_CONVERSION_CSV files
 */
void
meos_set_pg_conversion_csv(const char *path)
{
  PG_CONVERSION_CSV = malloc(strlen(path) + 1);
  strcpy(PG_CONVERSION_CSV, path);
}

/**
 * @brief Structure that represents a record of the PG_CONVERSION_CSV file 
 */
typedef struct
{
  int32 conoid;
  char conname[64];
  char conforencoding[16];
  char contoencoding[16];
  char conproc[64];
} pg_conversion_record;

/**
 * @brief Structure that represents an element of the PgEncMap array 
 */
struct EnumMap {
  const char* name;
  pg_enc value;
};

/**
 * @brief Enumeration map that associates an enum value with its string
 * representation
 * @note Theis map MUST be kept synchronized with the enum definition in file
 * pg_wchar.h 
 */
struct EnumMap PgEncMap[] = {
  {"PG_SQL_ASCII", PG_SQL_ASCII},
  {"PG_EUC_JP", PG_EUC_JP},
  {"PG_EUC_CN", PG_EUC_CN},
  {"PG_EUC_KR", PG_EUC_KR},
  {"PG_EUC_TW", PG_EUC_TW},
  {"PG_EUC_JIS_2004", PG_EUC_JIS_2004},
  {"PG_UTF8", PG_UTF8},
  {"PG_MULE_INTERNAL", PG_MULE_INTERNAL},
  {"PG_LATIN1", PG_LATIN1},
  {"PG_LATIN2", PG_LATIN2},
  {"PG_LATIN3", PG_LATIN3},
  {"PG_LATIN4", PG_LATIN4},
  {"PG_LATIN5", PG_LATIN5},
  {"PG_LATIN6", PG_LATIN6},
  {"PG_LATIN7", PG_LATIN7},
  {"PG_LATIN8", PG_LATIN8},
  {"PG_LATIN9", PG_LATIN9},
  {"PG_LATIN10", PG_LATIN10},
  {"PG_WIN1256", PG_WIN1256},
  {"PG_WIN1258", PG_WIN1258},
  {"PG_WIN866", PG_WIN866},
  {"PG_WIN874", PG_WIN874},
  {"PG_KOI8R", PG_KOI8R},
  {"PG_WIN1251", PG_WIN1251},
  {"PG_WIN1252", PG_WIN1252},
  {"PG_ISO_8859_5", PG_ISO_8859_5},
  {"PG_ISO_8859_6", PG_ISO_8859_6},
  {"PG_ISO_8859_7", PG_ISO_8859_7},
  {"PG_ISO_8859_8", PG_ISO_8859_8},
  {"PG_WIN1250", PG_WIN1250},
  {"PG_WIN1253", PG_WIN1253},
  {"PG_WIN1254", PG_WIN1254},
  {"PG_WIN1255", PG_WIN1255},
  {"PG_WIN1257", PG_WIN1257},
  {"PG_KOI8U", PG_KOI8U},
  {"PG_SJIS", PG_SJIS},
  {"PG_BIG5", PG_BIG5},
  {"PG_GBK", PG_GBK},
  {"PG_UHC", PG_UHC},
  {"PG_GB18030", PG_GB18030},
  {"PG_JOHAB", PG_JOHAB},
  {"PG_SHIFT_JIS_2004", PG_SHIFT_JIS_2004}, 
};

/**
 * @brief Function that uses the enumeration map above to convert the enum
 * string value read in the file pg_conversion.csv into the corresponding
 * enum value
 */
int 
stringToEnum(const char* str)
{
  for (size_t i = 0; i < sizeof(PgEncMap) / sizeof(PgEncMap[0]); ++i)
  {
    if (strcmp(str, PgEncMap[i].name) == 0) {
      return PgEncMap[i].value;
    }
  }
  return -1; /* Error indicator */
}

/*****************************************************************************/

/**
 * @brief Structure that represents an element of the PgFuncMap array 
 */
struct FuncMap {
  const char *name;
  pg_con_fn func;
};

/**
 * @brief Enumeration map that associates an enum value with its string
 * representation
 * @note Theis map MUST be kept synchronized with the enum definition in file
 * pg_wchar.h 
 */
struct FuncMap PgFuncMap[] = {
  {"koi8r_to_mic", koi8r_to_mic},
  {"mic_to_koi8r", mic_to_koi8r},
  {"iso_to_mic", iso_to_mic},
  {"mic_to_iso", mic_to_iso},
  {"win1251_to_mic", win1251_to_mic},
  {"mic_to_win1251", mic_to_win1251},
  {"win866_to_mic", win866_to_mic},
  {"mic_to_win866", mic_to_win866},
  {"koi8r_to_win1251", koi8r_to_win1251},
  {"win1251_to_koi8r", win1251_to_koi8r},
  {"koi8r_to_win866", koi8r_to_win866},
  {"win866_to_koi8r", win866_to_koi8r},
  {"win866_to_win1251", win866_to_win1251},
  {"win1251_to_win866", win1251_to_win866},
  {"iso_to_koi8r", iso_to_koi8r},
  {"koi8r_to_iso", koi8r_to_iso},
  {"iso_to_win1251", iso_to_win1251},
  {"win1251_to_iso", win1251_to_iso},
  {"iso_to_win866", iso_to_win866},
  {"win866_to_iso", win866_to_iso},
  {"euc_cn_to_mic", euc_cn_to_mic},
  {"mic_to_euc_cn", mic_to_euc_cn},
  {"euc_jp_to_sjis", euc_jp_to_sjis},
  {"sjis_to_euc_jp", sjis_to_euc_jp},
  {"euc_jp_to_mic", euc_jp_to_mic},
  {"sjis_to_mic", sjis_to_mic},
  {"mic_to_euc_jp", mic_to_euc_jp},
  {"mic_to_sjis", mic_to_sjis},
  {"euc_kr_to_mic", euc_kr_to_mic},
  {"mic_to_euc_kr", mic_to_euc_kr},
  {"euc_tw_to_big5", euc_tw_to_big5},
  {"big5_to_euc_tw", big5_to_euc_tw},
  {"euc_tw_to_mic", euc_tw_to_mic},
  {"big5_to_mic", big5_to_mic},
  {"mic_to_euc_tw", mic_to_euc_tw},
  {"mic_to_big5", mic_to_big5},
  {"latin2_to_mic", latin2_to_mic},
  {"mic_to_latin2", mic_to_latin2},
  {"win1250_to_mic", win1250_to_mic},
  {"mic_to_win1250", mic_to_win1250},
  {"latin2_to_win1250", latin2_to_win1250},
  {"win1250_to_latin2", win1250_to_latin2},
  {"latin1_to_mic", latin1_to_mic},
  {"mic_to_latin1", mic_to_latin1},
  {"latin3_to_mic", latin3_to_mic},
  {"mic_to_latin3", mic_to_latin3},
  {"latin4_to_mic", latin4_to_mic},
  {"mic_to_latin4", mic_to_latin4},
  {"big5_to_utf8", big5_to_utf8},
  {"utf8_to_big5", utf8_to_big5},
  {"utf8_to_koi8r", utf8_to_koi8r},
  {"koi8r_to_utf8", koi8r_to_utf8},
  {"utf8_to_koi8u", utf8_to_koi8u},
  {"koi8u_to_utf8", koi8u_to_utf8},
  {"utf8_to_win", utf8_to_win},
  {"win_to_utf8", win_to_utf8},
  {"utf8_to_win", utf8_to_win},
  {"win_to_utf8", win_to_utf8},
  {"utf8_to_win", utf8_to_win},
  {"win_to_utf8", win_to_utf8},
  {"utf8_to_win", utf8_to_win},
  {"win_to_utf8", win_to_utf8},
  {"utf8_to_win", utf8_to_win},
  {"win_to_utf8", win_to_utf8},
  {"utf8_to_win", utf8_to_win},
  {"win_to_utf8", win_to_utf8},
  {"utf8_to_win", utf8_to_win},
  {"win_to_utf8", win_to_utf8},
  {"utf8_to_win", utf8_to_win},
  {"win_to_utf8", win_to_utf8},
  {"utf8_to_win", utf8_to_win},
  {"win_to_utf8", win_to_utf8},
  {"utf8_to_win", utf8_to_win},
  {"win_to_utf8", win_to_utf8},
  {"utf8_to_win", utf8_to_win},
  {"win_to_utf8", win_to_utf8},
  {"euc_cn_to_utf8", euc_cn_to_utf8},
  {"utf8_to_euc_cn", utf8_to_euc_cn},
  {"euc_jp_to_utf8", euc_jp_to_utf8},
  {"utf8_to_euc_jp", utf8_to_euc_jp},
  {"euc_kr_to_utf8", euc_kr_to_utf8},
  {"utf8_to_euc_kr", utf8_to_euc_kr},
  {"euc_tw_to_utf8", euc_tw_to_utf8},
  {"utf8_to_euc_tw", utf8_to_euc_tw},
  {"gb18030_to_utf8", gb18030_to_utf8},
  {"utf8_to_gb18030", utf8_to_gb18030},
  {"gbk_to_utf8", gbk_to_utf8},
  {"utf8_to_gbk", utf8_to_gbk},
  {"utf8_to_iso8859", utf8_to_iso8859},
  {"iso8859_to_utf8", iso8859_to_utf8},
  {"utf8_to_iso8859", utf8_to_iso8859},
  {"iso8859_to_utf8", iso8859_to_utf8},
  {"utf8_to_iso8859", utf8_to_iso8859},
  {"iso8859_to_utf8", iso8859_to_utf8},
  {"utf8_to_iso8859", utf8_to_iso8859},
  {"iso8859_to_utf8", iso8859_to_utf8},
  {"utf8_to_iso8859", utf8_to_iso8859},
  {"iso8859_to_utf8", iso8859_to_utf8},
  {"utf8_to_iso8859", utf8_to_iso8859},
  {"iso8859_to_utf8", iso8859_to_utf8},
  {"utf8_to_iso8859", utf8_to_iso8859},
  {"iso8859_to_utf8", iso8859_to_utf8},
  {"utf8_to_iso8859", utf8_to_iso8859},
  {"iso8859_to_utf8", iso8859_to_utf8},
  {"utf8_to_iso8859", utf8_to_iso8859},
  {"iso8859_to_utf8", iso8859_to_utf8},
  {"utf8_to_iso8859", utf8_to_iso8859},
  {"iso8859_to_utf8", iso8859_to_utf8},
  {"utf8_to_iso8859", utf8_to_iso8859},
  {"iso8859_to_utf8", iso8859_to_utf8},
  {"utf8_to_iso8859", utf8_to_iso8859},
  {"iso8859_to_utf8", iso8859_to_utf8},
  {"utf8_to_iso8859", utf8_to_iso8859},
  {"iso8859_to_utf8", iso8859_to_utf8},
  {"iso8859_1_to_utf8", iso8859_1_to_utf8},
  {"utf8_to_iso8859_1", utf8_to_iso8859_1},
  {"johab_to_utf8", johab_to_utf8},
  {"utf8_to_johab", utf8_to_johab},
  {"sjis_to_utf8", sjis_to_utf8},
  {"utf8_to_sjis", utf8_to_sjis},
  {"uhc_to_utf8", uhc_to_utf8},
  {"utf8_to_uhc", utf8_to_uhc},
  {"euc_jis_2004_to_utf8", euc_jis_2004_to_utf8},
  {"utf8_to_euc_jis_2004", utf8_to_euc_jis_2004},
  {"shift_jis_2004_to_utf8", shift_jis_2004_to_utf8},
  {"utf8_to_shift_jis_2004", utf8_to_shift_jis_2004},
  {"euc_jis_2004_to_shift_jis_2004", euc_jis_2004_to_shift_jis_2004},
  {"shift_jis_2004_to_euc_jis_2004", shift_jis_2004_to_euc_jis_2004},
};

/**
 * @brief Function that uses the function map above to convert the function
 * name read in the file pg_conversion.csv into the corresponding function
 * point
 */
pg_con_fn 
stringToFunc(const char* str)
{
  for (size_t i = 0; i < sizeof(PgFuncMap) / sizeof(PgFuncMap[0]); ++i)
  {
    if (strcmp(str, PgFuncMap[i].name) == 0) {
      return PgFuncMap[i].func;
    }
  }
  return NULL; /* Error indicator */
}

/*****************************************************************************/

/**
 * @brief Find "default" conversion function from the first encoding to the
 * second encoding
 * @return If found, returns a pointer to the function, otherwise NULL.
 */
pg_con_fn
FindDefaultConversion(int32 for_encoding, int32 to_encoding)
{
  FILE *file = fopen(PG_CONVERSION_CSV, "r");
  if (! file)
  {
    elog(ERROR, "Cannot open the pg_conversion.csv file");
    return NULL;
  }

  bool found = false;
  pg_conversion_record rec;
  pg_con_fn result;

  /* Read the first line of the file with the headers */
  char header_buffer[MAX_LENGTH_HEADER];
  int read = fscanf(file, "%1023s\n", header_buffer);
  if (ferror(file) || read != 1)
  {
    elog(ERROR, "Error reading the pg_conversion.csv file");
    return NULL;
  }

  /* Continue reading the file */
  do
  {
    /* Read each line from the file */
    read = fscanf(file, "%d,%63[^,^\n],%15[^,^\n],%15[^,^\n],%63[^,^\n]\n",
      &rec.conoid, rec.conname, rec.conforencoding, rec.contoencoding,
      rec.conproc);
    if (ferror(file))
    {
      elog(ERROR, "Error reading the pg_conversion.csv file");
      return NULL;
    }

    /* Ignore the records with NULL values */
    if (read == 5 && for_encoding == stringToEnum(rec.conforencoding) &&
        to_encoding == stringToEnum(rec.conforencoding))
    {
      result = stringToFunc(rec.conproc) ;
      found = true;
    }
  } while (! feof(file));

  /* Close the input file */
  fclose(file);
  
  if (! found)
  {
    elog(ERROR, "Conversion function from %d to %d was not found\n", 
      for_encoding, to_encoding);
    return NULL;
  }

  return result;
}

/*****************************************************************************/
