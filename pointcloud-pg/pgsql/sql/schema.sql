-- See https://github.com/pgpointcloud/pointcloud/issues/71
set client_min_messages to ERROR;
select '#71', PC_SchemaIsValid('<xml/>'::xml::text);
