#include <postgres.h>

void
h3_assert(int error)
{
	if (error)
		ereport(ERROR, (
						errcode(ERRCODE_EXTERNAL_ROUTINE_EXCEPTION),
						errmsg("error code: %i", error),
						errhint("https://h3geo.org/docs/library/errors#table-of-error-codes")));
}
