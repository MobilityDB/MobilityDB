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

static PCBYTES initbytes(uint8_t *bytes, size_t size, uint32_t interp)
{
  PCBYTES pcb;
  pcb.bytes = bytes;
  pcb.size = size;
  pcb.interpretation = interp;
  pcb.npoints = pcb.size / pc_interpretation_size(pcb.interpretation);
  pcb.compression = PC_DIM_NONE;
  pcb.readonly = PC_TRUE;
  return pcb;
}

/*
 * Run-length encode a byte stream by word.
 * Lots of identical words means great
 * compression ratios.
 */
static void test_run_length_encoding()
{
  char *bytes;
  int nr;
  PCBYTES pcb, epcb, pcb2;

  /*
  typedef struct
  {
          size_t size;
          uint32_t npoints;
          uint32_t interpretation;
          uint32_t compression;
          uint8_t *bytes;
  } PCBYTES;
  */

  bytes = "aaaabbbbccdde";
  pcb = initbytes((uint8_t *)bytes, strlen(bytes), PC_UINT8);
  nr = pc_bytes_run_count(&pcb);
  CU_ASSERT_EQUAL(nr, 5);

  bytes = "a";
  pcb = initbytes((uint8_t *)bytes, strlen(bytes), PC_UINT8);
  nr = pc_bytes_run_count(&pcb);
  CU_ASSERT_EQUAL(nr, 1);

  bytes = "aa";
  pcb = initbytes((uint8_t *)bytes, strlen(bytes), PC_UINT8);
  nr = pc_bytes_run_count(&pcb);
  CU_ASSERT_EQUAL(nr, 1);

  bytes = "ab";
  pcb = initbytes((uint8_t *)bytes, strlen(bytes), PC_UINT8);
  nr = pc_bytes_run_count(&pcb);
  CU_ASSERT_EQUAL(nr, 2);

  bytes = "abcdefg";
  pcb = initbytes((uint8_t *)bytes, strlen(bytes), PC_UINT8);
  nr = pc_bytes_run_count(&pcb);
  CU_ASSERT_EQUAL(nr, 7);

  bytes = "aabcdefg";
  pcb = initbytes((uint8_t *)bytes, strlen(bytes), PC_UINT8);
  nr = pc_bytes_run_count(&pcb);
  CU_ASSERT_EQUAL(nr, 7);

  bytes = "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
          "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
          "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
          "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
          "cccccccccccccccccccccccccccc";
  pcb = initbytes((uint8_t *)bytes, strlen(bytes), PC_UINT8);
  nr = pc_bytes_run_count(&pcb);
  CU_ASSERT_EQUAL(nr, 1);

  epcb = pc_bytes_run_length_encode(pcb);
  pcb2 = pc_bytes_run_length_decode(epcb);

  CU_ASSERT_EQUAL(pcb.compression, PC_DIM_NONE);
  CU_ASSERT_EQUAL(epcb.compression, PC_DIM_RLE);
  CU_ASSERT_EQUAL(pcb2.compression, PC_DIM_NONE);

  CU_ASSERT_EQUAL(memcmp(pcb.bytes, pcb2.bytes, pcb.size), 0);
  CU_ASSERT_EQUAL(pcb.size, pcb2.size);
  CU_ASSERT_EQUAL(pcb.npoints, pcb2.npoints);
  pc_bytes_free(epcb);
  pc_bytes_free(pcb2);

  bytes = "aabcdefg";
  pcb = initbytes((uint8_t *)bytes, strlen(bytes), PC_UINT8);
  epcb = pc_bytes_run_length_encode(pcb);
  pcb2 = pc_bytes_run_length_decode(epcb);
  CU_ASSERT_EQUAL(memcmp(pcb.bytes, pcb2.bytes, pcb.size), 0);
  CU_ASSERT_EQUAL(pcb.size, pcb2.size);
  CU_ASSERT_EQUAL(pcb.npoints, pcb2.npoints);
  pc_bytes_free(epcb);
  pc_bytes_free(pcb2);

  bytes = (char *)((uint32_t[]){10, 10, 10, 20, 20, 30, 20, 20});
  pcb = initbytes((uint8_t *)bytes, 8, PC_UINT32);
  epcb = pc_bytes_run_length_encode(pcb);
  pcb2 = pc_bytes_run_length_decode(epcb);
  CU_ASSERT_EQUAL(memcmp(pcb.bytes, pcb2.bytes, pcb.size), 0);
  CU_ASSERT_EQUAL(pcb.size, pcb2.size);
  CU_ASSERT_EQUAL(pcb.npoints, pcb2.npoints);
  pc_bytes_free(epcb);
  pc_bytes_free(pcb2);

  bytes = (char *)((uint16_t[]){10, 10, 10, 20, 20, 30, 20, 20});
  pcb = initbytes((uint8_t *)bytes, 8, PC_UINT16);
  epcb = pc_bytes_run_length_encode(pcb);
  pcb2 = pc_bytes_run_length_decode(epcb);
  CU_ASSERT_EQUAL(memcmp(pcb.bytes, pcb2.bytes, pcb.size), 0);
  CU_ASSERT_EQUAL(pcb.size, pcb2.size);
  CU_ASSERT_EQUAL(pcb.npoints, pcb2.npoints);
  pc_bytes_free(epcb);
  pc_bytes_free(pcb2);
}

