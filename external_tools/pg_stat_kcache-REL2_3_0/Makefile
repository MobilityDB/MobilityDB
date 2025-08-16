EXTENSION    = pg_stat_kcache
EXTVERSION   = $(shell grep default_version $(EXTENSION).control | sed -e "s/default_version[[:space:]]*=[[:space:]]*'\([^']*\)'/\1/")
TESTS        = $(wildcard test/sql/*.sql)
REGRESS      = $(patsubst test/sql/%.sql,%,$(TESTS))
REGRESS_OPTS = --inputdir=test

PG_CONFIG ?= pg_config

MODULE_big = pg_stat_kcache
OBJS = pg_stat_kcache.o

all:

release-zip: all
	git archive --format zip --prefix=pg_stat_kcache-${EXTVERSION}/ --output ./pg_stat_kcache-${EXTVERSION}.zip HEAD
	unzip ./pg_stat_kcache-$(EXTVERSION).zip
	rm ./pg_stat_kcache-$(EXTVERSION).zip
	rm ./pg_stat_kcache-$(EXTVERSION)/.gitignore
	sed -i -e "s/__VERSION__/$(EXTVERSION)/g"  ./pg_stat_kcache-$(EXTVERSION)/META.json
	zip -r ./pg_stat_kcache-$(EXTVERSION).zip ./pg_stat_kcache-$(EXTVERSION)/
	rm ./pg_stat_kcache-$(EXTVERSION) -rf


DATA = $(wildcard *--*.sql)
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
