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
 * @brief Functions for gathering statistics from temporal alphanumeric columns
 */

#ifndef __TEMPORAL_ANALYZE_H__
#define __TEMPORAL_ANALYZE_H__

/* PostgreSQL */
#include <postgres.h>
#include <statistics/extended_stats_internal.h>
#include <fmgr.h>

/*****************************************************************************/

/*
 * Extra data for compute_stats function
 * Structure based on the ArrayAnalyzeExtraData from file array_typanalyze.c
 */
typedef struct
{
  /* Information about array element type */
  Oid typid;           /**< element type's OID */
  Oid eq_opr;          /**< default equality operator's OID */
  Oid lt_opr;          /**< default less than operator's OID */
  bool typbyval;       /**< physical properties of element type */
  int16 typlen;
  char typalign;

  /* Information about the value part of array element */
  Oid value_typid;     /**< element type's OID */
  Oid value_eq_opr;    /**< default equality operator's OID */
  Oid value_lt_opr;    /**< default less than operator's OID */
  bool value_typbyval; /**< physical properties of element type */
  int16 value_typlen;
  char value_typalign;

  /* Information about the temporal part of array element */
  Oid time_typid;      /**< element type's OID */
  Oid time_eq_opr;     /**< default equality operator's OID */
  Oid time_lt_opr;     /**< default less than operator's OID */
  bool time_typbyval;  /**< physical properties of element type */
  int16 time_typlen;
  char time_typalign;

  /*
   * Lookup data for element type's comparison and hash functions (these are
   * in the type's typcache entry, which we expect to remain valid over the
   * lifespan of the ANALYZE run)
   */
  FmgrInfo *cmp;
  FmgrInfo *hash;
  FmgrInfo *value_cmp;
  FmgrInfo *value_hash;
  FmgrInfo *time_cmp;
  FmgrInfo *time_hash;

  /* Saved state from std_typanalyze() */
  AnalyzeAttrComputeStatsFunc std_compute_stats;
  void *std_extra_data;
} TemporalAnalyzeExtraData;

/*
 * Extra information used by the default analysis routines
 */
typedef struct
{
  int count;    /**< # of duplicates */
  int first;    /**< values[] index of first occurrence */
} ScalarMCVItem;

typedef struct
{
  SortSupport ssup;
  int *tupnoLink;
} CompareScalarsContext;

/*****************************************************************************
 * Statistics information for temporal types
 *****************************************************************************/

extern void temporal_extra_info(VacAttrStats *stats);

extern void tinstant_compute_stats(VacAttrStats *stats,
  AnalyzeAttrFetchFunc fetchfunc, int samplerows, double totalrows);

extern void tsequenceset_compute_stats(VacAttrStats *stats,
  AnalyzeAttrFetchFunc fetchfunc, int samplerows, double totalrows);

/*****************************************************************************/

extern Datum generic_analyze(FunctionCallInfo fcinfo,
  void (*functemp)(VacAttrStats *, AnalyzeAttrFetchFunc, int, double));

/*****************************************************************************/

#endif