/*
 * Strip the common bits off a stream and pack the
 * remaining bits in behind. Test bit counting and
 * round-trip encode/decode paths.
 */
static void test_sigbits_encoding()
{
  uint8_t *bytes;
  uint16_t *bytes16, *ebytes16;
  uint32_t *bytes32, *ebytes32;
  uint64_t *bytes64, *ebytes64;

  uint32_t count, nelems;
  uint8_t common8;
  uint16_t common16;
  uint32_t common32;
  uint64_t common64;
  PCBYTES pcb, epcb, pcb2;

  /*
  01100001 a
  01100010 b
  01100011 c
  01100000 `
  */
  bytes = (uint8_t *)"abc";
  pcb = initbytes(bytes, strlen((char *)bytes), PC_UINT8);
  common8 = pc_bytes_sigbits_count_8(&pcb, &count);
  CU_ASSERT_EQUAL(count, 6);
  CU_ASSERT_EQUAL(common8, '`');

  bytes = (uint8_t *)"abcdef";
  pcb = initbytes(bytes, strlen((char *)bytes), PC_UINT8);
  common8 = pc_bytes_sigbits_count_8(&pcb, &count);
  CU_ASSERT_EQUAL(count, 5);
  CU_ASSERT_EQUAL(common8, '`');

  /*
  0110000101100001 aa
  0110001001100010 bb
  0110001101100011 cc
  0110000000000000 24576
  */
  bytes = (uint8_t *)"aabbcc";
  pcb = initbytes(bytes, strlen((char *)bytes), PC_UINT16);
  count = pc_bytes_sigbits_count(&pcb);
  CU_ASSERT_EQUAL(count, 6);

  /*
  "abca" encoded:
  base      a  b  c  a
  01100000 01 10 11 01
  */
  bytes = (uint8_t *)"abcaabcaabcbabcc";
  pcb = initbytes((uint8_t *)bytes, strlen((char *)bytes), PC_INT8);
  epcb = pc_bytes_sigbits_encode(pcb);
  CU_ASSERT_EQUAL(epcb.bytes[0], 2);   /* unique bit count */
  CU_ASSERT_EQUAL(epcb.bytes[1], 96);  /* common bits */
  CU_ASSERT_EQUAL(epcb.bytes[2], 109); /* packed byte */
  CU_ASSERT_EQUAL(epcb.bytes[3], 109); /* packed byte */
  CU_ASSERT_EQUAL(epcb.bytes[4], 110); /* packed byte */
  CU_ASSERT_EQUAL(epcb.bytes[5], 111); /* packed byte */
  pc_bytes_free(epcb);

  /*
  "abca" encoded:
  base       a   b   c   d   a   b
  01100000 001 010 011 100 001 010
  */
  bytes = (uint8_t *)"abcdab";
  pcb = initbytes(bytes, strlen((char *)bytes), PC_INT8);
  epcb = pc_bytes_sigbits_encode(pcb);
  CU_ASSERT_EQUAL(epcb.bytes[0], 3);   /* unique bit count */
  CU_ASSERT_EQUAL(epcb.bytes[1], 96);  /* common bits */
  CU_ASSERT_EQUAL(epcb.bytes[2], 41);  /* packed byte */
  CU_ASSERT_EQUAL(epcb.bytes[3], 194); /* packed byte */

  pcb2 = pc_bytes_sigbits_decode(epcb);
  CU_ASSERT_EQUAL(pcb2.bytes[0], 'a');
  CU_ASSERT_EQUAL(pcb2.bytes[1], 'b');
  CU_ASSERT_EQUAL(pcb2.bytes[2], 'c');
  CU_ASSERT_EQUAL(pcb2.bytes[3], 'd');
  CU_ASSERT_EQUAL(pcb2.bytes[4], 'a');
  CU_ASSERT_EQUAL(pcb2.bytes[5], 'b');

  CU_ASSERT_EQUAL(pcb.compression, PC_DIM_NONE);
  CU_ASSERT_EQUAL(epcb.compression, PC_DIM_SIGBITS);
  CU_ASSERT_EQUAL(pcb2.compression, PC_DIM_NONE);

  pc_bytes_free(pcb2);
  pc_bytes_free(epcb);

  /* Test the 16 bit implementation path */
  nelems = 6;
  bytes16 = (uint16_t[]){
      24929, /* 0110000101100001 */
      24930, /* 0110000101100010 */
      24931, /* 0110000101100011 */
      24932, /* 0110000101100100 */
      24933, /* 0110000101100101 */
      24934  /* 0110000101100110 */
  };
  /* encoded 0110000101100 001 010 011 100 101 110 */
  bytes = (uint8_t *)bytes16;
  pcb = initbytes(bytes, nelems * 2, PC_INT16);

  /* Test the 16 bit implementation path */
  common16 = pc_bytes_sigbits_count_16(&pcb, &count);
  CU_ASSERT_EQUAL(common16, 24928);
  CU_ASSERT_EQUAL(count, 13);
  epcb = pc_bytes_sigbits_encode(pcb);
  ebytes16 = (uint16_t *)(epcb.bytes);
  // printf("commonbits %d\n", commonbits);
  CU_ASSERT_EQUAL(ebytes16[0], 3);     /* unique bit count */
  CU_ASSERT_EQUAL(ebytes16[1], 24928); /* common bits */
  CU_ASSERT_EQUAL(ebytes16[2], 10699); /* packed uint16 one */

  /* uint8_t* pc_bytes_sigbits_decode(const uint8_t *bytes, uint32_t
   * interpretation, uint32_t nelems) */
  pcb2 = pc_bytes_sigbits_decode(epcb);
  pc_bytes_free(epcb);
  bytes16 = (uint16_t *)(pcb2.bytes);
  CU_ASSERT_EQUAL(bytes16[0], 24929);
  CU_ASSERT_EQUAL(bytes16[1], 24930);
  CU_ASSERT_EQUAL(bytes16[2], 24931);
  CU_ASSERT_EQUAL(bytes16[3], 24932);
  CU_ASSERT_EQUAL(bytes16[4], 24933);
  CU_ASSERT_EQUAL(bytes16[5], 24934);
  pc_bytes_free(pcb2);

  /* Test the 32 bit implementation path */
  nelems = 6;

  bytes32 = (uint32_t[]){
      103241, /* 0000000000000001 1001 0011 0100 1001 */
      103251, /* 0000000000000001 1001 0011 0101 0011 */
      103261, /* 0000000000000001 1001 0011 0101 1101 */
      103271, /* 0000000000000001 1001 0011 0110 0111 */
      103281, /* 0000000000000001 1001 0011 0111 0001 */
      103291  /* 0000000000000001 1001 0011 0111 1011 */
  };
  bytes = (uint8_t *)bytes32;
  pcb = initbytes(bytes, nelems * 4, PC_INT32);

  common32 = pc_bytes_sigbits_count_32(&pcb, &count);
  CU_ASSERT_EQUAL(count, 26); /* common bits count */
  CU_ASSERT_EQUAL(common32, 103232);

  epcb = pc_bytes_sigbits_encode(pcb);
  ebytes32 = (uint32_t *)(epcb.bytes);
  CU_ASSERT_EQUAL(ebytes32[0], 6);         /* unique bit count */
  CU_ASSERT_EQUAL(ebytes32[1], 103232);    /* common bits */
  CU_ASSERT_EQUAL(ebytes32[2], 624388039); /* packed uint32 */

  pcb2 = pc_bytes_sigbits_decode(epcb);
  pc_bytes_free(epcb);
  bytes32 = (uint32_t *)(pcb2.bytes);
  CU_ASSERT_EQUAL(bytes32[0], 103241);
  CU_ASSERT_EQUAL(bytes32[1], 103251);
  CU_ASSERT_EQUAL(bytes32[2], 103261);
  CU_ASSERT_EQUAL(bytes32[3], 103271);
  CU_ASSERT_EQUAL(bytes32[4], 103281);
  CU_ASSERT_EQUAL(bytes32[5], 103291);
  pc_bytes_free(pcb2);

  /* What if all the words are the same? */
  nelems = 6;
  bytes16 = (uint16_t[]){
      24929, /* 0000000000000001 1001 0011 0100 1001 */
      24929, /* 0000000000000001 1001 0011 0101 0011 */
      24929, /* 0000000000000001 1001 0011 0101 1101 */
      24929, /* 0000000000000001 1001 0011 0110 0111 */
      24929, /* 0000000000000001 1001 0011 0111 0001 */
      24929  /* 0000000000000001 1001 0011 0111 1011 */
  };
  bytes = (uint8_t *)bytes16;
  pcb = initbytes(bytes, nelems * 2, PC_INT16);
  epcb = pc_bytes_sigbits_encode(pcb);
  pcb2 = pc_bytes_sigbits_decode(epcb);
  pc_bytes_free(epcb);
  pc_bytes_free(pcb2);

  /* Test the 64 bit implementation path */

  nelems = 6;

  bytes64 = (uint64_t[]){
      103241, /* 32x0 0000000000000001 1001 0011 0100 1001 */
      103251, /* 32x0 0000000000000001 1001 0011 0101 0011 */
      103261, /* 32x0 0000000000000001 1001 0011 0101 1101 */
      103271, /* 32x0 0000000000000001 1001 0011 0110 0111 */
      103281, /* 32x0 0000000000000001 1001 0011 0111 0001 */
      103291  /* 32x0 0000000000000001 1001 0011 0111 1011 */
  };
  bytes = (uint8_t *)bytes64;
  pcb = initbytes(bytes, nelems * 8, PC_INT64);

  common64 = pc_bytes_sigbits_count_64(&pcb, &count);
  CU_ASSERT_EQUAL(count, 58); /* common bits count */
  CU_ASSERT_EQUAL(common64, 103232);

  epcb = pc_bytes_sigbits_encode(pcb);
  ebytes64 = (uint64_t *)(epcb.bytes);
  CU_ASSERT_EQUAL(ebytes64[0], 6);                   /* unique bit count */
  CU_ASSERT_EQUAL(ebytes64[1], 103232);              /* common bits */
  CU_ASSERT_EQUAL(ebytes64[2], 2681726210471362560); /* packed uint64 */

  pcb2 = pc_bytes_sigbits_decode(epcb);
  pc_bytes_free(epcb);
  bytes64 = (uint64_t *)(pcb2.bytes);
  CU_ASSERT_EQUAL(bytes64[0], 103241);
  CU_ASSERT_EQUAL(bytes64[1], 103251);
  CU_ASSERT_EQUAL(bytes64[2], 103261);
  CU_ASSERT_EQUAL(bytes64[3], 103271);
  CU_ASSERT_EQUAL(bytes64[4], 103281);
  CU_ASSERT_EQUAL(bytes64[5], 103291);
  pc_bytes_free(pcb2);
}

