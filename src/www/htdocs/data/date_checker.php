<?php
require_once('essentials.class.php');
require_once('pg.include.php');

$essentials = new essentials();

$date = $essentials->dateParser($argv[1], false);
$date_check = @pg_num_rows(@pg_query($pg, "SELECT \"table_name\" FROM \"information_schema\".\"tables\" WHERE \"table_schema\" = 'InterestingIPs' AND \"table_name\" = '".date('Y-m-d', $date)."' LIMIT 1"));
$first_date = array_shift(@pg_fetch_assoc(@pg_query($pg, "SELECT \"table_name\" FROM \"information_schema\".\"tables\" WHERE \"table_schema\" = 'InterestingIPs' ORDER BY \"table_name\" ASC LIMIT 1")));
$last_date = array_shift(@pg_fetch_assoc(@pg_query($pg, "SELECT \"table_name\" FROM \"information_schema\".\"tables\" WHERE \"table_schema\" = 'InterestingIPs' ORDER BY \"table_name\" DESC LIMIT 1")));

if($date && $date_check == 1)
{
	$date = date('Y-m-d', $date);
	$success = true;
}
else
{
	$date = $last_date;
	$success = false;
}

$date_checker = array(
	"date_checker" => array(
		"success" => $success,
		"date" => $date,
		"first_date" => $first_date,
		"last_date" => $last_date
	)
);

echo json_encode($date_checker);
?>
