/*-------------------------------------------------------------------------
 *
 * heapam_handler.c
 *	  heap table access method code
 *
 * Portions Copyright (c) 1996-2017, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/backend/access/heap/heapam_handler.c
 *
 *
 * NOTES
 *	  This file contains the heap_ routines which implement
 *	  the POSTGRES heap table access method used for all POSTGRES
 *	  relations.
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/heapam.h"
#include "access/tableamapi.h"
#include "storage/lmgr.h"
#include "utils/builtins.h"
#include "utils/rel.h"
#include "utils/tqual.h"


/* ----------------------------------------------------------------
 *				storage AM support routines for heapam
 * ----------------------------------------------------------------
 */

static bool
heapam_fetch(Relation relation,
			 ItemPointer tid,
			 Snapshot snapshot,
			 TableTuple * stuple,
			 Buffer *userbuf,
			 bool keep_buf,
			 Relation stats_relation)
{
	HeapTupleData tuple;

	*stuple = NULL;
	if (heap_fetch(relation, tid, snapshot, &tuple, userbuf, keep_buf, stats_relation))
	{
		*stuple = heap_copytuple(&tuple);
		return true;
	}

	return false;
}

/*
 * Insert a heap tuple from a slot, which may contain an OID and speculative
 * insertion token.
 */
static Oid
heapam_heap_insert(Relation relation, TupleTableSlot *slot, CommandId cid,
				   int options, BulkInsertState bistate, InsertIndexTuples IndexFunc,
				   EState *estate, List *arbiterIndexes, List **recheckIndexes)
{
	Oid			oid;
	HeapTuple	tuple = NULL;

	if (slot->tts_storage)
	{
		HeapamTuple *htuple = slot->tts_storage;

		tuple = htuple->hst_heaptuple;

		if (relation->rd_rel->relhasoids)
			HeapTupleSetOid(tuple, InvalidOid);
	}
	else
	{
		/*
		 * Obtain the physical tuple to insert, building from the slot values.
		 * XXX: maybe the slot already contains a physical tuple in the right
		 * format?  In fact, if the slot isn't fully deformed, this is
		 * completely bogus ...
		 */
		tuple = heap_form_tuple(slot->tts_tupleDescriptor,
								slot->tts_values,
								slot->tts_isnull);
	}

	/* Set the OID, if the slot has one */
	if (slot->tts_tupleOid != InvalidOid)
		HeapTupleHeaderSetOid(tuple->t_data, slot->tts_tupleOid);

	/* Update the tuple with table oid */
	if (slot->tts_tableOid != InvalidOid)
		tuple->t_tableOid = slot->tts_tableOid;

	/* Set the speculative insertion token, if the slot has one */
	if ((options & HEAP_INSERT_SPECULATIVE) && slot->tts_speculativeToken)
		HeapTupleHeaderSetSpeculativeToken(tuple->t_data, slot->tts_speculativeToken);

	/* Perform the insertion, and copy the resulting ItemPointer */
	oid = heap_insert(relation, tuple, cid, options, bistate);
	ItemPointerCopy(&tuple->t_self, &slot->tts_tid);

	if (slot->tts_storage == NULL)
		ExecStoreTuple(tuple, slot, InvalidBuffer, true);

	if ((estate != NULL) && (estate->es_result_relation_info->ri_NumIndices > 0))
	{
		Assert(IndexFunc != NULL);

		if (options & HEAP_INSERT_SPECULATIVE)
		{
			bool		specConflict = false;

			*recheckIndexes = (IndexFunc) (slot, estate, true,
										   &specConflict,
										   arbiterIndexes);

			/* adjust the tuple's state accordingly */
			if (!specConflict)
				heap_finish_speculative(relation, slot);
			else
			{
				heap_abort_speculative(relation, slot);
				slot->tts_specConflict = true;
			}
		}
		else
		{
			*recheckIndexes = (IndexFunc) (slot, estate, false,
										   NULL, arbiterIndexes);
		}
	}

	return oid;
}

static HTSU_Result
heapam_heap_delete(Relation relation, ItemPointer tid, CommandId cid,
				   Snapshot crosscheck, bool wait, DeleteIndexTuples IndexFunc,
				   HeapUpdateFailureData *hufd)
{
	/*
	 * Currently Deleting of index tuples are handled at vacuum, in case
	 * if the storage itself is cleaning the dead tuples by itself, it is
	 * the time to call the index tuple deletion also.
	 */
	return heap_delete(relation, tid, cid, crosscheck, wait, hufd);
}


/*
 * Locks tuple and fetches its newest version and TID.
 *
 *	relation - table containing tuple
 *	*tid - TID of tuple to lock (rest of struct need not be valid)
 *	snapshot - snapshot indentifying required version (used for assert check only)
 *	*stuple - tuple to be returned
 *	cid - current command ID (used for visibility test, and stored into
 *		  tuple's cmax if lock is successful)
 *	mode - indicates if shared or exclusive tuple lock is desired
 *	wait_policy - what to do if tuple lock is not available
 *	flags – indicating how do we handle updated tuples
 *	*hufd - filled in failure cases
 *
 * Function result may be:
 *	HeapTupleMayBeUpdated: lock was successfully acquired
 *	HeapTupleInvisible: lock failed because tuple was never visible to us
 *	HeapTupleSelfUpdated: lock failed because tuple updated by self
 *	HeapTupleUpdated: lock failed because tuple updated by other xact
 *	HeapTupleWouldBlock: lock couldn't be acquired and wait_policy is skip
 *
 * In the failure cases other than HeapTupleInvisible, the routine fills
 * *hufd with the tuple's t_ctid, t_xmax (resolving a possible MultiXact,
 * if necessary), and t_cmax (the last only for HeapTupleSelfUpdated,
 * since we cannot obtain cmax from a combocid generated by another
 * transaction).
 * See comments for struct HeapUpdateFailureData for additional info.
 */
