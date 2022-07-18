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
 * @brief Internal types used in particular for computing the average and
 * centroid temporal aggregates.
 */
#include "general/doublen.h"

/* C */
#include <float.h>
/* PostgreSQL */
#include <libpq/pqformat.h>
#include <utils/float.h>
/* MobilityDB */
#include <meos.h>
#include "general/temporal_util.h"

/*****************************************************************************
 * Send/receive functions
 *****************************************************************************/

/**
 * @brief Receive function for double2 values
 */
double2 *
double2_recv(StringInfo buf)
{
  double2 *result = palloc(sizeof(double2));
  const char *bytes = pq_getmsgbytes(buf, sizeof(double2));
  memcpy(result, bytes, sizeof(double2));
  return result;
}

/**
 * @brief Send function for double2 values
 */
bytea *
double2_send(double2 *d)
{
  StringInfoData buf;
  pq_begintypsend(&buf);
  pq_sendbytes(&buf, (void *) d, sizeof(double2));
  return (bytea *) pq_endtypsend(&buf);
}

/*****************************************************************************/

/**
 * @brief Receive function for double3 values
 */
double3 *
double3_recv(StringInfo buf)
{
  double3 *result = palloc(sizeof(double3));
  const char *bytes = pq_getmsgbytes(buf, sizeof(double3));
  memcpy(result, bytes, sizeof(double3));
  return result;
}

/**
 * @brief Send function for double3 values
 */
bytea *
double3_send(double3 *d)
{
  StringInfoData buf;
  pq_begintypsend(&buf);
  pq_sendbytes(&buf, (void *) d, sizeof(double3));
  return (bytea *) pq_endtypsend(&buf);
}

/*****************************************************************************/

/**
 * @brief Receive function for double4 values
 */
double4 *
double4_recv(StringInfo buf)
{
  double4 *result = palloc(sizeof(double4));
  const char *bytes = pq_getmsgbytes(buf, sizeof(double4));
  memcpy(result, bytes, sizeof(double4));
  return result;
}

/**
 * @brief Send function for double3 values
 */
bytea *
double4_send(double4 *d)
{
  StringInfoData buf;
  pq_begintypsend(&buf);
  pq_sendbytes(&buf, (void *) d, sizeof(double4));
  return (bytea *) pq_endtypsend(&buf);
}

/*****************************************************************************/
