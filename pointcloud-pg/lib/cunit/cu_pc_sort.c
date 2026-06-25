/***********************************************************************
 * cu_pc_sort.c
 *
 *        Testing for the schema API functions
 *
 ***********************************************************************/

#include "CUnit/Basic.h"
#include "cu_tester.h"

/* GLOBALS ************************************************************/

static PCSCHEMA *schema = NULL;
static const char *xmlfile = "data/simple-schema.xml";
static const double precision = 0.000001;

// SIMPLE SCHEMA
// int32_t x
// int32_t y
// int32_t z
// int16_t intensity

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

static void test_sort_simple()
{
  // 00 endian (big)
  // 00000000 pcid
  // 00000000 compression
  // 00000002 npoints
  // 0000000800000003000000050006 pt1 (XYZi)
  // 0000000200000001000000040008 pt2 (XYZi)

  // init data
  PCPOINTLIST *lisort;
  PCPATCH *pasort;
  double d1;
  double d2;
  char *hexbuf = "0000000000000000000000000200000008000000030000000500060000000"
                 "200000001000000040008";
  size_t hexsize = strlen(hexbuf);

  uint8_t *wkb = pc_bytes_from_hexbytes(hexbuf, hexsize);
  PCPATCH *pa = pc_patch_from_wkb(schema, wkb, hexsize / 2);
  PCPOINTLIST *li = pc_pointlist_from_patch(pa);
  const char *X[] = {"X"};

  // check that initial data are not sorted
  pc_point_get_double_by_name(pc_pointlist_get_point(li, 0), "X", &d1);
  pc_point_get_double_by_name(pc_pointlist_get_point(li, 1), "X", &d2);

  CU_ASSERT_DOUBLE_EQUAL(d1, 0.08, precision);
  CU_ASSERT_DOUBLE_EQUAL(d2, 0.02, precision);

  // sort on X attribute and check if data are well sorted
  pasort = pc_patch_sort(pa, X, 1);
  lisort = pc_pointlist_from_patch(pasort);

  pc_point_get_double_by_name(pc_pointlist_get_point(lisort, 0), "X", &d1);
  pc_point_get_double_by_name(pc_pointlist_get_point(lisort, 1), "X", &d2);

  CU_ASSERT_DOUBLE_EQUAL(d1, 0.02, precision);
  CU_ASSERT_DOUBLE_EQUAL(d2, 0.08, precision);

  // free
  pc_pointlist_free(li);
  pc_pointlist_free(lisort);
  pc_patch_free(pa);
  pc_patch_free(pasort);
  pcfree(wkb);
}

static void test_sort_consistency()
{
  // 00 endian (big)
  // 00000000 pcid
  // 00000000 compression
  // 00000002 npoints
  // 0000000800000003000000050006 pt1 (XYZi)
  // 0000000200000001000000040008 pt2 (XYZi)

  // init data
  PCPATCH *pasort;
  char *pastr, *pasortstr;
  uint8_t *wkbsort;
  char *hexbuf = "0000000000000000000000000200000008000000030000000500060000000"
                 "200000001000000040008";
  size_t hexsize = strlen(hexbuf);
  uint8_t *wkb = pc_bytes_from_hexbytes(hexbuf, hexsize);
  PCPATCH *pa = pc_patch_from_wkb(schema, wkb, hexsize / 2);
  PCPOINTLIST *li = pc_pointlist_from_patch(pa);
  const char *X[] = {"X"};

  // sort on X attribute
  pasort = pc_patch_sort(pa, X, 1);

  // chek consistency
  wkbsort = pc_patch_to_wkb(pasort, &hexsize);
  CU_ASSERT_EQUAL(pc_wkb_get_pcid(wkb), pc_wkb_get_pcid(wkbsort));
  CU_ASSERT_EQUAL(wkb_get_npoints(wkb), wkb_get_npoints(wkbsort));
  CU_ASSERT_EQUAL(wkb_get_compression(wkb), wkb_get_compression(wkbsort));

  pastr = pc_patch_to_string(pa);
  CU_ASSERT_STRING_EQUAL(
      pastr, "{\"pcid\":0,\"pts\":[[0.08,0.03,0.05,6],[0.02,0.01,0.04,8]]}");

  pasortstr = pc_patch_to_string(pasort);
  CU_ASSERT_STRING_EQUAL(
      pasortstr,
      "{\"pcid\":0,\"pts\":[[0.02,0.01,0.04,8],[0.08,0.03,0.05,6]]}");

  // free
  pcfree(wkb);
  pcfree(wkbsort);
  pcfree(pastr);
  pcfree(pasortstr);
  pc_patch_free(pasort);
  pc_patch_free(pa);
  pc_pointlist_free(li);
}

