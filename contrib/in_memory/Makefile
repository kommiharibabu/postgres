# contrib/in_memory/Makefile

MODULE_big = in_memory

EXTRA_SRCS = heapam.c heapam_visibility.c hio.c pruneheap.c rewriteheap.c \
	     syncscan.c tuptoaster.c vacuumlazy.c visibilitymap.c bufmgr.c buf_table.c \
	     localbuf.c
EXTRA_OBJS := $(patsubst %.c,%.o,$(EXTRA_SRCS)) 

OBJS = in_memoryam_handler.o $(EXTRA_OBJS)

EXTENSION = in_memory
DATA = in_memory--1.0.sql
PGFILEDESC = "in_memory - provides in_memory table support"

LDFLAGS_SL += -Bsymbolic-functions $(filter -lm, $(LIBS))

REGRESS_OPTS = --temp-config $(top_srcdir)/contrib/in_memory/in_memory.conf
REGRESS = in_memory
# Disabled because these tests require "shared_preload_libraries=in_memory",
# which typical installcheck users do not have (e.g. buildfarm clients).
NO_INSTALLCHECK = 1

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/in_memory
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif

	
maintainer-clean:
	rm -f $(EXTRA_OBJS)
	rm -f $(EXTRA_SRCS)
