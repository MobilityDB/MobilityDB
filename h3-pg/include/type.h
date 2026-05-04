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

#ifndef H3_TYPE_H
#define H3_TYPE_H

#include <h3api.h>

/*
 * DatumGetH3Index
 *		Returns H3 index value of a datum.
 *
 * Note: this macro hides whether h3 index is pass by value or by reference.
 */

#ifdef USE_FLOAT8_BYVAL
#define DatumGetH3Index(X) ((H3Index) (X))
#else
#define DatumGetH3Index(X) (* ((H3Index *) DatumGetPointer(X)))
#endif

/*
 * H3IndexGetDatum
 *		Returns datum representation for an H3 index.
 *
 * Note: if H3 index is pass by reference, this function returns a reference
 * to palloc'd space.
 */

#ifdef USE_FLOAT8_BYVAL
#define H3IndexGetDatum(X) ((Datum) (X))
#else
#define H3IndexGetDatum(X) Int64GetDatum((int64) (X))
#endif

/* Macros for fetching arguments and returning results of h3 index type */

#define PG_GETARG_H3INDEX(n) DatumGetH3Index(PG_GETARG_DATUM(n))
#define PG_RETURN_H3INDEX(x) return H3IndexGetDatum(x)

/* use origin resolution minus one when no resolution is given */
#define PG_GETARG_OPTIONAL_RES(n, cell, offset) \
	PG_NARGS() == (n + 1) ? PG_GETARG_INT32(1) : getResolution(cell) + offset

#endif /* H3_TYPE_H */
