/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief MEOS-owned cache of parsed pgPointCloud PCSCHEMA values,
 *   keyed by pcid.  See @c meos/include/pointcloud/meos_schema_hook.h
 *   for the design rationale.
 */

/* PostgreSQL */
#include <postgres.h>
#if ! MEOS
  #include <utils/memutils.h>  /* TopMemoryContext */
#endif
/* pgPointcloud */
#include "pc_api.h"
#include "pc_api_internal.h"
/* libxml2 — the pointcloud family already links it for pc_schema_from_xml */
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "stringbuffer.h"
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>
#include "temporal/temporal.h"
#include "pointcloud/meos_schema_hook.h"

/*****************************************************************************
 * Cache state
 *****************************************************************************/

/**
 * @brief
 */
typedef struct schema_entry {
  uint32_t pcid;
  PCSCHEMA *schema;
  int32_t srid;      /* cached so callers need not dereference PCSCHEMA */
  char *xml_text;    /* NULL if registered without XML */
} schema_entry;

/* Small dynamic array; linear scan.  Workloads rarely exceed a handful
 * of pcids loaded at once, so a hash table is over-engineering. */
static schema_entry *cache_buf = NULL;
static int cache_count = 0;
static int cache_cap = 0;

meos_pc_schema_fn_t meos_pc_schema_fn = NULL;
meos_pc_parse_xml_fn_t meos_pc_parse_xml_fn = NULL;

/* Registration, answering whether the cache took the schema; the public
 * entries below answer nothing, so a caller that must know reads this */
static bool schema_register(uint32_t pcid, PCSCHEMA *schema,
  const char *xml_text);

/*****************************************************************************
 * Public API
 *****************************************************************************/

/**
 * @brief Internal helper — copy @p xml into long-lived memory.
 * @return palloc'd cstring (TopMemoryContext on PG, malloc on standalone)
 * or NULL when @p xml is NULL.
 */
static char *
copy_xml_long_lived(const char *xml)
{
  if (! xml)
    return NULL;
  size_t len = strlen(xml);
#if ! MEOS
  MemoryContext oldctx = MemoryContextSwitchTo(TopMemoryContext);
#endif
  char *out = palloc(len + 1);
  memcpy(out, xml, len + 1);
#if ! MEOS
  MemoryContextSwitchTo(oldctx);
#endif
  return out;
}

/**
 * @brief Internal helper — make room for one more cache entry.
 */
static void
ensure_cache_capacity(void)
{
  if (cache_count < cache_cap)
    return;
  int new_cap = cache_cap ? cache_cap * 2 : 8;
#if ! MEOS
  MemoryContext oldctx = MemoryContextSwitchTo(TopMemoryContext);
#endif
  schema_entry *new_buf = palloc(sizeof(schema_entry) * new_cap);
  if (cache_buf)
  {
    memcpy(new_buf, cache_buf, sizeof(schema_entry) * cache_count);
    pfree(cache_buf);
  }
  cache_buf = new_buf;
  cache_cap = new_cap;
#if ! MEOS
  MemoryContextSwitchTo(oldctx);
#endif
}

/**
 * @brief Return true if a schema is one the point cloud library accepts
 * @details The library states what a schema must hold — an X and a Y
 *   dimension, at least one dimension, and a dimension at every position of
 *   its layout — and every reader of the cache takes those for granted. The
 *   document parser applies this test itself, so a schema reaching the cache
 *   any other way is the one that would otherwise arrive unchecked.
 */
static bool
pcschema_registrable(uint32_t pcid, const PCSCHEMA *schema)
{
  if (! schema)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The schema of pcid %u is null", pcid);
    return false;
  }
  if (! pc_schema_is_valid(schema))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The schema of pcid %u is not one the point cloud library accepts: it "
      "states an X and a Y dimension and a dimension at every position of its "
      "layout", pcid);
    return false;
  }
  return true;
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Register a parsed PCSCHEMA in the MEOS-owned cache.
 */
void
meos_pc_schema_register(uint32_t pcid, PCSCHEMA *schema)
{
  meos_pc_schema_register_xml(pcid, schema, NULL);
}

/**
 * @brief Return a copy of a string allocated by the point cloud library, so
 * that freeing the schema releases it
 */
