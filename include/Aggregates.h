#ifndef MOBILITYDB_AGGREGATES_H
#define MOBILITYDB_AGGREGATES_H

extern Temporal *temporal_transform_tcount(Temporal *temp);

extern TemporalInst *tnumberinst_transform_tavg(TemporalInst *inst);
extern TemporalInst **tnumberi_transform_tavg(TemporalI *ti);
extern int tnumberseq_transform_tavg(TemporalSeq **result, TemporalSeq *seq);
extern int tnumbers_transform_tavg(TemporalSeq **result, TemporalS *ts);

extern TemporalI *temporalinst_tavg_finalfn(TemporalInst **instants, int count);
extern TemporalS *temporalseq_tavg_finalfn(TemporalSeq **sequences, int count);

extern TemporalInst **
temporalinst_tagg2(TemporalInst **instants1, int count1, TemporalInst **instants2, 
	int count2, Datum (*func)(Datum, Datum), int *newcount);
extern TemporalSeq **
temporalseq_tagg2(TemporalSeq **sequences1, int count1, TemporalSeq **sequences2,
   int count2, Datum (*func)(Datum, Datum), bool crossings, int *newcount);

#endif