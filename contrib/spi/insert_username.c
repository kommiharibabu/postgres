/*
 * contrib/spi/insert_username.c
 *
 * insert user name in response to a trigger
 * usage:  insert_username (column_name)
 */
#include "postgres.h"

#include "access/htup_details.h"
#include "catalog/pg_type.h"
#include "commands/trigger.h"
#include "executor/spi.h"
#include "miscadmin.h"
#include "utils/builtins.h"
#include "utils/rel.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(insert_username);

Datum
insert_username(PG_FUNCTION_ARGS)
{
	TriggerData *trigdata = (TriggerData *) fcinfo->context;
	Trigger    *trigger;		/* to get trigger name */
	int			nargs;			/* # of arguments */
	Datum		newval;			/* new value of column */
	bool		newnull;		/* null flag */
	char	  **args;			/* arguments */
	char	   *relname;		/* triggered relation name */
	Relation	rel;			/* triggered relation */
	TupleTableSlot *retslot;
	TupleDesc	tupdesc;		/* tuple description */
	int			attnum;

	/* sanity checks from autoinc.c */
	if (!CALLED_AS_TRIGGER(fcinfo))
		/* internal error */
		elog(ERROR, "insert_username: not fired by trigger manager");
	if (!TRIGGER_FIRED_FOR_ROW(trigdata->tg_event))
		/* internal error */
		elog(ERROR, "insert_username: must be fired for row");
	if (!TRIGGER_FIRED_BEFORE(trigdata->tg_event))
		/* internal error */
		elog(ERROR, "insert_username: must be fired before event");

	if (TRIGGER_FIRED_BY_INSERT(trigdata->tg_event))
		retslot = trigdata->tg_trigslot;
	else if (TRIGGER_FIRED_BY_UPDATE(trigdata->tg_event))
		retslot = trigdata->tg_newslot;
	else
		/* internal error */
		elog(ERROR, "insert_username: cannot process DELETE events");

	rel = trigdata->tg_relation;
	relname = SPI_getrelname(rel);

	trigger = trigdata->tg_trigger;

	nargs = trigger->tgnargs;
	if (nargs != 1)
		/* internal error */
		elog(ERROR, "insert_username (%s): one argument was expected", relname);

	args = trigger->tgargs;
	tupdesc = rel->rd_att;

	attnum = SPI_fnumber(tupdesc, args[0]);

	if (attnum <= 0)
		ereport(ERROR,
				(errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION),
				 errmsg("\"%s\" has no attribute \"%s\"", relname, args[0])));

	if (SPI_gettypeid(tupdesc, attnum) != TEXTOID)
		ereport(ERROR,
				(errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION),
				 errmsg("attribute \"%s\" of \"%s\" must be type TEXT",
						args[0], relname)));

	/* create fields containing name */
	newval = CStringGetTextDatum(GetUserNameFromId(GetUserId(), false));
	newnull = false;

	/* construct new tuple */
	retslot = heap_modify_slot_by_cols(retslot,
										 1, &attnum, &newval, &newnull);

	pfree(relname);

	return PointerGetDatum(retslot->tts_tuple);
}
