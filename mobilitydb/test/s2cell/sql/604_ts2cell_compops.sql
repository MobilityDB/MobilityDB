-- Ever, always and temporal comparisons between a ts2cell and a cell, and
-- between two ts2cell values.

-------------------------------------------------------------------------------
-- Ever and always
-------------------------------------------------------------------------------

SELECT eEqual(ts2cell '[47c3c3@2001-01-01, 54b5c9@2001-01-02]', s2cell '47c3c3');
SELECT aEqual(ts2cell '[47c3c3@2001-01-01, 54b5c9@2001-01-02]', s2cell '47c3c3');
SELECT aEqual(ts2cell '[47c3c3@2001-01-01, 47c3c3@2001-01-02]', s2cell '47c3c3');
SELECT eNotEqual(ts2cell '[47c3c3@2001-01-01, 54b5c9@2001-01-02]', s2cell '47c3c3');
SELECT eEqual(s2cell '54b5c9', ts2cell '[47c3c3@2001-01-01, 54b5c9@2001-01-02]');
SELECT eEqual(ts2cell '[47c3c3@2001-01-01]', ts2cell '[47c3c3@2001-01-01]');

-------------------------------------------------------------------------------
-- Temporal comparisons
-------------------------------------------------------------------------------

SELECT asText(ts2cell '[47c3c3@2001-01-01, 54b5c9@2001-01-02]' #= s2cell '47c3c3');
SELECT asText(ts2cell '[47c3c3@2001-01-01, 54b5c9@2001-01-02]' #<> s2cell '47c3c3');
SELECT asText(s2cell '47c3c3' #= ts2cell '[47c3c3@2001-01-01, 54b5c9@2001-01-02]');

-------------------------------------------------------------------------------
