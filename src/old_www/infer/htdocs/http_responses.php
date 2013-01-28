<?php
include('include/postgreSQL.php');
include('include/shared.php');
include('include/accessControl.php');
include('include/checkSession.php');
include('include/create_table.php');

$response_format = array(
	"heading" => array(
		'Time',
		'Version',
		'Status',
		'Reason',
		'Response',
		'Content Type',
	),
	"display" => array(
		'"time_seconds"',
		'"version"',
		'"status"',
		'"reason"',
		'"response"',
		'"content_type"'
	),
	"type" => array(
		'date',
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	)
);

$pageToken = '*';
$page = 1;
$numResults = 10;
$tokenIndex = array_search($pageToken, $url);
if ($tokenIndex !== false) {
	if (isset($url[$tokenIndex + 2])) {
		$numResults = $url[$tokenIndex + 2];
		unset($url[$tokenIndex + 2]);
	}
	if (isset($url[$tokenIndex + 1])) {
		$page = $url[$tokenIndex + 1];
		unset($url[$tokenIndex + 1]);
	}
	unset($url[$tokenIndex]);
}	

$schemaName = "HTTPResponses";
if (!isset($url[1])) {
	$lastDay = getLastPGTable($postgreSQL, $schemaName);
	header('Location: /http_responses/' . $lastDay);
	exit;
}

$title = makeNavMsg($url, false);
makeHeader($postgreSQL, $title);
message(makeNavMsg($url));

if (!existsPGTable($postgreSQL, $schemaName, $url[1])) {
	message("No HTTP-response data available for specified date.", true);
	include("include/footer.html");
	exit;
}

$baseURL = '/';
$baseURL .= implode('/', $url);

//$linkBaseURL = '/' . implode('/', array_slice($url, 0, 3));

$date = $url[1];

$where = array();
if (isset($url[2])) {
	array_push($where, array(array('"destination_ip"', '=', ip2long($url[2]))));
	if (isset($url[3])) {
		array_push($where, array(array('"source_ip"', '=', ip2long($url[3]))));
	}
}

$order = array('"time_seconds"');

createTable($postgreSQL, $schemaName, $date, $numResults, $page, 
			$response_format['heading'],
			$response_format['display'],
			$response_format['type'],
			$baseURL,
			NULL,
			NULL,
			NULL,
			$where,
			NULL,
			$order,
			NULL);

include('include/footer.html');
?>
