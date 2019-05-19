/*tgeompointinst*/
--left
select * from tgeompointinst_tbl where inst << Point(50 50);
select * from tgeompointinst_tbl where inst << tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointinst_tbl where inst << tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointinst_tbl where inst << tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointinst_tbl where inst << tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--overleft
select * from tgeompointinst_tbl where inst &< Point(70 70);
select * from tgeompointinst_tbl where inst &< tgeompointinst 'Point(70 70)@2012-03-01 08:00:00';
select * from tgeompointinst_tbl where inst &< tgeompointseq 'Point(30 30)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointinst_tbl where inst &< tgeompoints '{Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(50 50)->Point(70 70)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointinst_tbl where inst &< tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--right
select * from tgeompointinst_tbl where inst >> Point(50 50);
select * from tgeompointinst_tbl where inst >> tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointinst_tbl where inst >> tgeompointseq 'Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointinst_tbl where inst >> tgeompoints '{Point(10 10)->Point(30 30)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(30 30)->Point(50 50)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointinst_tbl where inst >> tgeompointi '{Point(30 30)@2012-03-01 08:00:00, Point(50 50)@2012-05-01 08:00:00}';

--overright
select * from tgeompointinst_tbl where inst &> Point(30 30)
select * from tgeompointinst_tbl where inst &> tgeompointinst 'Point(30 30)@2012-03-01 08:00:00';
select * from tgeompointinst_tbl where inst &> tgeompointseq 'Point(30 30)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointinst_tbl where inst &> tgeompoints '{Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(50 50)->Point(70 70)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointinst_tbl where inst &> tgeompointi '{Point(30 30)@2012-03-01 08:00:00, Point(50 50)@2012-05-01 08:00:00}';

--below
select * from tgeompointinst_tbl where inst <<| Point(50 50);
select * from tgeompointinst_tbl where inst <<| tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointinst_tbl where inst <<| tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointinst_tbl where inst <<| tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointinst_tbl where inst <<| tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--overbelow
select * from tgeompointinst_tbl where inst &<| Point(70 70);
select * from tgeompointinst_tbl where inst &<| tgeompointinst 'Point(70 70)@2012-03-01 08:00:00';
select * from tgeompointinst_tbl where inst &<| tgeompointseq 'Point(30 30)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointinst_tbl where inst &<| tgeompoints '{Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(50 50)->Point(70 70)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointinst_tbl where inst &<| tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--above
select * from tgeompointinst_tbl where inst |>> Point(50 50);
select * from tgeompointinst_tbl where inst |>> tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointinst_tbl where inst |>> tgeompointseq 'Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointinst_tbl where inst |>> tgeompoints '{Point(10 10)->Point(30 30)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(30 30)->Point(50 50)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointinst_tbl where inst |>> tgeompointi '{Point(30 30)@2012-03-01 08:00:00, Point(50 50)@2012-05-01 08:00:00}';

--overabove
select * from tgeompointinst_tbl where inst |&> Point(30 30)
select * from tgeompointinst_tbl where inst |&> tgeompointinst 'Point(30 30)@2012-03-01 08:00:00';
select * from tgeompointinst_tbl where inst |&> tgeompointseq 'Point(30 30)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointinst_tbl where inst |&> tgeompoints '{Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(50 50)->Point(70 70)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointinst_tbl where inst |&> tgeompointi '{Point(30 30)@2012-03-01 08:00:00, Point(50 50)@2012-05-01 08:00:00}';

--overlaps
select * from tgeompointinst_tbl where inst && tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointinst_tbl where inst && tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointinst_tbl where inst && tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointinst_tbl where inst && tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--contains
select * from tgeompointinst_tbl where inst @> tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointinst_tbl where inst @> tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointinst_tbl where inst @> tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointinst_tbl where inst @> tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--contained by
select * from tgeompointinst_tbl where inst <@ tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointinst_tbl where inst <@ tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointinst_tbl where inst <@ tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointinst_tbl where inst <@ tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--same
select * from tgeompointinst_tbl where inst ~= tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointinst_tbl where inst ~= tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointinst_tbl where inst ~= tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointinst_tbl where inst ~= tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--before
select * from tgeompointinst_tbl where inst <<# timestamptz '2012-05-01 08:00:00';
select * from tgeompointinst_tbl where inst <<# tboolinst 'true@2012-05-01 08:00:00';
select * from tgeompointinst_tbl where inst <<# tboolseq 'true@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointinst_tbl where inst <<# tbools '{true@[2012-05-01 08:00:00, 2012-07-01 08:00:00), false@[2012-07-01 08:00:00, 2012-09-01)}';
select * from tgeompointinst_tbl where inst <<# tbooli '{true@2012-05-01 08:00:00, false@2012-07-01 08:00:00}';
select * from tgeompointinst_tbl where inst <<# tintinst '5@2012-05-01 08:00:00';
select * from tgeompointinst_tbl where inst <<# tintseq '5@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointinst_tbl where inst <<# tints '{5@[2012-05-01 08:00:00, 2012-07-01 08:00:00), 6@[2012-07-01 08:00:00, 2012-09-01)}';
select * from tgeompointinst_tbl where inst <<# tinti '{5@2012-05-01 08:00:00, 6@2012-07-01 08:00:00}';
select * from tgeompointinst_tbl where inst <<# tfloatinst '3.5@2012-05-01 08:00:00';
select * from tgeompointinst_tbl where inst <<# tfloatseq '3.5@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointinst_tbl where inst <<# tfloats '{3.5@[2012-05-01 08:00:00, 2012-07-01 08:00:00), 5.6@[2012-07-01 08:00:00, 2012-09-01)}';
select * from tgeompointinst_tbl where inst <<# tfloati '{3.5@2012-05-01 08:00:00, 5.6@2012-07-01 08:00:00}';
select * from tgeompointinst_tbl where inst <<# tgeompointinst 'Point(0 0)@2012-05-01 08:00:00';
select * from tgeompointinst_tbl where inst <<# tgeompointseq 'Point(0 0)->Point(10 10)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointinst_tbl where inst <<# tgeompoints '{Point(0 0)->Point(10 10)@[2012-05-01 08:00:00, 2012-07-01 08:00:00), Point(10 10)->Point(30 30)@[2012-07-01 08:00:00, 2012-09-01 08:00:00)}';
select * from tgeompointinst_tbl where inst <<# tgeompointi '{Point(0 0)@2012-05-01 08:00:00, Point(10 10)@2012-07-01 08:00:00}';

--overbefore
select * from tgeompointinst_tbl where inst &<# timestamptz '2012-07-01 08:00:00';
select * from tgeompointinst_tbl where inst &<# tboolinst 'true@2012-07-01 08:00:00';
select * from tgeompointinst_tbl where inst &<# tboolseq 'true@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointinst_tbl where inst &<# tbools '{true@[2012-03-01 08:00:00, 2012-05-01 08:00:00), false@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointinst_tbl where inst &<# tbooli '{true@2012-05-01 08:00:00, false@2012-07-01 08:00:00}';
select * from tgeompointinst_tbl where inst &<# tintinst '5@2012-07-01 08:00:00';
select * from tgeompointinst_tbl where inst &<# tintseq '5@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointinst_tbl where inst &<# tints '{5@[2012-03-01 08:00:00, 2012-05-01 08:00:00), 6@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointinst_tbl where inst &<# tinti '{5@2012-05-01 08:00:00, 6@2012-07-01 08:00:00}';
select * from tgeompointinst_tbl where inst &<# tfloatinst '4.5@2012-07-01 08:00:00';
select * from tgeompointinst_tbl where inst &<# tfloatseq '4.5@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointinst_tbl where inst &<# tfloats '{4.5@[2012-03-01 08:00:00, 2012-05-01 08:00:00), 5.6@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointinst_tbl where inst &<# tfloati '{4.5@2012-05-01 08:00:00, 5.6@2012-07-01 08:00:00}';
select * from tgeompointinst_tbl where inst &<# tgeompointinst 'Point(0 0)@2012-07-01 08:00:00';
select * from tgeompointinst_tbl where inst &<# tgeompointseq 'Point(0 0)->Point(10 10)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointinst_tbl where inst &<# tgeompoints '{Point(0 0)->Point(10 10)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(10 10)->Point(30 30)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointinst_tbl where inst &<# tgeompointi '{Point(0 0)@2012-05-01 08:00:00, Point(10 10)@2012-07-01 08:00:00}';

--after
select * from tgeompointinst_tbl where inst #>> timestamptz '2012-05-01 08:00:00';
select * from tgeompointinst_tbl where inst #>> tboolinst 'true@2012-05-01 08:00:00';
select * from tgeompointinst_tbl where inst #>> tboolseq 'true@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointinst_tbl where inst #>> tbools '{true@[2012-01-01, 2012-03-01 08:00:00), false@[2012-03-01 08:00:00, 2012-05-01 08:00:00)}';
select * from tgeompointinst_tbl where inst #>> tbooli '{true@2012-03-01 08:00:00, false@2012-05-01 08:00:00}';
select * from tgeompointinst_tbl where inst #>> tintinst '5@2012-05-01 08:00:00';
select * from tgeompointinst_tbl where inst #>> tintseq '5@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointinst_tbl where inst #>> tints '{5@[2012-01-01, 2012-03-01 08:00:00), 6@[2012-03-01 08:00:00, 2012-05-01 08:00:00)}';
select * from tgeompointinst_tbl where inst #>> tinti '{5@2012-03-01 08:00:00, 6@2012-05-01 08:00:00}';
select * from tgeompointinst_tbl where inst #>> tfloatinst '3.5@2012-05-01 08:00:00';
select * from tgeompointinst_tbl where inst #>> tfloatseq '3.5@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointinst_tbl where inst #>> tfloats '{3.5@[2012-01-01, 2012-03-01 08:00:00), 5.6@[2012-03-01 08:00:00, 2012-05-01 08:00:00)}';
select * from tgeompointinst_tbl where inst #>> tfloati '{3.5@2012-03-01 08:00:00, 5.6@2012-05-01 08:00:00}';
select * from tgeompointinst_tbl where inst #>> tgeompointinst 'Point(0 0)@2012-05-01 08:00:00';
select * from tgeompointinst_tbl where inst #>> tgeompointseq 'Point(0 0)->Point(10 10)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointinst_tbl where inst #>> tgeompoints '{Point(0 0)->Point(10 10)@[2012-01-01 08:00:00, 2012-03-01 08:00:00), Point(10 10)->Point(30 30)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)}';
select * from tgeompointinst_tbl where inst #>> tgeompointi '{Point(0 0)@2012-03-01 08:00:00, Point(10 10)@2012-05-01 08:00:00}';

--overafter
select * from tgeompointinst_tbl where inst #&> timestamptz '2012-03-01 08:00:00';
select * from tgeompointinst_tbl where inst #&> tboolinst 'true@2012-03-01 08:00:00';
select * from tgeompointinst_tbl where inst #&> tboolseq 'true@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointinst_tbl where inst #&> tbools '{true@[2012-03-01 08:00:00, 2012-05-01 08:00:00), false@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointinst_tbl where inst #&> tbooli '{true@2012-03-01 08:00:00, false@2012-05-01 08:00:00}';
select * from tgeompointinst_tbl where inst #&> tintinst '5@2012-03-01 08:00:00';
select * from tgeompointinst_tbl where inst #&> tintseq '5@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointinst_tbl where inst #&> tints '{5@[2012-03-01 08:00:00, 2012-05-01 08:00:00), 6@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointinst_tbl where inst #&> tinti '{5@2012-03-01 08:00:00, 6@2012-05-01 08:00:00}';
select * from tgeompointinst_tbl where inst #&> tfloatinst '3.5@2012-03-01 08:00:00';
select * from tgeompointinst_tbl where inst #&> tfloatseq '3.5@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointinst_tbl where inst #&> tfloats '{3.5@[2012-03-01 08:00:00, 2012-05-01 08:00:00), 5.6@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointinst_tbl where inst #&> tfloati '{3.5@2012-03-01 08:00:00, 5.6@2012-05-01 08:00:00}';
select * from tgeompointinst_tbl where inst #&> tgeompointinst 'Point(0 0)@2012-03-01 08:00:00';
select * from tgeompointinst_tbl where inst #&> tgeompointseq 'Point(0 0)->Point(10 10)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointinst_tbl where inst #&> tgeompoints '{Point(0 0)->Point(10 10)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(10 10)->Point(30 30)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointinst_tbl where inst #&> tgeompointi '{Point(0 0)@2012-03-01 08:00:00, Point(10 10)@2012-05-01 08:00:00}';

--distance
select * from tgeompointinst_tbl where (inst <-> tgeompointinst 'Point(50 50)@2012-03-01 08:00:00') < 10;

/*tgeompointi*/
--left
select * from tgeompointi_tbl where ti << Point(50 50);
select * from tgeompointi_tbl where ti << tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointi_tbl where ti << tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointi_tbl where ti << tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointi_tbl where ti << tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--overleft
select * from tgeompointi_tbl where ti &< Point(70 70);
select * from tgeompointi_tbl where ti &< tgeompointinst 'Point(70 70)@2012-03-01 08:00:00';
select * from tgeompointi_tbl where ti &< tgeompointseq 'Point(30 30)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointi_tbl where ti &< tgeompoints '{Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(50 50)->Point(70 70)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointi_tbl where ti &< tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--right
select * from tgeompointi_tbl where ti >> Point(50 50);
select * from tgeompointi_tbl where ti >> tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointi_tbl where ti >> tgeompointseq 'Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointi_tbl where ti >> tgeompoints '{Point(10 10)->Point(30 30)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(30 30)->Point(50 50)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointi_tbl where ti >> tgeompointi '{Point(30 30)@2012-03-01 08:00:00, Point(50 50)@2012-05-01 08:00:00}';

--overright
select * from tgeompointi_tbl where ti &> Point(30 30)
select * from tgeompointi_tbl where ti &> tgeompointinst 'Point(30 30)@2012-03-01 08:00:00';
select * from tgeompointi_tbl where ti &> tgeompointseq 'Point(30 30)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointi_tbl where ti &> tgeompoints '{Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(50 50)->Point(70 70)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointi_tbl where ti &> tgeompointi '{Point(30 30)@2012-03-01 08:00:00, Point(50 50)@2012-05-01 08:00:00}';

--below
select * from tgeompointi_tbl where ti <<| Point(50 50);
select * from tgeompointi_tbl where ti <<| tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointi_tbl where ti <<| tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointi_tbl where ti <<| tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointi_tbl where ti <<| tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--overbelow
select * from tgeompointi_tbl where ti &<| Point(70 70);
select * from tgeompointi_tbl where ti &<| tgeompointinst 'Point(70 70)@2012-03-01 08:00:00';
select * from tgeompointi_tbl where ti &<| tgeompointseq 'Point(30 30)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointi_tbl where ti &<| tgeompoints '{Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(50 50)->Point(70 70)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointi_tbl where ti &<| tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--above
select * from tgeompointi_tbl where ti |>> Point(50 50);
select * from tgeompointi_tbl where ti |>> tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointi_tbl where ti |>> tgeompointseq 'Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointi_tbl where ti |>> tgeompoints '{Point(10 10)->Point(30 30)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(30 30)->Point(50 50)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointi_tbl where ti |>> tgeompointi '{Point(30 30)@2012-03-01 08:00:00, Point(50 50)@2012-05-01 08:00:00}';

--overabove
select * from tgeompointi_tbl where ti |&> Point(30 30)
select * from tgeompointi_tbl where ti |&> tgeompointinst 'Point(30 30)@2012-03-01 08:00:00';
select * from tgeompointi_tbl where ti |&> tgeompointseq 'Point(30 30)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointi_tbl where ti |&> tgeompoints '{Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(50 50)->Point(70 70)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointi_tbl where ti |&> tgeompointi '{Point(30 30)@2012-03-01 08:00:00, Point(50 50)@2012-05-01 08:00:00}';

--overlaps
select * from tgeompointi_tbl where ti && tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointi_tbl where ti && tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointi_tbl where ti && tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointi_tbl where ti && tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--contains
select * from tgeompointi_tbl where ti @> tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointi_tbl where ti @> tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointi_tbl where ti @> tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointi_tbl where ti @> tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--contained by
select * from tgeompointi_tbl where ti <@ tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointi_tbl where ti <@ tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointi_tbl where ti <@ tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointi_tbl where ti <@ tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--same
select * from tgeompointi_tbl where ti ~= tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointi_tbl where ti ~= tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointi_tbl where ti ~= tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointi_tbl where ti ~= tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--before
select * from tgeompointi_tbl where ti <<# timestamptz '2012-05-01 08:00:00';
select * from tgeompointi_tbl where ti <<# tboolinst 'true@2012-05-01 08:00:00';
select * from tgeompointi_tbl where ti <<# tboolseq 'true@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointi_tbl where ti <<# tbools '{true@[2012-05-01 08:00:00, 2012-07-01 08:00:00), false@[2012-07-01 08:00:00, 2012-09-01)}';
select * from tgeompointi_tbl where ti <<# tbooli '{true@2012-05-01 08:00:00, false@2012-07-01 08:00:00}';
select * from tgeompointi_tbl where ti <<# tintinst '5@2012-05-01 08:00:00';
select * from tgeompointi_tbl where ti <<# tintseq '5@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointi_tbl where ti <<# tints '{5@[2012-05-01 08:00:00, 2012-07-01 08:00:00), 6@[2012-07-01 08:00:00, 2012-09-01)}';
select * from tgeompointi_tbl where ti <<# tinti '{5@2012-05-01 08:00:00, 6@2012-07-01 08:00:00}';
select * from tgeompointi_tbl where ti <<# tfloatinst '3.5@2012-05-01 08:00:00';
select * from tgeompointi_tbl where ti <<# tfloatseq '3.5@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointi_tbl where ti <<# tfloats '{3.5@[2012-05-01 08:00:00, 2012-07-01 08:00:00), 5.6@[2012-07-01 08:00:00, 2012-09-01)}';
select * from tgeompointi_tbl where ti <<# tfloati '{3.5@2012-05-01 08:00:00, 5.6@2012-07-01 08:00:00}';
select * from tgeompointi_tbl where ti <<# tgeompointinst 'Point(0 0)@2012-05-01 08:00:00';
select * from tgeompointi_tbl where ti <<# tgeompointseq 'Point(0 0)->Point(10 10)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointi_tbl where ti <<# tgeompoints '{Point(0 0)->Point(10 10)@[2012-05-01 08:00:00, 2012-07-01 08:00:00), Point(10 10)->Point(30 30)@[2012-07-01 08:00:00, 2012-09-01 08:00:00)}';
select * from tgeompointi_tbl where ti <<# tgeompointi '{Point(0 0)@2012-05-01 08:00:00, Point(10 10)@2012-07-01 08:00:00}';

--overbefore
select * from tgeompointi_tbl where ti &<# timestamptz '2012-07-01 08:00:00';
select * from tgeompointi_tbl where ti &<# tboolinst 'true@2012-07-01 08:00:00';
select * from tgeompointi_tbl where ti &<# tboolseq 'true@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointi_tbl where ti &<# tbools '{true@[2012-03-01 08:00:00, 2012-05-01 08:00:00), false@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointi_tbl where ti &<# tbooli '{true@2012-05-01 08:00:00, false@2012-07-01 08:00:00}';
select * from tgeompointi_tbl where ti &<# tintinst '5@2012-07-01 08:00:00';
select * from tgeompointi_tbl where ti &<# tintseq '5@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointi_tbl where ti &<# tints '{5@[2012-03-01 08:00:00, 2012-05-01 08:00:00), 6@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointi_tbl where ti &<# tinti '{5@2012-05-01 08:00:00, 6@2012-07-01 08:00:00}';
select * from tgeompointi_tbl where ti &<# tfloatinst '4.5@2012-07-01 08:00:00';
select * from tgeompointi_tbl where ti &<# tfloatseq '4.5@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointi_tbl where ti &<# tfloats '{4.5@[2012-03-01 08:00:00, 2012-05-01 08:00:00), 5.6@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointi_tbl where ti &<# tfloati '{4.5@2012-05-01 08:00:00, 5.6@2012-07-01 08:00:00}';
select * from tgeompointi_tbl where ti &<# tgeompointinst 'Point(0 0)@2012-07-01 08:00:00';
select * from tgeompointi_tbl where ti &<# tgeompointseq 'Point(0 0)->Point(10 10)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointi_tbl where ti &<# tgeompoints '{Point(0 0)->Point(10 10)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(10 10)->Point(30 30)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointi_tbl where ti &<# tgeompointi '{Point(0 0)@2012-05-01 08:00:00, Point(10 10)@2012-07-01 08:00:00}';

--after
select * from tgeompointi_tbl where ti #>> timestamptz '2012-05-01 08:00:00';
select * from tgeompointi_tbl where ti #>> tboolinst 'true@2012-05-01 08:00:00';
select * from tgeompointi_tbl where ti #>> tboolseq 'true@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointi_tbl where ti #>> tbools '{true@[2012-01-01, 2012-03-01 08:00:00), false@[2012-03-01 08:00:00, 2012-05-01 08:00:00)}';
select * from tgeompointi_tbl where ti #>> tbooli '{true@2012-03-01 08:00:00, false@2012-05-01 08:00:00}';
select * from tgeompointi_tbl where ti #>> tintinst '5@2012-05-01 08:00:00';
select * from tgeompointi_tbl where ti #>> tintseq '5@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointi_tbl where ti #>> tints '{5@[2012-01-01, 2012-03-01 08:00:00), 6@[2012-03-01 08:00:00, 2012-05-01 08:00:00)}';
select * from tgeompointi_tbl where ti #>> tinti '{5@2012-03-01 08:00:00, 6@2012-05-01 08:00:00}';
select * from tgeompointi_tbl where ti #>> tfloatinst '3.5@2012-05-01 08:00:00';
select * from tgeompointi_tbl where ti #>> tfloatseq '3.5@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointi_tbl where ti #>> tfloats '{3.5@[2012-01-01, 2012-03-01 08:00:00), 5.6@[2012-03-01 08:00:00, 2012-05-01 08:00:00)}';
select * from tgeompointi_tbl where ti #>> tfloati '{3.5@2012-03-01 08:00:00, 5.6@2012-05-01 08:00:00}';
select * from tgeompointi_tbl where ti #>> tgeompointinst 'Point(0 0)@2012-05-01 08:00:00';
select * from tgeompointi_tbl where ti #>> tgeompointseq 'Point(0 0)->Point(10 10)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointi_tbl where ti #>> tgeompoints '{Point(0 0)->Point(10 10)@[2012-01-01 08:00:00, 2012-03-01 08:00:00), Point(10 10)->Point(30 30)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)}';
select * from tgeompointi_tbl where ti #>> tgeompointi '{Point(0 0)@2012-03-01 08:00:00, Point(10 10)@2012-05-01 08:00:00}';

--overafter
select * from tgeompointi_tbl where ti #&> timestamptz '2012-03-01 08:00:00';
select * from tgeompointi_tbl where ti #&> tboolinst 'true@2012-03-01 08:00:00';
select * from tgeompointi_tbl where ti #&> tboolseq 'true@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointi_tbl where ti #&> tbools '{true@[2012-03-01 08:00:00, 2012-05-01 08:00:00), false@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointi_tbl where ti #&> tbooli '{true@2012-03-01 08:00:00, false@2012-05-01 08:00:00}';
select * from tgeompointi_tbl where ti #&> tintinst '5@2012-03-01 08:00:00';
select * from tgeompointi_tbl where ti #&> tintseq '5@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointi_tbl where ti #&> tints '{5@[2012-03-01 08:00:00, 2012-05-01 08:00:00), 6@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointi_tbl where ti #&> tinti '{5@2012-03-01 08:00:00, 6@2012-05-01 08:00:00}';
select * from tgeompointi_tbl where ti #&> tfloatinst '3.5@2012-03-01 08:00:00';
select * from tgeompointi_tbl where ti #&> tfloatseq '3.5@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointi_tbl where ti #&> tfloats '{3.5@[2012-03-01 08:00:00, 2012-05-01 08:00:00), 5.6@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointi_tbl where ti #&> tfloati '{3.5@2012-03-01 08:00:00, 5.6@2012-05-01 08:00:00}';
select * from tgeompointi_tbl where ti #&> tgeompointinst 'Point(0 0)@2012-03-01 08:00:00';
select * from tgeompointi_tbl where ti #&> tgeompointseq 'Point(0 0)->Point(10 10)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointi_tbl where ti #&> tgeompoints '{Point(0 0)->Point(10 10)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(10 10)->Point(30 30)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointi_tbl where ti #&> tgeompointi '{Point(0 0)@2012-03-01 08:00:00, Point(10 10)@2012-05-01 08:00:00}';

--distance
select * from tgeompointi_tbl where (ti <-> tgeompointi '{Point(30 30)@2012-03-01 08:00:00, Point(50 50)@2012-05-01 08:00:00}') < 10;

/*tgeompointseq*/
--left
select * from tgeompointseq_tbl where per << Point(50 50);
select * from tgeompointseq_tbl where per << tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointseq_tbl where per << tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointseq_tbl where per << tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointseq_tbl where per << tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--overleft
select * from tgeompointseq_tbl where per &< Point(70 70);
select * from tgeompointseq_tbl where per &< tgeompointinst 'Point(70 70)@2012-03-01 08:00:00';
select * from tgeompointseq_tbl where per &< tgeompointseq 'Point(30 30)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointseq_tbl where per &< tgeompoints '{Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(50 50)->Point(70 70)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointseq_tbl where per &< tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--right
select * from tgeompointseq_tbl where per >> Point(50 50);
select * from tgeompointseq_tbl where per >> tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointseq_tbl where per >> tgeompointseq 'Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointseq_tbl where per >> tgeompoints '{Point(10 10)->Point(30 30)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(30 30)->Point(50 50)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointseq_tbl where per >> tgeompointi '{Point(30 30)@2012-03-01 08:00:00, Point(50 50)@2012-05-01 08:00:00}';

--overright
select * from tgeompointseq_tbl where per &> Point(30 30)
select * from tgeompointseq_tbl where per &> tgeompointinst 'Point(30 30)@2012-03-01 08:00:00';
select * from tgeompointseq_tbl where per &> tgeompointseq 'Point(30 30)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointseq_tbl where per &> tgeompoints '{Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(50 50)->Point(70 70)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointseq_tbl where per &> tgeompointi '{Point(30 30)@2012-03-01 08:00:00, Point(50 50)@2012-05-01 08:00:00}';

--below
select * from tgeompointseq_tbl where per <<| Point(50 50);
select * from tgeompointseq_tbl where per <<| tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointseq_tbl where per <<| tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointseq_tbl where per <<| tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointseq_tbl where per <<| tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--overbelow
select * from tgeompointseq_tbl where per &<| Point(70 70);
select * from tgeompointseq_tbl where per &<| tgeompointinst 'Point(70 70)@2012-03-01 08:00:00';
select * from tgeompointseq_tbl where per &<| tgeompointseq 'Point(30 30)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointseq_tbl where per &<| tgeompoints '{Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(50 50)->Point(70 70)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointseq_tbl where per &<| tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--above
select * from tgeompointseq_tbl where per |>> Point(50 50);
select * from tgeompointseq_tbl where per |>> tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointseq_tbl where per |>> tgeompointseq 'Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointseq_tbl where per |>> tgeompoints '{Point(10 10)->Point(30 30)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(30 30)->Point(50 50)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointseq_tbl where per |>> tgeompointi '{Point(30 30)@2012-03-01 08:00:00, Point(50 50)@2012-05-01 08:00:00}';

--overabove
select * from tgeompointseq_tbl where per |&> Point(30 30)
select * from tgeompointseq_tbl where per |&> tgeompointinst 'Point(30 30)@2012-03-01 08:00:00';
select * from tgeompointseq_tbl where per |&> tgeompointseq 'Point(30 30)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointseq_tbl where per |&> tgeompoints '{Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(50 50)->Point(70 70)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointseq_tbl where per |&> tgeompointi '{Point(30 30)@2012-03-01 08:00:00, Point(50 50)@2012-05-01 08:00:00}';

--overlaps
select * from tgeompointseq_tbl where per && tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointseq_tbl where per && tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointseq_tbl where per && tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointseq_tbl where per && tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--contains
select * from tgeompointseq_tbl where per @> tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointseq_tbl where per @> tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointseq_tbl where per @> tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointseq_tbl where per @> tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--contained by
select * from tgeompointseq_tbl where per <@ tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointseq_tbl where per <@ tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointseq_tbl where per <@ tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointseq_tbl where per <@ tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--same
select * from tgeompointseq_tbl where per ~= tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompointseq_tbl where per ~= tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointseq_tbl where per ~= tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointseq_tbl where per ~= tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--before
select * from tgeompointseq_tbl where per <<# timestamptz '2012-05-01 08:00:00';
select * from tgeompointseq_tbl where per <<# tboolinst 'true@2012-05-01 08:00:00';
select * from tgeompointseq_tbl where per <<# tboolseq 'true@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointseq_tbl where per <<# tbools '{true@[2012-05-01 08:00:00, 2012-07-01 08:00:00), false@[2012-07-01 08:00:00, 2012-09-01)}';
select * from tgeompointseq_tbl where per <<# tbooli '{true@2012-05-01 08:00:00, false@2012-07-01 08:00:00}';
select * from tgeompointseq_tbl where per <<# tintinst '5@2012-05-01 08:00:00';
select * from tgeompointseq_tbl where per <<# tintseq '5@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointseq_tbl where per <<# tints '{5@[2012-05-01 08:00:00, 2012-07-01 08:00:00), 6@[2012-07-01 08:00:00, 2012-09-01)}';
select * from tgeompointseq_tbl where per <<# tinti '{5@2012-05-01 08:00:00, 6@2012-07-01 08:00:00}';
select * from tgeompointseq_tbl where per <<# tfloatinst '3.5@2012-05-01 08:00:00';
select * from tgeompointseq_tbl where per <<# tfloatseq '3.5@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointseq_tbl where per <<# tfloats '{3.5@[2012-05-01 08:00:00, 2012-07-01 08:00:00), 5.6@[2012-07-01 08:00:00, 2012-09-01)}';
select * from tgeompointseq_tbl where per <<# tfloati '{3.5@2012-05-01 08:00:00, 5.6@2012-07-01 08:00:00}';
select * from tgeompointseq_tbl where per <<# tgeompointinst 'Point(0 0)@2012-05-01 08:00:00';
select * from tgeompointseq_tbl where per <<# tgeompointseq 'Point(0 0)->Point(10 10)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointseq_tbl where per <<# tgeompoints '{Point(0 0)->Point(10 10)@[2012-05-01 08:00:00, 2012-07-01 08:00:00), Point(10 10)->Point(30 30)@[2012-07-01 08:00:00, 2012-09-01 08:00:00)}';
select * from tgeompointseq_tbl where per <<# tgeompointi '{Point(0 0)@2012-05-01 08:00:00, Point(10 10)@2012-07-01 08:00:00}';

--overbefore
select * from tgeompointseq_tbl where per &<# timestamptz '2012-07-01 08:00:00';
select * from tgeompointseq_tbl where per &<# tboolinst 'true@2012-07-01 08:00:00';
select * from tgeompointseq_tbl where per &<# tboolseq 'true@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointseq_tbl where per &<# tbools '{true@[2012-03-01 08:00:00, 2012-05-01 08:00:00), false@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointseq_tbl where per &<# tbooli '{true@2012-05-01 08:00:00, false@2012-07-01 08:00:00}';
select * from tgeompointseq_tbl where per &<# tintinst '5@2012-07-01 08:00:00';
select * from tgeompointseq_tbl where per &<# tintseq '5@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointseq_tbl where per &<# tints '{5@[2012-03-01 08:00:00, 2012-05-01 08:00:00), 6@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointseq_tbl where per &<# tinti '{5@2012-05-01 08:00:00, 6@2012-07-01 08:00:00}';
select * from tgeompointseq_tbl where per &<# tfloatinst '4.5@2012-07-01 08:00:00';
select * from tgeompointseq_tbl where per &<# tfloatseq '4.5@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointseq_tbl where per &<# tfloats '{4.5@[2012-03-01 08:00:00, 2012-05-01 08:00:00), 5.6@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointseq_tbl where per &<# tfloati '{4.5@2012-05-01 08:00:00, 5.6@2012-07-01 08:00:00}';
select * from tgeompointseq_tbl where per &<# tgeompointinst 'Point(0 0)@2012-07-01 08:00:00';
select * from tgeompointseq_tbl where per &<# tgeompointseq 'Point(0 0)->Point(10 10)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompointseq_tbl where per &<# tgeompoints '{Point(0 0)->Point(10 10)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(10 10)->Point(30 30)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointseq_tbl where per &<# tgeompointi '{Point(0 0)@2012-05-01 08:00:00, Point(10 10)@2012-07-01 08:00:00}';

--after
select * from tgeompointseq_tbl where per #>> timestamptz '2012-05-01 08:00:00';
select * from tgeompointseq_tbl where per #>> tboolinst 'true@2012-05-01 08:00:00';
select * from tgeompointseq_tbl where per #>> tboolseq 'true@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointseq_tbl where per #>> tbools '{true@[2012-01-01, 2012-03-01 08:00:00), false@[2012-03-01 08:00:00, 2012-05-01 08:00:00)}';
select * from tgeompointseq_tbl where per #>> tbooli '{true@2012-03-01 08:00:00, false@2012-05-01 08:00:00}';
select * from tgeompointseq_tbl where per #>> tintinst '5@2012-05-01 08:00:00';
select * from tgeompointseq_tbl where per #>> tintseq '5@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointseq_tbl where per #>> tints '{5@[2012-01-01, 2012-03-01 08:00:00), 6@[2012-03-01 08:00:00, 2012-05-01 08:00:00)}';
select * from tgeompointseq_tbl where per #>> tinti '{5@2012-03-01 08:00:00, 6@2012-05-01 08:00:00}';
select * from tgeompointseq_tbl where per #>> tfloatinst '3.5@2012-05-01 08:00:00';
select * from tgeompointseq_tbl where per #>> tfloatseq '3.5@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointseq_tbl where per #>> tfloats '{3.5@[2012-01-01, 2012-03-01 08:00:00), 5.6@[2012-03-01 08:00:00, 2012-05-01 08:00:00)}';
select * from tgeompointseq_tbl where per #>> tfloati '{3.5@2012-03-01 08:00:00, 5.6@2012-05-01 08:00:00}';
select * from tgeompointseq_tbl where per #>> tgeompointinst 'Point(0 0)@2012-05-01 08:00:00';
select * from tgeompointseq_tbl where per #>> tgeompointseq 'Point(0 0)->Point(10 10)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointseq_tbl where per #>> tgeompoints '{Point(0 0)->Point(10 10)@[2012-01-01 08:00:00, 2012-03-01 08:00:00), Point(10 10)->Point(30 30)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)}';
select * from tgeompointseq_tbl where per #>> tgeompointi '{Point(0 0)@2012-03-01 08:00:00, Point(10 10)@2012-05-01 08:00:00}';

--overafter
select * from tgeompointseq_tbl where per #&> timestamptz '2012-03-01 08:00:00';
select * from tgeompointseq_tbl where per #&> tboolinst 'true@2012-03-01 08:00:00';
select * from tgeompointseq_tbl where per #&> tboolseq 'true@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointseq_tbl where per #&> tbools '{true@[2012-03-01 08:00:00, 2012-05-01 08:00:00), false@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointseq_tbl where per #&> tbooli '{true@2012-03-01 08:00:00, false@2012-05-01 08:00:00}';
select * from tgeompointseq_tbl where per #&> tintinst '5@2012-03-01 08:00:00';
select * from tgeompointseq_tbl where per #&> tintseq '5@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointseq_tbl where per #&> tints '{5@[2012-03-01 08:00:00, 2012-05-01 08:00:00), 6@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointseq_tbl where per #&> tinti '{5@2012-03-01 08:00:00, 6@2012-05-01 08:00:00}';
select * from tgeompointseq_tbl where per #&> tfloatinst '3.5@2012-03-01 08:00:00';
select * from tgeompointseq_tbl where per #&> tfloatseq '3.5@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointseq_tbl where per #&> tfloats '{3.5@[2012-03-01 08:00:00, 2012-05-01 08:00:00), 5.6@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointseq_tbl where per #&> tfloati '{3.5@2012-03-01 08:00:00, 5.6@2012-05-01 08:00:00}';
select * from tgeompointseq_tbl where per #&> tgeompointinst 'Point(0 0)@2012-03-01 08:00:00';
select * from tgeompointseq_tbl where per #&> tgeompointseq 'Point(0 0)->Point(10 10)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompointseq_tbl where per #&> tgeompoints '{Point(0 0)->Point(10 10)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(10 10)->Point(30 30)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompointseq_tbl where per #&> tgeompointi '{Point(0 0)@2012-03-01 08:00:00, Point(10 10)@2012-05-01 08:00:00}';

--distance
select * from tgeompointseq_tbl where (per <-> tgeompointseq 'Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)') < 10;


/*tgeompoints*/
--left
select * from tgeompoints_tbl where tp << Point(50 50);
select * from tgeompoints_tbl where tp << tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompoints_tbl where tp << tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompoints_tbl where tp << tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompoints_tbl where tp << tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--overleft
select * from tgeompoints_tbl where tp &< Point(70 70);
select * from tgeompoints_tbl where tp &< tgeompointinst 'Point(70 70)@2012-03-01 08:00:00';
select * from tgeompoints_tbl where tp &< tgeompointseq 'Point(30 30)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompoints_tbl where tp &< tgeompoints '{Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(50 50)->Point(70 70)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompoints_tbl where tp &< tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--right
select * from tgeompoints_tbl where tp >> Point(50 50);
select * from tgeompoints_tbl where tp >> tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompoints_tbl where tp >> tgeompointseq 'Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompoints_tbl where tp >> tgeompoints '{Point(10 10)->Point(30 30)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(30 30)->Point(50 50)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompoints_tbl where tp >> tgeompointi '{Point(30 30)@2012-03-01 08:00:00, Point(50 50)@2012-05-01 08:00:00}';

--overright
select * from tgeompoints_tbl where tp &> Point(30 30)
select * from tgeompoints_tbl where tp &> tgeompointinst 'Point(30 30)@2012-03-01 08:00:00';
select * from tgeompoints_tbl where tp &> tgeompointseq 'Point(30 30)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompoints_tbl where tp &> tgeompoints '{Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(50 50)->Point(70 70)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompoints_tbl where tp &> tgeompointi '{Point(30 30)@2012-03-01 08:00:00, Point(50 50)@2012-05-01 08:00:00}';

--below
select * from tgeompoints_tbl where tp <<| Point(50 50);
select * from tgeompoints_tbl where tp <<| tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompoints_tbl where tp <<| tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompoints_tbl where tp <<| tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompoints_tbl where tp <<| tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--overbelow
select * from tgeompoints_tbl where tp &<| Point(70 70);
select * from tgeompoints_tbl where tp &<| tgeompointinst 'Point(70 70)@2012-03-01 08:00:00';
select * from tgeompoints_tbl where tp &<| tgeompointseq 'Point(30 30)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompoints_tbl where tp &<| tgeompoints '{Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(50 50)->Point(70 70)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompoints_tbl where tp &<| tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--above
select * from tgeompoints_tbl where tp |>> Point(50 50);
select * from tgeompoints_tbl where tp |>> tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompoints_tbl where tp |>> tgeompointseq 'Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompoints_tbl where tp |>> tgeompoints '{Point(10 10)->Point(30 30)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(30 30)->Point(50 50)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompoints_tbl where tp |>> tgeompointi '{Point(30 30)@2012-03-01 08:00:00, Point(50 50)@2012-05-01 08:00:00}';

--overabove
select * from tgeompoints_tbl where tp |&> Point(30 30)
select * from tgeompoints_tbl where tp |&> tgeompointinst 'Point(30 30)@2012-03-01 08:00:00';
select * from tgeompoints_tbl where tp |&> tgeompointseq 'Point(30 30)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompoints_tbl where tp |&> tgeompoints '{Point(30 30)->Point(50 50)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(50 50)->Point(70 70)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompoints_tbl where tp |&> tgeompointi '{Point(30 30)@2012-03-01 08:00:00, Point(50 50)@2012-05-01 08:00:00}';

--overlaps
select * from tgeompoints_tbl where tp && tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompoints_tbl where tp && tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompoints_tbl where tp && tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompoints_tbl where tp && tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--contains
select * from tgeompoints_tbl where tp @> tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompoints_tbl where tp @> tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompoints_tbl where tp @> tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompoints_tbl where tp @> tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--contained by
select * from tgeompoints_tbl where tp <@ tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompoints_tbl where tp <@ tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompoints_tbl where tp <@ tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompoints_tbl where tp <@ tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--same
select * from tgeompoints_tbl where tp ~= tgeompointinst 'Point(50 50)@2012-03-01 08:00:00';
select * from tgeompoints_tbl where tp ~= tgeompointseq 'Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompoints_tbl where tp ~= tgeompoints '{Point(50 50)->Point(70 70)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(70 70)->Point(90 90)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompoints_tbl where tp ~= tgeompointi '{Point(50 50)@2012-03-01 08:00:00, Point(70 70)@2012-05-01 08:00:00}';

--before
select * from tgeompoints_tbl where tp <<# timestamptz '2012-05-01 08:00:00';
select * from tgeompoints_tbl where tp <<# tboolinst 'true@2012-05-01 08:00:00';
select * from tgeompoints_tbl where tp <<# tboolseq 'true@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompoints_tbl where tp <<# tbools '{true@[2012-05-01 08:00:00, 2012-07-01 08:00:00), false@[2012-07-01 08:00:00, 2012-09-01)}';
select * from tgeompoints_tbl where tp <<# tbooli '{true@2012-05-01 08:00:00, false@2012-07-01 08:00:00}';
select * from tgeompoints_tbl where tp <<# tintinst '5@2012-05-01 08:00:00';
select * from tgeompoints_tbl where tp <<# tintseq '5@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompoints_tbl where tp <<# tints '{5@[2012-05-01 08:00:00, 2012-07-01 08:00:00), 6@[2012-07-01 08:00:00, 2012-09-01)}';
select * from tgeompoints_tbl where tp <<# tinti '{5@2012-05-01 08:00:00, 6@2012-07-01 08:00:00}';
select * from tgeompoints_tbl where tp <<# tfloatinst '3.5@2012-05-01 08:00:00';
select * from tgeompoints_tbl where tp <<# tfloatseq '3.5@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompoints_tbl where tp <<# tfloats '{3.5@[2012-05-01 08:00:00, 2012-07-01 08:00:00), 5.6@[2012-07-01 08:00:00, 2012-09-01)}';
select * from tgeompoints_tbl where tp <<# tfloati '{3.5@2012-05-01 08:00:00, 5.6@2012-07-01 08:00:00}';
select * from tgeompoints_tbl where tp <<# tgeompointinst 'Point(0 0)@2012-05-01 08:00:00';
select * from tgeompoints_tbl where tp <<# tgeompointseq 'Point(0 0)->Point(10 10)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompoints_tbl where tp <<# tgeompoints '{Point(0 0)->Point(10 10)@[2012-05-01 08:00:00, 2012-07-01 08:00:00), Point(10 10)->Point(30 30)@[2012-07-01 08:00:00, 2012-09-01 08:00:00)}';
select * from tgeompoints_tbl where tp <<# tgeompointi '{Point(0 0)@2012-05-01 08:00:00, Point(10 10)@2012-07-01 08:00:00}';

--overbefore
select * from tgeompoints_tbl where tp &<# timestamptz '2012-07-01 08:00:00';
select * from tgeompoints_tbl where tp &<# tboolinst 'true@2012-07-01 08:00:00';
select * from tgeompoints_tbl where tp &<# tboolseq 'true@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompoints_tbl where tp &<# tbools '{true@[2012-03-01 08:00:00, 2012-05-01 08:00:00), false@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompoints_tbl where tp &<# tbooli '{true@2012-05-01 08:00:00, false@2012-07-01 08:00:00}';
select * from tgeompoints_tbl where tp &<# tintinst '5@2012-07-01 08:00:00';
select * from tgeompoints_tbl where tp &<# tintseq '5@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompoints_tbl where tp &<# tints '{5@[2012-03-01 08:00:00, 2012-05-01 08:00:00), 6@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompoints_tbl where tp &<# tinti '{5@2012-05-01 08:00:00, 6@2012-07-01 08:00:00}';
select * from tgeompoints_tbl where tp &<# tfloatinst '4.5@2012-07-01 08:00:00';
select * from tgeompoints_tbl where tp &<# tfloatseq '4.5@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompoints_tbl where tp &<# tfloats '{4.5@[2012-03-01 08:00:00, 2012-05-01 08:00:00), 5.6@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompoints_tbl where tp &<# tfloati '{4.5@2012-05-01 08:00:00, 5.6@2012-07-01 08:00:00}';
select * from tgeompoints_tbl where tp &<# tgeompointinst 'Point(0 0)@2012-07-01 08:00:00';
select * from tgeompoints_tbl where tp &<# tgeompointseq 'Point(0 0)->Point(10 10)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)';
select * from tgeompoints_tbl where tp &<# tgeompoints '{Point(0 0)->Point(10 10)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(10 10)->Point(30 30)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompoints_tbl where tp &<# tgeompointi '{Point(0 0)@2012-05-01 08:00:00, Point(10 10)@2012-07-01 08:00:00}';

--after
select * from tgeompoints_tbl where tp #>> timestamptz '2012-05-01 08:00:00';
select * from tgeompoints_tbl where tp #>> tboolinst 'true@2012-05-01 08:00:00';
select * from tgeompoints_tbl where tp #>> tboolseq 'true@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompoints_tbl where tp #>> tbools '{true@[2012-01-01, 2012-03-01 08:00:00), false@[2012-03-01 08:00:00, 2012-05-01 08:00:00)}';
select * from tgeompoints_tbl where tp #>> tbooli '{true@2012-03-01 08:00:00, false@2012-05-01 08:00:00}';
select * from tgeompoints_tbl where tp #>> tintinst '5@2012-05-01 08:00:00';
select * from tgeompoints_tbl where tp #>> tintseq '5@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompoints_tbl where tp #>> tints '{5@[2012-01-01, 2012-03-01 08:00:00), 6@[2012-03-01 08:00:00, 2012-05-01 08:00:00)}';
select * from tgeompoints_tbl where tp #>> tinti '{5@2012-03-01 08:00:00, 6@2012-05-01 08:00:00}';
select * from tgeompoints_tbl where tp #>> tfloatinst '3.5@2012-05-01 08:00:00';
select * from tgeompoints_tbl where tp #>> tfloatseq '3.5@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompoints_tbl where tp #>> tfloats '{3.5@[2012-01-01, 2012-03-01 08:00:00), 5.6@[2012-03-01 08:00:00, 2012-05-01 08:00:00)}';
select * from tgeompoints_tbl where tp #>> tfloati '{3.5@2012-03-01 08:00:00, 5.6@2012-05-01 08:00:00}';
select * from tgeompoints_tbl where tp #>> tgeompointinst 'Point(0 0)@2012-05-01 08:00:00';
select * from tgeompoints_tbl where tp #>> tgeompointseq 'Point(0 0)->Point(10 10)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompoints_tbl where tp #>> tgeompoints '{Point(0 0)->Point(10 10)@[2012-01-01 08:00:00, 2012-03-01 08:00:00), Point(10 10)->Point(30 30)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)}';
select * from tgeompoints_tbl where tp #>> tgeompointi '{Point(0 0)@2012-03-01 08:00:00, Point(10 10)@2012-05-01 08:00:00}';

--overafter
select * from tgeompoints_tbl where tp #&> timestamptz '2012-03-01 08:00:00';
select * from tgeompoints_tbl where tp #&> tboolinst 'true@2012-03-01 08:00:00';
select * from tgeompoints_tbl where tp #&> tboolseq 'true@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompoints_tbl where tp #&> tbools '{true@[2012-03-01 08:00:00, 2012-05-01 08:00:00), false@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompoints_tbl where tp #&> tbooli '{true@2012-03-01 08:00:00, false@2012-05-01 08:00:00}';
select * from tgeompoints_tbl where tp #&> tintinst '5@2012-03-01 08:00:00';
select * from tgeompoints_tbl where tp #&> tintseq '5@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompoints_tbl where tp #&> tints '{5@[2012-03-01 08:00:00, 2012-05-01 08:00:00), 6@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompoints_tbl where tp #&> tinti '{5@2012-03-01 08:00:00, 6@2012-05-01 08:00:00}';
select * from tgeompoints_tbl where tp #&> tfloatinst '3.5@2012-03-01 08:00:00';
select * from tgeompoints_tbl where tp #&> tfloatseq '3.5@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompoints_tbl where tp #&> tfloats '{3.5@[2012-03-01 08:00:00, 2012-05-01 08:00:00), 5.6@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompoints_tbl where tp #&> tfloati '{3.5@2012-03-01 08:00:00, 5.6@2012-05-01 08:00:00}';
select * from tgeompoints_tbl where tp #&> tgeompointinst 'Point(0 0)@2012-03-01 08:00:00';
select * from tgeompoints_tbl where tp #&> tgeompointseq 'Point(0 0)->Point(10 10)@[2012-03-01 08:00:00, 2012-05-01 08:00:00)';
select * from tgeompoints_tbl where tp #&> tgeompoints '{Point(0 0)->Point(10 10)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(10 10)->Point(30 30)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}';
select * from tgeompoints_tbl where tp #&> tgeompointi '{Point(0 0)@2012-03-01 08:00:00, Point(10 10)@2012-05-01 08:00:00}';

--distance
select * from tgeompoints_tbl where (tp <-> tgeompoints '{Point(0 0)->Point(30 30)@[2012-03-01 08:00:00, 2012-05-01 08:00:00), Point(30 30)->Point(50 50)@[2012-05-01 08:00:00, 2012-07-01 08:00:00)}') < 10;


