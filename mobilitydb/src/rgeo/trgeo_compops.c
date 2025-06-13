/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @file
 * @brief Ever/always and temporal comparisons for temporal poses
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <funcapi.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_rgeo.h>
#include "temporal/temporal.h"
#include "temporal/type_util.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"
#include "pg_geo/postgis.h"
#include "pg_geo/tspatial.h"

/*****************************************************************************
 * Ever/always comparison functions
 *****************************************************************************/

PGDLLEXPORT Datum Ever_eq_geo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_geo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_comp_ever
 * @brief Return true if a temporal rigid geometry is ever equal to a geometry
 * @sqlfn ever_eq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_geo_trgeo(PG_FUNCTION_ARGS)
{
  return EAcomp_geo_tspatial(fcinfo, &ever_eq_geo_trgeo);
}

PGDLLEXPORT Datum Always_eq_geo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_geo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_comp_ever
 * @brief Return true if a temporal rigid geometry is always equal to a
 * geometry
 * @sqlfn always_eq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_geo_trgeo(PG_FUNCTION_ARGS)
{
  return EAcomp_geo_tspatial(fcinfo, &always_eq_geo_trgeo);
}

PGDLLEXPORT Datum Ever_ne_geo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_geo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_comp_ever
 * @brief Return true if a temporal rigid geometry is ever different from a
 * geometry
 * @sqlfn ever_ne()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_geo_trgeo(PG_FUNCTION_ARGS)
{
  return EAcomp_geo_tspatial(fcinfo, &ever_ne_geo_trgeo);
}

PGDLLEXPORT Datum Always_ne_geo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_geo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_comp_ever
 * @brief Return true if a temporal rigid geometry is always different from a
 * geometry
 * @sqlfn always_ne()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_geo_trgeo(PG_FUNCTION_ARGS)
{
  return EAcomp_geo_tspatial(fcinfo, &always_ne_geo_trgeo);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_trgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_trgeo_geo);
/**
 * @ingroup mobilitydb_rgeo_comp_ever
 * @brief Return true if a temporal rigid geometry is ever equal to a geometry
 * @sqlfn ever_eq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_trgeo_geo(PG_FUNCTION_ARGS)
{
  return EAcomp_tspatial_geo(fcinfo, &ever_eq_trgeo_geo);
}

PGDLLEXPORT Datum Always_eq_trgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_trgeo_geo);
/**
 * @ingroup mobilitydb_rgeo_comp_ever
 * @brief Return true if a temporal rigid geometry is always equal to a
 * geometry
 * @sqlfn always_eq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_trgeo_geo(PG_FUNCTION_ARGS)
{
  return EAcomp_tspatial_geo(fcinfo, &always_eq_trgeo_geo);
}

PGDLLEXPORT Datum Ever_ne_trgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_trgeo_geo);
/**
 * @ingroup mobilitydb_rgeo_comp_ever
 * @brief Return true if a temporal rigid geometry is ever different from a
 * geometry
 * @sqlfn ever_ne()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_trgeo_geo(PG_FUNCTION_ARGS)
{
  return EAcomp_tspatial_geo(fcinfo, &ever_ne_trgeo_geo);
}

PGDLLEXPORT Datum Always_ne_trgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_trgeo_geo);
/**
 * @ingroup mobilitydb_rgeo_comp_ever
 * @brief Return true if a temporal rigid geometry is always different from a
 * geometry
 * @sqlfn always_ne()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_trgeo_geo(PG_FUNCTION_ARGS)
{
  return EAcomp_tspatial_geo(fcinfo, &always_ne_trgeo_geo);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_trgeo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_trgeo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_comp_ever
 * @brief Return true if two temporal poses are ever equal
 * @sqlfn ever_eq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_trgeo_trgeo(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &ever_eq_trgeo_trgeo);
}

PGDLLEXPORT Datum Always_eq_trgeo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_trgeo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_comp_ever
 * @brief Return true if two temporal poses are always equal
 * @sqlfn always_eq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_trgeo_trgeo(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &always_eq_trgeo_trgeo);
}

PGDLLEXPORT Datum Ever_ne_trgeo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_trgeo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_comp_ever
 * @brief Return true if two temporal poses are ever different
 * @sqlfn ever_ne()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_trgeo_trgeo(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &ever_ne_trgeo_trgeo);
}

PGDLLEXPORT Datum Always_ne_trgeo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_trgeo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_comp_ever
 * @brief Return true if two temporal poses are always different
 * @sqlfn always_ne()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_trgeo_trgeo(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &always_ne_trgeo_trgeo);
}

/*****************************************************************************
 * Temporal comparison functions
 *****************************************************************************/

PGDLLEXPORT Datum Teq_geo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_geo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_comp_temp
 * @brief Return true if a temporal rigid geometry is ever equal to a gs
 * @sqlfn temporal_teq()
 * @sqlop @p #=
 */
inline Datum
Teq_geo_trgeo(PG_FUNCTION_ARGS)
{
  return Tcomp_geo_tspatial(fcinfo, &teq_geo_trgeo);
}

PGDLLEXPORT Datum Tne_geo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_geo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_comp_temp
 * @brief Return true if a temporal rigid geometry is ever different from a
 * geometry
 * @sqlfn temporal_tne()
 * @sqlop @p #<>
 */
inline Datum
Tne_geo_trgeo(PG_FUNCTION_ARGS)
{
  return Tcomp_geo_tspatial(fcinfo, &tne_geo_trgeo);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_trgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_trgeo_geo);
/**
 * @ingroup mobilitydb_rgeo_comp_temp
 * @brief Return true if a temporal rigid geometry is ever equal to a gs
 * @sqlfn temporal_teq()
 * @sqlop @p #=
 */
inline Datum
Teq_trgeo_geo(PG_FUNCTION_ARGS)
{
  return Tcomp_tspatial_geo(fcinfo, &teq_trgeo_geo);
}

PGDLLEXPORT Datum Tne_trgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_trgeo_geo);
/**
 * @ingroup mobilitydb_rgeo_comp_temp
 * @brief Return true if a temporal rigid geometry is ever different from a
 * geometry
 * @sqlfn temporal_tne()
 * @sqlop @p #<>
 */
inline Datum
Tne_trgeo_geo(PG_FUNCTION_ARGS)
{
  return Tcomp_tspatial_geo(fcinfo, &tne_trgeo_geo);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_trgeo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_trgeo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_comp_temp
 * @brief Return true if two temporal poses are ever equal
 * @sqlfn temporal_teq()
 * @sqlop @p #=
 */
inline Datum
Teq_trgeo_trgeo(PG_FUNCTION_ARGS)
{
  return Tcomp_temporal_temporal(fcinfo, &datum2_eq);
}

PGDLLEXPORT Datum Tne_trgeo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_trgeo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_comp_temp
 * @brief Return true if two temporal poses are ever different
 * @sqlfn temporal_tne()
 * @sqlop @p #<>
 */
inline Datum
Tne_trgeo_trgeo(PG_FUNCTION_ARGS)
{
  return Tcomp_temporal_temporal(fcinfo, &datum2_ne);
}

/*****************************************************************************/
