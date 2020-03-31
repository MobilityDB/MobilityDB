static Datum
tpoints_to_geo_measure(TemporalS *ts, TemporalS *measure)
{
	/* Instantaneous sequence */
	if (ts->count == 1)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, 0);
		TemporalSeq *seq2 = temporals_seq_n(measure, 0);
		return tpointseq_to_geo_measure(seq1, seq2);
	}

	uint8_t colltype = 0;
	LWGEOM **geoms = palloc(sizeof(LWGEOM *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		TemporalSeq *m = temporals_seq_n(measure, i);
		geoms[i] = tpointseq_to_geo_measure1(seq, m);
		/* Output type not initialized */
		if (! colltype)
			colltype = lwtype_get_collectiontype(geoms[i]->type);
			/* Input type not compatible with output */
			/* make output type a collection */
		else if (colltype != COLLECTIONTYPE &&
				 lwtype_get_collectiontype(geoms[i]->type) != colltype)
			colltype = COLLECTIONTYPE;
	}
	// TODO add the bounding box instead of ask PostGIS to compute it again
	// GBOX *box = stbox_to_gbox(temporalseq_bbox_ptr(seq));
	LWGEOM *coll = (LWGEOM *) lwcollection_construct(colltype,
													 geoms[0]->srid, NULL, (uint32_t) ts->count, geoms);
	Datum result = PointerGetDatum(geometry_serialize(coll));
	/* We cannot lwgeom_free(geoms[i] or lwgeom_free(coll) */
	pfree(geoms);
	return result;
}