static void test_sort_one_point()
{
  // 00 endian (big)
  // 00000000 pcid
  // 00000000 compression
  // 00000001 npoints
  // 0000000200000003000000050006 pt1 (XYZi)

  // init data
  PCPATCH *pasort;
  char *pastr, *pasortstr;
  uint8_t *wkbsort;
  char *hexbuf = "000000000000000000000000010000000200000003000000050006";
  size_t hexsize = strlen(hexbuf);
  uint8_t *wkb = pc_bytes_from_hexbytes(hexbuf, hexsize);
  PCPATCH *pa = pc_patch_from_wkb(schema, wkb, hexsize / 2);
  PCPOINTLIST *li = pc_pointlist_from_patch(pa);
  const char *X[] = {"X"};

  // sort on X attribute
  pasort = pc_patch_sort(pa, X, 1);

  // check consistency
  wkbsort = pc_patch_to_wkb(pasort, &hexsize);
  CU_ASSERT_EQUAL(pc_wkb_get_pcid(wkb), pc_wkb_get_pcid(wkbsort));
  CU_ASSERT_EQUAL(wkb_get_npoints(wkb), wkb_get_npoints(wkbsort));
  CU_ASSERT_EQUAL(wkb_get_compression(wkb), wkb_get_compression(wkbsort));

  pastr = pc_patch_to_string(pa);
  pasortstr = pc_patch_to_string(pasort);
  CU_ASSERT_STRING_EQUAL(pastr, pasortstr);

  // free
  pcfree(wkb);
  pcfree(wkbsort);
  pcfree(pastr);
  pcfree(pasortstr);
  pc_patch_free(pa);
  pc_patch_free(pasort);
  pc_pointlist_free(li);
}

static void test_sort_stable()
{
  // 00 endian (big)
  // 00000000 pcid
  // 00000000 compression
  // 00000002 npoints
  // 0000000800000003000000050006 pt1 (XYZi)
  // 0000000200000003000000040008 pt2 (XYZi)
  // 0000000200000003000000040009 pt3 (XYZi)

  // init data
  PCPATCH *pasort;
  char *hexbuf = "0000000000000000000000000300000008000000030000000500060000000"
                 "2000000030000000400080000000200000003000000040009";
  size_t hexsize = strlen(hexbuf);
  uint8_t *wkb = pc_bytes_from_hexbytes(hexbuf, hexsize);
  PCPATCH *pa = pc_patch_from_wkb(schema, wkb, hexsize / 2);
  PCPOINTLIST *li = pc_pointlist_from_patch(pa);
  const char *dims[] = {"Y"};

  // sort on Y attribute
  pasort = pc_patch_sort(pa, dims, 1);

  // check that sort is stable
  char *pastr = pc_patch_to_string(pa);
  char *pasortstr = pc_patch_to_string(pasort);
  CU_ASSERT_STRING_EQUAL(pastr, pasortstr);

  // free
  free(pastr);
  free(pasortstr);
  pcfree(wkb);
  pc_patch_free(pa);
  pc_patch_free(pasort);
  pc_pointlist_free(li);
}

