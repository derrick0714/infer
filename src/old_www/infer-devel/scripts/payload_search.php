#!/usr/local/bin/php
<?php
//require_once('./../htdocs/site/search/common.php');
$infer_frontend_root = exec('/usr/local/bin/infer_config frontend-root');
$ps_path = '/usr/local/bin/infer_payload_search';
$index_schema = "Indexes";
$index_table = "payloadQueries";

$ps_schema = "PayloadQueries";
$tmp = array_search("-t", $argv) + 1;
$ps_id = $argv[$tmp];
$tmp=array_search('--start-time',$argv)+1;
//foreach($argv as $key=> $value){
//	echo $key . ' = > ' . $value . '<br/>';
//	}
$pid = pcntl_fork();
if ($pid == 0) {
	// child. run payload_search
	pcntl_exec($ps_path, array_slice($argv, 1));
}
else {
	// parent. update time info when done.
	include(__DIR__ . '/../htdocs/data/pg.include.php');

	if (!$pg) {
		echo 'Error: ' . pg_last_error($pg) . "\n";
		exit();
	}

	$query = 'UPDATE "' . $index_schema . '"."' . $index_table . '" ' .
			 'SET "pid" = \'' . $pid .  '\' ' .
			 'WHERE "id" = \'' . $ps_id . '\';';
	pg_query($pg, $query);

	$status = 0;
	pcntl_waitpid($pid, $status);
	$query = 'SELECT * FROM "' . $index_schema . '"."' . $index_table . '" ' .
			 'WHERE "id" = \'' . $ps_id . '\';';

	$result = pg_query($pg, $query);
	if (!$result) {
		echo 'Error: ' . pg_last_error($pg) . "\n";
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
		pg_query($pg, $query);
	}
}
?>
