/* contrib/global_temp/global_temp--1.0.sql */

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION global_temp" to load this file. \quit

-- Register functions.

CREATE FUNCTION global_temp_tableam_handler(internal)
RETURNS table_am_handler
AS 'MODULE_PATHNAME'
LANGUAGE C VOLATILE STRICT;

CREATE ACCESS METHOD global_temp TYPE table handler global_temp_tableam_handler;
