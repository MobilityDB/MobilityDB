/***********************************************************************
 * cu_pc_patch_lazperf.c
 *
 *				Testing for the LazPerf API functions
 *
 *	Copyright (c) 2016 Paul Blottiere, Oslandia
 *
 ***********************************************************************/

#include "CUnit/Basic.h"
#include "cu_tester.h"

/* GLOBALS ************************************************************/

static PCSCHEMA *simpleschema = NULL;
static PCSCHEMA *multipledimschema = NULL;
static const char *simplexmlfile = "data/simple-schema.xml";
static const char *multipledimxmlfile =
    "data/simple-schema-laz-multiple-dim.xml";

/* Setup/teardown for this suite */
static int init_suite(void)
{
  char *xmlstr = file_to_str(simplexmlfile);
  simpleschema = pc_schema_from_xml(xmlstr);
  pcfree(xmlstr);
  if (!simpleschema)
    return 1;

  xmlstr = file_to_str(multipledimxmlfile);
  multipledimschema = pc_schema_from_xml(xmlstr);
  pcfree(xmlstr);
  if (!multipledimschema)
    return 1;

  return 0;
}

static int clean_suite(void)
{
  pc_schema_free(simpleschema);
  pc_schema_free(multipledimschema);
  return 0;
}

#ifdef HAVE_LAZPERF
static void test_schema_compression_lazperf(void)
{
  PCSCHEMA *myschema = NULL;
  char *myxmlfile = "data/simple-schema-laz.xml";
  char *xmlstr = file_to_str(myxmlfile);
  myschema = pc_schema_from_xml(xmlstr);

  CU_ASSERT_PTR_NOT_NULL(myschema);
  int compression = myschema->compression;
  CU_ASSERT_EQUAL(compression, PC_LAZPERF);

  pc_schema_free(myschema);
  pcfree(xmlstr);
}

static void test_patch_lazperf()
{
  PCPOINT *pt;
  int i;
  int npts = 400;
  PCPOINTLIST *pl;
  PCPATCH_LAZPERF *pal;
  PCPATCH_UNCOMPRESSED *paul, *pauref;

  // build a list of points
  pl = pc_pointlist_make(npts);

  for (i = 0; i < npts; i++)
  {
    pt = pc_point_make(simpleschema);
    pc_point_set_double_by_name(pt, "x", i * 2.0);
    pc_point_set_double_by_name(pt, "y", i * 1.9);
    pc_point_set_double_by_name(pt, "Z", i * 0.34);
    pc_point_set_double_by_name(pt, "intensity", 10);
    pc_pointlist_add_point(pl, pt);
  }

  // compress the list in a lazperf patch
  pal = pc_patch_lazperf_from_pointlist(pl);

  // get an uncompressed patch from the lazperf patch
  paul = pc_patch_uncompressed_from_lazperf(pal);

  // get an uncompressed patch directly from the pointlist
  pauref = pc_patch_uncompressed_from_pointlist(pl);

  // test the number of points
  CU_ASSERT_EQUAL(pal->npoints, pauref->npoints);
  CU_ASSERT_EQUAL(paul->npoints, pauref->npoints);

  // test bounds
  CU_ASSERT_DOUBLE_EQUAL(pal->bounds.xmax, pauref->bounds.xmax, 0.0001);
  CU_ASSERT_DOUBLE_EQUAL(paul->bounds.ymax, pauref->bounds.ymax, 0.000001);

  // test type
  CU_ASSERT_EQUAL(pal->type, PC_LAZPERF);
  CU_ASSERT_EQUAL(paul->type, pauref->type);

  // test readonly
  CU_ASSERT_EQUAL(pauref->readonly, paul->readonly);
  CU_ASSERT_EQUAL(pauref->readonly, pal->readonly);

  // test datasize
  CU_ASSERT_EQUAL(paul->datasize, pauref->datasize);

  // free
  pc_pointlist_free(pl);
  pc_patch_free((PCPATCH *)pal);
  pc_patch_free((PCPATCH *)paul);
  pc_patch_free((PCPATCH *)pauref);
}

