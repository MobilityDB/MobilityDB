/***********************************************************************
 * cu_pc_util.c
 *
 *        Testing for the util functions
 *
 * Portions Copyright (c) 2017, Oslandia
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
  if (!schema)
    return 1;

  return 0;
}

static int clean_suite(void)
{
  pc_schema_free(schema);
  return 0;
}

/* TESTS **************************************************************/

static void test_bounding_diagonal_wkb_from_bounds()
{
  PCBOUNDS bounds;
  size_t wkbsize;
  uint8_t *wkb;
  char *wkbhex;

  bounds.xmin = -10;
  bounds.xmax = 10;
  bounds.ymin = -10;
  bounds.ymax = 10;

  wkb = pc_bounding_diagonal_wkb_from_bounds(&bounds, schema, &wkbsize);
  CU_ASSERT(wkb != NULL);
  CU_ASSERT(wkbsize == 41);

  wkbhex = pc_hexbytes_from_bytes(wkb, wkbsize);
  CU_ASSERT(wkbhex != NULL);
  CU_ASSERT_STRING_EQUAL(wkbhex, "01020000000200000000000000000024C000000000000"
                                 "024C000000000000024400000000000002440");

  pcfree(wkb);
  pcfree(wkbhex);
}

static void test_bounding_diagonal_wkb_from_stats()
{
  PCSTATS *stats;
  size_t wkbsize;
  uint8_t *wkb;
  char *wkbhex;

  stats = pc_stats_new(schema);

  pc_point_set_x(&stats->min, -10);
  pc_point_set_x(&stats->max, 10);
  pc_point_set_y(&stats->min, -10);
  pc_point_set_y(&stats->max, 10);
  pc_point_set_z(&stats->min, -10);
  pc_point_set_z(&stats->max, 10);

  wkb = pc_bounding_diagonal_wkb_from_stats(stats, &wkbsize);
  CU_ASSERT(wkb != NULL);
  CU_ASSERT(wkbsize == 73);

  wkbhex = pc_hexbytes_from_bytes(wkb, wkbsize);
  CU_ASSERT(wkbhex != NULL);
  CU_ASSERT_STRING_EQUAL(wkbhex,
                         "01020000C00200000000000000000024C000000000000024C0000"
                         "00000000024C00000000000000000000000000000244000000000"
                         "0000244000000000000024400000000000000000");

  pc_stats_free(stats);
  pcfree(wkb);
  pcfree(wkbhex);
}

/* REGISTER ***********************************************************/

CU_TestInfo util_tests[] = {PC_TEST(test_bounding_diagonal_wkb_from_bounds),
                            PC_TEST(test_bounding_diagonal_wkb_from_stats),
                            CU_TEST_INFO_NULL};

CU_SuiteInfo util_suite = {.pName = "util",
                           .pInitFunc = init_suite,
                           .pCleanupFunc = clean_suite,
                           .pTests = util_tests};