static void test_sort_patch_is_sorted_no_compression()
{
  // 00 endian (big)
  // 00000000 pcid
  // 00000000 compression
  // 00000002 npoints
  // 0000000800000003000000050006 pt1 (XYZi)
  // 0000000200000003000000040008 pt2 (XYZi)
  // 0000000200000003000000040009 pt3 (XYZi)

  // init data
  PCPATCH *pasort;
  char *hexbuf = "0000000000000000000000000300000008000000030000000500060000000"
                 "2000000030000000400080000000200000003000000040009";
  size_t hexsize = strlen(hexbuf);
  uint8_t *wkb = pc_bytes_from_hexbytes(hexbuf, hexsize);
  PCPATCH *pa = pc_patch_from_wkb(schema, wkb, hexsize / 2);
  PCPOINTLIST *li = pc_pointlist_from_patch(pa);
  const char *X[] = {"X"};

  CU_ASSERT_EQUAL(pc_patch_is_sorted(pa, X, 1, PC_FALSE), PC_FALSE);
  CU_ASSERT_EQUAL(pc_patch_is_sorted(pa, X, 1, PC_TRUE), PC_FALSE);

  pasort = pc_patch_sort(pa, X, 1);
  CU_ASSERT_EQUAL(pc_patch_is_sorted(pasort, X, 1, PC_FALSE), PC_FALSE);
  CU_ASSERT_EQUAL(pc_patch_is_sorted(pasort, X, 1, PC_TRUE), PC_TRUE);

  // free
  pcfree(wkb);
  pc_patch_free(pa);
  pc_patch_free(pasort);
  pc_pointlist_free(li);
}

static void
test_sort_patch_is_sorted_compression_dimensional(enum DIMCOMPRESSIONS dimcomp)
{
  // init data
  PCPATCH_DIMENSIONAL *padim1, *padim2, *padimsort;
  PCPOINT *pt;
  PCPOINTLIST *pl;
  int i;
  int ndims = 1;
  int npts = PCDIMSTATS_MIN_SAMPLE + 1; // force to keep custom compression
  const char *X[] = {"X"};

  // build a dimensional patch
  pl = pc_pointlist_make(npts);

  for (i = npts; i >= 0; i--)
  {
    pt = pc_point_make(schema);
    pc_point_set_double_by_name(pt, "x", i);
    pc_point_set_double_by_name(pt, "y", i);
    pc_point_set_double_by_name(pt, "Z", i);
    pc_point_set_double_by_name(pt, "intensity", 10);
    pc_pointlist_add_point(pl, pt);
  }

  padim1 = pc_patch_dimensional_from_pointlist(pl);

  // set dimensional compression for each dimension
  PCDIMSTATS *stats = pc_dimstats_make(schema);
  pc_dimstats_update(stats, padim1);
  for (i = 0; i < padim1->schema->ndims; i++)
    stats->stats[i].recommended_compression = dimcomp;

  // compress patch
  padim2 = pc_patch_dimensional_compress(padim1, stats);

  // test that patch is not sorted
  CU_ASSERT_EQUAL(pc_patch_is_sorted((PCPATCH *)padim2, X, ndims, PC_FALSE),
                  PC_FALSE);
  CU_ASSERT_EQUAL(pc_patch_is_sorted((PCPATCH *)padim2, X, ndims, PC_TRUE),
                  PC_FALSE);

  // sort
  padimsort = (PCPATCH_DIMENSIONAL *)pc_patch_sort((PCPATCH *)padim2, X, 1);

  // test that resulting data is sorted
  CU_ASSERT_EQUAL(pc_patch_is_sorted((PCPATCH *)padimsort, X, ndims, PC_TRUE),
                  PC_TRUE);

  // free
  pc_dimstats_free(stats);
  pc_patch_free((PCPATCH *)padim1);
  pc_patch_free((PCPATCH *)padim2);
  pc_patch_free((PCPATCH *)padimsort);
  pc_pointlist_free(pl);
}

static void test_sort_patch_is_sorted_compression_dimensional_none()
{
  test_sort_patch_is_sorted_compression_dimensional(PC_DIM_NONE);
}

