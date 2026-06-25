/***********************************************************************
 * pc_schema.c
 *
 *  Pointclound schema handling. Parse and emit the XML format for
 *  representing packed multidimensional point data.
 *
 *  PgSQL Pointcloud is free and open source software provided
 *  by the Government of Canada
 *  Copyright (c) 2013 Natural Resources Canada
 *
 ***********************************************************************/

#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "pc_api_internal.h"
#include "stringbuffer.h"

static char *INTERPRETATION_STRINGS[NUM_INTERPRETATIONS] = {
    "unknown",  "int8_t",  "uint8_t",  "int16_t", "uint16_t", "int32_t",
    "uint32_t", "int64_t", "uint64_t", "double",  "float"};

static size_t INTERPRETATION_SIZES[NUM_INTERPRETATIONS] = {
    -1,    /* PC_UNKNOWN */
    1,  1, /* PC_INT8, PC_UINT8, */
    2,  2, /* PC_INT16, PC_UINT16 */
    4,  4, /* PC_INT32, PC_UINT32 */
    8,  8, /* PC_INT64, PC_UINT64 */
    8,  4  /* PC_DOUBLE, PC_FLOAT */
};

/** Convert XML string token to type interpretation number */
const char *pc_interpretation_string(uint32_t interp)
{
  if (interp < NUM_INTERPRETATIONS)
    return INTERPRETATION_STRINGS[interp];
  else
    return "unknown";
}

/** Convert XML string token to type interpretation number */
static int pc_interpretation_number(const char *str)
{
  if (str[0] == 'i' || str[0] == 'I')
  {
    if (str[3] == '8')
      return PC_INT8;
    if (str[3] == '1')
      return PC_INT16;
    if (str[3] == '3')
      return PC_INT32;
    if (str[3] == '6')
      return PC_INT64;
  }
  else if (str[0] == 'u' || str[0] == 'U')
  {
    if (str[4] == '8')
      return PC_UINT8;
    if (str[4] == '1')
      return PC_UINT16;
    if (str[4] == '3')
      return PC_UINT32;
    if (str[4] == '6')
      return PC_UINT64;
  }
  else if (str[0] == 'd' || str[0] == 'D')
  {
    return PC_DOUBLE;
  }
  else if (str[0] == 'f' || str[0] == 'F')
  {
    return PC_FLOAT;
  }
  else
    return PC_UNKNOWN;

  return PC_UNKNOWN;
}

const char *pc_compression_name(int num)
{
  switch (num)
  {
  case PC_NONE:
    return "none";
  case PC_DIMENSIONAL:
    return "dimensional";
  case PC_LAZPERF:
    return "laz";
  default:
    return "UNKNOWN";
  }
}

static int pc_compression_number(const char *str)
{
  if (!str)
    return PC_NONE;

  if ((str[0] == 'd' || str[0] == 'D') && (strcasecmp(str, "dimensional") == 0))
  {
    return PC_DIMENSIONAL;
  }

  if ((str[0] == 'l' || str[0] == 'L') && (strcasecmp(str, "laz") == 0))
  {
    return PC_LAZPERF;
  }

  if ((str[0] == 'n' || str[0] == 'N') && (strcasecmp(str, "none") == 0))
  {
    return PC_NONE;
  }

  return PC_NONE;
}

/** Convert type interpretation number size in bytes */
size_t pc_interpretation_size(uint32_t interp)
{
  if (interp < NUM_INTERPRETATIONS)
  {
    return INTERPRETATION_SIZES[interp];
  }
  else
  {
    pcerror("pc_interpretation_size: invalid interpretation");
    return 0;
  }
}

/** Allocate clean memory for a PCDIMENSION struct */
static PCDIMENSION *pc_dimension_new()
{
  PCDIMENSION *pcd = pcalloc(sizeof(PCDIMENSION));
  /* Default scaling value is 1! */
  pcd->scale = 1.0;
  return pcd;
}

