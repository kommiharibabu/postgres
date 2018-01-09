/*-------------------------------------------------------------------------
 *
 * tableam.h
 *	  POSTGRES table access method definitions.
 *
 *
 * Portions Copyright (c) 1996-2017, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/access/tableam.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef TABLEAM_H
#define TABLEAM_H

#include "access/heapam.h"
#include "access/tableam_common.h"
#include "executor/tuptable.h"
#include "nodes/execnodes.h"

typedef union tuple_data
{
	TransactionId xid;
	CommandId	cid;
	ItemPointerData tid;
}			tuple_data;

typedef enum tuple_data_flags
{
	XMIN = 0,
	UPDATED_XID,
	CMIN,
	TID,
	CTID
}			tuple_data_flags;

/* Function pointer to let the index tuple insert from storage am */
typedef List *(*InsertIndexTuples) (TupleTableSlot *slot, EState *estate, bool noDupErr,
									bool *specConflict, List *arbiterIndexes);

/* Function pointer to let the index tuple delete from storage am */
typedef void (*DeleteIndexTuples) (Relation rel, ItemPointer tid, TransactionId old_xmin);

extern bool table_fetch(Relation relation,
			  ItemPointer tid,
			  Snapshot snapshot,
			  TableTuple * stuple,
			  Buffer *userbuf,
			  bool keep_buf,
			  Relation stats_relation);

extern HTSU_Result table_lock_tuple(Relation relation, ItemPointer tid, TableTuple * stuple,
				   CommandId cid, LockTupleMode mode, LockWaitPolicy wait_policy,
				   bool follow_updates,
				   Buffer *buffer, HeapUpdateFailureData *hufd);

extern Oid table_insert(Relation relation, TupleTableSlot *slot, CommandId cid,
			   int options, BulkInsertState bistate, InsertIndexTuples IndexFunc,
			   EState *estate, List *arbiterIndexes, List **recheckIndexes);

extern HTSU_Result table_delete(Relation relation, ItemPointer tid, CommandId cid,
			   Snapshot crosscheck, bool wait, DeleteIndexTuples IndexFunc,
			   HeapUpdateFailureData *hufd);

extern HTSU_Result table_update(Relation relation, ItemPointer otid, TupleTableSlot *slot,
			   EState *estate, CommandId cid, Snapshot crosscheck, bool wait,
			   HeapUpdateFailureData *hufd, LockTupleMode *lockmode,
			   InsertIndexTuples IndexFunc, List **recheckIndexes);

extern void table_multi_insert(Relation relation, HeapTuple *tuples, int ntuples,
					 CommandId cid, int options, BulkInsertState bistate);

extern tuple_data table_tuple_get_data(Relation relation, TableTuple tuple, tuple_data_flags flags);

extern TableTuple table_tuple_by_datum(Relation relation, Datum data, Oid tableoid);

extern void table_get_latest_tid(Relation relation,
					   Snapshot snapshot,
					   ItemPointer tid);

extern void table_sync(Relation rel);

#endif		/* TABLEAM_H */
