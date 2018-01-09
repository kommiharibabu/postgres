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
#include "utils/builtins.h"


Datum
heap_tableam_handler(PG_FUNCTION_ARGS)
{
	TableAmRoutine *amroutine = makeNode(TableAmRoutine);

	amroutine->snapshot_satisfies = HeapTupleSatisfies;

	amroutine->snapshot_satisfiesUpdate = HeapTupleSatisfiesUpdate;
	amroutine->snapshot_satisfiesVacuum = HeapTupleSatisfiesVacuum;

	amroutine->slot_storageam = slot_tableam_handler;

	PG_RETURN_POINTER(amroutine);
}
