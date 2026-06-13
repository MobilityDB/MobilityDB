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
 * @brief Public Jsonb / JsonPath type composition for the JSON family.
 *
 * This header exposes @p Jsonb and @p JsonPath as opaque varlena-based types
 * (as @p text / @p bytea are exposed in postgres_ext_defs.in.h) so it is a
 * self-contained PUBLIC header: external consumers and the exported
 * meos_json.h get the types without any internal pgtypes header. The full
 * @p JsonbContainer / @p JEntry layout lives in the internal pgtypes sources,
 * which the library compiles against directly; this header is never included
 * by the library build. The public base json/jsonb/jsonpath function
 * prototypes are declared in meos/include/meos_json.h so that they live under
 * the public scan root and are catalogued by the MEOS-API generator for every
 * binding.
 */

#ifndef __PG_JSON_H__
#define __PG_JSON_H__

typedef struct varlena Jsonb;
typedef struct varlena JsonPath;

#endif /* __PG_JSON_H__ */
