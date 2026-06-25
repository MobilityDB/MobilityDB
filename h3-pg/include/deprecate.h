/*
 * Copyright 2023 Zacharias Knudsen
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <postgres.h>

#include <fmgr.h> // PG_FUNCTION_INFO_V1

#include "error.h"

/*	Inspired by PostGIS legacy function handling */
/*	https://github.com/postgis/postgis/blob/master/postgis/postgis_legacy.c */

#define H3_DEPRECATE(version, funcname) \
	PGDLLEXPORT PG_FUNCTION_INFO_V1(funcname); \
	Datum funcname(PG_FUNCTION_ARGS) \
	{ \
		ereport(ERROR, (\
			errcode(ERRCODE_FEATURE_NOT_SUPPORTED), \
			errmsg("A stored procedure tried to use deprecated C function '%s'", \
				   __func__), \
			errdetail("Library function '%s' was deprecated in h3 %s", \
					  __func__, version), \
			errhint("Consider running: ALTER EXTENSION h3 UPDATE") \
		)); \
		PG_RETURN_POINTER(NULL); \
	}

#define H3_SOFT_DEPRECATE(oldfunc, newfunc) \
	Datum newfunc(PG_FUNCTION_ARGS); \
	PGDLLEXPORT PG_FUNCTION_INFO_V1(oldfunc); \
	Datum oldfunc(PG_FUNCTION_ARGS) \
	{ \
		H3_DEPRECATION(#oldfunc " will be deprecated in favor of " #newfunc " next major release"); \
		return newfunc(fcinfo); \
	}
