/**
 * PointN(GEOMETRY,INTEGER) -- find the first linestring in GEOMETRY,
 * @return the point at index INTEGER (1 is 1st point).  Return NULL if
 *     there is no LINESTRING(..) in GEOMETRY or INTEGER is out of bounds.
 */
GSERIALIZED *
PGIS_LWGEOM_pointn_linestring(GSERIALIZED *geom, int where)
{
  LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
  LWPOINT *lwpoint = NULL;
  int type = lwgeom->type;

  /* If index is negative, count backward */
  if( where < 1 )
  {
    int count = -1;
    if ( type == LINETYPE || type == CIRCSTRINGTYPE || type == COMPOUNDTYPE )
      count = lwgeom_count_vertices(lwgeom);
    if(count >0)
    {
      /* only work if we found the total point number */
      /* converting where to positive backward indexing, +1 because 1 indexing */
      where = where + count + 1;
    }
    if (where < 1)
      PG_RETURN_NULL();
  }

  if ( type == LINETYPE || type == CIRCSTRINGTYPE )
  {
    /* OGC index starts at one, so we substract first. */
    lwpoint = lwline_get_lwpoint((LWLINE*)lwgeom, where - 1);
  }
  else if ( type == COMPOUNDTYPE )
  {
    lwpoint = lwcompound_get_lwpoint((LWCOMPOUND*)lwgeom, where - 1);
  }

  lwgeom_free(lwgeom);
  PG_FREE_IF_COPY(geom, 0);

  if ( ! lwpoint )
    PG_RETURN_NULL();

  return lwpoint_as_lwgeom(lwpoint);
}
