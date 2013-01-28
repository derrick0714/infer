<?php
require_once('../essentials.class.php');
require_once('../pg.include.php');

$essentials = new essentials();

$ip = ip2long($argv[1]);
$date = $essentials->dateParser($argv[2], strtotime(date('Y-m-d', time() - 86400)));

if($ip)
{
	$ht = @pg_fetch_assoc(@pg_query($pg, "SELECT \"topInboundPorts\", \"topInboundTraffic\" FROM \"TopPorts\".\"".date('Y-m-d', $date)."\" WHERE \"ip\" = '".$ip."' LIMIT 1"));
	
	if($ht)
	{
		$ports = explode(',', substr($ht['topInboundPorts'], 1, -1));
		$traffic = explode(',', substr($ht['topInboundTraffic'], 1, -1));
		for($i = 0; $i < count($ports); $i++)
			$breakdown[] = array($argv[1], intval($ports[$i]), floatval($traffic[$i]));
	}
}

$ingress_port_traffic = array(
	"port_traffic" => array(
		"display_name" => "Ingress Port Traffic",
		"direction" => "ingress",
		"breakdown" => $breakdown
	)
);

echo json_encode($ingress_port_traffic);
?>