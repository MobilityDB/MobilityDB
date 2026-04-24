/***********************************************************************
 * pc_api.h
 *
 *  Structures and function signatures for point clouds
 *
 *  PgSQL Pointcloud is free and open source software provided
 *  by the Government of Canada
 *  Copyright (c) 2013 Natural Resources Canada
 *
 ***********************************************************************/

#ifndef _PC_API_H
#define _PC_API_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"
#include "pc_config.h"

#ifndef __GNUC__
#define __attribute__ (x)
#endif

/**********************************************************************
 * DATA STRUCTURES
 */

/**
 * Compression types for PCPOINTS in a PCPATCH
 */
enum COMPRESSIONS
{
  PC_NONE = 0,
  PC_DIMENSIONAL = 1,
  PC_LAZPERF = 2
};

/**
 * Flags of endianness for inter-architecture
 * data transfers.
 */
enum ENDIANS
{
  PC_XDR = 0, /* Big */
  PC_NDR = 1  /* Little */
};

typedef enum
{
  PC_GT,
  PC_LT,
  PC_EQUAL,
  PC_BETWEEN
} PC_FILTERTYPE;

/**
 * We need to hold a cached in-memory version of the format's
 * XML structure for speed, and this is it.
 */
typedef struct
{
  char *name;
  char *description;
  uint32_t position;
  uint32_t size;
  uint32_t byteoffset;
  uint32_t interpretation;
  double scale;
  double offset;
  uint8_t active;
} PCDIMENSION;

typedef struct
{
  uint32_t pcid;        /* Unique ID for schema */
  uint32_t ndims;       /* How many dimensions does this schema have? */
  size_t size;          /* How wide (bytes) is a point with this schema? */
  PCDIMENSION **dims;   /* Array of dimension pointers */
  uint32_t srid;        /* Foreign key reference to SPATIAL_REF_SYS */
  PCDIMENSION *xdim;    /* pointer to the x dimension within dims */
  PCDIMENSION *ydim;    /* pointer to the y dimension within dims */
  PCDIMENSION *zdim;    /* pointer to the z dimension within dims */
  PCDIMENSION *mdim;    /* pointer to the m dimension within dims */
  uint32_t compression; /* Compression type applied to the data */
  hashtable *namehash;  /* Look-up from dimension name to pointer */
} PCSCHEMA;

/* Used for dimensional patch statistics */
typedef struct
{
  uint32_t total_runs;
  uint32_t total_commonbits;
  uint32_t recommended_compression;
} PCDIMSTAT;

typedef struct
{
  int32_t ndims;
  uint32_t total_points;
  uint32_t total_patches;
  PCDIMSTAT *stats;
} PCDIMSTATS;

/**
 * Uncompressed structure for in-memory handling
 * of points. A read-only PgSQL point can be wrapped in
 * one of these by pointing the data element at the
 * PgSQL memory and setting the capacity to 0
 * to indicate it is read-only.
 */
typedef struct
{
  int8_t readonly;
  const PCSCHEMA *schema;
  uint8_t *data; /* A serialized version of the data */
} PCPOINT;

typedef struct
{
  void *mem; /* An opaque memory buffer to be freed on destruction if not NULL
              */
  uint32_t npoints;
  uint32_t maxpoints;
  PCPOINT **points;
} PCPOINTLIST;

typedef struct
{
  size_t size;
  uint32_t npoints;
  uint32_t interpretation;
  uint32_t compression;
  uint32_t readonly;
  uint8_t *bytes;
} PCBYTES;

typedef struct
{
  double xmin;
  double xmax;
  double ymin;
  double ymax;
} PCBOUNDS;

/* Used for generic patch statistics */
typedef struct
{
  PCPOINT min;
  PCPOINT max;
  PCPOINT avg;
} PCSTATS;

/**
 * Uncompressed Structure for in-memory handling
 * of patches. A read-only PgSQL patch can be wrapped in
 * one of these by pointing the data element at the
 * PgSQL memory and setting the capacity to 0
 * to indicate it is read-only.
 */

#define PCPATCH_COMMON                                                         \
  int type;                                                                    \
  int8_t readonly;                                                             \
  const PCSCHEMA *schema;                                                      \
  uint32_t npoints;                                                            \
  PCBOUNDS bounds;                                                             \
  PCSTATS *stats;

