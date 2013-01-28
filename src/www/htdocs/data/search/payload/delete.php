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
				   'SELECT * from "Indexes"."payloadQueries" ' .
				   'WHERE id=\'' . $query_id . '\'');
if (pg_num_rows($result) == 0) {
	header("HTTP/1.1 404 Not Found");
	exit;
}

$row = pg_fetch_assoc($result);

if ($row['pid'] && 
	($row['status'] == HBF_RUNNING || $row['status'] == HBF_PAUSED))
{
	posix_kill($row['pid'], SIGKILL);
}

pg_query($pg,
		 'DELETE FROM "Indexes"."payloadQueries" ' .
		 'WHERE id=\'' . $query_id . '\'');
pg_query($pg,
		 'DROP TABLE IF EXISTS ' . 
		 	'"SearchQueries"."' . $row['id'] . '_dns"');
pg_query($pg,
		 'DROP TABLE IF EXISTS ' .
		 	'"SearchQueries"."' . $row['id'] . '_dns_response"');
pg_query($pg,
		 'DROP TABLE IF EXISTS ' .
		 	'"SearchQueries"."' . $row['id'] . '_http"');
pg_query($pg,
		 'DROP TABLE IF EXISTS ' .
		 	'"SearchQueries"."' . $row['id'] . '_neoflow"');

header('HTTP/1.1 303 See Other');
$redirect_url = 'http://';
if ($_SERVER['HTTPS']) {
	$redirect_url = 'https://';
};
$redirect_url .= $_SERVER['HTTP_HOST'] . '/search/#/queries/';
header('Location: ' . $redirect_url);

?>
