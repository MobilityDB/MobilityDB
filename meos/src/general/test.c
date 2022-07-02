





LWPOINT *
lwpointarr_remove_duplicates(LWPOINT **lwpoints, int count)
{
  if (count == 1)
    return lwpoint_clone(lwpoints[0]);

  const LWPOINT **newpoints = palloc(sizeof(LWPOINT *) * count);
  memcpy(newpoints, points, sizeof(LWPOINT *) * count);
  lwpointarr_sort(newpoints, count);
  int newcount = lwpointarr_remove_duplicates(newpoints, count);
  if (FLAGS_GET_Z(lwpoints[0]->flags))
  {
    
  }
  else
  {
    
  }
  LWGEOM *result =  ?
    (LWGEOM *) lwline_from_lwgeom_array(lwpoints[0]->srid, (uint32_t) count,
      lwpoints) :
    (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE, lwpoints[0]->srid,
      NULL, (uint32_t) count, lwpoints);
  FLAGS_SET_Z(result->flags, FLAGS_GET_Z(lwpoints[0]->flags));
  FLAGS_SET_GEODETIC(result->flags, FLAGS_GET_GEODETIC(lwpoints[0]->flags));
  return result;
}