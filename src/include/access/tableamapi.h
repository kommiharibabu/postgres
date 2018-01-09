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

#include "access/heapam.h"
#include "access/tableam.h"
#include "nodes/execnodes.h"
#include "nodes/nodes.h"
#include "fmgr.h"
#include "utils/snapshot.h"


/*
 * Storage routine function hooks
 */
typedef bool (*SnapshotSatisfies_function) (TableTuple htup, Snapshot snapshot, Buffer buffer);
typedef HTSU_Result (*SnapshotSatisfiesUpdate_function) (TableTuple htup, CommandId curcid, Buffer buffer);
typedef HTSV_Result (*SnapshotSatisfiesVacuum_function) (TableTuple htup, TransactionId OldestXmin, Buffer buffer);

typedef Oid (*TupleInsert_function) (Relation rel, TupleTableSlot *slot, CommandId cid,
									 int options, BulkInsertState bistate, InsertIndexTuples IndexFunc,
									 EState *estate, List *arbiterIndexes, List **recheckIndexes);

typedef HTSU_Result (*TupleDelete_function) (Relation relation,
											 ItemPointer tid,
											 CommandId cid,
											 Snapshot crosscheck,
											 bool wait,
											 DeleteIndexTuples IndexFunc,
											 HeapUpdateFailureData *hufd);

typedef HTSU_Result (*TupleUpdate_function) (Relation relation,
											 ItemPointer otid,
											 TupleTableSlot *slot,
											 EState *estate,
											 CommandId cid,
											 Snapshot crosscheck,
											 bool wait,
											 HeapUpdateFailureData *hufd,
											 LockTupleMode *lockmode,
											 InsertIndexTuples IndexFunc,
											 List **recheckIndexes);

typedef bool (*TupleFetch_function) (Relation relation,
									 ItemPointer tid,
									 Snapshot snapshot,
									 TableTuple * tuple,
									 Buffer *userbuf,
									 bool keep_buf,
									 Relation stats_relation);

typedef HTSU_Result (*TupleLock_function) (Relation relation,
										   ItemPointer tid,
										   TableTuple * tuple,
										   CommandId cid,
										   LockTupleMode mode,
										   LockWaitPolicy wait_policy,
										   bool follow_update,
										   Buffer *buffer,
										   HeapUpdateFailureData *hufd);

typedef void (*MultiInsert_function) (Relation relation, HeapTuple *tuples, int ntuples,
									  CommandId cid, int options, BulkInsertState bistate);

typedef void (*TupleGetLatestTid_function) (Relation relation,
											Snapshot snapshot,
											ItemPointer tid);

typedef tuple_data(*GetTupleData_function) (TableTuple tuple, tuple_data_flags flags);

typedef TableTuple(*TupleFromDatum_function) (Datum data, Oid tableoid);

typedef void (*RelationSync_function) (Relation relation);


typedef HeapScanDesc (*ScanBegin_function) (Relation relation,
											Snapshot snapshot,
											int nkeys, ScanKey key,
											ParallelHeapScanDesc parallel_scan,
											bool allow_strat,
											bool allow_sync,
											bool allow_pagemode,
											bool is_bitmapscan,
											bool is_samplescan,
											bool temp_snap);
typedef void (*ScanSetlimits_function) (HeapScanDesc sscan, BlockNumber startBlk, BlockNumber numBlks);

/* must return a TupleTableSlot? */
typedef TableTuple(*ScanGetnext_function) (HeapScanDesc scan,
											 ScanDirection direction);

typedef TupleTableSlot *(*ScanGetnextSlot_function) (HeapScanDesc scan,
													 ScanDirection direction, TupleTableSlot *slot);

typedef void (*ScanEnd_function) (HeapScanDesc scan);


typedef void (*ScanGetpage_function) (HeapScanDesc scan, BlockNumber page);
typedef void (*ScanRescan_function) (HeapScanDesc scan, ScanKey key, bool set_params,
									 bool allow_strat, bool allow_sync, bool allow_pagemode);
typedef void (*ScanUpdateSnapshot_function) (HeapScanDesc scan, Snapshot snapshot);

typedef bool (*HotSearchBuffer_function) (ItemPointer tid, Relation relation,
										  Buffer buffer, Snapshot snapshot, HeapTuple heapTuple,
										  bool *all_dead, bool first_call);


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

	slot_tableam_hook slot_storageam;

	/* Operations on physical tuples */
	TupleInsert_function tuple_insert;	/* heap_insert */
	TupleUpdate_function tuple_update;	/* heap_update */
	TupleDelete_function tuple_delete;	/* heap_delete */
	TupleFetch_function tuple_fetch;	/* heap_fetch */
	TupleLock_function tuple_lock;	/* heap_lock_tuple */
	MultiInsert_function multi_insert;	/* heap_multi_insert */
	TupleGetLatestTid_function tuple_get_latest_tid;	/* heap_get_latest_tid */

	GetTupleData_function get_tuple_data;
	TupleFromDatum_function tuple_from_datum;

	RelationSync_function relation_sync;	/* heap_sync */

	/* Operations on relation scans */
	ScanBegin_function scan_begin;
	ScanSetlimits_function scansetlimits;
	ScanGetnext_function scan_getnext;
	ScanGetnextSlot_function scan_getnextslot;
	ScanEnd_function scan_end;
	ScanGetpage_function scan_getpage;
	ScanRescan_function scan_rescan;
	ScanUpdateSnapshot_function scan_update_snapshot;
	HotSearchBuffer_function hot_search_buffer; /* heap_hot_search_buffer */

}			TableAmRoutine;

extern TableAmRoutine * GetTableAmRoutine(Oid amhandler);
extern TableAmRoutine * GetTableAmRoutineByAmId(Oid amoid);
extern TableAmRoutine * GetHeapamTableAmRoutine(void);

#endif							/* TABLEEAMAPI_H */
