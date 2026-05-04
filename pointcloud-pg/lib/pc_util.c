/***********************************************************************
 * pc_util.c
 *
 *  Handy functions used by the library.
 *
 *  PgSQL Pointcloud is free and open source software provided
 *  by the Government of Canada
 *  Copyright (c) 2013 Natural Resources Canada
 *
 ***********************************************************************/

#include "pc_api_internal.h"
#include <float.h>

/**********************************************************************************
 * WKB AND ENDIANESS UTILITIES
 */

/* Our static character->number map. Anything > 15 is invalid */
static uint8_t hex2char[256] = {
    /* not Hex characters */
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    /* 0-9 */
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 20, 20, 20, 20, 20, 20,
    /* A-F */
    20, 10, 11, 12, 13, 14, 15, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    /* not Hex characters */
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    /* a-f */
    20, 10, 11, 12, 13, 14, 15, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    /* not Hex characters (upper 128 characters) */
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20};

uint8_t *pc_bytes_from_hexbytes(const char *hexbuf, size_t hexsize)
{
  uint8_t *buf = NULL;
  register uint8_t h1, h2;
  int i;

  if (hexsize % 2)
    pcerror("Invalid hex string, length (%d) has to be a multiple of two!",
            hexsize);

  buf = pcalloc(hexsize / 2);

  if (!buf)
    pcerror("Unable to allocate memory buffer.");

  for (i = 0; i < hexsize / 2; i++)
  {
    h1 = hex2char[(int)hexbuf[2 * i]];
    h2 = hex2char[(int)hexbuf[2 * i + 1]];
    if (h1 > 15)
      pcerror("Invalid hex character (%c) encountered", hexbuf[2 * i]);
    if (h2 > 15)
      pcerror("Invalid hex character (%c) encountered", hexbuf[2 * i + 1]);
    /* First character is high bits, second is low bits */
    buf[i] = ((h1 & 0x0F) << 4) | (h2 & 0x0F);
  }
  return buf;
}

static char *hexchr = "0123456789ABCDEF";

char *pc_hexbytes_from_bytes(const uint8_t *bytebuf, size_t bytesize)
{
  char *buf =
      pcalloc(2 * bytesize + 1); /* 2 chars per byte + null terminator */
  int i;
  buf[2 * bytesize] = '\0';
  for (i = 0; i < bytesize; i++)
  {
    /* Top four bits to 0-F */
    buf[2 * i] = hexchr[bytebuf[i] >> 4];
    /* Bottom four bits to 0-F */
    buf[2 * i + 1] = hexchr[bytebuf[i] & 0x0F];
  }

  return buf;
}

/* 0 = xdr | big endian    */
/* 1 = ndr | little endian */
char machine_endian(void)
{
  static int check_int = 1; /* dont modify this!!! */
  return *((char *)&check_int);
}

int32_t int32_flip_endian(int32_t val)
{
  int i;
  uint8_t tmp;
  uint8_t b[4];
  memcpy(b, &val, 4);
  for (i = 0; i < 2; i++)
  {
    tmp = b[i];
    b[i] = b[3 - i];
    b[3 - i] = tmp;
  }
  memcpy(&val, b, 4);
  return val;
}

int16_t int16_flip_endian(int16_t val)
{
  uint8_t tmp;
  uint8_t b[2];
  memcpy(b, &val, 2);
  tmp = b[0];
  b[0] = b[1];
  b[1] = tmp;
  memcpy(&val, b, 2);
  return val;
}

int32_t wkb_get_int32(const uint8_t *wkb, int flip_endian)
{
  int32_t i;
  memcpy(&i, wkb, 4);
  if (flip_endian)
    return int32_flip_endian(i);
  else
    return i;
}

int16_t wkb_get_int16(const uint8_t *wkb, int flip_endian)
{
  int16_t i;
  memcpy(&i, wkb, 2);
  if (flip_endian)
    return int16_flip_endian(i);
  else
    return i;
}