static PCDIMENSION *pc_dimension_clone(const PCDIMENSION *dim)
{
  PCDIMENSION *pcd = pc_dimension_new();
  /* Copy all the inline data */
  memcpy(pcd, dim, sizeof(PCDIMENSION));
  /* Copy the referenced data */
  if (dim->name)
    pcd->name = pcstrdup(dim->name);
  if (dim->description)
    pcd->description = pcstrdup(dim->description);
  return pcd;
}

/** Release the memory behind the PCDIMENSION struct */
static void pc_dimension_free(PCDIMENSION *pcd)
{
  /* Assumption: No memory in the dimension is owned somewhere else */
  if (pcd->description)
    pcfree(pcd->description);
  if (pcd->name)
    pcfree(pcd->name);
  pcfree(pcd);
}

static PCSCHEMA *pc_schema_new(uint32_t ndims)
{
  PCSCHEMA *pcs = pcalloc(sizeof(PCSCHEMA));
  pcs->dims = pcalloc(sizeof(PCDIMENSION *) * ndims);
  pcs->namehash = create_string_hashtable();
  pcs->ndims = ndims;
  /* pcalloc memsets to 0, so xdim,ydim,zdim and mdim are already NULL */
  return pcs;
}

/** Complete the byte offsets of dimensions from the ordered sizes */
static void pc_schema_calculate_byteoffsets(PCSCHEMA *pcs)
{
  int i;
  size_t byteoffset = 0;
  for (i = 0; i < pcs->ndims; i++)
  {
    if (pcs->dims[i])
    {
      pcs->dims[i]->byteoffset = byteoffset;
      pcs->dims[i]->size = pc_interpretation_size(pcs->dims[i]->interpretation);
      byteoffset += pcs->dims[i]->size;
    }
  }
  pcs->size = byteoffset;
}

void pc_schema_set_dimension(PCSCHEMA *s, PCDIMENSION *d)
{
  s->dims[d->position] = d;
  if (d->name)
    hashtable_insert(s->namehash, d->name, d);
  pc_schema_calculate_byteoffsets(s);
}

PCSCHEMA *pc_schema_clone(const PCSCHEMA *s)
{
  int i;
  PCSCHEMA *pcs = pc_schema_new(s->ndims);
  pcs->pcid = s->pcid;
  pcs->srid = s->srid;
  pcs->compression = s->compression;
  for (i = 0; i < pcs->ndims; i++)
  {
    if (s->dims[i])
    {
      pc_schema_set_dimension(pcs, pc_dimension_clone(s->dims[i]));
    }
  }
  pcs->xdim = s->xdim ? pcs->dims[s->xdim->position] : NULL;
  pcs->ydim = s->ydim ? pcs->dims[s->ydim->position] : NULL;
  pcs->zdim = s->zdim ? pcs->dims[s->zdim->position] : NULL;
  pcs->mdim = s->mdim ? pcs->dims[s->mdim->position] : NULL;
  pc_schema_calculate_byteoffsets(pcs);
  return pcs;
}

/** Release the memory behind the PCSCHEMA struct */
void pc_schema_free(PCSCHEMA *pcs)
{
  int i;

  for (i = 0; i < pcs->ndims; i++)
  {
    if (pcs->dims[i])
    {
      pc_dimension_free(pcs->dims[i]);
      pcs->dims[i] = 0;
    }
  }
  pcfree(pcs->dims);

  if (pcs->namehash)
    hashtable_destroy(pcs->namehash, 0);

  pcfree(pcs);
}

