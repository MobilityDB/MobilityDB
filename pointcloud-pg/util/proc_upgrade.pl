# Copyright (C) 2015 Boundless

#!/usr/bin/perl -w

eval "exec perl -w $0 $@"
	if (0);

local $/;
local $sql = <STDIN>;
$sql =~ s/\nCREATE TYPE[^;]*;//gs;
$sql =~ s/\nCREATE AGGREGATE[^;]*;//gs;
$sql =~ s/\nCREATE CAST[^;]*;//gs;

print $sql;
