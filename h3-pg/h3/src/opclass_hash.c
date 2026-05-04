/*
 * Copyright 2024 Zacharias Knudsen
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
#include <h3api.h>

#include <fmgr.h>		 // PG_FUNCTION_ARGS
#include <access/hash.h> // hash_any

#include "type.h"

PGDLLEXPORT PG_FUNCTION_INFO_V1(h3index_hash);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3index_hash_extended);

Datum
h3index_hash(PG_FUNCTION_ARGS)
{
	H3Index		index = PG_GETARG_H3INDEX(0);
	Datum		hash = hash_any((unsigned char *) &index, sizeof(index));

	PG_RETURN_DATUM(hash);
}

Datum
h3index_hash_extended(PG_FUNCTION_ARGS)
{
	H3Index		index = PG_GETARG_H3INDEX(0);
	int64_t		seed = PG_GETARG_INT64(1);
	Datum		hash = hash_any_extended((unsigned char *) &index, sizeof(index), seed);

	PG_RETURN_DATUM(hash);
}