typedef struct
{
  PCPATCH_COMMON
} PCPATCH;

typedef struct
{
  PCPATCH_COMMON
  uint32_t maxpoints; /* How many points we can hold (or 0 for read-only) */
  size_t datasize;
  uint8_t *data; /* A serialized version of the data */
} PCPATCH_UNCOMPRESSED;

typedef struct
{
  PCPATCH_COMMON
  PCBYTES *bytes;
} PCPATCH_DIMENSIONAL;

typedef struct
{
  PCPATCH_COMMON
  size_t lazperfsize;
  uint8_t *lazperf;
} PCPATCH_LAZPERF;

/* Global function signatures for memory/logging handlers. */
typedef void *(*pc_allocator)(size_t size);
typedef void *(*pc_reallocator)(void *mem, size_t size);
typedef void (*pc_deallocator)(void *mem);
typedef void (*pc_message_handler)(const char *string, va_list ap)
    __attribute__((format(printf, 1, 0)));

/**********************************************************************
 * MEMORY MANAGEMENT
 */

/** Allocate memory using the appropriate means (system/db) */
void *pcalloc(size_t size);
/** Reallocate memory using the appropriate means (system/db) */
void *pcrealloc(void *mem, size_t size);
/** Free memory using the appropriate means (system/db) */
void pcfree(void *mem);
/** Emit an error message using the appropriate means (system/db) */
void pcerror(const char *fmt, ...);
/** Emit an info message using the appropriate means (system/db) */
void pcinfo(const char *fmt, ...);
/** Emit a warning message using the appropriate means (system/db) */
void pcwarn(const char *fmt, ...);

/** Set custom memory allocators and messaging (used by PgSQL module) */
void pc_set_handlers(pc_allocator allocator, pc_reallocator reallocator,
                     pc_deallocator deallocator,
                     pc_message_handler error_handler,
                     pc_message_handler info_handler,
                     pc_message_handler warning_handler);

/** Set program to use system memory allocators and messaging */
void pc_install_default_handlers(void);

/**********************************************************************
 * UTILITY
 */

/** Convert binary to hex */
uint8_t *pc_bytes_from_hexbytes(const char *hexbuf, size_t hexsize);
/** Convert hex to binary */
char *pc_hexbytes_from_bytes(const uint8_t *bytebuf, size_t bytesize);
/** Read the the PCID from WKB form of a POINT/PATCH */
uint32_t pc_wkb_get_pcid(const uint8_t *wkb);
/** Build an empty #PCDIMSTATS based on the schema */
PCDIMSTATS *pc_dimstats_make(const PCSCHEMA *schema);
/** Get compression name from enum */
const char *pc_compression_name(int num);

/**********************************************************************
 * SCHEMAS
 */

/** Release the memory in a schema structure */
void pc_schema_free(PCSCHEMA *pcs);
/** Build a schema structure from the XML serialisation */
PCSCHEMA *pc_schema_from_xml(const char *xmlstr);
/** Print out JSON readable format of schema */
char *pc_schema_to_json(const PCSCHEMA *pcs);
/** Extract dimension information by position */
PCDIMENSION *pc_schema_get_dimension(const PCSCHEMA *s, uint32_t dim);
/** Extract dimension information by name */
PCDIMENSION *pc_schema_get_dimension_by_name(const PCSCHEMA *s,
                                             const char *name);
/** Check if the schema has all the information we need to work with data */
uint32_t pc_schema_is_valid(const PCSCHEMA *s);
/** Create a full copy of the schema and dimensions it contains */
PCSCHEMA *pc_schema_clone(const PCSCHEMA *s);
/** Add/overwrite a dimension in a schema */
void pc_schema_set_dimension(PCSCHEMA *s, PCDIMENSION *d);
/** Check/set the xyzm positions in the dimension list */
void pc_schema_check_xyzm(PCSCHEMA *s);
/** Get the width in bytes of a single point in the schema */
size_t pc_schema_get_size(const PCSCHEMA *s);
/** Check whether the schemas have the same dimensions at the same positions */
uint32_t pc_schema_same_dimensions(const PCSCHEMA *s1, const PCSCHEMA *s2);
/** Check whether the schemas have compatible dimension interpretations */
uint32_t pc_schema_same_interpretations(const PCSCHEMA *s1, const PCSCHEMA *s2);

