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
 * @brief General functions for temporal points
 */

#include "point/tpoint.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal.h"
#include "npoint/tnpoint_boxops.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Copy a geometry
 * @note The @p gserialized_copy function is not available anymore in
 * PostGIS 3
 */
GSERIALIZED *
geo_copy(const GSERIALIZED *g)
{
  assert(g);
  GSERIALIZED *result = palloc(VARSIZE(g));
  memcpy(result, g, VARSIZE(g));
  return result;
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the last argument initialized with the spatiotemporal box of a
 * temporal point
 * @param[in] temp Temporal point
 * @param[out] box Temporal box
 */
void
tspatial_set_stbox(const Temporal *temp, STBox *box)
{
  assert(temp); assert(box); assert(tspatial_type(temp->temptype));
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      if (tgeo_type(temp->temptype))
        tpointinst_set_stbox((TInstant *) temp, box);
#if NPOINT
      else if (temp->temptype == T_TNPOINT)
        tnpointinst_set_stbox((TInstant *) temp, box);
#endif
      else
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "Unknown temporal point type: %u", temp->temptype);
      break;
    case TSEQUENCE:
      tspatialseq_set_stbox((TSequence *) temp, box);
      break;
    default: /* TSEQUENCESET */
      tspatialseqset_set_stbox((TSequenceSet *) temp, box);
  }
  return;
}

/**
 * @ingroup meos_box_conversion
 * @brief Return a temporal point converted to a spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #Tpoint_to_stbox()
 */
STBox *
tpoint_to_stbox(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_tspatial_type(temp->temptype))
    return NULL;
  STBox *result = palloc(sizeof(STBox));
  tspatial_set_stbox(temp, result);
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_spatial_transf
 * @brief Return the bounding box of a geometry expanded on the spatial
 * dimension
 * @param[in] gs Geometry
 * @param[in] d Value
 * @csqlfn #Geo_expand_space()
 */
STBox *
geo_expand_space(const GSERIALIZED *gs, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs) || gserialized_is_empty(gs))
    return NULL;

  STBox box;
  geo_set_stbox(gs, &box);
  return stbox_expand_space(&box, d);
}

/**
 * @ingroup meos_temporal_spatial_transf
 * @brief Return the bounding box of a temporal point expanded on the spatial
 * dimension
 * @param[in] temp Temporal point
 * @param[in] d Value
 * @return On error return @p NULL
 * @csqlfn #Tpoint_expand_space()
 */
STBox *
tpoint_expand_space(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      /* This function is also called for tnpoint */
      ! ensure_tspatial_type(temp->temptype))
    return NULL;

  STBox box;
  tspatial_set_stbox(temp, &box);
  return stbox_expand_space(&box, d);
}

/*****************************************************************************/
