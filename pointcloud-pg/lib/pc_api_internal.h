/***********************************************************************
 * pc_api_internal.h
 *
 *  Signatures we need to share within the library, but not for
 *  use outside it.
 *
 *  PgSQL Pointcloud is free and open source software provided
 *  by the Government of Canada
 *
 *  Copyright (c) 2013 Natural Resources Canada
 *  Copyright (c) 2013 OpenGeo
 *
 ***********************************************************************/

#ifndef _PC_API_INTERNAL_H
#define _PC_API_INTERNAL_H

#include "pc_api.h"

/**
 * Utility defines
 */
#define PC_TRUE 1
#define PC_FALSE 0
#define PC_SUCCESS 1
#define PC_FAILURE 0

/**
 * How many compression types do we support?
 */
#define PCCOMPRESSIONTYPES 2

/**
 * Memory allocation for patch starts at 64 points
 */
#define PCPATCH_DEFAULT_MAXPOINTS 64

/**
 * How many points to sample before considering
 * a PCDIMSTATS done?
 */
#define PCDIMSTATS_MIN_SAMPLE 10000

/**
 * Interpretation types for our dimension descriptions
 */
#define NUM_INTERPRETATIONS 11

enum INTERPRETATIONS
{
  PC_UNKNOWN = 0,
  PC_INT8 = 1,
  PC_UINT8 = 2,
  PC_INT16 = 3,
  PC_UINT16 = 4,
  PC_INT32 = 5,
  PC_UINT32 = 6,
  PC_INT64 = 7,
  PC_UINT64 = 8,
  PC_DOUBLE = 9,
  PC_FLOAT = 10
};

enum DIMCOMPRESSIONS
{
  PC_DIM_NONE = 0,
  PC_DIM_RLE = 1,
  PC_DIM_SIGBITS = 2,
  PC_DIM_ZLIB = 3
};

/* PCDOUBLESTAT are members of PCDOUBLESTATS */
typedef struct
{
  double min;
  double max;
  double sum;
} PCDOUBLESTAT;

/* PCDOUBLESTATS are internal to calculating stats in this module */
typedef struct
{
  uint32_t npoints;
  PCDOUBLESTAT *dims;
} PCDOUBLESTATS;

typedef struct
{
  uint32_t nset;
  uint32_t npoints;
  uint8_t *map;
} PCBITMAP;

/** What is the endianness of this system? */
char machine_endian(void);

/** Flips the bytes of an int32_t */
int32_t int32_flip_endian(int32_t val);

/** Read the the npoints from WKB form of a PATCH */
uint32_t wkb_get_compression(const uint8_t *wkb);

/** Read an int32 from a byte array, flipping if requested */
int32_t wkb_get_int32(const uint8_t *wkb, int flip_endian);

/** Read an int16 from a byte array, flipping if requested */
int16_t wkb_get_int16(const uint8_t *wkb, int flip_endian);

/** Read the number of points from a wkb */
uint32_t wkb_get_npoints(const uint8_t *wkb);

/** Write a double into a byte array */
uint8_t *wkb_set_double(uint8_t *wkb, double d);

/** Write a uint32 into a byte array */
uint8_t *wkb_set_uint32(uint8_t *wkb, uint32_t i);

/** Write a char into a byte array */
uint8_t *wkb_set_char(uint8_t *wkb, char c);

/** Force a byte array into the machine endianness */
uint8_t *uncompressed_bytes_flip_endian(const uint8_t *bytebuf,
                                        const PCSCHEMA *schema,
                                        uint32_t npoints);

/** Update a value using the scale/offset info from a dimension */
double pc_value_scale_offset(double val, const PCDIMENSION *dim);

/** Remove the scale/offset values from a number before storage */
double pc_value_unscale_unoffset(double val, const PCDIMENSION *dim);

/** Read interpretation type from buffer and cast to double */
double pc_double_from_ptr(const uint8_t *ptr, uint32_t interpretation);

/** Write value to buffer in the interpretation type */
int pc_double_to_ptr(uint8_t *ptr, uint32_t interpretation, double val);

/** Return number of bytes in a given interpretation */
size_t pc_interpretation_size(uint32_t interp);

/** Convert XML string token to type interpretation number */
const char *pc_interpretation_string(uint32_t interp);

/** Copy a string within the global memory management context */
char *pcstrdup(const char *str);

/** Scales/offsets double, casts to appropriate dimension type, and writes into
 * point */
int pc_point_set_double_by_index(PCPOINT *pt, uint32_t idx, double val);

/** Scales/offsets double, casts to appropriate dimension type, and writes into
 * point */
int pc_point_set_double_by_name(PCPOINT *pt, const char *name, double val);

/** Scales/offsets double, casts to appropriate dimension type, and writes into
 * point */
int pc_point_set_double(PCPOINT *pt, const PCDIMENSION *dim, double val);

/****************************************************************************
 * DIMENSION STATISTICS
 */