/**********************************************************************
 * PCPOINTLIST
 */

/** Allocate a pointlist */
PCPOINTLIST *pc_pointlist_make(uint32_t npoints);

/** Free a pointlist, including the points contained therein */
void pc_pointlist_free(PCPOINTLIST *pl);

/** Add a point to the list, expanding buffer as necessary */
void pc_pointlist_add_point(PCPOINTLIST *pl, PCPOINT *pt);

/** Get a point from the list */
PCPOINT *pc_pointlist_get_point(const PCPOINTLIST *pl, int i);

/**********************************************************************
 * PCPOINT
 */

/** Create a new PCPOINT */
PCPOINT *pc_point_make(const PCSCHEMA *s);

/** Create a new readonly PCPOINT on top of a data buffer */
PCPOINT *pc_point_from_data(const PCSCHEMA *s, const uint8_t *data);

/** Create a new read/write PCPOINT from a double array  with an offset */
PCPOINT *pc_point_from_double_array(const PCSCHEMA *s, double *array,
                                    uint32_t offset, uint32_t stride);

/**
 * Return an allocated double array of doubles representing point values
 *
 * The number of elements in the array is equal to pt->schema->n_dims
 */
double *pc_point_to_double_array(const PCPOINT *pt);

/** Frees the PTPOINT and data (if not readonly). Does not free referenced
 * schema */
void pc_point_free(PCPOINT *pt);

/** Get dimension value by dimension name */
int pc_point_get_double_by_name(const PCPOINT *pt, const char *name,
                                double *val);

/** Get dimension value by dimension index */
int pc_point_get_double_by_index(const PCPOINT *pt, uint32_t idx, double *val);

/** Read a double right off the data area, applying scale/offset  */
int pc_point_get_double(const PCPOINT *pt, const PCDIMENSION *dim, double *val);

/** Set dimension value by dimension name */
int pc_point_set_double_by_name(PCPOINT *pt, const char *name, double val);

/** Set dimension value by dimension index */
int pc_point_set_double_by_index(PCPOINT *pt, uint32_t idx, double val);

/** Write a double to the data area after unapplying scale/offset */
int pc_point_set_double(PCPOINT *pt, const PCDIMENSION *dim, double val);

/** Returns X coordinate */
int pc_point_get_x(const PCPOINT *pt, double *val);

/** Returns Y coordinate */
int pc_point_get_y(const PCPOINT *pt, double *val);

/** Returns Z coordinate */
int pc_point_get_z(const PCPOINT *pt, double *val);

/** Returns M coordinate */
int pc_point_get_m(const PCPOINT *pt, double *val);

/** Set the X coordinate */
int pc_point_set_x(PCPOINT *pt, double val);

/** Set the Y coordinate */
int pc_point_set_y(PCPOINT *pt, double val);

/** Set the Z coordinate */
int pc_point_set_z(PCPOINT *pt, double val);

/** Set the M coordinate */
int pc_point_set_m(PCPOINT *pt, double val);

/** Create a new readwrite PCPOINT from a hex byte array */
PCPOINT *pc_point_from_wkb(const PCSCHEMA *s, uint8_t *wkb, size_t wkbsize);

/** Returns serialized form of point */
uint8_t *pc_point_to_wkb(const PCPOINT *pt, size_t *wkbsize);

/** Returns text form of point */
char *pc_point_to_string(const PCPOINT *pt);

/** Return the OGC WKB version of the point */
uint8_t *pc_point_to_geometry_wkb(const PCPOINT *pt, size_t *wkbsize);

/**********************************************************************
 * PCPATCH
 */

/** Create new PCPATCH from a PCPOINT set. Copies data, doesn't take ownership
 * of points */
PCPATCH *pc_patch_from_pointlist(const PCPOINTLIST *ptl);

/** Returns a list of points extracted from patch */
PCPOINTLIST *pc_pointlist_from_patch(const PCPATCH *patch);

/** Merge a set of patches into a single patch */
PCPATCH *pc_patch_from_patchlist(PCPATCH **palist, int numpatches);

/** Free patch memory, respecting read-only status. Does not free referenced
 * schema */
