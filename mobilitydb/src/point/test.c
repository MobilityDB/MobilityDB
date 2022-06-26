Datum LWGEOM_line_locate_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(LWGEOM_line_locate_point);
Datum LWGEOM_line_locate_point(PG_FUNCTION_ARGS)
{
	GSERIALIZED *geom1 = PG_GETARG_GSERIALIZED_P(0);
	GSERIALIZED *geom2 = PG_GETARG_GSERIALIZED_P(1);
	LWLINE *lwline;
	LWPOINT *lwpoint;
	POINTARRAY *pa;
	POINT4D p, p_proj;
	double ret;

	if ( gserialized_get_type(geom1) != LINETYPE )
	{
		elog(ERROR,"line_locate_point: 1st arg isn't a line");
		PG_RETURN_NULL();
	}
	if ( gserialized_get_type(geom2) != POINTTYPE )
	{
		elog(ERROR,"line_locate_point: 2st arg isn't a point");
		PG_RETURN_NULL();
	}

	gserialized_error_if_srid_mismatch(geom1, geom2, __func__);

	lwline = lwgeom_as_lwline(lwgeom_from_gserialized(geom1));
	lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(geom2));

	pa = lwline->points;
	lwpoint_getPoint4d_p(lwpoint, &p);

	ret = ptarray_locate_point(pa, &p, NULL, &p_proj);

	PG_RETURN_FLOAT8(ret);
}/*
** geography_azimuth(GSERIALIZED *g1, GSERIALIZED *g2)
** returns direction between points (north = 0)
** azimuth (bearing) and distance
*/
Datum
POSTGIS_geography_azimuth(const GSERIALIZED *g1, const GSERIALIZED *g2)
{
  double azimuth;
  uint32_t type1, type2;

  /* Only return for points. */
  type1 = gserialized_get_type(g1);
  type2 = gserialized_get_type(g2);
  if ( type1 != POINTTYPE || type2 != POINTTYPE )
  {
    elog(ERROR, "ST_Azimuth(geography, geography) is only valid for point inputs");
    PG_RETURN_NULL();
  }

  lwgeom1 = lwgeom_from_gserialized(g1);
  lwgeom2 = lwgeom_from_gserialized(g2);

  /* EMPTY things cannot be used */
  if ( lwgeom_is_empty(lwgeom1) || lwgeom_is_empty(lwgeom2) )
  {
    lwgeom_free(lwgeom1);
    lwgeom_free(lwgeom2);
    elog(ERROR, "ST_Azimuth(geography, geography) cannot work with empty points");
    PG_RETURN_NULL();
  }

  /* Initialize spheroid */
  /* We currently cannot use the following statement since PROJ4 API is not
   * available directly to MobilityDB. */
  // spheroid_init_from_srid(gserialized_get_srid(g1), &s);
  SPHEROID s;
  spheroid_init(&s, WGS84_MAJOR_AXIS, WGS84_MINOR_AXIS);

  /* Calculate the direction */
  azimuth = lwgeom_azumith_spheroid(lwgeom_as_lwpoint(lwgeom1), lwgeom_as_lwpoint(lwgeom2), &s);

  /* Clean up */
  lwgeom_free(lwgeom1);
  lwgeom_free(lwgeom2);

  /* Return NULL for unknown (same point) azimuth */
  if( isnan(azimuth) )
  {
    PG_RETURN_NULL();
  }

  PG_RETURN_FLOAT8(azimuth);
}
