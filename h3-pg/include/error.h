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

#ifndef H3_ERROR_H
#define H3_ERROR_H

void h3_assert(int error);

#define ASSERT(condition, code, msg, ...)  \
	if (0 == (condition)) ereport(ERROR, ( \
		errcode(code),					   \
		errmsg(msg, ##__VA_ARGS__)		   \
	))

#define ENSURE_TYPEFUNC_COMPOSITE(x)				   \
	ASSERT(											   \
		x == TYPEFUNC_COMPOSITE,					   \
		ERRCODE_INVALID_PARAMETER_VALUE,			   \
		"Function returning record called in context " \
		"that cannot accept type record"			   \
	)

#define H3_DEPRECATION(msg)			  \
	ereport(WARNING, (				  \
		errmsg("Deprecation notice: %s", msg) \
	))

#define DEBUG(msg, ...)			   \
	ereport(NOTICE, (			   \
		errmsg(msg, ##__VA_ARGS__) \
	))

#define DEBUG_H3INDEX(h3index) DEBUG("index: %lx", h3index)

#endif /* H3_ERROR_H */
