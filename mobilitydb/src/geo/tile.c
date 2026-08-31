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
 * @brief Restrict a temporal point to a tile defined by a spatiotemporal box.
 *
 * In addition to applying the atStbox() function we must remove the upper
 * bound for each dimension. The following figure shows the borders that
 * are removed (represented by x) for a 2D tile and 3D tile
 *
 * @code
 * xxxxxxxxxxxxxxxxx
 * |               x
 * |               x
 * |---------------x
 *
 *   xxxxxxxxxxxxxxxxx
 *  xxxxxxxxxxxxxxxxxx
 * xxxxxxxxxxxxxxxxxxx
 * |               xxx
 * |               xx
 * |---------------x
 * @endcode
 *
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 */
Temporal *
tpoint_at_tile(const Temporal *temp, const STBox *box)
{
  /* Call to atStbox() that includes the upper bounds for each coordinate */
  Temporal *at_stbox = tpoint_restrict_stbox(temp, box, REST_AT);
  /* Split the temporal point into temporal floats for each coordinate */
  Temporal *temp_x = tpoint_get_coord(at_stbox, 0);
  Temporal *temp_y = tpoint_get_coord(at_stbox, 1);
  Temporal *temp_z = NULL;
  bool hasz = MEOS_FLAGS_GET_Z(box->flags);
  if (hasz)
    temp_z = tpoint_get_coord(at_stbox, 2);
  /* Remove from the temporal floats the upper bound for each coordinate */
  Temporal *minus_x = temporal_restrict_value(temp_x, Float8GetDatum(box->xmax),
    REST_MINUS);
  Temporal *minus_y = temporal_restrict_value(temp_y, Float8GetDatum(box->ymax),
    REST_MINUS);
  Temporal *minus_z = NULL;
  if (hasz)
    minus_z = temporal_restrict_value(temp_z, Float8GetDatum(box->zmax),
      REST_MINUS);
  Temporal *result = NULL;
  if (minus_x && minus_y && (! hasz || minus_z))
  {
    SpanSet *ps_x = temporal_time(minus_x);
    SpanSet *ps_y = temporal_time(minus_y);
    SpanSet *ps_xy = intersection_spanset_spanset(ps_x, ps_y);
    if (ps_xy)
    {
      SpanSet *ps_z, *ps_xyz;
      if (hasz)
      {
        ps_z = temporal_time(minus_z);
        ps_xyz = intersection_spanset_spanset(ps_xy, ps_z);
      }
      else
        ps_xyz = ps_xy;
      if (ps_xyz)
      {
        result = temporal_restrict_tstzspanset(temp, ps_xyz, REST_AT);
        if (hasz)
          pfree(ps_xyz);
      }
      pfree(ps_xy);
      if (hasz)
        pfree(ps_z);
    }
    pfree(ps_x); pfree(ps_y);
  }
  pfree(temp_x); pfree(temp_y);
  if (minus_x) pfree(minus_x);
  if (minus_y) pfree(minus_y);
  if (hasz)
  {
    pfree(temp_z);
    if (minus_z) pfree(minus_z);
  }
  return result;
}
