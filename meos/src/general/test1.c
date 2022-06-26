
 
 
 

 

 
 
/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal geometric point is always equal to a point.
 * @sqlop @p %=
 */ 
bool tgeompoint_always_eq(const Temporal *temp, GSERIALIZED *gs);
{
  return tpoint_always_eq(temp, PointerGetDatum(gs));
}
 
/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal geographic point is always equal to a point.
 * @sqlop @p %=
 */ 
bool tgeogpoint_always_eq(const Temporal *temp, GSERIALIZED *gs);
{
  return tpoint_always_eq(temp, PointerGetDatum(gs));
}
 
/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal geometric point is ever equal to a point.
 * @sqlop @p ?=
 */ 
bool tgeompoint_ever_eq(const Temporal *temp, GSERIALIZED *gs);
{
  return tpoint_ever_eq(temp, PointerGetDatum(gs));

}
 
/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal geographic point is ever equal to a point.
 * @sqlop @p ?=
 */ 
bool tgeogpoint_ever_eq(const Temporal *temp, GSERIALIZED *gs);
{
  return tpoint_ever_eq(temp, PointerGetDatum(gs));
}



 
