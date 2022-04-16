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
 * @file tnumber_distance.h
 * Distance functions for temporal numbers.
 */

#ifndef __TNUMBER_DISTANCE_H__
#define __TNUMBER_DISTANCE_H__

/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_type.h>
#include <float.h>
/* MobilityDB */
#include "general/tempcache.h"
#include "general/temporal.h"

/*****************************************************************************/

/* Distance functions */

extern Temporal *distance_tnumber_number(const Temporal *temp, Datum value,
  CachedType valuetype, CachedType restype);
extern Temporal *distance_tnumber_tnumber(const Temporal *temp1,
  const Temporal *temp2, CachedType restype);

extern Datum number_distance(Datum l, Datum r, CachedType typel,
  CachedType typer);

/* Nearest approach distance */

extern double nad_tnumber_number(const Temporal *temp, Datum value,
  CachedType basetype);
extern double nad_tbox_tbox(const TBOX *box1, const TBOX *box2);
extern double nad_tnumber_tbox(const Temporal *temp, const TBOX *box);

// NAI and shortestline functions are not yet implemented
// Are they useful ?

/*****************************************************************************/

#endif
