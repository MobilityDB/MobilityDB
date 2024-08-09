







Span *
tnumber_value_time_boxes(const Temporal *temp, Datum vsize, Datum vorigin,
  int *count) 
{
  assert(temp);

  /* Initialize state */
  int nbins;
  SpanBinState *state = tnumber_value_time_tile_init(temp, vsize, duration,
    vorigin, torigin, &nbins);
  if (! state)
    return NULL;

  Span *result = palloc(sizeof(Span) * nbins);
  /* We need to loop since atTbox may be NULL */
  int i = 0;
  while (true)
  {
    /* Stop when we have used up all the grid tiles */
    if (state->done)
    {
      pfree(state);
      break;
    }

    /* Get current tile (if any) and advance state
     * It is necessary to test if we found a tile since the previous tile
     * may be the last one set in the associated bit matrix */
    Span span;
    if (! span_bin_state_get(state, &span))
    {
      pfree(state);
      break;
    }
    span_bin_state_next(state);

    /* Restrict the temporal number to the span and compute its bounding span */
    Temporal *atspan = tnumber_at_tbox(state->temp, &span);
    if (atspan == NULL)
      continue;
    tnumber_set_span(atspan, &span);
    pfree(atspan);

    /* Copy the span to the result */
    memcpy(&result[i++], &span, sizeof(Span));
  }
  *count = i;
  return result;
}