static void test_pointlist_lazperf()
{
  PCPOINT *pt;
  int i;
  int npts = 400;
  PCPOINTLIST *pl1, *pl2;
  PCPATCH_LAZPERF *pch1;
  PCPATCH_UNCOMPRESSED *pa1, *pa2;
  char *wkt1, *wkt2;

  // build a list of points
  pl1 = pc_pointlist_make(npts);

  for (i = 0; i < npts; i++)
  {
    pt = pc_point_make(simpleschema);
    pc_point_set_double_by_name(pt, "x", i * 2.0);
    pc_point_set_double_by_name(pt, "y", i * 1.9);
    pc_point_set_double_by_name(pt, "Z", i * 0.34);
    pc_point_set_double_by_name(pt, "intensity", 10);
    pc_pointlist_add_point(pl1, pt);
  }

  // compress the list in a lazperf patch
  pch1 = pc_patch_lazperf_from_pointlist(pl1);

  // decompress the lazperf patch in a pointlist
  pl2 = pc_pointlist_from_lazperf(pch1);

  // test that the string representation of pointlist is equal
  pa1 = pc_patch_uncompressed_from_pointlist(pl1);
  pa2 = pc_patch_uncompressed_from_lazperf(pch1);

  wkt1 = pc_patch_uncompressed_to_string(pa1);
  wkt2 = pc_patch_uncompressed_to_string(pa2);

  CU_ASSERT_STRING_EQUAL(wkt1, wkt2);

  pc_patch_free((PCPATCH *)pch1);
  pc_patch_free((PCPATCH *)pa1);
  pc_patch_free((PCPATCH *)pa2);
  pc_pointlist_free(pl1);
  pc_pointlist_free(pl2);
  pcfree(wkt1);
  pcfree(wkt2);
}

static void test_to_string_lazperf()
{
  PCPOINT *pt;
  int i;
  int npts = 400;
  PCPOINTLIST *pl;
  PCPATCH_LAZPERF *pal;
  PCPATCH_UNCOMPRESSED *pau;
  char *str1, *str2;

  // build a list of points
  pl = pc_pointlist_make(npts);

  for (i = 0; i < npts; i++)
  {
    pt = pc_point_make(simpleschema);
    pc_point_set_double_by_name(pt, "x", i * 2.0);
    pc_point_set_double_by_name(pt, "y", i * 1.9);
    pc_point_set_double_by_name(pt, "Z", i * 0.34);
    pc_point_set_double_by_name(pt, "intensity", 10);
    pc_pointlist_add_point(pl, pt);
  }

  // build patch
  pau = pc_patch_uncompressed_from_pointlist(pl);
  pal = pc_patch_lazperf_from_pointlist(pl);

  // get string
  str1 = pc_patch_uncompressed_to_string(pau);
  str2 = pc_patch_lazperf_to_string(pal);

  // compare
  CU_ASSERT_STRING_EQUAL(str1, str2);

  // free
  pc_patch_free((PCPATCH *)pal);
  pc_patch_free((PCPATCH *)pau);
  pc_pointlist_free(pl);
  pcfree(str1);
  pcfree(str2);
}

static void test_wkb_lazperf()
{
  PCPOINT *pt;
  int i;
  int npts = 400;
  PCPOINTLIST *pl;
  PCPATCH_LAZPERF *pal1, *pal2;
  PCPATCH_UNCOMPRESSED *pau;
  uint8_t *wkb1, *wkb2;
  size_t wkbsize;

  // build a list of points
  pl = pc_pointlist_make(npts);

  for (i = 0; i < npts; i++)
  {
    pt = pc_point_make(simpleschema);
    pc_point_set_double_by_name(pt, "x", i * 2.0);
    pc_point_set_double_by_name(pt, "y", i * 1.9);
    pc_point_set_double_by_name(pt, "Z", i * 0.34);
    pc_point_set_double_by_name(pt, "intensity", 10);
    pc_pointlist_add_point(pl, pt);
  }

  // build patch lazperf
  pal1 = pc_patch_lazperf_from_pointlist(pl);

  // get the corresponding wkb
  wkb1 = pc_patch_lazperf_to_wkb(pal1, &wkbsize);

  // rebuild a lazperf patch thanks to the wkb
  pal2 =
      (PCPATCH_LAZPERF *)pc_patch_lazperf_from_wkb(pal1->schema, wkb1, wkbsize);

  // get the wkb reference
  pau = pc_patch_uncompressed_from_pointlist(pl);
  wkb2 = pc_patch_uncompressed_to_wkb(pau, &wkbsize);

  // compare wkb
  CU_ASSERT_STRING_EQUAL(wkb1, wkb2);

  // free
  pc_patch_free((PCPATCH *)pal1);
  pc_patch_free((PCPATCH *)pal2);
  pc_patch_free((PCPATCH *)pau);
  pc_pointlist_free(pl);
  pcfree(wkb1);
  pcfree(wkb2);
}