/** Analyze the bytes in the #PCPATCH_DIMENSIONAL and update the #PCDIMSTATS */
int pc_dimstats_update(PCDIMSTATS *pds, const PCPATCH_DIMENSIONAL *pdl);
/** Free the PCDIMSTATS memory */
void pc_dimstats_free(PCDIMSTATS *pds);
char *pc_dimstats_to_string(const PCDIMSTATS *pds);

/****************************************************************************
 * PATCHES
 */

/** Returns newly allocated patch that only contains the points fitting the
 * filter condition */
PCPATCH *pc_patch_filter(const PCPATCH *pa, uint32_t dimnum,
                         PC_FILTERTYPE filter, double val1, double val2);
void pc_patch_free_stats(PCPATCH *pa);

/* DIMENSIONAL PATCHES */
char *pc_patch_dimensional_to_string(const PCPATCH_DIMENSIONAL *pa);
PCPATCH_DIMENSIONAL *
pc_patch_dimensional_from_uncompressed(const PCPATCH_UNCOMPRESSED *pa);
PCPATCH_DIMENSIONAL *
pc_patch_dimensional_compress(const PCPATCH_DIMENSIONAL *pdl, PCDIMSTATS *pds);
PCPATCH_DIMENSIONAL *
pc_patch_dimensional_decompress(const PCPATCH_DIMENSIONAL *pdl);
void pc_patch_dimensional_free(PCPATCH_DIMENSIONAL *pdl);
int pc_patch_dimensional_compute_extent(PCPATCH_DIMENSIONAL *pdl);
uint8_t *pc_patch_dimensional_to_wkb(const PCPATCH_DIMENSIONAL *patch,
                                     size_t *wkbsize);
PCPATCH *pc_patch_dimensional_from_wkb(const PCSCHEMA *schema,
                                       const uint8_t *wkb, size_t wkbsize);
PCPATCH_DIMENSIONAL *
pc_patch_dimensional_from_pointlist(const PCPOINTLIST *pdl);
PCPOINTLIST *pc_pointlist_from_dimensional(const PCPATCH_DIMENSIONAL *pdl);
PCPATCH_DIMENSIONAL *
pc_patch_dimensional_clone(const PCPATCH_DIMENSIONAL *patch);
PCPOINT *pc_patch_dimensional_pointn(const PCPATCH_DIMENSIONAL *pdl, int n);

/* UNCOMPRESSED PATCHES */
char *pc_patch_uncompressed_to_string(const PCPATCH_UNCOMPRESSED *patch);
uint8_t *pc_patch_uncompressed_to_wkb(const PCPATCH_UNCOMPRESSED *patch,
                                      size_t *wkbsize);
PCPATCH *pc_patch_uncompressed_from_wkb(const PCSCHEMA *s, const uint8_t *wkb,
                                        size_t wkbsize);
PCPATCH_UNCOMPRESSED *pc_patch_uncompressed_make(const PCSCHEMA *s,
                                                 uint32_t maxpoints);
int pc_patch_uncompressed_compute_extent(PCPATCH_UNCOMPRESSED *patch);
int pc_patch_uncompressed_compute_stats(PCPATCH_UNCOMPRESSED *patch);
void pc_patch_uncompressed_free(PCPATCH_UNCOMPRESSED *patch);
uint8_t *pc_patch_uncompressed_readonly(PCPATCH_UNCOMPRESSED *patch);
PCPOINTLIST *pc_pointlist_from_uncompressed(const PCPATCH_UNCOMPRESSED *patch);
PCPATCH_UNCOMPRESSED *
pc_patch_uncompressed_from_pointlist(const PCPOINTLIST *pl);
PCPATCH_UNCOMPRESSED *
pc_patch_uncompressed_from_dimensional(const PCPATCH_DIMENSIONAL *pdl);
int pc_patch_uncompressed_add_point(PCPATCH_UNCOMPRESSED *c, const PCPOINT *p);
PCPOINT *pc_patch_uncompressed_pointn(const PCPATCH_UNCOMPRESSED *patch, int n);

/* LAZPERF PATCHES */
PCPATCH_LAZPERF *pc_patch_lazperf_from_pointlist(const PCPOINTLIST *pl);
PCPATCH_LAZPERF *
pc_patch_lazperf_from_uncompressed(const PCPATCH_UNCOMPRESSED *pa);
PCPOINTLIST *pc_pointlist_from_lazperf(const PCPATCH_LAZPERF *palaz);
PCPATCH_UNCOMPRESSED *
pc_patch_uncompressed_from_lazperf(const PCPATCH_LAZPERF *palaz);
int pc_patch_lazperf_compute_extent(PCPATCH_LAZPERF *patch);
char *pc_patch_lazperf_to_string(const PCPATCH_LAZPERF *pa);
void pc_patch_lazperf_free(PCPATCH_LAZPERF *palaz);
uint8_t *pc_patch_lazperf_to_wkb(const PCPATCH_LAZPERF *patch, size_t *wkbsize);
PCPATCH *pc_patch_lazperf_from_wkb(const PCSCHEMA *schema, const uint8_t *wkb,
                                   size_t wkbsize);
