<?php
$pg_dbname = 'dbname='.exec('/usr/local/bin/infer_config postgresql.dbname');
$pg_user = 'user='.exec('/usr/local/bin/infer_config postgresql.user');
$pg_password = 'password='.exec('/usr/local/bin/infer_config postgresql.password');

$temp = exec('/usr/local/bin/infer_config postgresql.port');
if($temp == '') $temp = 5432;
$pg_port = 'port='.$temp;

$pg = pg_connect($pg_port.' '.$pg_dbname.' '.$pg_user.' '.$pg_password) or die('could not connect to postgresql server');
/*
* Row insertion wrapper for PostgreSQL. Returns true upon success, or false
* upon failure, at which point the pg_last_error() function may be used to
*examine what went wrong.
*/
function insertPGRow(&$postgreSQL, $schema, $table) {
	$query = 'INSERT INTO "' . $schema . '"."' . $table . '" VALUES (';
	for ($arg = 3; $arg < func_num_args(); ++$arg) {
		$_arg = func_get_arg($arg);
		/* NULL values are represented by PHP's false boolean literal. */
		if ($_arg === false || strlen($_arg) == 0) {
			$query .= 'NULL';
		}
		else {
			/* Non-NULL values are quoted and escaped. */
			$query .= '\'' . pg_escape_string($postgreSQL, $_arg) . '\'';
		}
		if ($arg < (func_num_args() - 1)) {
			$query .= ', ';
		}
	}
	$query .= ')';
	return (pg_query($postgreSQL, $query) !== false) ;
}
?>
