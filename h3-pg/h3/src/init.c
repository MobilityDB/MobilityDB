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

/* required for MSVC */
#define _USE_MATH_DEFINES

#include <postgres.h>

#include <fmgr.h> // PG_MODULE_MAGIC

#include "guc.h"

/* see https://www.postgresql.org/docs/current/xfunc-c.html#XFUNC-C-DYNLOAD */
PG_MODULE_MAGIC;

void
_PG_init(void)
{
	/* In case we allow using shared library h3, */
	/* we could make version number assertion here */

	_guc_init();
}
