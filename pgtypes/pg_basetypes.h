/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/


/**
 * @file
 * @brief The PostgreSQL base types the pgtypes headers and the installed
 * <meos.h> are written against
 * @details A translation unit reaches these names either from PostgreSQL
 * itself — the backend's `postgres.h`, or the copy this tree vendors, both of
 * which define `POSTGRES_H` — or from here. Stating them in one place is what
 * keeps the two readings the same type: `int64` is `long int` in PostgreSQL 17
 * and `int64_t` here, and where those differ a second definition of the name
 * is a redefinition with a different type rather than a harmless repetition.
 */

#ifndef __PG_BASETYPES_H__
#define __PG_BASETYPES_H__

#include <stdint.h>

#ifndef POSTGRES_H

typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
typedef int64_t int64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef uint64_t uint64;

typedef float float4;
typedef double float8;

#endif /* POSTGRES_H */

#endif /* __PG_BASETYPES_H__ */
