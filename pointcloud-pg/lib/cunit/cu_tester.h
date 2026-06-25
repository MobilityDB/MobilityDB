/***********************************************************************
 * cu_tester.h
 *
 *        Testing harness for PgSQL PointClouds header
 *
 * Portions Copyright (c) 2012, OpenGeo
 *
 ***********************************************************************/

#ifndef _CU_TESTER_H
#define _CU_TESTER_H

#include "pc_api_internal.h"

#define PC_TEST(test_func)                                                     \
  {                                                                            \
#test_func, test_func                                                      \
  }
#define MAX_CUNIT_MSG_LENGTH 512
#define CU_ASSERT_SUCCESS(rv) CU_ASSERT((rv) == PC_SUCCESS)
#define CU_ASSERT_FAILURE(rv) CU_ASSERT((rv) == PC_FAILURE)

/* Read a file (XML) into a cstring */
char *file_to_str(const char *fname);

/* Resets cu_error_msg back to blank. */
void cu_error_msg_reset(void);

#endif
