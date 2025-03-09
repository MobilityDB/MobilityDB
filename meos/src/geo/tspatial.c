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

#include "geo/tgeo.h"

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
#if CBUFFER
  #include "cbuffer/tcbuffer_boxops.h"
#endif 
#if NPOINT
  #include "npoint/tnpoint_boxops.h"
#endif 

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_spatial_transf
 * @brief Return in the last argument the spatiotemporal box of a temporal
 * spatial value
 * @param[in] temp Temporal spatial value
 * @param[out] box Spatiotemporal box
 */
void
tspatial_set_stbox(const Temporal *temp, STBox *box)
{
  assert(temp); assert(box); assert(tspatial_type(temp->temptype));
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      if (tgeo_type_all(temp->temptype))
        tgeoinst_set_stbox((TInstant *) temp, box);
#if CBUFFER
      else if (temp->temptype == T_TCBUFFER)
        tcbufferinst_set_stbox((TInstant *) temp, box);
#endif
#if NPOINT
      else if (temp->temptype == T_TNPOINT)
        tnpointinst_set_stbox((TInstant *) temp, box);
#endif
      else
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "Unknown temporal point type: %s", meostype_name(temp->temptype));
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
 * @ingroup meos_temporal_spatial_transf
 * @brief Return a temporal spatial value converted to a spatiotemporal box
 * @param[in] temp Temporal spatial value
 * @csqlfn #Tspatial_to_stbox()
 */
STBox *
tspatial_stbox(const Temporal *temp)
{
#if MEOS
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_tspatial_type(temp->temptype))
    return NULL;
#else
  assert(temp); assert(tspatial_type(temp->temptype));
#endif /* MEOS */

  STBox *result = palloc(sizeof(STBox));
  tspatial_set_stbox(temp, result);
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_spatial_transf
 * @brief Return the bounding box of a geometry/geography expanded on the
 * spatial dimension
 * @param[in] gs Geometry/geography
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
 * @brief Return the bounding box of a temporal spatial value expanded on the
 * spatial dimension
 * @param[in] temp Temporal spatial value
 * @param[in] d Value
 * @return On error return @p NULL
 * @csqlfn #Tspatial_expand_space()
 */
STBox *
tspatial_expand_space(const Temporal *temp, double d)
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
