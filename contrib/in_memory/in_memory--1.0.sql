/* contrib/in_memory/in_memory--1.0.sql */

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION in_memory" to load this file. \quit

-- Register functions.

CREATE FUNCTION in_memory_tableam_handler(internal)
RETURNS table_am_handler
AS 'MODULE_PATHNAME'
LANGUAGE C VOLATILE STRICT;

CREATE ACCESS METHOD in_memory TYPE table handler in_memory_tableam_handler;