/** Convert a PCSCHEMA to a human-readable JSON string */
char *pc_schema_to_json(const PCSCHEMA *pcs)
{
  int i;
  char *str;
  stringbuffer_t *sb = stringbuffer_create();
  stringbuffer_append(sb, "{");

  if (pcs->pcid)
    stringbuffer_aprintf(sb, "\"pcid\" : %d,\n", pcs->pcid);
  if (pcs->srid)
    stringbuffer_aprintf(sb, "\"srid\" : %d,\n", pcs->srid);
  if (pcs->compression)
    stringbuffer_aprintf(sb, "\"compression\" : %d,\n", pcs->compression);

  if (pcs->ndims)
  {

    stringbuffer_append(sb, "\"dims\" : [\n");

    for (i = 0; i < pcs->ndims; i++)
    {
      if (pcs->dims[i])
      {
        PCDIMENSION *d = pcs->dims[i];

        if (i)
          stringbuffer_append(sb, ",");
        stringbuffer_append(sb, "\n { \n");

        if (d->name)
          stringbuffer_aprintf(sb, "  \"name\" : \"%s\",\n", d->name);
        if (d->description)
          stringbuffer_aprintf(sb, "  \"description\" : \"%s\",\n",
                               d->description);

        stringbuffer_aprintf(sb, "  \"size\" : %d,\n", d->size);
        stringbuffer_aprintf(sb, "  \"byteoffset\" : %d,\n", d->byteoffset);
        stringbuffer_aprintf(sb, "  \"scale\" : %g,\n", d->scale);
        stringbuffer_aprintf(sb, "  \"interpretation\" : \"%s\",\n",
                             pc_interpretation_string(d->interpretation));
        stringbuffer_aprintf(sb, "  \"offset\" : %g,\n", d->offset);

        stringbuffer_aprintf(sb, "  \"active\" : %d\n", d->active);
        stringbuffer_append(sb, " }");
      }
    }
    stringbuffer_append(sb, "\n]\n");
  }
  stringbuffer_append(sb, "}\n");
  str = stringbuffer_getstringcopy(sb);
  stringbuffer_destroy(sb);
  return str;
}

void pc_schema_check_xyzm(PCSCHEMA *s)
{
  int i;
  for (i = 0; i < s->ndims; i++)
  {
    char *dimname = s->dims[i]->name;
    if (!dimname)
      continue;
    if (strcasecmp(dimname, "X") == 0 ||
        strcasecmp(dimname, "Longitude") == 0 ||
        strcasecmp(dimname, "Lon") == 0)
    {
      s->xdim = s->dims[i];
      continue;
    }
    if (strcasecmp(dimname, "Y") == 0 || strcasecmp(dimname, "Latitude") == 0 ||
        strcasecmp(dimname, "Lat") == 0)
    {
      s->ydim = s->dims[i];
      continue;
    }
    if (strcasecmp(dimname, "Z") == 0 || strcasecmp(dimname, "H") == 0 ||
        strcasecmp(dimname, "Height") == 0)
    {
      s->zdim = s->dims[i];
      continue;
    }
    if (strcasecmp(dimname, "M") == 0 || strcasecmp(dimname, "T") == 0 ||
        strcasecmp(dimname, "Time") == 0 || strcasecmp(dimname, "GPSTime") == 0)
    {
      s->mdim = s->dims[i];
      continue;
    }
  }
}

static char *xml_node_get_content(xmlNodePtr node)
{
  xmlNodePtr cur = node->children;
  if (cur)
  {
    do
    {
      if (cur->type == XML_TEXT_NODE)
      {
        return (char *)(cur->content);
      }
    } while ((cur = cur->next));
  }
  return "";
}