PCPOINT *pc_patch_lazperf_pointn(const PCPATCH_LAZPERF *patch, int n);

/****************************************************************************
 * BYTES
 */

/** Construct empty byte array (zero out attribute and allocate byte buffer) */
PCBYTES pc_bytes_make(const PCDIMENSION *dim, uint32_t npoints);
/** Empty the byte array (free the byte buffer) */
void pc_bytes_free(PCBYTES bytes);
/** Apply the compresstion to the byte array in place, freeing the original byte
 * buffer */
PCBYTES pc_bytes_encode(PCBYTES pcb, int compression);
/** Convert the bytes in #PCBYTES to PC_DIM_NONE compression */
PCBYTES pc_bytes_decode(PCBYTES epcb);

/** Convert value bytes to RLE bytes */
PCBYTES pc_bytes_run_length_encode(const PCBYTES pcb);
/** Convert RLE bytes to value bytes */
PCBYTES pc_bytes_run_length_decode(const PCBYTES pcb);
/** Convert value bytes to bit packed bytes */
PCBYTES pc_bytes_sigbits_encode(const PCBYTES pcb);
/** Convert bit packed bytes to value bytes */
PCBYTES pc_bytes_sigbits_decode(const PCBYTES pcb);
/** Compress bytes using zlib */
PCBYTES pc_bytes_zlib_encode(const PCBYTES pcb);
/** De-compress bytes using zlib */
PCBYTES pc_bytes_zlib_decode(const PCBYTES pcb);

/** How many runs are there in a value array? */
uint32_t pc_bytes_run_count(const PCBYTES *pcb);
/** How many bits are shared by all elements of this array? */
uint32_t pc_bytes_sigbits_count(const PCBYTES *pcb);
/** Using an 8-bit word, what is the common word and number of bits in common?
 */
uint8_t pc_bytes_sigbits_count_8(const PCBYTES *pcb, uint32_t *nsigbits);
/** Using an 16-bit word, what is the common word and number of bits in common?
 */
uint16_t pc_bytes_sigbits_count_16(const PCBYTES *pcb, uint32_t *nsigbits);
/** Using an 32-bit word, what is the common word and number of bits in common?
 */
uint32_t pc_bytes_sigbits_count_32(const PCBYTES *pcb, uint32_t *nsigbits);
/** Using an 64-bit word, what is the common word and number of bits in common?
 */
uint64_t pc_bytes_sigbits_count_64(const PCBYTES *pcb, uint32_t *nsigbits);

/* NOTE: stats are gathered without applying scale and offset */
PCBYTES pc_bytes_filter(const PCBYTES *pcb, const PCBITMAP *map,
                        PCDOUBLESTAT *stats);

PCBITMAP *pc_bytes_bitmap(const PCBYTES *pcb, PC_FILTERTYPE filter, double val1,
                          double val2);
int pc_bytes_minmax(const PCBYTES *pcb, double *min, double *max, double *avg);

/** getting the n-th point out of a PCBYTE into a buffer */
void pc_bytes_uncompressed_to_ptr(uint8_t *buf, PCBYTES pcb, int n);
void pc_bytes_run_length_to_ptr(uint8_t *buf, PCBYTES pcb, int n);
void pc_bytes_sigbits_to_ptr_32(uint8_t *buf, PCBYTES pcb, int n);
void pc_bytes_sigbits_to_ptr(uint8_t *buf, PCBYTES pcb, int n);
void pc_bytes_zlib_to_ptr(uint8_t *buf, PCBYTES pcb, int n);
void pc_bytes_to_ptr(uint8_t *buf, PCBYTES pcb, int n);

/****************************************************************************
 * BOUNDS
 */

/** Initialize with very large mins and very small maxes */
void pc_bounds_init(PCBOUNDS *b);
/** Copy a bounds */
PCSTATS *pc_stats_clone(const PCSTATS *stats);
/** Expand extents of b1 to encompass b2 */
void pc_bounds_merge(PCBOUNDS *b1, const PCBOUNDS *b2);

/****************************************************************************
 * BITMAPS
 */

/** Allocate new unset bitmap */
PCBITMAP *pc_bitmap_new(uint32_t npoints);
/** Deallocate bitmap */
void pc_bitmap_free(PCBITMAP *map);
/** Set indicated bit on bitmap if filter and value are consistent */
void pc_bitmap_filter(PCBITMAP *map, PC_FILTERTYPE filter, int i, double d,
                      double val1, double val2);

/** Read indicated bit of bitmap */
#define pc_bitmap_get(map, i) ((map)->map[(i)])

#endif /* _PC_API_INTERNAL_H */
