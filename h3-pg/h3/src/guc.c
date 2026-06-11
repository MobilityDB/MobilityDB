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

#include <utils/guc.h> // DefineCustom*Variable

bool		h3_guc_strict = false;
bool		h3_guc_extend_antimeridian = false;

void
_guc_init(void)
{
	DefineCustomBoolVariable("h3.strict",
						 "Enable strict indexing (fail on invalid lng/lat).",
							 NULL,
							 &h3_guc_strict,
							 false,
							 PGC_USERSET,
							 0,
							 NULL,
							 NULL,
							 NULL);

	DefineCustomBoolVariable("h3.extend_antimeridian",
					   "Extend boundaries by 180th meridian, when possible.",
							 NULL,
							 &h3_guc_extend_antimeridian,
							 false,
							 PGC_USERSET,
							 0,
							 NULL,
							 NULL,
							 NULL);
}