/*
 * Encode and decode a byte stream. Data matches?
 */
static void test_zlib_encoding()
{
  uint8_t *bytes;
  PCBYTES pcb, epcb, pcb2;
  /*
  uint8_t *
  pc_bytes_zlib_encode(const uint8_t *bytes, uint32_t interpretation, uint32_t
  nelems) uint8_t * pc_bytes_zlib_decode(const uint8_t *bytes, uint32_t
  interpretation)
  */
  bytes = (uint8_t *)"abcaabcaabcbabcc";
  pcb = initbytes(bytes, strlen((char *)bytes), PC_INT8);
  epcb = pc_bytes_zlib_encode(pcb);
  pcb2 = pc_bytes_zlib_decode(epcb);

  CU_ASSERT_EQUAL(pcb.compression, PC_DIM_NONE);
  CU_ASSERT_EQUAL(epcb.compression, PC_DIM_ZLIB);
  CU_ASSERT_EQUAL(pcb2.compression, PC_DIM_NONE);

  CU_ASSERT_EQUAL(memcmp(pcb.bytes, pcb2.bytes, pcb.size), 0);
  pc_bytes_free(epcb);
  pc_bytes_free(pcb2);
}

static void test_rle_filter()
{
  char *bytes;
  PCBYTES pcb, epcb, fpcb;
  PCBITMAP *map1, *map2;
  int i;

  /*
  typedef struct
  {
          size_t size;
          uint32_t npoints;
          uint32_t interpretation;
          uint32_t compression;
          uint8_t *bytes;
  } PCBYTES;
  */

  bytes = "aaaabbbbccdd";
  pcb = initbytes((uint8_t *)bytes, strlen(bytes), PC_UINT8);
  epcb = pc_bytes_run_length_encode(pcb);
  CU_ASSERT_EQUAL(epcb.bytes[0], 4);

  map1 = pc_bytes_bitmap(&epcb, PC_GT, 'b', 'b');
  CU_ASSERT_EQUAL(map1->nset, 4);
  map2 = pc_bytes_bitmap(&epcb, PC_GT, 'a', 'a');
  CU_ASSERT_EQUAL(map2->nset, 8);

  fpcb = pc_bytes_filter(&epcb, map1, NULL);
  CU_ASSERT_EQUAL(fpcb.bytes[0], 2);
  CU_ASSERT_EQUAL(fpcb.bytes[1], 'c');
  CU_ASSERT_EQUAL(fpcb.bytes[2], 2);
  CU_ASSERT_EQUAL(fpcb.bytes[3], 'd');
  CU_ASSERT_EQUAL(fpcb.size, 4);
  CU_ASSERT_EQUAL(fpcb.npoints, 4);
  pc_bytes_free(fpcb);
  pc_bitmap_free(map1);

  fpcb = pc_bytes_filter(&epcb, map2, NULL);
  CU_ASSERT_EQUAL(fpcb.bytes[0], 4);
  CU_ASSERT_EQUAL(fpcb.bytes[1], 'b');
  CU_ASSERT_EQUAL(fpcb.bytes[2], 2);
  CU_ASSERT_EQUAL(fpcb.bytes[3], 'c');
  CU_ASSERT_EQUAL(fpcb.size, 6);
  CU_ASSERT_EQUAL(fpcb.npoints, 8);
  pc_bytes_free(fpcb);
  pc_bitmap_free(map2);
  pc_bytes_free(epcb);

  bytes = (char *)((uint32_t[]){10, 10, 10, 20, 20, 30, 20, 20});
  pcb = initbytes((uint8_t *)bytes, 8 * 4, PC_UINT32);
  epcb = pc_bytes_run_length_encode(pcb);
  map1 = pc_bytes_bitmap(&epcb, PC_LT, 25, 25); /* strip out the 30 */
  CU_ASSERT_EQUAL(map1->nset, 7);
  fpcb = pc_bytes_filter(&epcb, map1, NULL);
  CU_ASSERT_EQUAL(fpcb.size,
                  15); /* three runs (2x10, 2x20, 2x20), of 5 bytes eachh */
  CU_ASSERT_EQUAL(fpcb.npoints, 7);
  pc_bytes_free(fpcb);
  pc_bytes_free(pcb);
  pc_bitmap_free(map1);

  bytes = (char *)((uint16_t[]){1, 2, 3, 4, 5, 6, 7, 8});
  pcb = initbytes((uint8_t *)bytes, 8 * 2, PC_UINT16);
  map1 = pc_bytes_bitmap(&pcb, PC_BETWEEN, 2.5,
                         4.5); /* everything except entries 3 and 4 */
  CU_ASSERT_EQUAL(map1->nset, 2);
  fpcb = pc_bytes_filter(&epcb, map1,
                         NULL);   /* Should have only two entry, 10, 20 */
  CU_ASSERT_EQUAL(fpcb.size, 10); /* two runs (1x10, 1x20), of 5 bytes eachh */
  CU_ASSERT_EQUAL(fpcb.npoints, 2);
  CU_ASSERT_EQUAL(fpcb.bytes[0], 1);
  CU_ASSERT_EQUAL(fpcb.bytes[5], 1);
  memcpy(&i, fpcb.bytes + 1, 4);
  CU_ASSERT_EQUAL(i, 10);
  memcpy(&i, fpcb.bytes + 6, 4);
  CU_ASSERT_EQUAL(i, 20);

  pc_bytes_free(fpcb);
  pc_bytes_free(pcb);
  pc_bitmap_free(map1);
  pc_bytes_free(epcb);
}

