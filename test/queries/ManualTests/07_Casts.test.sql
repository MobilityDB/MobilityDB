-- Instants

select cast (tboolinst 'true@2001-01-01 08:00:00' as tboolseq);
select cast (tboolinst 'true@2001-01-01 08:00:00' as tboolp);
select cast (tintinst '2@2001-01-01 08:00:00' as tintseq);
select cast (tintinst '2@2001-01-01 08:00:00' as tints);
select cast (tintinst '2@2001-01-01 08:00:00' as tfloatinst);
select cast (tintinst '2@2001-01-01 08:00:00' as tfloatseq);
select cast (tintinst '2@2001-01-01 08:00:00' as tfloats);
select cast (tfloatinst '2@2001-01-01 08:00:00' as tfloatseq);
select cast (tfloatinst '2@2001-01-01 08:00:00' as tfloats);

-- seqiods

select cast (tboolseq 'true@[2001-01-01 08:00:00, 2001-01-01 08:05:00)' as tboolp);
select cast (tintseq '2@[2001-01-01 08:00:00, 2001-01-01 08:05:00)' as tints);
select cast (tintseq '2@[2001-01-01 08:00:00, 2001-01-01 08:05:00)' as tfloatseq);
select cast (tintseq '2@[2001-01-01 08:00:00, 2001-01-01 08:05:00)' as tfloats);
select cast (tfloatseq '2@[2001-01-01 08:00:00, 2001-01-01 08:05:00)' as tfloats);

-- Temporals

select cast (tints '{2@[2001-01-01 08:00:00, 2001-01-01 08:05:00), 3@[2001-01-01 08:05:00, 2001-01-01 08:07:00)}' as tfloats);
select cast (tinti '{2@2001-01-01 08:00:00, 2@2001-01-01 08:05:00, 3@2001-01-01 08:06:00}' as tfloati);
