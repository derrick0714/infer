<?php
require_once('../essentials.class.php');
require_once('../pg.include.php');

$essentials = new essentials();

$ip = $argv[1] == "*" ? false : ip2long($argv[1]);
$time_begin = $essentials->dateParser($argv[2], strtotime(date('Y-m-d', time() - 86400)));
$time_end = $essentials->dateParser($argv[3], $time_begin + 86400);

$ht = @pg_query($pg, "SELECT \"interval_start\", \"ingress_bytes_per_second\" FROM \"Stats\".\"".($ip ? "host_" : "")."bandwidth_utilization\" WHERE ".($ip ? "\"host\" = '$ip' AND " : "")."\"interval_start\" >= '".$time_begin."' AND \"interval_start\" < '".$time_end."' ORDER BY \"interval_start\" ASC");

if($ht)
{
	while($ht_row = pg_fetch_assoc($ht))
	{
		$history[] = array_map('floatval', array_values($ht_row));
	}
}

$ingress_bandwidth_utilization = array(
	"bandwidth_utilization" => array(
		"direction" => "ingress",
		"history" => $history
	)
);

echo json_encode($ingress_bandwidth_utilization);
?>