/** Population a PCSCHEMA struct from the XML representation */
PCSCHEMA *pc_schema_from_xml(const char *xml_str)
{
  xmlDocPtr xml_doc = NULL;
  xmlNodePtr xml_root = NULL;
  xmlNsPtr xml_ns = NULL;
  xmlXPathContextPtr xpath_ctx = NULL;
  xmlXPathObjectPtr xpath_obj = NULL;
  xmlNodeSetPtr nodes;
  PCSCHEMA *s = NULL;
  const char *xml_ptr = xml_str;

  /* Roll forward to start of XML string */
  while ((*xml_ptr != '\0') && (*xml_ptr != '<'))
  {
    xml_ptr++;
  }

  size_t xml_size = strlen(xml_ptr);
  static xmlChar *xpath_str = (xmlChar *)("/pc:PointCloudSchema/pc:dimension");
  static xmlChar *xpath_metadata_str =
      (xmlChar *)("/pc:PointCloudSchema/pc:metadata/Metadata");

  /* Parse XML doc */
  xmlInitParser();
  xml_doc = xmlReadMemory(xml_ptr, xml_size, NULL, NULL, 0);
  if (!xml_doc)
  {
    pcwarn("unable to parse schema XML");
    goto cleanup;
  }

  /* Capture the namespace */
  xml_root = xmlDocGetRootElement(xml_doc);
  if (xml_root->ns)
    xml_ns = xml_root->ns;

  /* Create xpath evaluation context */
  xpath_ctx = xmlXPathNewContext(xml_doc);
  if (!xpath_ctx)
  {
    pcwarn("unable to create new XPath context to read schema XML");
    goto cleanup;
  }

  /* Register the root namespace if there is one */
  if (xml_ns)
    xmlXPathRegisterNs(xpath_ctx, (xmlChar *)"pc", xml_ns->href);

  /* Evaluate xpath expression */
  xpath_obj = xmlXPathEvalExpression(xpath_str, xpath_ctx);
  if (!xpath_obj)
  {
    pcwarn("unable to evaluate xpath expression \"%s\" against schema XML",
           xpath_str);
    goto cleanup;
  }

  /* Iterate on the dimensions we found */
  if ((nodes = xpath_obj->nodesetval))
  {
    int ndims = nodes->nodeNr;
    int i;
    s = pc_schema_new(ndims);

    for (i = 0; i < ndims; i++)
    {
      /* This is a "dimension" */
      if (nodes->nodeTab[i]->type == XML_ELEMENT_NODE)
      {
        xmlNodePtr cur = nodes->nodeTab[i];
        xmlNodePtr child;
        PCDIMENSION *d = pc_dimension_new();

        /* These are the values of the dimension */
        for (child = cur->children; child; child = child->next)
        {
          if (child->type == XML_ELEMENT_NODE && child->children != NULL)
          {
            char *content = (char *)(child->children->content);
            char *name = (char *)(child->name);
            if (strcmp(name, "name") == 0)
              d->name = pcstrdup(content);
            else if (strcmp(name, "description") == 0)
              d->description = pcstrdup(content);
            else if (strcmp(name, "size") == 0)
              d->size = atoi(content);
            else if (strcmp(name, "active") == 0)
              d->active = atoi(content);
            else if (strcmp(name, "position") == 0)
              d->position = atoi(content) - 1;
            else if (strcmp(name, "interpretation") == 0)
              d->interpretation = pc_interpretation_number(content);
            else if (strcmp(name, "scale") == 0)
              d->scale = atof(content);
            else if (strcmp(name, "offset") == 0)
              d->offset = atof(content);
            else if (strcmp(name, "uuid") == 0)
            /* Ignore this tag for now */ {
            }
            else if (strcmp(name, "parent_uuid") == 0)
            /* Ignore this tag for now */ {
            }
            else
              pcinfo("unhandled schema type element \"%s\" "
                     "encountered",
                     name);
          }
        }

        /* Convert interprestation to size */
        d->size = pc_interpretation_size(d->interpretation);

        /* Store the dimension in the schema */
        if (d->position >= ndims)
        {
          pcwarn("schema dimension states position \"%d\", but "
                 "number of XML "
                 "dimensions is \"%d\"",
                 d->position + 1, ndims);
          pc_dimension_free(d);
          goto cleanup;
        }
        else if (s->dims[d->position])
        {
          pcwarn("schema dimension at position \"%d\" is declared twice",
                 d->position + 1, ndims);
          pc_dimension_free(d);
          goto cleanup;
        }
        pc_schema_set_dimension(s, d);
      }
    }

    /* Complete the byte offsets of dimensions from the ordered sizes */
    pc_schema_calculate_byteoffsets(s);
    /* Check XYZM positions */
    pc_schema_check_xyzm(s);
  }

  xmlXPathFreeObject(xpath_obj);

  /* SEARCH FOR METADATA ENTRIES */
  xpath_obj = xmlXPathEvalExpression(xpath_metadata_str, xpath_ctx);
  if (!xpath_obj)
  {
    pcwarn("unable to evaluate xpath expression \"%s\" against schema XML",
           xpath_metadata_str);
    goto cleanup;
  }

  /* Iterate on the <Metadata> we find */
  if ((nodes = xpath_obj->nodesetval))
  {
    int i;

    for (i = 0; i < nodes->nodeNr; i++)
    {
      char *metadata_name = "";
      char *metadata_value = "";
      /* Read the metadata name and value from the node */
      /* <Metadata name="somename">somevalue</Metadata> */
      xmlNodePtr cur = nodes->nodeTab[i];
      if (cur->type == XML_ELEMENT_NODE &&
          strcmp((char *)(cur->name), "Metadata") == 0)
      {
        metadata_name = (char *)xmlGetProp(cur, (xmlChar *)"name");
        metadata_value = xml_node_get_content(cur);
      }

      /* Store the compression type on the schema */
      if (strcmp(metadata_name, "compression") == 0)
      {
        int compression = pc_compression_number(metadata_value);
        if (compression >= 0)
        {
          s->compression = compression;
        }
      }
      xmlFree(metadata_name);
    }
  }

cleanup:
  if (s && !pc_schema_is_valid(s))
  {
    pc_schema_free(s);
    s = NULL;
  }

  if (xpath_obj)
    xmlXPathFreeObject(xpath_obj);
  if (xpath_ctx)
    xmlXPathFreeContext(xpath_ctx);
  if (xml_doc)
    xmlFreeDoc(xml_doc);
  xmlCleanupParser();

  return s;
}

