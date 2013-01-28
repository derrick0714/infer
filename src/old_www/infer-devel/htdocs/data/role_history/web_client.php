<?php
require_once('../essentials.class.php');
require_once('../pg.include.php');

$essentials = new essentials();

$date = $essentials->dateParser($argv[1], strtotime(date('Y-m-d', time() - 86400)));
$daysToShow = 14;

$ms = @pg_fetch_assoc(@pg_query($pg, "SELECT \"ports\", \"initiator\" FROM \"Maps\".\"monitoredServices\" WHERE \"name\" = 'Web Client'"));
$ports = explode(',', substr($ms['ports'], 1, -1));
$column = ($ms['initiator'] == 1 ? '"outbound' : '"inbound').'PortTraffic"';

for($i = 0; $i < $daysToShow; $i++, $date -= 86400)
{
	$ip = @pg_fetch_assoc(@pg_query($pg, "SELECT \"interestingPorts\" FROM \"Indexes\".\"interestingPorts\" WHERE \"date\" = '".date('Y-m-d', $date)."'"));
	if(!$ip) continue;
	
	$indexes = array_flip(array_intersect(explode(',', substr($ip['interestingPorts'], 1, -1)), $ports));
	if(count($indexes) <= 0) continue;
	
	foreach($indexes as $index) $q[] = $column.'['.($index + 1).']';
	$where = implode(' + ', $q);

	$ht = @pg_query($pg, "SELECT COUNT(*) FROM \"HostTraffic\".\"".date('Y-m-d', $date)."\" WHERE $where > '0'");
	if($ht)
	{
		$ht = @pg_fetch_assoc($ht);
		if($ht) $history[] = array($date, $ht['count']);
		else $history[] = array($date, 0);
	}
}

$role = array(
	"role" => array(
		"name" => "web_client",
		"display_name" => "Web Client",
		"history" => $history
	)
);

echo json_encode($role);
?>