static char *
pcschema_string_copy(const char *str)
{
  size_t size = strlen(str) + 1;
  char *result = pcalloc(size);
  memcpy(result, str, size);
  return result;
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Register the schema that a point cloud identifier names, stated as
 *   its dimensions
 * @param[in] pcid Identifier the schema is registered under
 * @param[in] srid Spatial reference identifier every value of the schema
 *   carries
 * @param[in] compression Name of the compression applied to the data,
 *   @p NULL for none
 * @param[in] dims Dimensions the schema states, in any order
 * @param[in] ndims Number of dimensions
 * @return A newly allocated schema, @p NULL on error
 * @note The schema is built through the constructor of the point cloud
 *   library, so the size of a dimension, the offset of a dimension within a
 *   point and the width of a point are the ones that library computes
 */
PCSCHEMA *
meos_pc_schema_from_dims(uint32_t pcid, int32_t srid,
  const char *compression, const PCDimensionSpec *dims, int ndims)
{
  /* Ensure the validity of the arguments */
  if (! ensure_not_null((void *) dims))
    return NULL;
  if (ndims < 1)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The point cloud schema of pcid %u must state at least one dimension",
      pcid);
    return NULL;
  }

  int comp = pc_compression_number(compression);
  PCSCHEMA *schema = pc_schema_new((uint32_t) ndims);
  schema->pcid = pcid;
  schema->srid = (uint32_t) srid;
  schema->compression = (uint32_t) comp;

  for (int i = 0; i < ndims; i++)
  {
    const PCDimensionSpec *spec = &dims[i];
    /* A position is stated from 1 while the schema holds it from 0 */
    if (spec->position < 1 || spec->position > ndims)
    {
      pc_schema_free(schema);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "The point cloud schema of pcid %u states position %d, which is "
        "outside the %d dimensions it declares", pcid, spec->position, ndims);
      return NULL;
    }
    if (! spec->name || ! *spec->name)
    {
      pc_schema_free(schema);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "The point cloud schema of pcid %u states a dimension with no name "
        "at position %d", pcid, spec->position);
      return NULL;
    }
    int interp = pc_interpretation_number(spec->interpretation);
    if (interp == PC_UNKNOWN)
    {
      pc_schema_free(schema);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "The point cloud schema of pcid %u states the unknown interpretation "
        "\"%s\" for dimension \"%s\"", pcid,
        spec->interpretation ? spec->interpretation : "", spec->name);
      return NULL;
    }
    if (schema->dims[spec->position - 1])
    {
      pc_schema_free(schema);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "The point cloud schema of pcid %u states position %d twice",
        pcid, spec->position);
      return NULL;
    }

    PCDIMENSION *dim = pcalloc(sizeof(PCDIMENSION));
    dim->position = (uint32_t) spec->position - 1;
    dim->name = pcschema_string_copy(spec->name);
    dim->description = spec->description ?
      pcschema_string_copy(spec->description) : NULL;
    dim->interpretation = (uint32_t) interp;
    dim->scale = spec->scale;
    dim->offset = spec->offset;
    dim->active = spec->active ? 1 : 0;
    /* Sets the dimension, its name in the hash table of the schema, and the
     * size and the byte offset of every dimension */
    pc_schema_set_dimension(schema, dim);
  }

  /* Resolve the X, Y, Z and M dimensions from the names */
  pc_schema_check_xyzm(schema);
  return schema;
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Register the schema that a point cloud identifier names, stated as
 *   its dimensions
 * @param[in] pcid Identifier the schema is registered under
 * @param[in] srid Spatial reference identifier every value of the schema
 *   carries
 * @param[in] compression Name of the compression applied to the data,
 *   @p NULL for none
 * @param[in] dims Dimensions the schema states, in any order
 * @param[in] ndims Number of dimensions
 * @return True on success, false on error
 * @sqlfn pointCloudSchemaRegister()
 */
