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

#ifndef H3_SRF_H
#define H3_SRF_H

#include <h3api.h>

typedef struct
{
	H3Index    *indices;
	int		   *distances;
}	hexDistanceTuple;

/*	helper functions to return sets from user fctx */
Datum		srf_return_h3_indexes_from_user_fctx(PG_FUNCTION_ARGS);
Datum		srf_return_h3_index_distances_from_user_fctx(PG_FUNCTION_ARGS);

/*	macros to pass on fcinfo to above helpers */
#define SRF_RETURN_H3_INDEXES_FROM_USER_FCTX() \
	return srf_return_h3_indexes_from_user_fctx(fcinfo)
#define SRF_RETURN_H3_INDEX_DISTANCES_FROM_USER_FCTX() \
	return srf_return_h3_index_distances_from_user_fctx(fcinfo)

#endif /* H3_SRF_H */
