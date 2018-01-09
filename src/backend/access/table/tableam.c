/*-------------------------------------------------------------------------
 *
 * tableam.c
 *	  table access method code
 *
 * Portions Copyright (c) 1996-2017, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/backend/access/table/tableam.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/tableam.h"
#include "access/tableamapi.h"
#include "utils/rel.h"

/*
 *	table_fetch		- retrieve tuple with given tid
 */
bool
table_fetch(Relation relation,
			  ItemPointer tid,
			  Snapshot snapshot,
			  TableTuple * stuple,
			  Buffer *userbuf,
			  bool keep_buf,
			  Relation stats_relation)
{
	return relation->rd_tableamroutine->tuple_fetch(relation, tid, snapshot, stuple,
												 userbuf, keep_buf, stats_relation);
}


/*
 *	table_lock_tuple - lock a tuple in shared or exclusive mode
 */
HTSU_Result
table_lock_tuple(Relation relation, ItemPointer tid, TableTuple * stuple,
				   CommandId cid, LockTupleMode mode, LockWaitPolicy wait_policy,
				   bool follow_updates, Buffer *buffer, HeapUpdateFailureData *hufd)
{
	return relation->rd_tableamroutine->tuple_lock(relation, tid, stuple,
												cid, mode, wait_policy,
												follow_updates, buffer, hufd);
}

/*
 * Insert a tuple from a slot into table AM routine
 */
Oid
table_insert(Relation relation, TupleTableSlot *slot, CommandId cid,
			   int options, BulkInsertState bistate, InsertIndexTuples IndexFunc,
			   EState *estate, List *arbiterIndexes, List **recheckIndexes)
{
	return relation->rd_tableamroutine->tuple_insert(relation, slot, cid, options,
												  bistate, IndexFunc, estate,
												  arbiterIndexes, recheckIndexes);
}

/*
 * Delete a tuple from tid using table AM routine
 */
HTSU_Result
table_delete(Relation relation, ItemPointer tid, CommandId cid,
			   Snapshot crosscheck, bool wait, DeleteIndexTuples IndexFunc,
			   HeapUpdateFailureData *hufd)
{
	return relation->rd_tableamroutine->tuple_delete(relation, tid, cid,
												  crosscheck, wait, IndexFunc, hufd);
}

/*
 * update a tuple from tid using table AM routine
 */
HTSU_Result
table_update(Relation relation, ItemPointer otid, TupleTableSlot *slot,
			   EState *estate, CommandId cid, Snapshot crosscheck, bool wait,
			   HeapUpdateFailureData *hufd, LockTupleMode *lockmode,
			   InsertIndexTuples IndexFunc, List **recheckIndexes)
{
	return relation->rd_tableamroutine->tuple_update(relation, otid, slot, estate,
												  cid, crosscheck, wait, hufd,
												  lockmode, IndexFunc, recheckIndexes);
}


/*
 *	table_multi_insert	- insert multiple tuple into a table
 */
void
table_multi_insert(Relation relation, HeapTuple *tuples, int ntuples,
					 CommandId cid, int options, BulkInsertState bistate)
{
	relation->rd_tableamroutine->multi_insert(relation, tuples, ntuples,
										   cid, options, bistate);
}

tuple_data
table_tuple_get_data(Relation relation, TableTuple tuple, tuple_data_flags flags)
{
	return relation->rd_tableamroutine->get_tuple_data(tuple, flags);
}

TableTuple
table_tuple_by_datum(Relation relation, Datum data, Oid tableoid)
{
	if (relation)
		return relation->rd_tableamroutine->tuple_from_datum(data, tableoid);
	else
		return heap_form_tuple_by_datum(data, tableoid);
}

void
table_get_latest_tid(Relation relation,
					   Snapshot snapshot,
					   ItemPointer tid)
{
	relation->rd_tableamroutine->tuple_get_latest_tid(relation, snapshot, tid);
}

/*
 *	table_sync		- sync a heap, for use when no WAL has been written
 */
void
table_sync(Relation rel)
{
	rel->rd_tableamroutine->relation_sync(rel);
}
