# contrib/in_memory/Makefile

MODULE_big = in_memory

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

#HEAP_OBJS = heapam.c heapam_visibility.c hio.c pruneheap.c rewriteheap.c syncscan.c tuptoaster.c vacuumlazy.c visibilitymap.c

HEAP_OBJS = $(top_builddir)/src/backend/access/heap/*.c
SRCS = in_memoryam_handler.c $(HEAP_OBJS)
	
#HEAP_TARGS := $(patsubst $(top_builddir)/src/backend/access/heap/%.c,$(top_builddir)/src/backend/access/heap/%.o,$(HEAP_OBJS))

#HEAP_OBJS := $(top_builddir)/rc/backend/access/heap/%.o : $(top_builddir)/rc/backend/access/heap/%.c
#BUF_OBJS := $(top_builddir)/src/backend/storage/buffer/%.o : $(top_builddir)/src/backend/storage/buffer/bufmgr.c

#OBJS += $(HEAP_TARGS) $(BUF_OBJS)
	
OBJS = $(SRCS:.c=.o)

maintainer-clean:
	rm -f $(HEAP_OBJS)
	rm -f $(BUF_OBJS)