void pc_patch_free(PCPATCH *patch);

/** Create a compressed copy, using the compression schema referenced in the
 * PCSCHEMA */
PCPATCH *pc_patch_compress(const PCPATCH *patch, void *userdata);

/** Create an uncompressed copy */
PCPATCH *pc_patch_uncompress(const PCPATCH *patch);

/** Create a new readwrite PCPOINT from a byte array */
PCPATCH *pc_patch_from_wkb(const PCSCHEMA *s, uint8_t *wkb, size_t wkbsize);

/** Returns serialized form of point */
uint8_t *pc_patch_to_wkb(const PCPATCH *patch, size_t *wkbsize);

/** Returns text form of patch */
char *pc_patch_to_string(const PCPATCH *patch);

/** Return byte buffer size of serialization */
size_t pc_patch_dimensional_serialized_size(const PCPATCH_DIMENSIONAL *patch);

/** How big will the serialization be? */
size_t pc_bytes_serialized_size(const PCBYTES *pcb);

/** Write the representation down to a buffer */
int pc_bytes_serialize(const PCBYTES *pcb, uint8_t *buf, size_t *size);

/** Read a buffer up into a bytes structure */
int pc_bytes_deserialize(const uint8_t *buf, const PCDIMENSION *dim,
                         PCBYTES *pcb, int readonly, int flip_endian);

/** Wrap serialized stats in a new stats objects */
PCSTATS *pc_stats_new_from_data(const PCSCHEMA *schema, const uint8_t *mindata,
                                const uint8_t *maxdata, const uint8_t *avgdata);

/** Allocate a stats object */
PCSTATS *pc_stats_new(const PCSCHEMA *schema);

/** Free a stats object */
void pc_stats_free(PCSTATS *stats);

/** How big is the serialzation of a stats? */
size_t pc_stats_size(const PCSCHEMA *schema);

/** Calculate stats on an existing patch */
int pc_patch_compute_stats(PCPATCH *patch);

/** Calculate extent on an existing patch */
int pc_patch_compute_extent(PCPATCH *patch);

/** True/false if bounds intersect */
int pc_bounds_intersects(const PCBOUNDS *b1, const PCBOUNDS *b2);

/** Returns OGC WKB of the bounding diagonal of XY bounds */
uint8_t *pc_bounding_diagonal_wkb_from_bounds(const PCBOUNDS *bounds,
                                              const PCSCHEMA *schema,
                                              size_t *wkbsize);

/** Returns OGC WKB of the bounding diagonal of XY, XYZ, XYM or XYZM bounds */
uint8_t *pc_bounding_diagonal_wkb_from_stats(const PCSTATS *stats,
                                             size_t *wkbsize);

/** Subset batch based on less-than condition on dimension */
PCPATCH *pc_patch_filter_lt_by_name(const PCPATCH *pa, const char *name,
                                    double val);

/** Subset batch based on greater-than condition on dimension */
PCPATCH *pc_patch_filter_gt_by_name(const PCPATCH *pa, const char *name,
                                    double val);

/** Subset batch based on equality condition on dimension */
PCPATCH *pc_patch_filter_equal_by_name(const PCPATCH *pa, const char *name,
                                       double val);

/** Subset batch based on range condition on dimension */
PCPATCH *pc_patch_filter_between_by_name(const PCPATCH *pa, const char *name,
                                         double val1, double val2);

/** get point n */
PCPOINT *pc_patch_pointn(const PCPATCH *patch, int n);

/** Sorted patch after reordering points on dimensions */
PCPATCH *pc_patch_sort(const PCPATCH *pa, const char **name, int ndims);

/** True/false if the patch is sorted on dimension */
uint32_t pc_patch_is_sorted(const PCPATCH *pa, const char **name, int ndims,
                            char strict);

/** Subset batch based on index */
PCPATCH *pc_patch_range(const PCPATCH *pa, int first, int count);

/** assign a schema to the patch */
PCPATCH *pc_patch_set_schema(PCPATCH *patch, const PCSCHEMA *schema,
                             double def);

/** transform the patch based on the passed schema */
PCPATCH *pc_patch_transform(const PCPATCH *patch, const PCSCHEMA *schema,
                            double def);

#endif /* _PC_API_H */
