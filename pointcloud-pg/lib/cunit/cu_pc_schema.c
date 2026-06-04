/***********************************************************************
 * cu_pc_schema.c
 *
 *        Testing for the schema API functions
 *
 * Portions Copyright (c) 2012, OpenGeo
 *
 ***********************************************************************/

#include "CUnit/Basic.h"
#include "cu_tester.h"

/* GLOBALS ************************************************************/

static PCSCHEMA *schema = NULL;
static const char *xmlfile = "data/pdal-schema.xml";

/* Setup/teardown for this suite */
static int init_suite(void)
{
  char *xmlstr = file_to_str(xmlfile);
  schema = pc_schema_from_xml(xmlstr);
  pcfree(xmlstr);
  return schema ? 0 : -1;
}

static int clean_suite(void)
{
  pc_schema_free(schema);
  return 0;
}

/* TESTS **************************************************************/

static void test_schema_from_xml()
{
  char *xmlstr = file_to_str(xmlfile);
  PCSCHEMA *myschema = pc_schema_from_xml(xmlstr);
  CU_ASSERT_PTR_NOT_NULL(myschema);
  pcfree(xmlstr);

  // char *schemastr = pc_schema_to_json(schema);
  // printf("ndims %d\n", schema->ndims);
  // printf("name0 %s\n", schema->dims[0]->name);
  // printf("%s\n", schemastr);

  pc_schema_free(myschema);
}

static void test_schema_from_xml_with_empty_description()
{
  char *myxmlfile = "data/simple-schema-empty-description.xml";
  char *xmlstr = file_to_str(myxmlfile);
  PCSCHEMA *myschema = pc_schema_from_xml(xmlstr);

  CU_ASSERT_PTR_NOT_NULL(myschema);

  pc_schema_free(myschema);
  pcfree(xmlstr);
}

static void test_schema_from_xml_with_no_name()
{
  char *myxmlfile = "data/simple-schema-no-name.xml";
  char *xmlstr = file_to_str(myxmlfile);
  PCSCHEMA *myschema = pc_schema_from_xml(xmlstr);

  CU_ASSERT_PTR_NOT_NULL(myschema);

  pc_schema_free(myschema);
  pcfree(xmlstr);
}

static void test_schema_from_xml_with_empty_name()
{
  char *myxmlfile = "data/simple-schema-empty-name.xml";
  char *xmlstr = file_to_str(myxmlfile);
  PCSCHEMA *myschema = pc_schema_from_xml(xmlstr);

  CU_ASSERT_PTR_NOT_NULL(myschema);

  pc_schema_free(myschema);
  pcfree(xmlstr);
}

static void test_schema_size()
{
  size_t sz = schema->size;
  CU_ASSERT_EQUAL(sz, 37);
}

static void test_dimension_get()
{
  PCDIMENSION *d;

  d = pc_schema_get_dimension(schema, 0);
  CU_ASSERT_EQUAL(d->position, 0);
  CU_ASSERT_STRING_EQUAL(d->name, "X");

  d = pc_schema_get_dimension(schema, 1);
  CU_ASSERT_EQUAL(d->position, 1);
  CU_ASSERT_STRING_EQUAL(d->name, "Y");

  d = pc_schema_get_dimension_by_name(schema, "nothinghere");
  CU_ASSERT_EQUAL(d, NULL);

  d = pc_schema_get_dimension_by_name(schema, "Z");
  CU_ASSERT_EQUAL(d->position, 2);
  CU_ASSERT_STRING_EQUAL(d->name, "Z");

  d = pc_schema_get_dimension_by_name(schema, "z");
  CU_ASSERT_EQUAL(d->position, 2);
  CU_ASSERT_STRING_EQUAL(d->name, "Z");

  d = pc_schema_get_dimension_by_name(schema, "y");
  // printf("name %s\n", d->name);
  // printf("position %d\n", d->position);
  CU_ASSERT_EQUAL(d->position, 1);
  CU_ASSERT_STRING_EQUAL(d->name, "Y");
}

static void test_dimension_byteoffsets()
{
  PCDIMENSION *d;
  int i;
  int prev_byteoffset;
  int prev_size;
  int pc_size;

  for (i = 0; i < schema->ndims; i++)
  {
    d = pc_schema_get_dimension(schema, i);
    // printf("d=%d name='%s' size=%d byteoffset=%d\n", i, d->name, d->size,
    // d->byteoffset);
    if (i > 0)
    {
      CU_ASSERT_EQUAL(prev_size, pc_size);
      CU_ASSERT_EQUAL(prev_size, d->byteoffset - prev_byteoffset);
    }
    prev_byteoffset = d->byteoffset;
    prev_size = d->size;
    pc_size = pc_interpretation_size(d->interpretation);
  }
}

