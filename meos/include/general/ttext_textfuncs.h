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
 * @brief Temporal text functions: `textcat`, `lower`, `upper`.
 */

#ifndef __TTEXT_TEXTFUNCS_H__
#define __TTEXT_TEXTFUNCS_H__

/* PostgreSQL */
#include <postgres.h>
/* PostgreSQL */
#include "general/temporal.h"

/*****************************************************************************/

extern Datum datum_textcat(Datum l, Datum r);
extern Datum datum_lower(Datum value);
extern Datum datum_upper(Datum value);

extern Temporal *textfunc_ttext(const Temporal *temp,
  Datum (*func)(Datum value));
extern Temporal *textfunc_ttext_text(const Temporal *temp, Datum value,
  datum_func2 func, bool invert);
extern Temporal *textfunc_ttext_ttext(const Temporal *temp1,
  const Temporal *temp2, datum_func2 func);

/*****************************************************************************/

#endif
