#!/usr/local/bin/php
<?php

$infer_frontend_root = exec('/usr/local/bin/infer_config frontend-root');
$ps_path = '/usr/local/bin/infer_payload_search';
$index_schema = "Indexes";
$index_table = "payloadQueries";

$ps_schema = "PayloadQueries";
$tmp = array_search("-t", $argv) + 1;
$ps_id = $argv[$tmp];

$pid = pcntl_fork();
if ($pid == 0) {
	// child. run payload_search
	pcntl_exec($ps_path, array_slice($argv, 1));
}
else {
	// parent. update time info when done.
	include($infer_frontend_root . '/htdocs/include/postgreSQL.php');

	if (!$postgreSQL) {
		echo 'Error: ' . pg_last_error($postgreSQL) . "\n";
		exit();
	}

	$query = 'UPDATE "' . $index_schema . '"."' . $index_table . '" ' .
			 'SET "pid" = \'' . $pid . '\' ' .
			 'WHERE "id" = \'' . $ps_id . '\';';
	pg_query($postgreSQL, $query);

	$status = 0;
	pcntl_waitpid($pid, $status);
	
	$query = 'SELECT * FROM "' . $index_schema . '"."' . $index_table . '" ' .
			 'WHERE "id" = \'' . $ps_id . '\';';

	$result = pg_query($postgreSQL, $query);
	if (!$result) {
		echo 'Error: ' . pg_last_error($postgreSQL) . "\n";
		exit();
	}

	if (pg_num_rows($result) > 0) {
		$row = pg_fetch_assoc($result);
		$time = time();
		if ($row['resumeTime'] == NULL) {
			$duration = $row['duration'] + $time - $row['startTime'];
		} else {
			$duration = $row['duration'] + $time - $row['resumeTime'];
		}

		if ($status == 0) {
			$status = 1;
		}
		else if ($row['status'] == 2) {
			$status = 2;
		}
		else {
			$status = 4;
		}

		$query = 'UPDATE "' . $index_schema . '"."' . $index_table . '" ' .
				 'SET "duration" = \'' . $duration . '\', ' .
				 	 '"status" = \'' . $status . '\' ' .
				 'WHERE "id" = \'' . $ps_id . '\';';
		pg_query($postgreSQL, $query);
	}
}
?>