static void test_uncompressed_filter()
{
  char *bytes;
  PCBYTES pcb, fpcb;
  PCBITMAP *map1;

  /*
  typedef struct
  {
          size_t size;
          uint32_t npoints;
          uint32_t interpretation;
          uint32_t compression;
          uint8_t *bytes;
  } PCBYTES;
  */

  bytes = "aaaabbbbccdd";
  pcb = initbytes((uint8_t *)bytes, strlen(bytes), PC_UINT8);
  CU_ASSERT_EQUAL(pcb.bytes[0], 'a');
  CU_ASSERT_EQUAL(pcb.npoints, 12);

  map1 = pc_bytes_bitmap(&pcb, PC_GT, 'b', 'b');
  CU_ASSERT_EQUAL(map1->nset, 4);

  fpcb = pc_bytes_filter(&pcb, map1, NULL);
  CU_ASSERT_EQUAL(fpcb.bytes[0], 'c');
  CU_ASSERT_EQUAL(fpcb.size, 4);
  CU_ASSERT_EQUAL(fpcb.npoints, 4);
  pc_bytes_free(fpcb);
  pc_bitmap_free(map1);
  //    pc_bytes_free(epcb);
}

/* REGISTER ***********************************************************/

CU_TestInfo bytes_tests[] = {
    PC_TEST(test_run_length_encoding), PC_TEST(test_sigbits_encoding),
    PC_TEST(test_zlib_encoding),       PC_TEST(test_rle_filter),
    PC_TEST(test_uncompressed_filter), CU_TEST_INFO_NULL};

CU_SuiteInfo bytes_suite = {.pName = "bytes",
                            .pInitFunc = init_suite,
                            .pCleanupFunc = clean_suite,
                            .pTests = bytes_tests};