static HTSU_Result
heapam_lock_tuple(Relation relation, ItemPointer tid, TableTuple *stuple,
				CommandId cid, LockTupleMode mode,
				LockWaitPolicy wait_policy, bool follow_updates, Buffer *buffer,
				HeapUpdateFailureData *hufd)
{
	HTSU_Result		result;
	HeapTupleData	tuple;

	Assert(stuple != NULL);
	*stuple = NULL;

	tuple.t_self = *tid;
	result = heap_lock_tuple(relation, &tuple, cid, mode, wait_policy, follow_updates, buffer, hufd);

	*stuple = heap_copytuple(&tuple);

	return result;
}


static HTSU_Result
heapam_heap_update(Relation relation, ItemPointer otid, TupleTableSlot *slot,
				   EState *estate, CommandId cid, Snapshot crosscheck,
				   bool wait, HeapUpdateFailureData *hufd, LockTupleMode *lockmode,
				   InsertIndexTuples IndexFunc, List **recheckIndexes)
{
	HeapTuple	tuple;
	HTSU_Result result;

	if (slot->tts_storage)
	{
		HeapamTuple *htuple = slot->tts_storage;

		tuple = htuple->hst_heaptuple;
	}
	else
	{
		tuple = heap_form_tuple(slot->tts_tupleDescriptor,
								slot->tts_values,
								slot->tts_isnull);
	}

	/* Set the OID, if the slot has one */
	if (slot->tts_tupleOid != InvalidOid)
		HeapTupleHeaderSetOid(tuple->t_data, slot->tts_tupleOid);

	/* Update the tuple with table oid */
	if (slot->tts_tableOid != InvalidOid)
		tuple->t_tableOid = slot->tts_tableOid;

	result = heap_update(relation, otid, tuple, cid, crosscheck, wait,
						 hufd, lockmode);
	ItemPointerCopy(&tuple->t_self, &slot->tts_tid);

	if (slot->tts_storage == NULL)
		ExecStoreTuple(tuple, slot, InvalidBuffer, true);

	/*
	 * Note: instead of having to update the old index tuples associated with
	 * the heap tuple, all we do is form and insert new index tuples. This is
	 * because UPDATEs are actually DELETEs and INSERTs, and index tuple
	 * deletion is done later by VACUUM (see notes in ExecDelete). All we do
	 * here is insert new index tuples.  -cim 9/27/89
	 */

	/*
	 * insert index entries for tuple
	 *
	 * Note: heap_update returns the tid (location) of the new tuple in the
	 * t_self field.
	 *
	 * If it's a HOT update, we mustn't insert new index entries.
	 */
	if ((result == HeapTupleMayBeUpdated) &&
		((estate != NULL) && (estate->es_result_relation_info->ri_NumIndices > 0)) &&
		(!HeapTupleIsHeapOnly(tuple)))
		*recheckIndexes = (IndexFunc) (slot, estate, false, NULL, NIL);

	return result;
}

static tuple_data
heapam_get_tuple_data(TableTuple tuple, tuple_data_flags flags)
{
	tuple_data	result;

	switch (flags)
	{
		case XMIN:
			result.xid = HeapTupleHeaderGetXmin(((HeapTuple) tuple)->t_data);
			break;
		case UPDATED_XID:
			result.xid = HeapTupleHeaderGetUpdateXid(((HeapTuple) tuple)->t_data);
			break;
		case CMIN:
			result.cid = HeapTupleHeaderGetCmin(((HeapTuple) tuple)->t_data);
			break;
		case TID:
			result.tid = ((HeapTuple) tuple)->t_self;
			break;
		case CTID:
			result.tid = ((HeapTuple) tuple)->t_data->t_ctid;
			break;
		default:
			Assert(0);
			break;
	}

	return result;
}

static TableTuple
heapam_form_tuple_by_datum(Datum data, Oid tableoid)
{
	return heap_form_tuple_by_datum(data, tableoid);
}

Datum
heap_tableam_handler(PG_FUNCTION_ARGS)
{
	TableAmRoutine *amroutine = makeNode(TableAmRoutine);

	amroutine->snapshot_satisfies = HeapTupleSatisfies;

	amroutine->snapshot_satisfiesUpdate = HeapTupleSatisfiesUpdate;
	amroutine->snapshot_satisfiesVacuum = HeapTupleSatisfiesVacuum;

	amroutine->slot_storageam = slot_tableam_handler;

	amroutine->tuple_fetch = heapam_fetch;
	amroutine->tuple_insert = heapam_heap_insert;
	amroutine->tuple_delete = heapam_heap_delete;
	amroutine->tuple_update = heapam_heap_update;
	amroutine->tuple_lock = heapam_lock_tuple;
	amroutine->multi_insert = heap_multi_insert;

	amroutine->get_tuple_data = heapam_get_tuple_data;
	amroutine->tuple_from_datum = heapam_form_tuple_by_datum;
	amroutine->tuple_get_latest_tid = heap_get_latest_tid;
	amroutine->relation_sync = heap_sync;

	PG_RETURN_POINTER(amroutine);
}
