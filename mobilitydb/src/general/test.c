/**
 * @brief Return the maximum size in bytes of the period bounding box represented in
 * MF-JSON format
 */
static size_t
period_mfjson_size(void)
{
  /* The maximum size of a timestamptz is 35 characters, e.g.,
   * "2019-08-06 23:18:16.195062-09:30" */
  size_t size = sizeof("'stBoundedBy':{'period':{'begin':,'end':,'lower_inc':false,'upper_inc':false}},") +
    MOBDB_WKT_TIMESTAMPTZ_SIZE * 2;
  size += sizeof("'bbox':[,],");
  return size;
}

/**
 * @brief Write into the buffer the period bounding box represented in MF-JSON format
 */
static size_t
period_mfjson_buf(char *output, const Period *p)
{
  char *ptr = output;
  ptr += sprintf(ptr, "\"stBoundedBy\":{\"period\":{\"begin\":\"");
  ptr += datetimes_mfjson_buf(ptr, DatumGetTimestampTz(p->lower));
  ptr += sprintf(ptr, ",\"end\":\"");
  ptr += datetimes_mfjson_buf(ptr, DatumGetTimestampTz(p->upper));
  ptr += sprintf(ptr, ",\"lower_inc\":%s,'upper_inc':%s}},",
    p->lower_inc ? "true" : "false", p->upper_inc ? "true" : "false");
  return (ptr - output);
}

/**
 * @brief Return the maximum size in bytes of the temporal bounding box represented in
 * MF-JSON format
 */
static size_t
tbox_mfjson_size(int precision)
{
  /* The maximum size of a timestamptz is 35 characters, e.g.,
   * "2019-08-06 23:18:16.195062-09:30" */
  size_t size = sizeof("'stBoundedBy':{'period':{'begin':,'end':}},") +
    MOBDB_WKT_TIMESTAMPTZ_SIZE * 2;
  size += sizeof("'bbox':[,],");
  size +=  2 * (OUT_MAX_DIGS_DOUBLE + precision);
  return size;
}

/**
 * @brief Write into the buffer the temporal bounding box represented in MF-JSON format
 */
static size_t
tbox_mfjson_buf(char *output, const TBOX *bbox, int precision)
{
  assert (precision <= OUT_MAX_DOUBLE_PRECISION);
  char *ptr = output;
  ptr += sprintf(ptr, "\"stBoundedBy\":{\"bbox\":[");
  ptr += lwprint_double(bbox->xmin, precision, ptr);
  ptr += sprintf(ptr, ",");
  ptr += lwprint_double(bbox->xmax, precision, ptr);
  ptr += sprintf(ptr, "],\"period\":{\"begin\":\"");
  ptr += datetimes_mfjson_buf(ptr, bbox->tmin);
  ptr += sprintf(ptr, ",\"end\":\"");
  ptr += datetimes_mfjson_buf(ptr, bbox->tmax);
  ptr += sprintf(ptr, "\"}},");
  return (ptr - output);
}

/**
 * @brief Return the maximum size in bytes of the spatiotemporal bounding box
 * represented in MF-JSON format
 */
static size_t
stbox_mfjson_size(bool hasz, int precision)
{
  /* The maximum size of a timestamptz is 35 characters,
   * e.g., "2019-08-06 23:18:16.195062-09:30" */
  size_t size = sizeof("'stBoundedBy':{'period':{'begin':,'end':}},") +
    sizeof("\"2019-08-06T18:35:48.021455+02:30\",") * 2;
  if (! hasz)
  {
    size += sizeof("'bbox':[[,],[,]],");
    size +=  2 * 2 * (OUT_MAX_DIGS_DOUBLE + precision);
  }
  else
  {
    size += sizeof("\"bbox\":[[,,],[,,]],");
    size +=  2 * 3 * (OUT_MAX_DIGS_DOUBLE + precision);
  }
  return size;
}

/**
 * @brief Write into the buffer the spatiotemporal bounding box represented in
 * MF-JSON format
 */
