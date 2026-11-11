-- Position operators on a ts2cell. They read the bounding box, so a cell over
-- Brussels sits east of and above a cell over San Francisco, and a value at an
-- earlier time is before one at a later time.

-------------------------------------------------------------------------------
-- Space
-------------------------------------------------------------------------------

SELECT ts2cell '54b5c9@2001-01-01' << ts2cell '47c3c3@2001-01-01';
SELECT ts2cell '47c3c3@2001-01-01' >> ts2cell '54b5c9@2001-01-01';
SELECT ts2cell '47c3c3@2001-01-01' |>> ts2cell '54b5c9@2001-01-01';
SELECT ts2cell '54b5c9@2001-01-01' <<| ts2cell '47c3c3@2001-01-01';
SELECT ts2cell '47c3c3@2001-01-01' &< ts2cell '47c3c3@2001-01-01';

-------------------------------------------------------------------------------
-- Time
-------------------------------------------------------------------------------

SELECT ts2cell '47c3c3@2001-01-01' <<# ts2cell '54b5c9@2001-01-02';
SELECT ts2cell '54b5c9@2001-01-02' #>> ts2cell '47c3c3@2001-01-01';
SELECT ts2cell '47c3c3@2001-01-01' &<# ts2cell '47c3c3@2001-01-01';
SELECT ts2cell '47c3c3@2001-01-01' <<# tstzspan '[2001-01-02, 2001-01-03]';

-------------------------------------------------------------------------------
