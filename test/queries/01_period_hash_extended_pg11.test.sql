SELECT period_hash_extended('[2000-01-01,2000-01-02]') = period_hash_extended('[2000-01-01,2000-01-02]');
SELECT period_hash_extended('[2000-01-01,2000-01-02]') <> period_hash_extended('[2000-01-02,2000-01-02]');
SELECT count(*) FROM tbl_period WHERE period_hash_extended(p)=period_hash_extended(p);