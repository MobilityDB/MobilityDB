/*****************************************************************************
 *
 * tnumber_spgist.c
 * SP-GiST implementation of 4-dimensional quad tree over temporal
 * integers and floats.
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2020, Université libre de Bruxelles and MobilityDB
 * contributors
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

#ifndef __TNUMBER_SPGIST_H__
#define __TNUMBER_SPGIST_H__

#include <postgres.h>
#include <fmgr.h>
#include <catalog/pg_type.h>
#include "temporal.h"

/*****************************************************************************/

extern Datum sptbox_gist_config(PG_FUNCTION_ARGS);
extern Datum sptbox_gist_choose(PG_FUNCTION_ARGS);
extern Datum sptbox_gist_picksplit(PG_FUNCTION_ARGS);
extern Datum sptbox_gist_inner_consistent(PG_FUNCTION_ARGS);
extern Datum sptbox_gist_leaf_consistent(PG_FUNCTION_ARGS);
extern Datum sptnumber_gist_compress(PG_FUNCTION_ARGS);

/* The following functions are also called by tpoint_spgist.c */
extern int compareDoubles(const void *a, const void *b);

/*****************************************************************************/

#endif