static void test_patch_filter_lazperf_zero_point()
{
  PCPOINT *pt;
  int i;
  int npts = 5;
  PCPOINTLIST *pl;
  PCPATCH_LAZPERF *pal;
  PCPATCH *pa;

  // build a list of points
  pl = pc_pointlist_make(npts);

  for (i = 0; i < npts; i++)
  {
    pt = pc_point_make(simpleschema);
    pc_point_set_double_by_name(pt, "x", i * 2.0);
    pc_point_set_double_by_name(pt, "y", i * 1.9);
    pc_point_set_double_by_name(pt, "Z", i * 0.34);
    pc_point_set_double_by_name(pt, "intensity", 10);
    pc_pointlist_add_point(pl, pt);
  }

  // build patch lazperf
  pal = pc_patch_lazperf_from_pointlist(pl);

  // filter with a resulting patch of 0 point(s)
  pa = pc_patch_filter((PCPATCH *)pal, 0, PC_BETWEEN, 0.0, 0.0);
  CU_ASSERT_EQUAL(pa->npoints, 0);

  pc_patch_free((PCPATCH *)pal);
  pc_patch_free((PCPATCH *)pa);
  pc_pointlist_free(pl);
}

static void test_patch_compression_with_multiple_dimension()
{
  PCPOINT *pt;
  int i;
  int npts = 5;
  PCPOINTLIST *pl;
  PCPATCH_LAZPERF *pal;
  PCPATCH_UNCOMPRESSED *pau;
  char *str1, *str2;

  // build a list of points
  pl = pc_pointlist_make(npts);

  for (i = 0; i < npts; i++)
  {
    pt = pc_point_make(multipledimschema);
    pc_point_set_double_by_name(pt, "x", i * 2);
    pc_point_set_double_by_name(pt, "y", i * 1.9);
    pc_point_set_double_by_name(pt, "z", i * 0.34);
    pc_point_set_double_by_name(pt, "intensity", 10);
    pc_pointlist_add_point(pl, pt);
  }

  // build patchs
  pal = pc_patch_lazperf_from_pointlist(pl);
  pau = pc_patch_uncompressed_from_pointlist(pl);

  // compare str result
  str1 = pc_patch_lazperf_to_string(pal);
  str2 = pc_patch_uncompressed_to_string(pau);

  CU_ASSERT_STRING_EQUAL(str1, str2);

  pc_patch_free((PCPATCH *)pal);
  pc_patch_free((PCPATCH *)pau);
  pc_pointlist_free(pl);
  pcfree(str1);
  pcfree(str2);
}
#endif

/* REGISTER ***********************************************************/

CU_TestInfo lazperf_tests[] = {
#ifdef HAVE_LAZPERF
    PC_TEST(test_schema_compression_lazperf),
    PC_TEST(test_patch_lazperf),
    PC_TEST(test_pointlist_lazperf),
    PC_TEST(test_to_string_lazperf),
    PC_TEST(test_wkb_lazperf),
    PC_TEST(test_patch_filter_lazperf_zero_point),
    PC_TEST(test_patch_compression_with_multiple_dimension),
#endif
    CU_TEST_INFO_NULL};

CU_SuiteInfo lazperf_suite = {.pName = "lazperf",
                              .pInitFunc = init_suite,
                              .pCleanupFunc = clean_suite,
                              .pTests = lazperf_tests};
