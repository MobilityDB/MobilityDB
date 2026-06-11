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

#include <fmgr.h>			// PG_FUNCTION_INFO_V1
#include <utils/builtins.h> // cstring_to_text

#include "type.h"
#include "config.h"

PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_get_extension_version);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_pg_migrate_pass_by_reference);

/* Return version number for this extension (not main h3 lib) */
Datum
h3_get_extension_version(PG_FUNCTION_ARGS)
{
	PG_RETURN_TEXT_P(cstring_to_text(POSTGRESQL_H3_VERSION));
}

/*
 * Migration from pass-by-reference to pass-by-value
 * https://github.com/zachasme/h3-pg/issues/31
 */
Datum
h3_pg_migrate_pass_by_reference(PG_FUNCTION_ARGS)
{
	H3Index		cell = (*((H3Index *) DatumGetPointer(PG_GETARG_DATUM(0))));

	PG_RETURN_H3INDEX(cell);
}