static void test_schema_invalid_xy()
{
  // See https://github.com/pgpointcloud/pointcloud/issues/28
  char *xmlstr =
      "<pc:PointCloudSchema "
      "xmlns:pc='x'><pc:dimension>1</pc:dimension></pc:PointCloudSchema>";
  PCSCHEMA *myschema = pc_schema_from_xml(xmlstr);
  CU_ASSERT_PTR_NULL(myschema);
}

static void test_schema_missing_dimension()
{
  char *myxmlfile = "data/simple-schema-missing-dimension.xml";
  char *xmlstr = file_to_str(myxmlfile);
  PCSCHEMA *myschema = pc_schema_from_xml(xmlstr);

  CU_ASSERT_PTR_NULL(myschema);

  pcfree(xmlstr);
}

static void test_schema_empty()
{
  char *xmlstr = "";
  PCSCHEMA *myschema = pc_schema_from_xml(xmlstr);

  CU_ASSERT_PTR_NULL(myschema);
}

static void test_schema_compression(void)
{
  int compression = schema->compression;
  CU_ASSERT_EQUAL(compression, PC_DIMENSIONAL);
}

static void test_schema_clone(void)
{
  int i;
  PCSCHEMA *clone = pc_schema_clone(schema);
  hashtable *hash, *chash;
  CU_ASSERT_EQUAL(clone->pcid, schema->pcid);
  CU_ASSERT_EQUAL(clone->ndims, schema->ndims);
  CU_ASSERT_EQUAL(clone->size, schema->size);
  CU_ASSERT_EQUAL(clone->srid, schema->srid);
  CU_ASSERT_EQUAL(clone->xdim->position, schema->xdim->position);
  CU_ASSERT_EQUAL(clone->ydim->position, schema->ydim->position);
  CU_ASSERT_EQUAL(clone->zdim->position, schema->zdim->position);
  CU_ASSERT_EQUAL(clone->mdim->position, schema->mdim->position);
  CU_ASSERT_EQUAL(clone->compression, schema->compression);
  CU_ASSERT_NOT_EQUAL(clone->xdim, schema->xdim);         /* deep clone */
  CU_ASSERT_NOT_EQUAL(clone->ydim, schema->ydim);         /* deep clone */
  CU_ASSERT_NOT_EQUAL(clone->zdim, schema->zdim);         /* deep clone */
  CU_ASSERT_NOT_EQUAL(clone->mdim, schema->mdim);         /* deep clone */
  CU_ASSERT_NOT_EQUAL(clone->dims, schema->dims);         /* deep clone */
  CU_ASSERT_NOT_EQUAL(clone->namehash, schema->namehash); /* deep clone */
  hash = schema->namehash;
  chash = clone->namehash;
  CU_ASSERT_EQUAL(chash->tablelength, hash->tablelength);
  CU_ASSERT_EQUAL(chash->entrycount, hash->entrycount);
  CU_ASSERT_EQUAL(chash->loadlimit, hash->loadlimit);
  CU_ASSERT_EQUAL(chash->primeindex, hash->primeindex);
  CU_ASSERT_EQUAL(chash->hashfn, hash->hashfn);
  CU_ASSERT_EQUAL(chash->eqfn, hash->eqfn);
  CU_ASSERT(chash->table != hash->table); /* deep clone */
  for (i = 0; i < schema->ndims; ++i)
  {
    PCDIMENSION *dim = schema->dims[i];
    PCDIMENSION *cdim = clone->dims[i];
    CU_ASSERT(dim != cdim); /* deep clone */
    CU_ASSERT_STRING_EQUAL(cdim->name, dim->name);
    CU_ASSERT_STRING_EQUAL(cdim->description, dim->description);
    CU_ASSERT_EQUAL(cdim->position, dim->position);
    CU_ASSERT_EQUAL(cdim->size, dim->size);
    CU_ASSERT_EQUAL(cdim->byteoffset, dim->byteoffset);
    CU_ASSERT_EQUAL(cdim->interpretation, dim->interpretation);
    CU_ASSERT_EQUAL(cdim->scale, dim->scale);
    CU_ASSERT_EQUAL(cdim->offset, dim->offset);
    CU_ASSERT_EQUAL(cdim->active, dim->active);
    /* hash table is correctly setup */
    CU_ASSERT_EQUAL(cdim, hashtable_search(clone->namehash, dim->name));
  }

  pc_schema_free(clone);
}

