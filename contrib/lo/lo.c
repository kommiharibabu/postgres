/*
 *	PostgreSQL definitions for managed Large Objects.
 *
 *	contrib/lo/lo.c
 *
 */

#include "postgres.h"

#include "commands/trigger.h"
#include "executor/spi.h"
#include "utils/builtins.h"
#include "utils/rel.h"

PG_MODULE_MAGIC;


/*
 * This is the trigger that protects us from orphaned large objects
 */
PG_FUNCTION_INFO_V1(lo_manage);

Datum
lo_manage(PG_FUNCTION_ARGS)
{
	TriggerData *trigdata = (TriggerData *) fcinfo->context;
	int			attnum;			/* attribute number to monitor	*/
	char	  **args;			/* Args containing attr name	*/
	TupleDesc	tupdesc;		/* Tuple Descriptor				*/
	TupleTableSlot*	retslot;		/* Tuple to be returned			*/
	bool		isdelete;		/* are we deleting?				*/
	TupleTableSlot*	newslot;		/* The new value for tuple		*/
	TupleTableSlot*	trigslot;		/* The original value of tuple	*/

	if (!CALLED_AS_TRIGGER(fcinfo)) /* internal error */
		elog(ERROR, "%s: not fired by trigger manager",
			 trigdata->tg_trigger->tgname);

	if (!TRIGGER_FIRED_FOR_ROW(trigdata->tg_event)) /* internal error */
		elog(ERROR, "%s: must be fired for row",
			 trigdata->tg_trigger->tgname);

	/*
	 * Fetch some values from trigdata
	 */
	newslot = trigdata->tg_newslot;
	trigslot = trigdata->tg_trigslot;
	tupdesc = trigdata->tg_relation->rd_att;
	args = trigdata->tg_trigger->tgargs;

	if (args == NULL)			/* internal error */
		elog(ERROR, "%s: no column name provided in the trigger definition",
			 trigdata->tg_trigger->tgname);

	/* tuple to return to Executor */
	if (TRIGGER_FIRED_BY_UPDATE(trigdata->tg_event))
		retslot = newslot;
	else
		retslot = trigslot;

	/* Are we deleting the row? */
	isdelete = TRIGGER_FIRED_BY_DELETE(trigdata->tg_event);

	/* Get the column we're interested in */
	attnum = SPI_fnumber(tupdesc, args[0]);

	if (attnum <= 0)
		elog(ERROR, "%s: column \"%s\" does not exist",
			 trigdata->tg_trigger->tgname, args[0]);

	/*
	 * Handle updates
	 *
	 * Here, if the value of the monitored attribute changes, then the large
	 * object associated with the original value is unlinked.
	 */
	if (newslot != NULL)
	{
		char	   *orig = SPI_getslotvalue(trigslot, attnum);
		char	   *newv = SPI_getslotvalue(newslot, attnum);

		if (orig != NULL && (newv == NULL || strcmp(orig, newv) != 0))
			DirectFunctionCall1(be_lo_unlink,
								ObjectIdGetDatum(atooid(orig)));

		if (newv)
			pfree(newv);
		if (orig)
			pfree(orig);
	}

	/*
	 * Handle deleting of rows
	 *
	 * Here, we unlink the large object associated with the managed attribute
	 */
	if (isdelete)
	{
		char	   *orig = SPI_getslotvalue(trigslot, attnum);

		if (orig != NULL)
		{
			DirectFunctionCall1(be_lo_unlink,
								ObjectIdGetDatum(atooid(orig)));

			pfree(orig);
		}
	}

	return PointerGetDatum(retslot->tts_tuple);
}
