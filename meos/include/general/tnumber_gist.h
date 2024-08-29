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
 * @brief In memory index for STBox based on RTree
 */

#ifndef __TNUMBER_GIST__
#define __TNUMBER_GIST__

/* MEOS */
#include <meos.h>
#include <general/meos_catalog.h>
#include <general/temporal.h>
/* PostgreSQL*/
#include <postgres.h>
#include <access/gist.h>


/*****************************************************************************
 * Structs
 *****************************************************************************/
/**
 * Structure keeping context for the function stbox_gist_consider_split.
 *
 * Contains information about currently selected split and some general
 * information.
 */
typedef struct
{
  int  entriesCount;  /**< total number of entries being split */
  bboxunion boundingBox;  /**< minimum bounding box across all entries */
  /** Information about currently selected split follows */
  bool first;        /**< true if no split was selected yet */
  double leftUpper;  /**< upper bound of left interval */
  double rightLower; /**< lower bound of right interval */
  float4 ratio;
  float4 overlap;
  int  dim;          /**< axis of this split */
  double range;      /**< width of general MBR projection to the selected axis */
} ConsiderSplitContext;



/*****************************************************************************
 * Functions
 *****************************************************************************/

extern void bbox_picksplit( meosType bboxtype,
  void (*bbox_adjust)(void *, void *), double (*bbox_penalty)(void *, void *),
  GistEntryVector *entryvec, GIST_SPLITVEC *v );

extern double tbox_penalty(void *bbox1, void *bbox2);
extern float non_negative(float val);
extern int interval_cmp_lower(const void *i1, const void *i2);
extern int interval_cmp_upper(const void *i1, const void *i2);
extern void tbox_adjust(void *bbox1, void *bbox2);
/*****************************************************************************/

#endif /* __TNUMBER_GIST__ */
