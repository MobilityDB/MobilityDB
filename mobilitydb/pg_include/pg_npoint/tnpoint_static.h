/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @brief Network-based static point/segments
 */

#ifndef __PG_TNPOINT_STATIC_H__
#define __PG_TNPOINT_STATIC_H__

/* PostgreSQL */
#include <postgres.h>
#include <utils/array.h>
#include <catalog/pg_type.h>
#include <lib/stringinfo.h>
/* MobilityDB */
#include "npoint/tnpoint.h"

/*****************************************************************************/

/* General functions */

extern ArrayType *int64arr_to_array(const int64 *int64arr, int count);
extern ArrayType *nsegmentarr_to_array(Nsegment **nsegmentarr, int count);

/* Input/output functions */

extern Nsegment *nsegment_recv(StringInfo buf);
extern bytea *nsegment_send(const Nsegment *ns);

/* Transformation functions */

extern Datum datum_npoint_round(Datum npoint, Datum size);
extern Npoint *npoint_round(const Npoint *np, Datum size);
extern Nsegment *nsegment_round(const Nsegment *ns, Datum size);

/*****************************************************************************/

#endif /* __PG_TNPOINT_STATIC_H__ */
