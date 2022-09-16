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
 * @brief General functions for temporal sequences.
 */

#include "general/tsequence.h"

/* PostgreSQL */
#include <libpq/pqformat.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporaltypes.h"
/* MobilityDB */
#include "pg_general/tinstant.h"

/* The send and receive functions are needed for temporal aggregation */

/**
 * @brief Return a temporal sequence from its binary representation read from
 * a buffer.
 *
 * @param[in] buf Buffer
 * @param[in] temptype Temporal type
 */
TSequence *
tsequence_recv(StringInfo buf, mobdbType temptype)
{
  int count = (int) pq_getmsgint(buf, 4);
  bool lower_inc = (char) pq_getmsgbyte(buf);
  bool upper_inc = (char) pq_getmsgbyte(buf);
  interpType interp = (char) pq_getmsgbyte(buf);
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
    instants[i] = tinstant_recv(buf, temptype);
  return tsequence_make_free(instants, count, count, lower_inc, upper_inc,
    interp, NORMALIZE);
}

/**
 * @brief Write the binary representation of a temporal sequence into a buffer.
 *
 * @param[in] seq Temporal sequence
 * @param[in] buf Buffer
 */
void
tsequence_write(const TSequence *seq, StringInfo buf)
{
  pq_sendint32(buf, seq->count);
  pq_sendbyte(buf, seq->period.lower_inc ? (uint8) 1 : (uint8) 0);
  pq_sendbyte(buf, seq->period.upper_inc ? (uint8) 1 : (uint8) 0);
  pq_sendbyte(buf, (uint8) MOBDB_FLAGS_GET_INTERP(seq->flags));
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    tinstant_write(inst, buf);
  }
  return;
}

/*****************************************************************************/
