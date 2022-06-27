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
 * @file tpoint.c
 * @brief General functions for temporal points.
 */

#include "point/tpoint.h"

/* PostgreSQL */
// #include <utils/timestamp.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporaltypes.h"
#include "general/lifting.h"
#include "general/temporal_compops.h"
#include "point/tpoint_parser.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_spatialfuncs.h"

/* PostgreSQL removed pg_atoi in version 15 */
#if !MEOS && POSTGRESQL_VERSION_NUMBER >= 150000
/*
 * pg_atoi: convert string to integer
 *
 * allows any number of leading or trailing whitespace characters.
 *
 * 'size' is the sizeof() the desired integral result (1, 2, or 4 bytes).
 *
 * c, if not 0, is a terminator character that may appear after the
 * integer (plus whitespace).  If 0, the string must end after the integer.
 *
 * Unlike plain atoi(), this will throw elog() upon bad input format or
 * overflow.
 */
int32
pg_atoi(const char *s, int size, int c)
{
  long    l;
  char     *badp;

  /*
   * Some versions of strtol treat the empty string as an error, but some
   * seem not to.  Make an explicit test to be sure we catch it.
   */
  if (s == NULL)
    elog(ERROR, "NULL pointer");
  if (*s == 0)
    elog(ERROR, "invalid input syntax for type %s: \"%s\"", "integer", s);

  errno = 0;
  l = strtol(s, &badp, 10);

  /* We made no progress parsing the string, so bail out */
  if (s == badp)
    elog(ERROR, "invalid input syntax for type %s: \"%s\"", "integer", s);

  switch (size)
  {
    case sizeof(int32):
      if (errno == ERANGE
#if defined(HAVE_LONG_INT_64)
      /* won't get ERANGE on these with 64-bit longs... */
        || l < INT_MIN || l > INT_MAX
#endif
        )
        elog(ERROR, "value \"%s\" is out of range for type %s", s, "integer");
      break;
    case sizeof(int16):
      if (errno == ERANGE || l < SHRT_MIN || l > SHRT_MAX)
        elog(ERROR, "value \"%s\" is out of range for type %s", s, "smallint");
      break;
    case sizeof(int8):
      if (errno == ERANGE || l < SCHAR_MIN || l > SCHAR_MAX)
        elog(ERROR, "value \"%s\" is out of range for 8-bit integer", s);
      break;
    default:
      elog(ERROR, "unsupported result size: %d", size);
  }

  /*
   * Skip any trailing whitespace; if anything but whitespace remains before
   * the terminating character, bail out
   */
  while (*badp && *badp != c && isspace((unsigned char) *badp))
    badp++;

  if (*badp && *badp != c)
    elog(ERROR, "invalid input syntax for type %s: \"%s\"", "integer", s);

  return (int32) l;
}
#else
  /* To avoid including <utils/builtins.h> */
  extern int32 pg_atoi(const char *s, int size, int c);
#endif

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * Copy a GSERIALIZED. This function is not available anymore in PostGIS 3
 */
GSERIALIZED *
gserialized_copy(const GSERIALIZED *g)
{
  GSERIALIZED *result = palloc(VARSIZE(g));
  memcpy(result, g, VARSIZE(g));
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_box_cast
 * @brief Return the bounding box of a temporal point
 * @sqlfunc stbox()
 * @sqlop @p ::
 */
STBOX *
tpoint_to_stbox(const Temporal *temp)
{
  STBOX *result = palloc(sizeof(STBOX));
  temporal_set_bbox(temp, result);
  return result;
}

/*****************************************************************************
 * Expand functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_spatial_transf
 * @brief Return the bounding box of a temporal point expanded on the
 * spatial dimension
 * @sqlfunc expandSpatial()
 */
STBOX *
geo_expand_spatial(const GSERIALIZED *gs, double d)
{
  if (gserialized_is_empty(gs))
    return NULL;
  STBOX box;
  geo_set_stbox(gs, &box);
  STBOX *result = stbox_expand_spatial(&box, d);
  return result;
}

/**
 * @ingroup libmeos_temporal_spatial_transf
 * @brief Return the bounding box of a temporal point expanded on the
 * spatial dimension
 * @sqlfunc expandSpatial()
 */
STBOX *
tpoint_expand_spatial(const Temporal *temp, double d)
{
  STBOX box;
  temporal_set_bbox(temp, &box);
  STBOX *result = stbox_expand_spatial(&box, d);
  return result;
}

/*****************************************************************************
 * Temporal comparisons
 *****************************************************************************/

/**
 * @brief Return the temporal comparison of a temporal point and a point
 */
Temporal *
tcomp_tpoint_point(const Temporal *temp, const GSERIALIZED *gs,
  Datum (*func)(Datum, Datum, mobdbType, mobdbType), bool invert)
{
  if (gserialized_is_empty(gs))
    return NULL;
  ensure_point_type(gs);
  ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs));
  ensure_same_dimensionality_tpoint_gs(temp, gs);
  mobdbType basetype = temptype_basetype(temp->temptype);
  Temporal *result = tcomp_temporal_base(temp, PointerGetDatum(gs), basetype,
    func, invert);
  return result;
}

/*****************************************************************************/
