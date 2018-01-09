/*---------------------------------------------------------------------
 *
 * tableamapi.h
 *		API for Postgres table access methods
 *
 * Copyright (c) 2017, PostgreSQL Global Development Group
 *
 * src/include/access/tableamapi.h
 *---------------------------------------------------------------------
 */
#ifndef TABLEEAMAPI_H
#define TABLEEAMAPI_H

#include "access/tableam_common.h"
#include "nodes/nodes.h"
#include "fmgr.h"
#include "utils/snapshot.h"


/*
 * Storage routine function hooks
 */
typedef bool (*SnapshotSatisfies_function) (TableTuple htup, Snapshot snapshot, Buffer buffer);
typedef HTSU_Result (*SnapshotSatisfiesUpdate_function) (TableTuple htup, CommandId curcid, Buffer buffer);
typedef HTSV_Result (*SnapshotSatisfiesVacuum_function) (TableTuple htup, TransactionId OldestXmin, Buffer buffer);

/*
 * API struct for a table AM.  Note this must be stored in a single palloc'd
 * chunk of memory.
 *
 * XXX currently all functions are together in a single struct.  Would it be
 * worthwhile to split the slot-accessor functions to a different struct?
 * That way, MinimalTuple could be handled without a complete TableAmRoutine
 * for them -- it'd only have a few functions in TupleTableSlotAmRoutine or so.
 */
typedef struct TableAmRoutine
{
	NodeTag		type;

	SnapshotSatisfies_function snapshot_satisfies;
	SnapshotSatisfiesUpdate_function snapshot_satisfiesUpdate;	/* HeapTupleSatisfiesUpdate */
	SnapshotSatisfiesVacuum_function snapshot_satisfiesVacuum;	/* HeapTupleSatisfiesVacuum */

}			TableAmRoutine;

extern TableAmRoutine * GetTableAmRoutine(Oid amhandler);
extern TableAmRoutine * GetTableAmRoutineByAmId(Oid amoid);
extern TableAmRoutine * GetHeapamTableAmRoutine(void);

#endif							/* TABLEEAMAPI_H */
