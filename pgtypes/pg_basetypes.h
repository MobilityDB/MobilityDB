/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief The PostgreSQL scalar types the pgtypes headers and the installed
 * <meos.h> are written against
 * @details A translation unit reaches these names either from PostgreSQL
 * itself — the backend's headers, or the copies this tree vendors under
 * pgtypes/ — or from here. Each group below is emitted only when the
 * PostgreSQL header defining it is not yet in scope, as its include guard
 * tells: `POSTGRES_H` for the names `postgres.h` brings with `c.h` and
 * `postgres_ext.h`, `DATATYPE_TIMESTAMP_H`, `DATE_H` and `_PG_NUMERIC_H_` for
 * those of `datatype/timestamp.h`, `utils/date.h` and `utils/numeric.h`.
 *
 * Every declaration is the one of the vendored PostgreSQL 18 header, word for
 * word, which tools/scripts/check_typedef_sites.py verifies together
 * with the absence of any other definition. A unit reading a name from both
 * sites then reads the same type twice, which C11 accepts for a scalar or
 * pointer typedef; `int64` is `long int` in the `c.h` of PostgreSQL 16 and 17
 * and `int64_t` here, which on macOS are distinct types, and the `POSTGRES_H`
 * guard keeps a unit to one of them. A structure is not stated here, since a
 * second definition of a structure is a conflict even when it is identical:
 * `Interval` and `pg_prng_state` are stated by the installed <meos.h>, and
 * `TimeTzADT` by pg_time.h.
 */

#ifndef __PG_BASETYPES_H__
#define __PG_BASETYPES_H__

#include <stdint.h>

#ifndef POSTGRES_H

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float float4;
typedef double float8;

typedef char *Pointer;
typedef uintptr_t Datum;
typedef unsigned int Oid;

struct varlena;
typedef struct varlena bytea;
typedef struct varlena text;

#endif /* POSTGRES_H */

#ifndef DATATYPE_TIMESTAMP_H

typedef int64 Timestamp;
typedef int64 TimestampTz;
typedef int64 TimeOffset;
typedef int32 fsec_t;

#endif /* DATATYPE_TIMESTAMP_H */

#ifndef DATE_H

typedef int32 DateADT;
typedef int64 TimeADT;

#endif /* DATE_H */

#ifndef _PG_NUMERIC_H_

struct NumericData;
typedef struct NumericData *Numeric;

#endif /* _PG_NUMERIC_H_ */

#endif /* __PG_BASETYPES_H__ */
