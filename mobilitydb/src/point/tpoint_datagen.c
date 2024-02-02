/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief Data generator for MobilityDB
 *
 * These functions are used in the BerlinMOD data generator
 * https://github.com/MobilityDB/MobilityDB-BerlinMOD
 */

/* PostgreSQL */
#include <postgres.h>
#include <access/htup_details.h>
#include <access/tupdesc.h>    /* for * () */
#include <executor/executor.h>  /* for GetAttributeByName() */
#include <utils/typcache.h>
#include <utils/float.h>
#include <utils/lsyscache.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include "point/tpoint_datagen.h"
#include "point/tpoint_spatialfuncs.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/temporal.h"

/*****************************************************************************/

PGDLLEXPORT Datum Create_trip(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Create_trip);
/**
 * @brief Create a trip using the BerlinMOD data generator.
 *
 * @note This function is equivalent to the PL/pgSQL function
 * CreateTrip in the BerlinMOD generator but is written in C
 * to speed up the generation.
 */
Datum
Create_trip(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_not_empty_array(array);
  if (ARR_NDIM(array) > 1)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
      errmsg("1-dimensional array needed")));
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool disturbData = PG_GETARG_BOOL(2);
  text *messages = PG_GETARG_TEXT_PP(3);
  char *msgstr = text2cstring(messages);
  int32 msg = 0; /* 'minimal' by default */
  Datum *datums;
  bool *nulls;
  int count;
  int16 elemWidth;
  Oid elmeTypid = ARR_ELEMTYPE(array);
  bool elemTypeByVal, isNull;
  char elemAlignmentCode;
  HeapTupleHeader td;
  Form_pg_attribute att;

  get_typlenbyvalalign(elmeTypid, &elemWidth, &elemTypeByVal, &elemAlignmentCode);
  deconstruct_array(array, elmeTypid, elemWidth, elemTypeByVal,
    elemAlignmentCode, &datums, &nulls, &count);

  td = DatumGetHeapTupleHeader(datums[0]);

  /* Extract rowtype info and find a tupdesc */
  Oid tupType = HeapTupleHeaderGetTypeId(td);
  int32 tupTypmod = HeapTupleHeaderGetTypMod(td);
  TupleDesc tupdesc = lookup_rowtype_tupdesc(tupType, tupTypmod);
  /* Verify the type of the attributes */
  att = TupleDescAttr(tupdesc, 0);
  if (att->atttypid != type_oid(T_GEOMETRY))
  {
    PG_FREE_IF_COPY(array, 0);
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("First element of the record must be of type geometry")));
  }
  att = TupleDescAttr(tupdesc, 1);
  if (att->atttypid != FLOAT8OID)
  {
    PG_FREE_IF_COPY(array, 0);
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Second element of the record must be of type double precision")));
  }
  att = TupleDescAttr(tupdesc, 2);
  if (att->atttypid != INT4OID)
  {
    PG_FREE_IF_COPY(array, 0);
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Third element of the record must be of type integer")));
  }
  ReleaseTupleDesc(tupdesc);

  LWLINE **lines = palloc(sizeof(LWLINE *) * count);
  double *maxSpeeds = palloc(sizeof(double) * count);
  int *categories = palloc(sizeof(int) * count);
  for (int i = 0; i < count; i++)
  {
    if (nulls[i])
    {
      PG_FREE_IF_COPY(array, 0);
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Elements of the array cannot be NULL")));
    }
    else
    {
      td = DatumGetHeapTupleHeader(datums[i]);
      /* First Attribute: Linestring */
      GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(GetAttributeByNum(td, 1, &isNull));
      if (isNull)
      {
        PG_FREE_IF_COPY(array, 0);
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
          errmsg("Elements of the record cannot be NULL")));
      }
      if (gserialized_get_type(gs) != LINETYPE)
      {
        PG_FREE_IF_COPY(array, 0);
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
          errmsg("Geometry must be a linestring")));
      }
      lines[i] = lwgeom_as_lwline(lwgeom_from_gserialized(gs));
      /* Second Attribute: Maximum Speed */
      maxSpeeds[i] = DatumGetFloat8(GetAttributeByNum(td, 2, &isNull));
      if (isNull)
      {
        PG_FREE_IF_COPY(array, 0);
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
          errmsg("Elements of the record cannot be NULL")));
      }
      /* Third Attribute: Category */
      categories[i] = DatumGetInt32(GetAttributeByNum(td, 3, &isNull));
      if (isNull)
      {
        PG_FREE_IF_COPY(array, 0);
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
          errmsg("Elements of the record cannot be NULL")));
      }
    }
  }

  if (strcmp(msgstr, "minimal") == 0)
    msg = 0;
  else if (strcmp(msgstr, "medium") == 0)
    msg = 1;
  else if (strcmp(msgstr, "verbose") == 0)
    msg = 2;
  else if (strcmp(msgstr, "debug") == 0)
    msg = 3;
  pfree(msgstr);

  TSequence *result = create_trip(lines, maxSpeeds, categories,
    (uint32_t) count, t, disturbData, msg);

  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_TSEQUENCE_P(result);
}

/*****************************************************************************/