uint8_t *wkb_set_double(uint8_t *wkb, double d)
{
  memcpy(wkb, &d, 8);
  wkb += 8;
  return wkb;
}

uint8_t *wkb_set_uint32(uint8_t *wkb, uint32_t i)
{
  memcpy(wkb, &i, 4);
  wkb += 4;
  return wkb;
}

uint8_t *wkb_set_char(uint8_t *wkb, char c)
{
  memcpy(wkb, &c, 1);
  wkb += 1;
  return wkb;
}

uint32_t pc_wkb_get_pcid(const uint8_t *wkb)
{
  /* We expect the bytes to be in WKB format for PCPOINT/PCPATCH */
  /* byte 0:   endian */
  /* byte 1-4: pcid */
  /* ...data... */
  uint32_t pcid;
  memcpy(&pcid, wkb + 1, 4);
  if (wkb[0] != machine_endian())
  {
    pcid = int32_flip_endian(pcid);
  }
  return pcid;
}

uint32_t wkb_get_compression(const uint8_t *wkb)
{
  /* We expect the bytes to be in WKB format for PCPATCH */
  /* byte 0:   endian */
  /* byte 1-4: pcid */
  /* byte 5-8: compression */
  /* ...data... */
  uint32_t compression;
  memcpy(&compression, wkb + 1 + 4, 4);
  if (wkb[0] != machine_endian())
  {
    compression = int32_flip_endian(compression);
  }
  return compression;
}

uint32_t wkb_get_npoints(const uint8_t *wkb)
{
  /* We expect the bytes to be in WKB format for PCPATCH */
  /* byte 0:   endian */
  /* byte 1-4: pcid */
  /* byte 5-8: compression */
  /* byte 9-12: npoints */
  /* ...data... */
  uint32_t npoints;
  memcpy(&npoints, wkb + 1 + 4 + 4, 4);
  if (wkb[0] != machine_endian())
  {
    npoints = int32_flip_endian(npoints);
  }
  return npoints;
}

uint8_t *uncompressed_bytes_flip_endian(const uint8_t *bytebuf,
                                        const PCSCHEMA *schema,
                                        uint32_t npoints)
{
  int i, j, k;
  size_t bufsize = schema->size * npoints;
  uint8_t *buf = pcalloc(bufsize);

  memcpy(buf, bytebuf, bufsize);

  for (i = 0; i < npoints; i++)
  {
    for (j = 0; j < schema->ndims; j++)
    {
      PCDIMENSION *dimension = schema->dims[j];
      uint8_t *ptr = buf + i * schema->size + dimension->byteoffset;

      for (k = 0; k < ((dimension->size) / 2); k++)
      {
        int l = dimension->size - k - 1;
        uint8_t tmp = ptr[k];
        ptr[k] = ptr[l];
        ptr[l] = tmp;
      }
    }
  }

  return buf;
}

int pc_bounds_intersects(const PCBOUNDS *b1, const PCBOUNDS *b2)
{
  if (b1->xmin > b2->xmax || b1->xmax < b2->xmin || b1->ymin > b2->ymax ||
      b1->ymax < b2->ymin)
  {
    return PC_FALSE;
  }
  return PC_TRUE;
}

void pc_bounds_init(PCBOUNDS *b)
{
  b->xmin = b->ymin = DBL_MAX;
  b->xmax = b->ymax = -1 * DBL_MAX;
}

void pc_bounds_merge(PCBOUNDS *b1, const PCBOUNDS *b2)
{
  if (b2->xmin < b1->xmin)
    b1->xmin = b2->xmin;
  if (b2->ymin < b1->ymin)
    b1->ymin = b2->ymin;
  if (b2->xmax > b1->xmax)
    b1->xmax = b2->xmax;
  if (b2->ymax > b1->ymax)
    b1->ymax = b2->ymax;
}