uint32_t pc_schema_is_valid(const PCSCHEMA *s)
{
  int i;

  if (!s->xdim)
  {
    pcwarn("schema does not include an X coordinate");
    return PC_FALSE;
  }

  if (!s->ydim)
  {
    pcwarn("schema does not include a Y coordinate");
    return PC_FALSE;
  }

  if (!s->ndims)
  {
    pcwarn("schema has no dimensions");
    return PC_FALSE;
  }

  for (i = 0; i < s->ndims; i++)
  {
    if (!s->dims[i])
    {
      pcwarn("schema is missing a dimension at position %d", i);
      return PC_FALSE;
    }
  }

  return PC_TRUE;
}

PCDIMENSION *pc_schema_get_dimension(const PCSCHEMA *s, uint32_t dim)
{
  if (s && s->ndims > dim)
  {
    return s->dims[dim];
  }
  return NULL;
}

PCDIMENSION *pc_schema_get_dimension_by_name(const PCSCHEMA *s,
                                             const char *name)
{
  if (!(s && s->namehash))
    return NULL;

  return hashtable_search(s->namehash, name);
}

size_t pc_schema_get_size(const PCSCHEMA *s) { return s->size; }

/**
 * Return true if the schemas have the same dimensions with the same
 * interpretations and at the same locations. The scales and offsets
 * may be different though. Otherwise return false.
 */
uint32_t pc_schema_same_dimensions(const PCSCHEMA *s1, const PCSCHEMA *s2)
{
  size_t i;

  if (s1->ndims != s2->ndims)
    return PC_FALSE;

  for (i = 0; i < s1->ndims; i++)
  {
    PCDIMENSION *s1dim = s1->dims[i];
    PCDIMENSION *s2dim = s2->dims[i];

    if (strcasecmp(s1dim->name, s2dim->name) != 0)
      return PC_FALSE;

    if (s1dim->interpretation != s2dim->interpretation)
      return PC_FALSE;
  }

  return PC_TRUE;
}

/**
 * Return false if s1 and s2 don't have the same srids, or if there are
 * dimensions in s2 that are also in s1 but don't have the same interpretations,
 * scales or offsets. Otherwise return true. The function is used to determine
 * if re-interpretating the patch data is required when changing from one schema
 * (s1) to another (s2).
 */
uint32_t pc_schema_same_interpretations(const PCSCHEMA *s1, const PCSCHEMA *s2)
{
  size_t i;

  if (s1->srid != s2->srid)
    return PC_FALSE;

  for (i = 0; i < s2->ndims; i++)
  {
    PCDIMENSION *s2dim = s2->dims[i];
    PCDIMENSION *s1dim = pc_schema_get_dimension_by_name(s1, s2dim->name);

    if (s1dim)
    {
      if (s1dim->interpretation != s2dim->interpretation)
        return PC_FALSE;

      if (s1dim->scale != s2dim->scale)
        return PC_FALSE;

      if (s1dim->offset != s2dim->offset)
        return PC_FALSE;
    }
  }

  return PC_TRUE;
}