static void test_sort_patch_is_sorted_compression_dimensional_zlib()
{
  test_sort_patch_is_sorted_compression_dimensional(PC_DIM_ZLIB);
}

static void test_sort_patch_is_sorted_compression_dimensional_rle()
{
  test_sort_patch_is_sorted_compression_dimensional(PC_DIM_RLE);
}

static void test_sort_patch_is_sorted_compression_dimensional_sigbits()
{
  test_sort_patch_is_sorted_compression_dimensional(PC_DIM_SIGBITS);
}

static void test_sort_patch_ndims()
{
  // 00 endian (big)
  // 00000000 pcid
  // 00000000 compression
  // 00000002 npoints
  // 0000000800000001000000050006 pt1 (XYZi)
  // 0000000200000003000000040008 pt2 (XYZi)
  // 0000000200000002000000040008 pt2 (XYZi)

  // init data
  PCPATCH *pasort1, *pasort2;
  char *hexbuf = "0000000000000000000000000300000008000000040000000500060000000"
                 "2000000030000000400080000000200000002000000040009";
  size_t hexsize = strlen(hexbuf);
  uint8_t *wkb = pc_bytes_from_hexbytes(hexbuf, hexsize);
  PCPATCH *pa = pc_patch_from_wkb(schema, wkb, hexsize / 2);
  const char *X[] = {"X"};
  const char *Y[] = {"Y"};
  const char *X_Y[] = {"X", "Y"};

  // test that initial data is not sorted
  CU_ASSERT_EQUAL(pc_patch_is_sorted(pa, X, 1, PC_FALSE), PC_FALSE);
  CU_ASSERT_EQUAL(pc_patch_is_sorted(pa, Y, 1, PC_FALSE), PC_FALSE);
  CU_ASSERT_EQUAL(pc_patch_is_sorted(pa, X_Y, 2, PC_FALSE), PC_FALSE);

  // sort on X attribute and test
  pasort1 = pc_patch_sort(pa, X, 1);
  CU_ASSERT_EQUAL(pc_patch_is_sorted(pasort1, X, 1, PC_TRUE), PC_TRUE);
  CU_ASSERT_EQUAL(pc_patch_is_sorted(pasort1, Y, 1, PC_TRUE), PC_FALSE);
  CU_ASSERT_EQUAL(pc_patch_is_sorted(pasort1, X_Y, 2, PC_TRUE), PC_FALSE);

  // sort on X and Y and tst
  pasort2 = pc_patch_sort(pa, X_Y, 2);
  CU_ASSERT_EQUAL(pc_patch_is_sorted(pasort2, X, 1, PC_TRUE), PC_TRUE);
  CU_ASSERT_EQUAL(pc_patch_is_sorted(pasort2, Y, 1, PC_TRUE), PC_TRUE);
  CU_ASSERT_EQUAL(pc_patch_is_sorted(pasort2, X_Y, 2, PC_TRUE), PC_TRUE);

  // free
  pcfree(wkb);
  pc_patch_free(pasort1);
  pc_patch_free(pasort2);
  pc_patch_free(pa);
}

/* REGISTER ***********************************************************/

CU_TestInfo sort_tests[] = {
    PC_TEST(test_sort_simple),
    PC_TEST(test_sort_consistency),
    PC_TEST(test_sort_one_point),
    PC_TEST(test_sort_stable),
    PC_TEST(test_sort_patch_is_sorted_no_compression),
    PC_TEST(test_sort_patch_is_sorted_compression_dimensional_none),
    PC_TEST(test_sort_patch_is_sorted_compression_dimensional_zlib),
    PC_TEST(test_sort_patch_is_sorted_compression_dimensional_sigbits),
    PC_TEST(test_sort_patch_is_sorted_compression_dimensional_rle),
    PC_TEST(test_sort_patch_ndims),
    CU_TEST_INFO_NULL};

CU_SuiteInfo sort_suite = {.pName = "sort",
                           .pInitFunc = init_suite,
                           .pCleanupFunc = clean_suite,
                           .pTests = sort_tests};
