/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Bounding box operators for temporal network points.
 */

#ifndef __PG_TNPOINT_BOXOPS_H__
#define __PG_TNPOINT_BOXOPS_H__

/* PostgreSQL */
#include <postgres.h>
#include <utils/palloc.h>
#include <fmgr.h>
/* MEOS */
#include "general/temporal.h"
#include "npoint/tnpoint.h"

/*****************************************************************************/

extern Datum boxop_geo_tnpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *));
extern Datum boxop_tnpoint_geo_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *));
extern Datum boxop_stbox_tnpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *));
extern Datum boxop_tnpoint_stbox_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *));
extern Datum boxop_npoint_tnpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *));
extern Datum boxop_tnpoint_npoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *));
extern Datum boxop_tnpoint_tnpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *));

/*****************************************************************************/

#endif /* __PG_TNPOINT_BOXOPS_H__ */
