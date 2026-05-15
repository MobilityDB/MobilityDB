/***********************************************************************
 * pc_pgsql.h
 *
 *  Common header file for all PgSQL pointcloud functions.
 *
 *  PgSQL Pointcloud is free and open source software provided
 *  by the Government of Canada
 *  Copyright (c) 2013 Natural Resources Canada
 *
 ***********************************************************************/

#include "pc_api.h"

#include "postgres.h"
#include "utils/elog.h"

/* Try to move these down */
#include "catalog/pg_type.h" /* for CSTRINGOID */
#include "lib/stringinfo.h"  /* For binary input */
#include "utils/array.h"
#include "utils/builtins.h" /* for pg_atoi */

#define PG_GETARG_SERPOINT_P(argnum)                                           \
  (SERIALIZED_POINT *)PG_DETOAST_DATUM(PG_GETARG_DATUM(argnum))
#define PG_GETARG_SERPATCH_P(argnum)                                           \
  (SERIALIZED_PATCH *)PG_DETOAST_DATUM(PG_GETARG_DATUM(argnum))

#define PG_GETHEADER_SERPATCH_P(argnum)                                        \
  (SERIALIZED_PATCH *)PG_DETOAST_DATUM_SLICE(PG_GETARG_DATUM(argnum), 0,       \
                                             sizeof(SERIALIZED_PATCH))

#define PG_GETHEADERX_SERPATCH_P(argnum, extra)                                \
  (SERIALIZED_PATCH *)PG_DETOAST_DATUM_SLICE(PG_GETARG_DATUM(argnum), 0,       \
                                             sizeof(SERIALIZED_PATCH) + extra)

#define PG_GETHEADER_STATS_P(argnum, statsize)                                 \
  (uint8_t *)(((SERIALIZED_PATCH *)PG_DETOAST_DATUM_SLICE(                     \
                   PG_GETARG_DATUM(argnum), 0,                                 \
                   sizeof(SERIALIZED_PATCH) + statsize))                       \
                  ->data)

#define AUTOCOMPRESS_NO 0
#define AUTOCOMPRESS_YES 1

typedef struct
{
  char *schema;
  char *formats;
  char *formats_srid;
  char *formats_schema;
} PC_CONSTANTS;

/**
 * Serialized point type for clouds. Variable length, because there can be
 * an arbitrary number of dimensions. The pcid is a foreign key
 * reference to the POINTCLOUD_SCHEMAS table, where
 * the underlying structure of the data is described in XML,
 * the spatial reference system is indicated, and the data
 * packing scheme is indicated.
 */
typedef struct
{
  uint32_t size;
  uint32_t pcid;
  uint8_t data[1];
} SERIALIZED_POINT;

/**
 * PgSQL patch type (collection of points) for clouds.
 * Variable length, because there can be
 * an arbitrary number of points encoded within.
 * The pcid is a foriegn key reference to the
 * POINTCLOUD_SCHEMAS table, where
 * the underlying structure of the data is described in XML,
 * the spatial reference system is indicated, and the data
 * packing scheme is indicated.
 */
typedef struct
{
  uint32_t size;
  uint32_t pcid;
  uint32_t compression;
  uint32_t npoints;
  PCBOUNDS bounds;
  uint8_t data[1];
} SERIALIZED_PATCH;

/* PGSQL / POINTCLOUD UTILITY FUNCTIONS */
uint32 pcid_from_typmod(const int32 typmod);

/** Look-up the PCID in the POINTCLOUD_FORMATS table, and construct a PC_SCHEMA
 * from the XML therein */
#if PGSQL_VERSION < 120
PCSCHEMA *pc_schema_from_pcid(uint32_t pcid, FunctionCallInfoData *fcinfo);
#else
PCSCHEMA *pc_schema_from_pcid(uint32_t pcid, FunctionCallInfo fcinfo);
#endif

/** Look-up the PCID in the POINTCLOUD_FORMATS table, and construct a PC_SCHEMA
 * from the XML therein */
PCSCHEMA *pc_schema_from_pcid_uncached(uint32 pcid);

/** Turn a PCPOINT into a byte buffer suitable for saving in PgSQL */
SERIALIZED_POINT *pc_point_serialize(const PCPOINT *pcpt);

/** Turn a byte buffer into a PCPOINT for processing */
PCPOINT *pc_point_deserialize(const SERIALIZED_POINT *serpt,
                              const PCSCHEMA *schema);

/** Create a new readwrite PCPOINT from a hex string */
#if PGSQL_VERSION < 120
PCPOINT *pc_point_from_hexwkb(const char *hexwkb, size_t hexlen,
                              FunctionCallInfoData *fcinfo);
#else
PCPOINT *pc_point_from_hexwkb(const char *hexwkb, size_t hexlen,
                              FunctionCallInfo fcinfo);
#endif
/** Create a hex representation of a PCPOINT */
char *pc_point_to_hexwkb(const PCPOINT *pt);

/** How big will this thing be on disk? */
size_t pc_patch_serialized_size(const PCPATCH *patch);

/** Turn a PCPATCH into a byte buffer suitable for saving in PgSQL */
SERIALIZED_PATCH *pc_patch_serialize(const PCPATCH *patch, void *userdata);

/** Turn a PCPATCH into an uncompressed byte buffer */
SERIALIZED_PATCH *pc_patch_serialize_to_uncompressed(const PCPATCH *patch);

/** Turn a byte buffer into a PCPATCH for processing */
PCPATCH *pc_patch_deserialize(const SERIALIZED_PATCH *serpatch,
                              const PCSCHEMA *schema);

/** Create a new readwrite PCPATCH from a hex string */
#if PGSQL_VERSION < 120
PCPATCH *pc_patch_from_hexwkb(const char *hexwkb, size_t hexlen,
                              FunctionCallInfoData *fcinfo);
#else
PCPATCH *pc_patch_from_hexwkb(const char *hexwkb, size_t hexlen,
                              FunctionCallInfo fcinfo);
#endif

/** Create a hex representation of a PCPOINT */
char *pc_patch_to_hexwkb(const PCPATCH *patch);

/** Returns OGC WKB for envelope of PCPATCH */
uint8_t *pc_patch_to_geometry_wkb_envelope(const SERIALIZED_PATCH *pa,
                                           const PCSCHEMA *schema,
                                           size_t *wkbsize);

/** Read the first few bytes off an object to get the datum */
uint32 pcid_from_datum(Datum d);

PCSTATS *pc_patch_stats_deserialize(const PCSCHEMA *schema, const uint8_t *buf);

void pointcloud_init_constants_cache(void);
