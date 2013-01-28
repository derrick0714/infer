<?php
require_once('../essentials.class.php');
require_once('../pg.include.php');

$essentials = new essentials();

$host = ip2long($argv[1]);
$roleid = 21;
$date = $essentials->dateParser($argv[2], strtotime(date('Y-m-d', time() - 86400)));
$daysToShow = 14;

if($host)
{
	$currentDate = $date;
	for($i = 0; $i < $daysToShow; $i++, $currentDate -= 86400)
	{
		$query = @pg_query($pg, "SELECT * FROM \"Roles\".\"".date('Y-m-d', $currentDate)."\" WHERE \"ip\" = '$host' AND \"role\" = '$roleid' LIMIT 1");
		if($query)
		{
			$query = @pg_fetch_assoc($query);
			if($query) $history[] = array_map('intval', array($currentDate, $query['numHosts'], $query['numBytes'], $query['startTime'], $query['endTime']));
			else $history[] = array($currentDate, 0, 0, 0, 0);
		}
	}
}

$role = array(
	"role" => array(
		"name" => "postgresql_brute_forced",
		"display_name" => "PostgreSQL Brute Forced",
		"history" => $history
	)
);

echo json_encode($role);
?>