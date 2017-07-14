/*
 * contrib/spi/autoinc.c
 */
#include "postgres.h"

#include "access/htup_details.h"
#include "catalog/pg_type.h"
#include "commands/sequence.h"
#include "commands/trigger.h"
#include "executor/spi.h"
#include "utils/builtins.h"
#include "utils/rel.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(autoinc);

Datum
autoinc(PG_FUNCTION_ARGS)
{
	TriggerData *trigdata = (TriggerData *) fcinfo->context;
	Trigger    *trigger;		/* to get trigger name */
	int			nargs;			/* # of arguments */
	int		   *chattrs;		/* attnums of attributes to change */
	int			chnattrs = 0;	/* # of above */
	Datum	   *newvals;		/* vals of above */
	bool	   *newnulls;		/* null flags for above */
	char	  **args;			/* arguments */
	char	   *relname;		/* triggered relation name */
	Relation	rel;			/* triggered relation */
	TupleTableSlot *retslot = NULL;
	TupleDesc	tupdesc;		/* tuple description */
	bool		isnull;
	int			i;

	if (!CALLED_AS_TRIGGER(fcinfo))
		/* internal error */
		elog(ERROR, "not fired by trigger manager");
	if (!TRIGGER_FIRED_FOR_ROW(trigdata->tg_event))
		/* internal error */
		elog(ERROR, "must be fired for row");
	if (!TRIGGER_FIRED_BEFORE(trigdata->tg_event))
		/* internal error */
		elog(ERROR, "must be fired before event");

	if (TRIGGER_FIRED_BY_INSERT(trigdata->tg_event))
		retslot = trigdata->tg_trigslot;
	else if (TRIGGER_FIRED_BY_UPDATE(trigdata->tg_event))
		retslot = trigdata->tg_newslot;
	else
		/* internal error */
		elog(ERROR, "cannot process DELETE events");

	rel = trigdata->tg_relation;
	relname = SPI_getrelname(rel);

	trigger = trigdata->tg_trigger;

	nargs = trigger->tgnargs;
	if (nargs <= 0 || nargs % 2 != 0)
		/* internal error */
		elog(ERROR, "autoinc (%s): even number gt 0 of arguments was expected", relname);

	args = trigger->tgargs;
	tupdesc = rel->rd_att;

	chattrs = (int *) palloc(nargs / 2 * sizeof(int));
	newvals = (Datum *) palloc(nargs / 2 * sizeof(Datum));
	newnulls = (bool *) palloc(nargs / 2 * sizeof(bool));

	for (i = 0; i < nargs;)
	{
		int			attnum = SPI_fnumber(tupdesc, args[i]);
		int32		val;
		Datum		seqname;

		if (attnum <= 0)
			ereport(ERROR,
					(errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION),
					 errmsg("\"%s\" has no attribute \"%s\"",
							relname, args[i])));

		if (SPI_gettypeid(tupdesc, attnum) != INT4OID)
			ereport(ERROR,
					(errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION),
					 errmsg("attribute \"%s\" of \"%s\" must be type INT4",
							args[i], relname)));

		val = DatumGetInt32(SPI_getslotbinval(retslot, attnum, &isnull));

		if (!isnull && val != 0)
		{
			i += 2;
			continue;
		}

		i++;
		chattrs[chnattrs] = attnum;
		seqname = CStringGetTextDatum(args[i]);
		newvals[chnattrs] = DirectFunctionCall1(nextval, seqname);
		/* nextval now returns int64; coerce down to int32 */
		newvals[chnattrs] = Int32GetDatum((int32) DatumGetInt64(newvals[chnattrs]));
		if (DatumGetInt32(newvals[chnattrs]) == 0)
		{
			newvals[chnattrs] = DirectFunctionCall1(nextval, seqname);
			newvals[chnattrs] = Int32GetDatum((int32) DatumGetInt64(newvals[chnattrs]));
		}
		newnulls[chnattrs] = false;
		pfree(DatumGetTextPP(seqname));
		chnattrs++;
		i++;
	}

	if (chnattrs > 0)
	{
		retslot = heap_modify_slot_by_cols(retslot,
											chnattrs, chattrs,
											newvals, newnulls);
	}

	pfree(relname);
	pfree(chattrs);
	pfree(newvals);
	pfree(newnulls);

	return PointerGetDatum(retslot->tts_tuple);
}
