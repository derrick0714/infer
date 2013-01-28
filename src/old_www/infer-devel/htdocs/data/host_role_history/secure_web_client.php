<?php
require_once('../essentials.class.php');
require_once('../pg.include.php');

$essentials = new essentials();

$host = ip2long($argv[1]);
$date = $essentials->dateParser($argv[2], strtotime(date('Y-m-d', time() - 86400)));
$daysToShow = 14;

if($host)
{
	$ms = @pg_fetch_assoc(@pg_query($pg, "SELECT \"ports\", \"initiator\" FROM \"Maps\".\"monitoredServices\" WHERE \"name\" = 'Secure Web Client'"));
	$ports = explode(',', substr($ms['ports'], 1, -1));
	$direction = ($ms['initiator'] == 1 ? 'outbound' : 'inbound');

	for($i = 0; $i < $daysToShow; $i++, $date -= 86400)
	{
		$ip = @pg_fetch_assoc(@pg_query($pg, "SELECT \"interestingPorts\" FROM \"Indexes\".\"interestingPorts\" WHERE \"date\" = '".date('Y-m-d', $date)."'"));
		if(!$ip) continue;
		
		$indexes = array_flip(array_intersect(explode(',', substr($ip['interestingPorts'], 1, -1)), $ports));
		if(count($indexes) <= 0) continue;

		$ht = @pg_query($pg, "SELECT * FROM \"HostTraffic\".\"".date('Y-m-d', $date)."\" WHERE \"ip\" = '$host' LIMIT 1");
		if($ht)
		{
			$ht = @pg_fetch_assoc($ht);
			if($ht)
			{
				$ht[$direction.'PortIPs'] = array_map('intval', explode(',', substr($ht[$direction.'PortIPs'], 1, -1)));
				$ht[$direction.'PortTraffic'] = array_map('intval', explode(',', substr($ht[$direction.'PortTraffic'], 1, -1)));
				$ht[$direction.'PortFirstActivityTimes'] = array_map('intval', explode(',', substr($ht[$direction.'PortFirstActivityTimes'], 1, -1)));
				$ht[$direction.'PortLastActivityTimes'] = array_map('intval', explode(',', substr($ht[$direction.'PortLastActivityTimes'], 1, -1)));
				
				$external_hosts_contacted = 0;
				$traffic_sum = 0;
				$first_activity_times = array();
				$last_activity_times = array();
				foreach($indexes as $index)
				{
					$external_hosts_contacted += $ht[$direction.'PortIPs'][$index];
					$traffic_sum += $ht[$direction.'PortTraffic'][$index];
					$first_activity_times[] = $ht[$direction.'PortFirstActivityTimes'][$index];
					$last_activity_times[] = $ht[$direction.'PortLastActivityTimes'][$index];
				}
				
				$history[] = array($date, $external_hosts_contacted, $traffic_sum, min($first_activity_times), max($last_activity_times));
			}
			else
			{
				$history[] = array($date, 0, 0, 0, 0);
			}
		}
	}
}

$role = array(
	"role" => array(
		"name" => "secure_web_client",
		"display_name" => "Secure Web Client",
		"history" => $history
	)
);

echo json_encode($role);
?>