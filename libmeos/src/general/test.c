/**
 * @ingroup libmeos_temporal_transf
 * @brief Shift and/or scale the time span of the timestamp set value by the
 * two intervals.
 *
 * @pre The duration is greater than 0 if it is not NULL
 */
TimestampSet *
timestampset_shift_tscale(const TimestampSet *ts, const Interval *start,
  const Interval *duration)
{
  assert(start != NULL || duration != NULL);
  TimestampSet *result = timestampset_copy(seq);
  /* Shift and/or scale the bounding period */
  double orig_duration = (double) (ts->period.upper - ts->period.lower);
  period_shift_tscale(&result->period, start, duration);
  double new_duration = (double) (result->period.upper - result->period.lower);

  /* Set the first timestamp */
  result->elems[0] = result->period.lower;
  if (ts->count > 1)
  {
    /* Shift and/or scale from the second to the penultimate timestamp */
    for (int i = 1; i < ts->count - 1; i++)
    {
      TimestampTz t = result->elems[i];
      double fraction = (double) (t - ts->period.lower) / orig_duration;
      result->elems[i] = result->period.lower +
        (TimestampTz) (new_duration * fraction);
    }
    /* Set the last instant */
    result->elems[count - 1] = result->period.upper;
  }
  return result;
}

PG_FUNCTION_INFO_V1(Timestampset_tscale);
/**
 * Scale the time span of the temporal value by the interval
 */
PGDLLEXPORT Datum
Timestampset_tscale(PG_FUNCTION_ARGS)
{
  TimestampSet *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  TimestampSet *result = timestampset_shift_tscale(ts, NULL, duration);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_shift_tscale);
/**
 * Shift and scale the time span of the temporal value by the two intervals
 */
PGDLLEXPORT Datum
Timestampset_shift_tscale(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TEMPORAL_P(0);
  Interval *start = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  TimestampSet *result = timestampset_shift_tscale(ts, start, duration);
  PG_RETURN_POINTER(result);
}

