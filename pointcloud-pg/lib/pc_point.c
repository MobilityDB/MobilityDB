/***********************************************************************
 * pc_point.c
 *
 *  Pointclound point handling. Create, get and set values from the
 *  basic PCPOINT structure.
 *
 *  PgSQL Pointcloud is free and open source software provided
 *  by the Government of Canada
 *  Copyright (c) 2013 Natural Resources Canada
 *
 ***********************************************************************/

#include "pc_api_internal.h"
#include "stringbuffer.h"

PCPOINT *pc_point_make(const PCSCHEMA *s)
{
  size_t sz;
  PCPOINT *pt;

  if (!s)
  {
    pcerror("null schema passed into pc_point_make");
    return NULL;
  }

  /* Width of the data area */
  sz = s->size;
  if (!sz)
  {
    pcerror("invalid size calculation in pc_point_make");
    return NULL;
  }

  /* Make our own data area */
  pt = pcalloc(sizeof(PCPOINT));
  pt->data = pcalloc(sz);

  /* Set up basic info */
  pt->schema = s;
  pt->readonly = PC_FALSE;
  return pt;
};

PCPOINT *pc_point_from_data(const PCSCHEMA *s, const uint8_t *data)
{
  PCPOINT *pt;

  if (!s)
  {
    pcerror("null schema passed into pc_point_from_data");
    return NULL;
  }

  /* Reference the external data */
  pt = pcalloc(sizeof(PCPOINT));
  pt->data = (uint8_t *)data;

  /* Set up basic info */
  pt->schema = s;
  pt->readonly = PC_TRUE;
  return pt;
}

void pc_point_free(PCPOINT *pt)
{
  if (!pt->readonly)
  {
    pcfree(pt->data);
  }
  pcfree(pt);
}

int pc_point_get_double(const PCPOINT *pt, const PCDIMENSION *dim, double *val)
{
  uint8_t *ptr;
  double d;

  if (!dim)
    return PC_FAILURE;

  /* Read raw value from byte buffer */
  ptr = pt->data + dim->byteoffset;
  d = pc_double_from_ptr(ptr, dim->interpretation);
  d = pc_value_scale_offset(d, dim);

  *val = d;
  return PC_SUCCESS;
}

int pc_point_get_double_by_name(const PCPOINT *pt, const char *name,
                                double *val)
{
  PCDIMENSION *dim;
  dim = pc_schema_get_dimension_by_name(pt->schema, name);
  return pc_point_get_double(pt, dim, val);
}

int pc_point_get_double_by_index(const PCPOINT *pt, uint32_t idx, double *val)
{
  PCDIMENSION *dim;
  dim = pc_schema_get_dimension(pt->schema, idx);
  return pc_point_get_double(pt, dim, val);
}

int pc_point_set_double(PCPOINT *pt, const PCDIMENSION *dim, double val)
{
  uint8_t *ptr;

  if (!dim)
    return PC_FAILURE;

  /* Remove scale and offsets */
  val = pc_value_unscale_unoffset(val, dim);

  /* Get pointer into byte buffer */
  ptr = pt->data + dim->byteoffset;

  return pc_double_to_ptr(ptr, dim->interpretation, val);
}

int pc_point_set_double_by_index(PCPOINT *pt, uint32_t idx, double val)
{
  PCDIMENSION *dim;
  dim = pc_schema_get_dimension(pt->schema, idx);
  return pc_point_set_double(pt, dim, val);
}

int pc_point_set_double_by_name(PCPOINT *pt, const char *name, double val)
{
  PCDIMENSION *dim;
  dim = pc_schema_get_dimension_by_name(pt->schema, name);
  return pc_point_set_double(pt, dim, val);
}

int pc_point_get_x(const PCPOINT *pt, double *val)
{
  return pc_point_get_double(pt, pt->schema->xdim, val);
}

int pc_point_get_y(const PCPOINT *pt, double *val)
{
  return pc_point_get_double(pt, pt->schema->ydim, val);
}

int pc_point_get_z(const PCPOINT *pt, double *val)
{
  return pc_point_get_double(pt, pt->schema->zdim, val);
}

int pc_point_get_m(const PCPOINT *pt, double *val)
{
  return pc_point_get_double(pt, pt->schema->mdim, val);
}

int pc_point_set_x(PCPOINT *pt, double val)
{
  return pc_point_set_double(pt, pt->schema->xdim, val);
}

int pc_point_set_y(PCPOINT *pt, double val)
{
  return pc_point_set_double(pt, pt->schema->ydim, val);
}

int pc_point_set_z(PCPOINT *pt, double val)
{
  return pc_point_set_double(pt, pt->schema->zdim, val);
}

int pc_point_set_m(PCPOINT *pt, double val)
{
  return pc_point_set_double(pt, pt->schema->mdim, val);
}

char *pc_point_to_string(const PCPOINT *pt)
{
  /* { "pcid":1, "values":[<dim1>, <dim2>, <dim3>, <dim4>] }*/
  stringbuffer_t *sb = stringbuffer_create();
  char *str;
  int i;

  stringbuffer_aprintf(sb, "{\"pcid\":%d,\"pt\":[", pt->schema->pcid);
  for (i = 0; i < pt->schema->ndims; i++)
  {
    double d;
    if (!pc_point_get_double_by_index(pt, i, &d))
    {
      pcerror("pc_point_to_string: unable to read double at position %d", i);
    }
    if (i)
    {
      stringbuffer_append(sb, ",");
    }
    stringbuffer_aprintf(sb, "%g", d);
  }
  stringbuffer_append(sb, "]}");
  str = stringbuffer_getstringcopy(sb);
  stringbuffer_destroy(sb);
  return str;
}

