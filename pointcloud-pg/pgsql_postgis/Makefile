# pointcloud

include ../config.mk

#MODULE_big = pointcloud_postgis
#OBJS =
SED = sed
EXTENSION = pointcloud_postgis
EXTVERSION=$(shell cat ../Version.config)

UPGRADABLE = 1.1.0 1.1.1 1.2.0 1.2.1 1.2.2 1.2.3 1.2.4
UPGRADES = \
  $(shell echo $(UPGRADABLE) | \
     $(SED) 's/^/$(EXTENSION)--/' | \
     $(SED) 's/$$/--$(EXTVERSION).sql/' | \
     $(SED) 's/ /--$(EXTVERSION).sql $(EXTENSION)--/g') \
  $(EXTENSION)--$(EXTVERSION)--$(EXTVERSION)next.sql \
  $(EXTENSION)--$(EXTVERSION)next--$(EXTVERSION).sql

DATA_built = \
  $(EXTENSION).control \
  $(EXTENSION)--$(EXTVERSION).sql \
  $(UPGRADES)
#REGRESS = pointcloud

# Add in build/link flags for lib
#PG_CPPFLAGS += -I../lib
#SHLIB_LINK += $(filter -lm, $(LIBS)) $(XML2_LDFLAGS) $(ZLIB_LDFLAGS) ../lib/$(LIB_A)

# We are going to use PGXS for sure
include $(PGXS)

$(EXTENSION).control: $(EXTENSION).control.in Makefile
	$(SED) -e 's/@POINTCLOUD_VERSION@/$(EXTVERSION)/' $< > $@

$(EXTENSION)--$(EXTVERSION).sql: $(EXTENSION).sql.in Makefile
	$(SED) -e 's/@POINTCLOUD_VERSION@/$(EXTVERSION)/' $< > $@

# NOTE: relies on PERL being defined by PGXS
$(EXTENSION)--%--$(EXTVERSION).sql: $(EXTENSION)--$(EXTVERSION).sql ../util/proc_upgrade.pl
	cat $< | ../util/proc_upgrade.pl > $@

$(EXTENSION)--%--$(EXTVERSION)next.sql: $(EXTENSION)--$(EXTVERSION)next--$(EXTVERSION).sql
	ln -f $< $@
