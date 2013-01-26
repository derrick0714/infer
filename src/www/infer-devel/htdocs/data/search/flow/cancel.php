<?php

require_once('../../essentials.class.php');
require_once('../../search_essentials.class.php');
require_once('../../pg.include.php');

$infer_config_cmd = '/usr/local/bin/infer_config';

if ($_SERVER['REQUEST_METHOD'] != 'POST') {
	header('HTTP/1.1 405 Method Not Allowed');
	header('Allow: POST');
	exit;
};

if ($argc != 2) {
	header('HTTP/1.1 400 Bad Request');
	exit;
}

$query_id = $argv[1];

// OK, do the deed...
$result = pg_query($pg,
				   'SELECT * from "Indexes"."searchQueries" ' .
				   'WHERE id=\'' . $query_id . '\'');
if (pg_num_rows($result) == 0) {
	header("HTTP/1.1 404 Not Found");
	exit;
}

$row = pg_fetch_assoc($result);

if ($row['status'] == HBF_RUNNING || $row['status'] == HBF_PAUSED) {
	$time_now = time();
	$duration = $row['duration'];
	if ($row['pauseTime'] == NULL) {
		$duration = $time_now - $row['startTime'];
	}
	else if ($row['status'] != HBF_PAUSED) {
		$duration += $time_now - $row['resumeTime'];
	}

	pg_query($pg,
			 'UPDATE "Indexes"."searchQueries" ' .
			 "SET status = '" . HBF_CANCELLED . "', " .
				 "duration = '" . $duration . "' " .
			 "WHERE id = '" . $query_id . "'");

	if ($row['pid']) {
		posix_kill($row['pid'], SIGKILL);
	}
}

header('HTTP/1.1 303 See Other');
$redirect_url = 'http://';
if ($_SERVER['HTTPS']) {
	$redirect_url = 'https://';
};
$redirect_url .= $_SERVER['HTTP_HOST'] . '/search/#/queries/';
header('Location: ' . $redirect_url);

?>
