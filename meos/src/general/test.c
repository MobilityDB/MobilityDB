/**
 * @brief Restrict a temporal geometric point to (the complement of) an array
 * of points.
 * @sqlfunc atValues()
 */
static Temporal *
tgeogpoint_restrict_values(const Temporal *temp, GSERIALIZED **values,
  int count, bool atfunc)
{
  Datum *datumarr = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; i ++)
    datumarr[i] = PointerGetDatum(values[i]);
  Temporal *result = temporal_restrict_values(temp, datumarr, count, atfunc);
  pfree(datumarr);
  return result;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict a temporal geographic point to an array of points.
 * @sqlfunc atValues()
 */
Temporal *
tgeogpoint_at_values(const Temporal *temp, GSERIALIZED **values, int count)
{
  return tgeogpoint_restrict_values(temp, values, count, REST_AT);
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict a temporal geographic point to the complement of an array of
 * points.
 * @sqlfunc minusValues()
 */
Temporal *
tgeogpoint_minus_values(const Temporal *temp, GSERIALIZED **values, int count)
{
  return tgeogpoint_restrict_values(temp, values, count, REST_MINUS);
}