PCPOINT *pc_point_from_double_array(const PCSCHEMA *s, double *array,
                                    uint32_t offset, uint32_t stride)
{
  int i;
  PCPOINT *pt;

  if (!s)
  {
    pcerror("null schema passed into pc_point_from_double_array");
    return NULL;
  }

  if (stride != s->ndims)
  {
    pcerror("number of elements in schema and array do not match in "
            "pc_point_from_double_array");
    return NULL;
  }

  /* Reference the external data */
  pt = pcalloc(sizeof(PCPOINT));
  pt->data = pcalloc(s->size);
  pt->schema = s;
  pt->readonly = PC_FALSE;

  for (i = 0; i < stride; i++)
  {
    if (PC_FAILURE == pc_point_set_double_by_index(pt, i, array[offset + i]))
    {
      pcerror("failed to write value into dimension %d in "
              "pc_point_from_double_array",
              i);
      return NULL;
    }
  }

  return pt;
}

PCPOINT *pc_point_from_wkb(const PCSCHEMA *schema, uint8_t *wkb, size_t wkblen)
{
  /*
  byte:     endianness (1 = NDR, 0 = XDR)
  uint32:   pcid (key to POINTCLOUD_SCHEMAS)
  uchar[]:  data (interpret relative to pcid)
  */
  const size_t hdrsz = 1 + 4; /* endian + pcid */
  uint8_t wkb_endian;
  uint8_t *data;
  PCPOINT *pt;

  if (!wkblen)
  {
    pcerror("pc_point_from_wkb: zero length wkb");
  }

  wkb_endian = wkb[0];

  if ((wkblen - hdrsz) != schema->size)
  {
    pcerror("pc_point_from_wkb: wkb size inconsistent with schema size");
  }

  if (wkb_endian != machine_endian())
  {
    /* uncompressed_bytes_flip_endian creates a flipped copy */
    data = uncompressed_bytes_flip_endian(wkb + hdrsz, schema, 1);
  }
  else
  {
    data = pcalloc(schema->size);
    memcpy(data, wkb + hdrsz, wkblen - hdrsz);
  }

  pt = pc_point_from_data(schema, data);
  pt->readonly = PC_FALSE;
  return pt;
}

uint8_t *pc_point_to_wkb(const PCPOINT *pt, size_t *wkbsize)
{
  /*
  byte:     endianness (1 = NDR, 0 = XDR)
  uint32:   pcid (key to POINTCLOUD_SCHEMAS)
  uchar[]:  data (interpret relative to pcid)
  */
  char endian = machine_endian();
  size_t size = 1 + 4 + pt->schema->size;
  uint8_t *wkb = pcalloc(size);
  wkb[0] = endian;                             /* Write endian flag */
  memcpy(wkb + 1, &(pt->schema->pcid), 4);     /* Write PCID */
  memcpy(wkb + 5, pt->data, pt->schema->size); /* Write data */
  if (wkbsize)
    *wkbsize = size;
  return wkb;
}

uint8_t *pc_point_to_geometry_wkb(const PCPOINT *pt, size_t *wkbsize)
{
  static uint32_t srid_mask = 0x20000000;
  static uint32_t m_mask = 0x40000000;
  static uint32_t z_mask = 0x80000000;
  uint32_t wkbtype = 1;        /* WKB POINT */
  size_t size = 1 + 4 + 8 + 8; /* endian + type + dblX, + dblY */
  uint8_t *wkb, *ptr;
  uint32_t srid = pt->schema->srid;
  double x, y, z, m;
  int has_x = pc_point_get_x(pt, &x) == PC_SUCCESS;
  int has_y = pc_point_get_y(pt, &y) == PC_SUCCESS;
  int has_z = pc_point_get_z(pt, &z) == PC_SUCCESS;
  int has_m = pc_point_get_m(pt, &m) == PC_SUCCESS;

  if (!(has_x && has_y))
    return NULL;

  if (srid)
  {
    wkbtype |= srid_mask;
    size += 4;
  }

  if (has_z)
  {
    wkbtype |= z_mask;
    size += 8;
  }

  if (has_m)
  {
    wkbtype |= m_mask;
    size += 8;
  }

  wkb = pcalloc(size);
  ptr = wkb;

  ptr[0] = machine_endian(); /* Endian flag */
  ptr += 1;

  memcpy(ptr, &wkbtype, 4); /* WKB type */
  ptr += 4;

  if (srid != 0)
  {
    memcpy(ptr, &srid, 4); /* SRID */
    ptr += 4;
  }

  memcpy(ptr, &x, 8); /* X */
  ptr += 8;

  memcpy(ptr, &y, 8); /* Y */
  ptr += 8;

  if (has_z)
  {
    memcpy(ptr, &z, 8); /* Z */
    ptr += 8;
  }

  if (has_m)
  {
    memcpy(ptr, &m, 8); /* M */
    ptr += 8;
  }

  if (wkbsize)
    *wkbsize = size;
  return wkb;
}

/**
 * @brief this function convert a PCPOINT to an array of double containing
 *        all the dimension values of this point
 *
 * @param a pointer to the point to convert to double
 *
 * @return a pointer to an array of double containing all the dimensions
 *         of the point expressed as double precision
 *
 */
double *pc_point_to_double_array(const PCPOINT *p)
{
  int i;
  double *a = (double *)pcalloc(p->schema->ndims * sizeof(double));

  for (i = 0; i < p->schema->ndims; ++i)
  {
    pc_point_get_double_by_index(p, i, &(a[i]));
  }

  return a;
}
