-------------------------------------------------------------------------------
-- Tbox
-------------------------------------------------------------------------------

SELECT tbox 'TBOX((1.0, 2000-01-01), (1.0, 2000-01-02))'; -- Both X and T dimensions
SELECT tbox 'TBOX((1.0,), (1.0,))'; -- Only X dimension
SELECT tbox 'TBOX((, 2000-01-01), (, 2000-01-02))'; -- Only T dimension

SELECT tbox 'TBOX((2,2000-01-02),(1,2000-01-01))';

/* Errors */
SELECT tbox 'STBOX(1, 2000-01-02)';
SELECT tbox 'TBOX(1, 2000-01-02)';
SELECT tbox 'TBOX((AA, 2000-01-02))';
SELECT tbox 'TBOX((1, AA))';
SELECT tbox 'TBOX((1, 2000-01-01, 2))';
SELECT tbox 'TBOX((1, 2000-01-01),2, 2000-01-02))';
SELECT tbox 'TBOX((1, 2000-01-01),(AA, 2000-01-02))';
SELECT tbox 'TBOX((1, 2000-01-01),(2, AA))';
SELECT tbox 'TBOX((1, 2000-01-01),(2, 2000-01-02)';

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT tbox(1,'2000-01-01',2,'2000-01-02');
SELECT tbox(2,'2000-01-02',1,'2000-01-01');
SELECT tbox(1,2);
SELECT tbox(2,1);
SELECT tboxt('2000-01-01','2000-01-02');
SELECT tboxt('2000-01-02','2000-01-01');

-------------------------------------------------------------------------------
-- Accessor functions
-------------------------------------------------------------------------------

SELECT Xmin(tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))');
SELECT Xmax(tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))');
SELECT Tmin(tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))');
SELECT Tmax(tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))');

SELECT Xmin(tbox 'TBOX((1.0,), (2.0, ))');
SELECT Xmax(tbox 'TBOX((1.0,), (2.0, ))');
SELECT Tmin(tbox 'TBOX((1.0,), (2.0, ))');
SELECT Tmax(tbox 'TBOX((1.0,), (2.0, ))');

SELECT Xmin(tbox 'TBOX((, 2000-01-01), (, 2000-01-02))');
SELECT Xmax(tbox 'TBOX((, 2000-01-01), (, 2000-01-02))');
SELECT Tmin(tbox 'TBOX((, 2000-01-01), (, 2000-01-02))');
SELECT Tmax(tbox 'TBOX((, 2000-01-01), (, 2000-01-02))');

-------------------------------------------------------------------------------
-- Casting
-------------------------------------------------------------------------------

SELECT tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))'::floatrange;
SELECT tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))'::period;
SELECT tbox 'TBOX((1.0,), (2.0, ))'::floatrange;
SELECT tbox 'TBOX((1.0,), (2.0, ))'::period;
SELECT tbox 'TBOX((, 2000-01-01), (, 2000-01-02))'::floatrange;
SELECT tbox 'TBOX((, 2000-01-01), (, 2000-01-02))'::period;

-------------------------------------------------------------------------------
-- Operators
-------------------------------------------------------------------------------

SELECT tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))' * tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))';
SELECT tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))' * tbox 'TBOX((1.0,), (2.0,))';
SELECT tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))' * tbox 'TBOX((, 2000-01-01), (, 2000-01-02))';

SELECT tbox 'TBOX((1.0,), (2.0,))' * tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))';
SELECT tbox 'TBOX((1.0,), (2.0,))' * tbox 'TBOX((1.0,), (2.0,))';
SELECT tbox 'TBOX((1.0,), (2.0,))' * tbox 'TBOX((, 2000-01-01), (, 2000-01-02))';

SELECT tbox 'TBOX((, 2000-01-01), (, 2000-01-02))' * tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))';
SELECT tbox 'TBOX((, 2000-01-01), (, 2000-01-02))' * tbox 'TBOX((1.0,), (2.0,))';
SELECT tbox 'TBOX((, 2000-01-01), (, 2000-01-02))' * tbox 'TBOX((, 2000-01-01), (, 2000-01-02))';

SELECT tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))' * tbox 'TBOX((11.0, 2000-01-01), (12.0, 2000-01-02))';
SELECT tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))' * tbox 'TBOX((1.0, 2000-02-01), (2.0, 2000-02-02))';

-------------------------------------------------------------------------------
-- Comparison functions
-------------------------------------------------------------------------------

SELECT tbox_cmp(tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))', tbox 'TBOX((2.0, 2000-01-02), (1.0, 2000-01-02))'); 
SELECT tbox_cmp(tbox 'TBOX((2.0, 2000-01-02), (1.0, 2000-01-02))', tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))'); 
SELECT tbox_cmp(tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))', tbox 'TBOX((1.0, 2000-01-03), (1.0, 2000-01-02))'); 
SELECT tbox_cmp(tbox 'TBOX((1.0, 2000-01-03), (1.0, 2000-01-02))', tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))'); 
SELECT tbox_cmp(tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))', tbox 'TBOX((1.0, 2000-01-02), (2.0, 2000-01-02))'); 
SELECT tbox_cmp(tbox 'TBOX((1.0, 2000-01-02), (2.0, 2000-01-02))', tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))'); 
SELECT tbox_cmp(tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))', tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-03))'); 
SELECT tbox_cmp(tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-03))', tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))'); 

SELECT tbox_cmp(tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))', tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))'); 

SELECT tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))' = floatrange '[1, 2]'::tbox; 

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b = t2.b;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b <> t2.b;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b < t2.b;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b <= t2.b;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b > t2.b;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 where t1.b >= t2.b;

SELECT count(*) FROM tbl_tfloat WHERE temp::tbox IS NOT NULL;
SELECT count(*) FROM tbl_tfloat WHERE temp::tbox IS NOT NULL;

-------------------------------------------------------------------------------