static size_t
stbox_mfjson_buf(char *output, const STBOX *bbox, bool hasz, int precision)
{
  assert (precision <= OUT_MAX_DOUBLE_PRECISION);
  char *ptr = output;
  // ptr += sprintf(ptr, "\"stBoundedBy\":{\"bbox\":[[");
  ptr += lwprint_double(bbox->xmin, precision, ptr);
  ptr += sprintf(ptr, ",");
  ptr += lwprint_double(bbox->ymin, precision, ptr);
  if (hasz)
  {
    ptr += sprintf(ptr, ",");
    ptr += lwprint_double(bbox->zmin, precision, ptr);
  }
  ptr += sprintf(ptr, "],[");
  ptr += lwprint_double(bbox->xmax, precision, ptr);
  ptr += sprintf(ptr, ",");
  ptr += lwprint_double(bbox->ymax, precision, ptr);
  if (hasz)
  {
    ptr += sprintf(ptr, ",");
    ptr += lwprint_double(bbox->zmax, precision, ptr);
  }
  ptr += sprintf(ptr, "]],\"period\":{\"begin\":");
  ptr += datetimes_mfjson_buf(ptr, bbox->tmin);
  ptr += sprintf(ptr, ",\"end\":");
  ptr += datetimes_mfjson_buf(ptr, bbox->tmax);
  ptr += sprintf(ptr, "}},");
  return (ptr - output);
}

/**
 * @brief Return the maximum size in bytes of the bounding box corresponding 
 * to the temporal type represented in MF-JSON format
 */
static size_t
bbox_mfjson_size(CachedType temptype, bool hasz, int precision)
{
  size_t size;
  switch (temptype)
  {
    case T_TBOOL:
    case T_TTEXT:
      size = period_mfjson_size();
      break;
    case T_TINT:
    case T_TFLOAT:
      size = tbox_mfjson_size(precision);
      break;
    case T_TGEOMPOINT:
    case T_TGEOGPOINT:
      size = stbox_mfjson_size(hasz, precision);
      break;
    default: /* Error! */
      elog(ERROR, "Unknown temporal type: %d", temptype);
  }
  return size;
}

/**
 * @brief Write into the buffer the bounding box corresponding to the temporal type
 * represented in MF-JSON format
 */
static size_t
bbox_mfjson_buf(CachedType temptype, char *output, const bboxunion *bbox,
  bool hasz, int precision)
{
  switch (temptype)
  {
    case T_TBOOL:
    case T_TTEXT:
      return period_mfjson_buf(output, (Period *) bbox);
    case T_TINT:
    case T_TFLOAT:
      return tbox_mfjson_buf(output, (TBOX *) bbox, precision);
    case T_TGEOMPOINT:
    case T_TGEOGPOINT:
      return stbox_mfjson_buf(output, (STBOX *) bbox, hasz, precision);
    default: /* Error! */
      elog(ERROR, "Unknown temporal type: %d", temptype);
  }
}

/**
 * @brief Return the maximum size in bytes of the temporal type represented in
 * MF-JSON format
 */
static size_t
temptype_mfjson_size(CachedType temptype)
{
  size_t size;
  ensure_temporal_type(temptype);
  switch (temptype)
  {
    case T_TBOOL:
      size = sizeof("{'type':'MovingBoolean',");
      break;
    case T_TINT:
      size = sizeof("{'type':'MovingInteger',");
      break;
    case T_TFLOAT:
      size = sizeof("{'type':'MovingFloat',");
      break;
    case T_TTEXT:
      size = sizeof("{'type':'MovingText',");
      break;
    case T_TGEOMPOINT:
    case T_TGEOGPOINT:
      size = sizeof("{'type':'MovingPoint',");
      break;
    default: /* Error! */
      elog(ERROR, "Unknown temporal type: %d", temptype);
      break;
  }
  return size;
}

/**
 * @brief Write into the buffer the temporal type represented in MF-JSON format
 */
static size_t
temptype_mfjson_buf(char *output, CachedType temptype)
{
  char *ptr = output;
  ensure_temporal_type(temptype);
  switch (temptype)
  {
    case T_TBOOL:
      ptr += sprintf(ptr, "{\"type\":\"MovingBoolean\",");
      break;
    case T_TINT:
      ptr += sprintf(ptr, "{\"type\":\"MovingInteger\",");
      break;
    case T_TFLOAT:
      ptr += sprintf(ptr, "{\"type\":\"MovingFloat\",");
      break;
    case T_TTEXT:
      ptr += sprintf(ptr, "{\"type\":\"MovingText\",");
      break;
    case T_TGEOMPOINT:
    case T_TGEOGPOINT:
      ptr += sprintf(ptr, "{\"type\":\"MovingPoint\",");
      break;
    default: /* Error! */
      elog(ERROR, "Unknown temporal type: %d", temptype);
      break;
  }
  return (ptr - output);
}

/*****************************************************************************/