bool
meos_pc_schema_register_dims(uint32_t pcid, int32_t srid,
  const char *compression, const PCDimensionSpec *dims, int ndims)
{
  PCSCHEMA *schema = meos_pc_schema_from_dims(pcid, srid, compression, dims,
    ndims);
  if (! schema)
    return false;
  /* Registration refuses a schema the library does not accept, and nothing
   * else owns one it refused */
  if (! schema_register(pcid, schema, NULL))
  {
    pc_schema_free(schema);
    return false;
  }
  return true;
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Register a parsed PCSCHEMA along with its source XML in the
 *   MEOS-owned cache.
 */
/**
 * @brief Register a schema, answering whether the cache took it
 * @details The two public entries answer nothing, so this is what a caller
 *   that must know reads — the registration entry stating its own outcome and
 *   the lookup deciding what to answer for a schema a hook supplied.
 */
static bool
schema_register(uint32_t pcid, PCSCHEMA *schema, const char *xml_text)
{
  /* If already present, replace; preserve previously-cached XML when
   * the new call passes NULL for xml_text (so a parse-only re-register
   * doesn't accidentally drop a prior XML registration). */
  if (! pcschema_registrable(pcid, schema))
    return false;
  int32_t srid = (int32_t) schema->srid;
  for (int i = 0; i < cache_count; i++)
  {
    if (cache_buf[i].pcid == pcid)
    {
      cache_buf[i].schema = schema;
      cache_buf[i].srid = srid;
      if (xml_text)
      {
        if (cache_buf[i].xml_text)
          pfree(cache_buf[i].xml_text);
        cache_buf[i].xml_text = copy_xml_long_lived(xml_text);
      }
      return true;
    }
  }
  ensure_cache_capacity();
  cache_buf[cache_count].pcid = pcid;
  cache_buf[cache_count].schema = schema;
  cache_buf[cache_count].srid = srid;
  cache_buf[cache_count].xml_text = copy_xml_long_lived(xml_text);
  cache_count++;
  return true;
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Variant of @ref meos_pc_schema_register that also caches the document
 *   a schema is stated by
 */
void
meos_pc_schema_register_xml(uint32_t pcid, PCSCHEMA *schema,
  const char *xml_text)
{
  (void) schema_register(pcid, schema, xml_text);
}

/**
 * @brief Return the pgPointCloud document describing a schema
 * @details The element set is the one @c pc_schema_from_xml reads and the
 *   layout is the one the library's own documents carry: a @c pc:dimension
 *   per dimension holding its position, size, name, interpretation, scale,
 *   offset and active flag, and a @c pc:metadata naming the compression.
 *   @c pc_interpretation_string and @c pc_compression_number are the
 *   library's, so the spelling of an interpretation and of a compression is
 *   the library's too.
 * @note The document is what the library parses, so the only proof that it is
 *   right is the library reading it back: the test registers a schema as rows
 *   and compares the schema this document parses to the schema the rows build.
 */
static char *
pc_schema_as_xml(const PCSCHEMA *schema)
{
  assert(schema);
  const char *comp = "none";
  if (schema->compression == PC_DIMENSIONAL)
    comp = "dimensional";
  else if (schema->compression == PC_LAZPERF)
    comp = "laz";

  stringbuffer_t *sb = stringbuffer_create();
  stringbuffer_append(sb,
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<pc:PointCloudSchema xmlns:pc=\"http://pointcloud.org/schemas/PC/1.1\"\n"
    "    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n");
  for (int i = 0; i < (int) schema->ndims; i++)
  {
    const PCDIMENSION *d = schema->dims[i];
    if (! d)
      continue;
    stringbuffer_append(sb, "  <pc:dimension>\n");
    /* The parser reads a position counting from one and subtracts one */
    stringbuffer_aprintf(sb, "    <pc:position>%u</pc:position>\n",
      d->position + 1);
    stringbuffer_aprintf(sb, "    <pc:size>%u</pc:size>\n", d->size);
    stringbuffer_aprintf(sb, "    <pc:name>%s</pc:name>\n", d->name);
    stringbuffer_aprintf(sb,
      "    <pc:interpretation>%s</pc:interpretation>\n",
      pc_interpretation_string(d->interpretation));
    stringbuffer_aprintf(sb, "    <pc:scale>%.17g</pc:scale>\n", d->scale);
    stringbuffer_aprintf(sb, "    <pc:offset>%.17g</pc:offset>\n", d->offset);
    /* The parser reads the flag with atoi, so it is a number, not a word */
    stringbuffer_aprintf(sb, "    <pc:active>%u</pc:active>\n",
      (uint32_t) d->active);
    stringbuffer_append(sb, "  </pc:dimension>\n");
  }
  stringbuffer_append(sb, "  <pc:metadata>\n");
  stringbuffer_aprintf(sb,
    "    <Metadata name=\"compression\">%s</Metadata>\n", comp);
  stringbuffer_append(sb, "  </pc:metadata>\n"
    "</pc:PointCloudSchema>\n");
  char *result = stringbuffer_getstringcopy(sb);
  stringbuffer_destroy(sb);
  return result;
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Return the pgPointCloud document of a registered pcid
 * @details A schema parsed from a document keeps the document it came from. A
 *   schema stated as dimensions has none, so one describing it is rendered on
 *   the first call and kept, which is what lets a schema stated either way
 *   reach a reader that takes a document.
 * @return On a pcid no schema is registered for return @p NULL
 */
const char *
meos_pc_schema_xml(uint32_t pcid)
{
  for (int i = 0; i < cache_count; i++)
  {
    if (cache_buf[i].pcid != pcid)
      continue;
    if (! cache_buf[i].xml_text && cache_buf[i].schema)
    {
      char *rendered = pc_schema_as_xml(cache_buf[i].schema);
      /* The cache outlives the call, so the document is copied the way a
       * parsed one is. ⛔ The stringbuffer symbols the link resolves are
       * PostGIS's, not pgPointCloud's (see pc_stringbuffer_shim.c), so the
       * buffer comes from lwalloc and is released with pfree, never free */
      cache_buf[i].xml_text = copy_xml_long_lived(rendered);
      pfree(rendered);
    }
    return cache_buf[i].xml_text;
  }
  return NULL;
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Drop every entry from the MEOS schema cache.
 */
void
meos_pc_schema_clear(void)
{
  if (cache_buf)
  {
    for (int i = 0; i < cache_count; i++)
    {
      if (cache_buf[i].xml_text)
        pfree(cache_buf[i].xml_text);
    }
    pfree(cache_buf);
    cache_buf = NULL;
  }
  cache_count = 0;
  cache_cap = 0;
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Return the cached SRID for a pcid (SRID_INVALID on miss).
 *
 * The SRID is extracted from the PCSCHEMA at registration time so
 * callers (e.g. spatial_srid in tspatial_srid.c) do not need the full
 * PCSCHEMA struct definition.
 * @sqlfn pointCloudSchemaSRID()
 */
int32_t
meos_pc_schema_srid(uint32_t pcid)
{
  /* Cache hit */
  for (int i = 0; i < cache_count; i++)
  {
    if (cache_buf[i].pcid == pcid)
      return cache_buf[i].srid;
  }
  /* Trigger hook so the schema is registered; then retry.  The quiet
   * resolution is what states a miss as SRID_INVALID, which is the answer
   * this function documents. */
  const PCSCHEMA *s = meos_pc_schema_lookup(pcid);
  if (s)
  {
    for (int i = 0; i < cache_count; i++)
    {
      if (cache_buf[i].pcid == pcid)
        return cache_buf[i].srid;
    }
  }
  return SRID_INVALID;
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Return the compression a point cloud schema states
 * @param[in] pcid Identifier of the schema
 * @return On a pcid no schema is registered for return @p NULL
 * @details The name the catalog states is answered — @p none, @p dimensional
 *   or @p laz — which is what the SQL function of the same name answers. The
 *   compression field of the schema is an enumerator of the point cloud
 *   library, declared in its own header, so a number leaves a caller of this
 *   surface nothing to decode it with
 * @note The result is a string constant owned by that library, so a caller
 *   reads it and never frees it
 * @sqlfn pointCloudSchemaCompression()
 */
const char *
meos_pc_schema_compression(uint32_t pcid)
{
  const PCSCHEMA *s = meos_pc_schema_lookup(pcid);
  return s ? pc_compression_name((int) s->compression) : NULL;
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Return the number of active dimensions a point cloud schema states
 * @param[in] pcid Identifier of the schema
 * @return On a pcid no schema is registered for return -1
 * @details The ACTIVE dimensions are counted, which is what the SQL function
 *   of the same name answers. The @p ndims field of the schema is the width
 *   of its layout: an inactive dimension still occupies a slot and still
 *   contributes to the width of a point, so the byte offsets are indexed by
 *   it and it is a different quantity, not this one under another name
 * @sqlfn pointCloudSchemaNDims()
 */
int32_t
meos_pc_schema_ndims(uint32_t pcid)
{
  const PCSCHEMA *s = meos_pc_schema_lookup(pcid);
  if (! s)
    return -1;
  int32_t result = 0;
  for (uint32_t i = 0; i < s->ndims; i++)
    if (s->dims[i] && s->dims[i]->active)
      result++;
  return result;
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Resolve a parsed PCSCHEMA by pcid, with hook fallback, answering
 *   @p NULL where neither the cache nor a hook holds one.
 *
 * A caller that cannot answer without the schema asks through
 * @ref meos_pc_schema, which states the miss as an error; a caller reading a
 * property it can answer without one asks here and reads the miss itself.
 */
/*****************************************************************************
 * The document a host with no SQL states its schemas in
 *
 * A host with SQL states a schema as rows of pointcloud_schemas and
 * pointcloud_dimensions and installs the hook above, which reads them. A host
 * with no SQL has no table to write into, so it states the same schemas in a
 * pgPointCloud document, exactly as spatial reference systems reach a
 * standalone MEOS through a CSV that meos_set_spatial_ref_sys_csv names and
 * the road network through the one meos_set_ways_csv names.
 *
 * ⛔ The file is never a prerequisite: meos_pc_schema_register_dims alone
 * registers a schema, so a host without a filesystem is not blocked.
 *****************************************************************************/

/* Location of the point cloud schemas document, which a host states before a
 * schema is looked up. ⛔ There is NO default read, unlike the spatial
 * reference systems a standalone MEOS reads from a vendored CSV without being
 * asked: an EPSG code means the same thing everywhere, while a pcid names one
 * user's instrument, so a bundled schema answering pcid 1 would decode a value
 * of somebody else's pcid 1 into silently wrong coordinates rather than
 * reporting that no schema is registered. The document beside this file is a
 * starting point to copy, and a host names it like any other. */
char *POINTCLOUD_SCHEMAS_XML = NULL;

/* True once the document has been read, so a miss on a pcid it does not state
 * does not re-read it on every lookup */
static bool schemas_xml_read = false;

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Set the file the point cloud schemas are read from
 * @param[in] path Location of the document
 */
void
meos_set_pointcloud_schemas_xml(const char *path)
{
  if (! path)
    return;
  char *copy = malloc(strlen(path) + 1);
  if (! copy)
    return;
  strcpy(copy, path);
  POINTCLOUD_SCHEMAS_XML = copy;
  /* A path named after a lookup already missed is still read */
  schemas_xml_read = false;
}

/**
 * @brief Register every schema the document states
 * @details The enclosing element carries the identifier and the reference
 *   system of each schema, which a pgPointCloud document states nowhere: it
 *   holds the dimensions of one schema, carries no identifier, and its parser
 *   reads only the compression out of the metadata, so a document stating
 *   `spatialreference` still parses to srid 0. Each schema element is handed
 *   to the library's own parser verbatim, so the dimensions are read by
 *   pgPointCloud and by nothing of ours.
 */
static void
read_pointcloud_schemas_xml(void)
{
  schemas_xml_read = true;
  if (! POINTCLOUD_SCHEMAS_XML)
    return;
  xmlDoc *doc = xmlReadFile(POINTCLOUD_SCHEMAS_XML, NULL, XML_PARSE_NOERROR |
    XML_PARSE_NOWARNING);
  if (! doc)
    return;
  xmlNode *root = xmlDocGetRootElement(doc);
  for (xmlNode *cur = root ? root->children : NULL; cur; cur = cur->next)
  {
    if (cur->type != XML_ELEMENT_NODE ||
        strcmp((const char *) cur->name, "PointCloudSchema") != 0)
      continue;
    /* A schema element states nothing without the identifier a value carries */
    xmlChar *pcid_str = xmlGetProp(cur, (const xmlChar *) "pcid");
    if (! pcid_str)
      continue;
    xmlChar *srid_str = xmlGetProp(cur, (const xmlChar *) "srid");
    {
      /* Serialise the schema element back out so the library's parser reads
       * the document it defines, not a tree of ours.
       * ⛔ The element is copied into a document of its own and its namespaces
       * reconciled first: the `pc` prefix is declared on the ENCLOSING
       * element, so dumping the subtree alone yields prefixes bound to
       * nothing, and the library's parser answers `Namespace prefix pc on
       * PointCloudSchema is not defined` followed by an undefined-prefix XPath
       * error. */
      xmlDoc *one = xmlNewDoc((const xmlChar *) "1.0");
      xmlNode *copy = one ? xmlDocCopyNode(cur, one, 1) : NULL;
      if (copy)
      {
        xmlDocSetRootElement(one, copy);
        xmlReconciliateNs(one, copy);
        xmlChar *text = NULL;
        int len = 0;
        xmlDocDumpMemory(one, &text, &len);
        if (text)
        {
          PCSCHEMA *schema = pc_schema_from_xml((const char *) text);
          if (schema)
          {
            schema->pcid = (uint32_t) atoi((const char *) pcid_str);
            schema->srid = srid_str ?
              (uint32_t) atoi((const char *) srid_str) : 0;
            meos_pc_schema_register(schema->pcid, schema);
          }
          xmlFree(text);
        }
      }
      if (one)
        xmlFreeDoc(one);
    }
    xmlFree(pcid_str);
    if (srid_str)
      xmlFree(srid_str);
  }
  xmlFreeDoc(doc);
}

PCSCHEMA *
meos_pc_schema_lookup(uint32_t pcid)
{
  /* (1) Cache hit */
  for (int i = 0; i < cache_count; i++)
  {
    if (cache_buf[i].pcid == pcid)
      return cache_buf[i].schema;
  }
  /* (2) Hook fallback (e.g. PG catalog scan).  Registers on success
   * so subsequent lookups hit the cache directly. */
  if (meos_pc_schema_fn)
  {
    PCSCHEMA *s = meos_pc_schema_fn(pcid);
    if (! s)
      return NULL;
    /* Registration refuses a schema the library does not accept, and a schema
     * the cache would not take is not one to answer with either */
    if (! schema_register(pcid, s, NULL))
      return NULL;
    return s;
  }
  /* (3) A host that installs no hook states its schemas in a document */
  if (! schemas_xml_read)
  {
    read_pointcloud_schemas_xml();
    for (int i = 0; i < cache_count; i++)
    {
      if (cache_buf[i].pcid == pcid)
        return cache_buf[i].schema;
    }
  }
  return NULL;
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Resolve a parsed PCSCHEMA by pcid, with hook fallback, stating the
 *   absence of any facility that could resolve one as an error.
 *
 * A hook that answers @p NULL has looked the pcid up and not found it, which
 * the caller reads as the miss it is; no hook at all means the pcid could not
 * be looked up at all, and a caller needing the schema is stuck.
 */
PCSCHEMA *
meos_pc_schema(uint32_t pcid)
{
  PCSCHEMA *result = meos_pc_schema_lookup(pcid);
  if (! result && ! meos_pc_schema_fn)
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "PCSCHEMA for pcid %u not registered and no fallback hook "
      "installed — pre-populate via meos_pc_schema_register, or "
      "(in a PG backend) ensure mobilitydb_init has run", pcid);
  return result;
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Install the pgPointCloud library handlers for standalone MEOS.
 * @details The bundled libpc.a leaves its allocator, deallocator and
 *   message handlers as NULL function pointers until a host installs
 *   them. The PG backend does this in @c mobilitydb_init via
 *   @c pc_set_handlers (palloc-based). A standalone MEOS program has no
 *   such entry point, so the first libpc call that allocates or reports
 *   (@c pc_schema_from_xml, @c pc_point_get_x, …) dereferences a NULL
 *   handler and crashes. Installing the default system allocator/message
 *   handlers here makes a standalone MEOS process symmetric with the PG
 *   backend. Called from @c meos_initialize under @c #if POINTCLOUD.
 */
void
meos_initialize_pointcloud(void)
{
  pc_install_default_handlers();
}

/*****************************************************************************/