static uint32_t srid_mask = 0x20000000;
static uint32_t m_mask = 0x40000000;
static uint32_t z_mask = 0x80000000;

uint8_t *pc_bounding_diagonal_wkb_from_bounds(const PCBOUNDS *bounds,
                                              const PCSCHEMA *schema,
                                              size_t *wkbsize)
{
  uint8_t *wkb, *ptr;
  uint32_t wkbtype;
  size_t size;

  wkbtype = 2;                    /* WKB LINESTRING */
  size = 1 + 4 + 4 + (2 * 2 * 8); /* endian + type + npoints + 2 dbl pts */

  if (schema->srid != 0)
  {
    wkbtype |= srid_mask;
    size += 4;
  }

  wkb = pcalloc(size);
  ptr = wkb;

  ptr = wkb_set_char(ptr, machine_endian()); /* Endian flag */
  ptr = wkb_set_uint32(ptr, wkbtype);        /* Geometry type */

  if (schema->srid != 0)
  {
    ptr = wkb_set_uint32(ptr, schema->srid); /* SRID */
  }

  ptr = wkb_set_uint32(ptr, 2); /* NPOINTS = 2 */

  // point 1
  ptr = wkb_set_double(ptr, bounds->xmin);
  ptr = wkb_set_double(ptr, bounds->ymin);

  // point 2
  ptr = wkb_set_double(ptr, bounds->xmax);
  ptr = wkb_set_double(ptr, bounds->ymax);

  if (wkbsize)
    *wkbsize = size;

  return wkb;
}

uint8_t *pc_bounding_diagonal_wkb_from_stats(const PCSTATS *stats,
                                             size_t *wkbsize)
{
  const PCSCHEMA *schema = stats->min.schema;
  const PCPOINT *stat;
  uint8_t *wkb, *ptr;
  uint32_t wkbtype;
  size_t size;
  double val;

  wkbtype = 2;                    /* WKB LINESTRING */
  size = 1 + 4 + 4 + (2 * 2 * 8); /* endian + type + npoints + 2 dbl pts */

  if (schema->srid != 0)
  {
    wkbtype |= srid_mask;
    size += 4;
  }
  if (schema->zdim)
  {
    wkbtype |= z_mask;
    size += 2 * 8;
  }
  if (schema->mdim)
  {
    wkbtype |= m_mask;
    size += 2 * 8;
  }

  wkb = pcalloc(size);
  ptr = wkb;

  ptr = wkb_set_char(ptr, machine_endian()); /* Endian flag */
  ptr = wkb_set_uint32(ptr, wkbtype);        /* Geometry type */

  if (schema->srid != 0)
  {
    ptr = wkb_set_uint32(ptr, schema->srid); /* SRID */
  }

  ptr = wkb_set_uint32(ptr, 2); /* NPOINTS = 2 */

  // point 1
  stat = &stats->min;
  pc_point_get_x(stat, &val);
  ptr = wkb_set_double(ptr, val);
  pc_point_get_y(stat, &val);
  ptr = wkb_set_double(ptr, val);
  if (schema->zdim)
  {
    pc_point_get_z(stat, &val);
    ptr = wkb_set_double(ptr, val);
  }
  if (schema->mdim)
  {
    pc_point_get_m(stat, &val);
    ptr = wkb_set_double(ptr, val);
  }

  // point 2
  stat = &stats->max;
  pc_point_get_x(stat, &val);
  ptr = wkb_set_double(ptr, val);
  pc_point_get_y(stat, &val);
  ptr = wkb_set_double(ptr, val);
  if (schema->zdim)
  {
    pc_point_get_z(stat, &val);
    ptr = wkb_set_double(ptr, val);
  }
  if (schema->mdim)
  {
    pc_point_get_m(stat, &val);
    ptr = wkb_set_double(ptr, val);
  }

  if (wkbsize)
    *wkbsize = size;

  return wkb;
}
