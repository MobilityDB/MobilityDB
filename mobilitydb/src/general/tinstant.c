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
 * @brief General functions for temporal instants.
 */

#include "general/tinstant.h"

/* PostgreSQL */
#include <libpq/pqformat.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
/* MobilityDB */
#include "pg_general/temporal_util.h"

/*****************************************************************************
 * Send and receive functions
 *****************************************************************************/

/**
 * @brief Return a temporal instant from its binary representation read from
 * a buffer.
 *
 * @param[in] buf Buffer
 * @param[in] temptype Temporal type
 */
TInstant *
tinstant_recv(StringInfo buf, mobdbType temptype)
{
  TimestampTz t = call_recv(T_TIMESTAMPTZ, buf);
  int size = pq_getmsgint(buf, 4);
  StringInfoData buf2 =
  {
    .cursor = 0,
    .len = size,
    .maxlen = size,
    .data = buf->data + buf->cursor
  };
  mobdbType basetype = temptype_basetype(temptype);
  Datum value = call_recv(basetype, &buf2);
  buf->cursor += size;
  return tinstant_make(value, temptype, t);
}

/**
 * @brief Write the binary representation of a temporal instant into
 * a buffer.
 *
 * @param[in] inst Temporal instant
 * @param[in] buf Buffer
 */
void
tinstant_write(const TInstant *inst, StringInfo buf)
{
  mobdbType basetype = temptype_basetype(inst->temptype);
  bytea *bt = call_send(T_TIMESTAMPTZ, TimestampTzGetDatum(inst->t));
  bytea *bv = call_send(basetype, tinstant_value(inst));
  pq_sendbytes(buf, VARDATA(bt), VARSIZE(bt) - VARHDRSZ);
  pq_sendint32(buf, VARSIZE(bv) - VARHDRSZ);
  pq_sendbytes(buf, VARDATA(bv), VARSIZE(bv) - VARHDRSZ);
}

/*****************************************************************************/
