#!/usr/local/bin/php
<?php
//print_r($argv);
$infer_frontend_root = exec('/usr/local/bin/infer_config frontend-root');
$ps_path = '/usr/local/bin/infer_search';
$index_schema = "Indexes";
$index_table = "searchQueries";

$ps_schema = "SearchQueries";
$tmp = array_search("--query-id", $argv) + 1;
$ps_id = $argv[$tmp];

$pid = pcntl_fork();
if ($pid == 0) {
	// child. run payload_search
//	$argv[sizeof($argv)]="2>>/var/log/infer_search.log";//Send std::err to /var/log/infer_search.log
	pcntl_exec($ps_path, array_slice($argv, 1));
}
else {
	// parent. update time info when done.
	include(__DIR__ . '/../htdocs/data/pg.include.php');
	$postgreSQL=$pg;

	if (!$postgreSQL) {
		echo 'Error:[1] [' . $ps_id . ']' . pg_last_error($postgreSQL) . "\n";
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
		echo 'Error:[2] [' . $ps_id . ']' . pg_last_error($postgreSQL) . "\n";
		exit();
	}

	if (pg_num_rows($result) > 0) {
		$row = pg_fetch_assoc($result);
		$time = time();
		// FIXME should we really update the duration if it was paused, then cancelled?
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
		//Get Result Count
		$result_set=pg_query($pg,'Select * from "'. $index_schema . '"."' . $index_table . '" where "id"=\'' . $ps_id . '\'');
		$row=pg_fetch_assoc($result_set);
		if(!$row){
			echo 'Error[3] [' . $ps_id . ']' . pg_last_error();
			exit(0);
			}
		pg_query($pg,'CREATE INDEX "' . $ps_id . '_neoflow_start_time_idx"  on "SearchQueries"."'. $ps_id . '_neoflow" (start_time)');
		pg_query($pg,'CREATE INDEX "' . $ps_id . '_http_request_host_idx"  on "SearchQueries"."'. $ps_id . '_http" (request_host)');
		$count=preg_match('/.*http_.*/',$row['filter'],$matches);
		$numResults=0;
		if($count==0)
			//non-HTTP flow queries
			$result_set=pg_query($pg,'SELECT COUNT(*) as "C" from "SearchQueries"."'.$ps_id.'_neoflow"');
		else
		//HTTP flow Queries
			$result_set=pg_query($pg,'SELECT COUNT(*) as "C" from "SearchQueries"."'.$ps_id.'_http"');
		$row_c=pg_fetch_assoc($result_set);
		$numResults=$row_c["C"];
		$query = 'UPDATE "' . $index_schema . '"."' . $index_table . '" ' .
				 'SET "duration" = \'' . $duration . '\', ' .
				 	'"numResults"=\'' . $numResults . '\',' . 
					 '"status" = \'' . $status . '\' ' .
				 'WHERE "id" = \'' . $ps_id . '\';';
		$result_set=pg_query($pg, $query);
		if(!$result_set)
			echo 'Error [4] [' . $ps_id . ']' .  pg_last_error();
	}
}
?>