static void test_schema_clone_empty_description(void)
{
  PCSCHEMA *myschema, *clone;

  char *myxmlfile = "data/simple-schema-empty-description.xml";
  char *xmlstr = file_to_str(myxmlfile);

  myschema = pc_schema_from_xml(xmlstr);
  CU_ASSERT_PTR_NOT_NULL(myschema);

  clone = pc_schema_clone(myschema);
  CU_ASSERT_PTR_NOT_NULL(clone);
  CU_ASSERT_EQUAL(clone->ndims, myschema->ndims);
  CU_ASSERT_NOT_EQUAL(clone->dims[0]->name, myschema->dims[0]->name);
  CU_ASSERT_STRING_EQUAL(clone->dims[0]->name, myschema->dims[0]->name);
  CU_ASSERT_EQUAL(clone->dims[0]->description, myschema->dims[0]->description);
  pc_schema_free(myschema);
  pc_schema_free(clone);
  pcfree(xmlstr);
}

static void test_schema_clone_no_name(void)
{
  PCSCHEMA *myschema, *clone;

  /* See https://github.com/pgpointcloud/pointcloud/issues/66 */
  char *myxmlfile = "data/simple-schema-no-name.xml";
  char *xmlstr = file_to_str(myxmlfile);

  myschema = pc_schema_from_xml(xmlstr);
  CU_ASSERT_PTR_NOT_NULL(myschema);
  clone = pc_schema_clone(myschema);
  CU_ASSERT_PTR_NOT_NULL(clone);
  CU_ASSERT_EQUAL(clone->ndims, myschema->ndims);
  CU_ASSERT_PTR_NULL(clone->dims[0]->name);
  CU_ASSERT_PTR_NULL(clone->dims[0]->description);
  pc_schema_free(myschema);
  pc_schema_free(clone);
  pcfree(xmlstr);
}

static void test_schema_clone_empty_name(void)
{
  PCSCHEMA *myschema, *clone;

  char *myxmlfile = "data/simple-schema-empty-name.xml";
  char *xmlstr = file_to_str(myxmlfile);

  myschema = pc_schema_from_xml(xmlstr);
  CU_ASSERT_PTR_NOT_NULL(myschema);
  clone = pc_schema_clone(myschema);
  CU_ASSERT_PTR_NOT_NULL(clone);
  CU_ASSERT_EQUAL(clone->ndims, myschema->ndims);
  CU_ASSERT_PTR_NULL(clone->dims[0]->name);
  CU_ASSERT_PTR_NULL(clone->dims[0]->description);
  pc_schema_free(myschema);
  pc_schema_free(clone);
  pcfree(xmlstr);
}

static void test_schema_same_dimensions(void)
{
  PCSCHEMA *s1, *s2;
  PCDIMENSION *tmp;
  PCDIMENSION dim;

  char *xmlfile = "data/simple-schema.xml";
  char *xmlstr = file_to_str(xmlfile);

  s1 = pc_schema_from_xml(xmlstr);
  pcfree(xmlstr);

  s2 = pc_schema_clone(s1);

  // same schemas
  CU_ASSERT_EQUAL(pc_schema_same_dimensions(s1, s2), PC_TRUE);

  // different number of dimensions
  s2->ndims = 1;
  CU_ASSERT_EQUAL(pc_schema_same_dimensions(s1, s2), PC_FALSE);
  s2->ndims = s1->ndims;

  // different dimension positions
  tmp = s2->dims[0];
  s2->dims[0] = s2->dims[1];
  s2->dims[1] = tmp;
  CU_ASSERT_EQUAL(pc_schema_same_dimensions(s1, s2), PC_FALSE);
  s2->dims[1] = s2->dims[0];
  s2->dims[0] = tmp;

  // different dimension name
  tmp = s2->dims[0];
  dim.name = pcstrdup("foo");
  dim.interpretation = tmp->interpretation;
  s2->dims[0] = &dim;
  CU_ASSERT_EQUAL(pc_schema_same_dimensions(s1, s2), PC_FALSE);
  s2->dims[0] = tmp;
  pcfree(dim.name);

  // different interpretations
  tmp = s2->dims[0];
  dim.name = tmp->name;
  dim.interpretation = PC_FLOAT;
  s2->dims[0] = &dim;
  CU_ASSERT_EQUAL(pc_schema_same_dimensions(s1, s2), PC_FALSE);
  s2->dims[0] = tmp;

  pc_schema_free(s1);
  pc_schema_free(s2);
}

