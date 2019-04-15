# contrib/global_temp/Makefile

MODULE_big = global_temp
OBJS = bufmgr.o heapam.o heapam_visibility.o hio.o global_tempam_handler.o localbuf.o \
	pruneheap.o rewriteheap.o syncscan.o tuptoaster.o vacuumlazy.o visibilitymap.o $(WIN32RES)

EXTENSION = global_temp
DATA = global_temp--1.0.sql
PGFILEDESC = "global_temp - provides global temporary table support"

LDFLAGS_SL += $(filter -lm, $(LIBS))

REGRESS_OPTS = --temp-config $(top_srcdir)/contrib/global_temp/global_temp.conf
REGRESS = global_temp
# Disabled because these tests require "shared_preload_libraries=global_temp",
# which typical installcheck users do not have (e.g. buildfarm clients).
NO_INSTALLCHECK = 1

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/global_temp
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
