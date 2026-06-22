/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief API of the Apache Arrow C Data Interface export for the Mobility
 * Engine Open Source (MEOS) library.
 */

#ifndef __MEOS_ARROW_H__
#define __MEOS_ARROW_H__

/* C */
#include <stdbool.h>
#include <stdint.h>

/* MEOS */
#include <meos.h>

/*****************************************************************************
 * Arrow C Data Interface conversion
 *****************************************************************************/

/* Arrow structures are forward-declared to keep this header free of an Arrow
 * dependency; the definitions live in arrow/arrow_c_data_interface.h. */
struct ArrowSchema;
struct ArrowArray;

extern bool meos_temporal_to_arrow(const Temporal *temp, struct ArrowSchema *out_schema, struct ArrowArray *out_array);
extern Temporal *meos_temporal_from_arrow(const struct ArrowSchema *schema, const struct ArrowArray *array);
extern Temporal *meos_temporal_arrow_roundtrip(const Temporal *temp);
extern bool meos_set_to_arrow(const Set *s, struct ArrowSchema *out_schema, struct ArrowArray *out_array);
extern Set *meos_set_from_arrow(const struct ArrowSchema *schema, const struct ArrowArray *array);
extern Set *meos_set_arrow_roundtrip(const Set *s);
extern bool meos_span_to_arrow(const Span *s, struct ArrowSchema *out_schema, struct ArrowArray *out_array);
extern Span *meos_span_from_arrow(const struct ArrowSchema *schema, const struct ArrowArray *array);
extern Span *meos_span_arrow_roundtrip(const Span *s);
extern bool meos_spanset_to_arrow(const SpanSet *ss, struct ArrowSchema *out_schema, struct ArrowArray *out_array);
extern SpanSet *meos_spanset_from_arrow(const struct ArrowSchema *schema, const struct ArrowArray *array);
extern SpanSet *meos_spanset_arrow_roundtrip(const SpanSet *ss);
extern bool meos_tbox_to_arrow(const TBox *box, struct ArrowSchema *out_schema, struct ArrowArray *out_array);
extern TBox *meos_tbox_from_arrow(const struct ArrowSchema *schema, const struct ArrowArray *array);
extern TBox *meos_tbox_arrow_roundtrip(const TBox *box);
extern bool meos_stbox_to_arrow(const STBox *box, struct ArrowSchema *out_schema, struct ArrowArray *out_array);
extern STBox *meos_stbox_from_arrow(const struct ArrowSchema *schema, const struct ArrowArray *array);
extern STBox *meos_stbox_arrow_roundtrip(const STBox *box);

/*****************************************************************************/

#endif /* __MEOS_ARROW_H__ */