static void test_schema_same_interpretations(void)
{
  PCSCHEMA *s1, *s2;
  PCDIMENSION *tmp;
  PCDIMENSION dim;

  char *xmlfile = "data/simple-schema.xml";
  char *xmlstr = file_to_str(xmlfile);

  s1 = pc_schema_from_xml(xmlstr);
  pcfree(xmlstr);

  s2 = pc_schema_clone(s1);

  // same schemas
  CU_ASSERT_EQUAL(pc_schema_same_interpretations(s1, s2), PC_TRUE);

  // different srid
  s2->srid = 100;
  CU_ASSERT_EQUAL(pc_schema_same_interpretations(s1, s2), PC_FALSE);
  s2->srid = s1->srid;

  // different dimension positions
  tmp = s2->dims[0];
  s2->dims[0] = s2->dims[1];
  s2->dims[1] = tmp;
  CU_ASSERT_EQUAL(pc_schema_same_interpretations(s1, s2), PC_TRUE);
  s2->dims[1] = s2->dims[0];
  s2->dims[0] = tmp;

  // first dimension in s1 doesn't exist in s2, and first dimension
  // in s2 does not exist in s1
  tmp = s2->dims[0];
  dim.name = pcstrdup("foo");
  dim.interpretation = tmp->interpretation;
  dim.scale = tmp->scale;
  dim.offset = tmp->offset;
  s2->dims[0] = &dim;
  CU_ASSERT_EQUAL(pc_schema_same_interpretations(s1, s2), PC_TRUE);
  s2->dims[0] = tmp;
  pcfree(dim.name);

  // different interpretations for a dimension
  tmp = s2->dims[0];
  dim.name = tmp->name;
  dim.interpretation = PC_FLOAT;
  dim.scale = tmp->scale;
  dim.offset = tmp->offset;
  s2->dims[0] = &dim;
  CU_ASSERT_EQUAL(pc_schema_same_interpretations(s1, s2), PC_FALSE);
  s2->dims[0] = tmp;

  // different scales for a dimension
  tmp = s2->dims[0];
  dim.name = tmp->name;
  dim.interpretation = tmp->interpretation;
  dim.scale = 0.08;
  dim.offset = tmp->offset;
  s2->dims[0] = &dim;
  CU_ASSERT_EQUAL(pc_schema_same_interpretations(s1, s2), PC_FALSE);
  s2->dims[0] = tmp;

  // different offsets for a dimension
  tmp = s2->dims[0];
  dim.name = tmp->name;
  dim.interpretation = tmp->interpretation;
  dim.scale = tmp->scale;
  dim.offset = 80;
  s2->dims[0] = &dim;
  CU_ASSERT_EQUAL(pc_schema_same_interpretations(s1, s2), PC_FALSE);
  s2->dims[0] = tmp;

  pc_schema_free(s1);
  pc_schema_free(s2);
}

/* REGISTER ***********************************************************/

CU_TestInfo schema_tests[] = {
    PC_TEST(test_schema_from_xml),
    PC_TEST(test_schema_from_xml_with_empty_description),
    PC_TEST(test_schema_from_xml_with_empty_name),
    PC_TEST(test_schema_from_xml_with_no_name),
    PC_TEST(test_schema_size),
    PC_TEST(test_dimension_get),
    PC_TEST(test_dimension_byteoffsets),
    PC_TEST(test_schema_compression),
    PC_TEST(test_schema_invalid_xy),
    PC_TEST(test_schema_missing_dimension),
    PC_TEST(test_schema_empty),
    PC_TEST(test_schema_clone),
    PC_TEST(test_schema_clone_empty_description),
    PC_TEST(test_schema_clone_no_name),
    PC_TEST(test_schema_clone_empty_name),
    PC_TEST(test_schema_same_dimensions),
    PC_TEST(test_schema_same_interpretations),
    CU_TEST_INFO_NULL};

CU_SuiteInfo schema_suite = {.pName = "schema",
                             .pInitFunc = init_suite,
                             .pCleanupFunc = clean_suite,
                             .pTests = schema_tests};
