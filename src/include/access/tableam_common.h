/*-------------------------------------------------------------------------
 *
 * tableam_common.h
 *	  POSTGRES table access method definitions shared across
 *	  all pluggable table access methods and server.
 *
 *
 * Portions Copyright (c) 1996-2017, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/access/tableam_common.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef TABLEAM_COMMON_H
#define TABLEAM_COMMON_H

#include "postgres.h"

#include "access/htup_details.h"
#include "access/transam.h"
#include "access/xact.h"
#include "access/xlog.h"
#include "storage/bufpage.h"
#include "storage/bufmgr.h"


/* A physical tuple coming from a table AM scan */
typedef void *TableTuple;

/* Result codes for HeapTupleSatisfiesVacuum */
typedef enum
{
	HEAPTUPLE_DEAD,				/* tuple is dead and deletable */
	HEAPTUPLE_LIVE,				/* tuple is live (committed, no deleter) */
	HEAPTUPLE_RECENTLY_DEAD,	/* tuple is dead, but not deletable yet */
	HEAPTUPLE_INSERT_IN_PROGRESS,	/* inserting xact is still in progress */
	HEAPTUPLE_DELETE_IN_PROGRESS	/* deleting xact is still in progress */
} HTSV_Result;

#endif							/* TABLEAM_COMMON_H */
