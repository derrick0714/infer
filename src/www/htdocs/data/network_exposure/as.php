<?php
require_once('../essentials.class.php');
require_once('../pg.include.php');

$essentials = new essentials();

$date = $essentials->dateParser($argv[1], strtotime(date('Y-m-d', time() - 86400)));

$ht = @pg_query($pg, "SELECT SUM(internal_hosts_contacted) as hosts_sum, SUM(ingress_bytes) as ingress_sum, SUM(egress_bytes) as egress_sum, count(*) as count FROM \"ASExposure\".\"".date('Y-m-d', $date)."\" WHERE asn != '0'");

$types = array(); 

if($ht)
{
	$ht_row = pg_fetch_assoc($ht);

	$count = $ht_row['count'];
	if ($count == 0) {
		break;
	}
	
	$avg_hosts = floor($ht_row['hosts_sum'] / $count);
	$avg_ingress_bytes = floor($ht_row['ingress_bytes_sum'] / $count);
	$avg_egress_bytes = floor($ht_row['egress_bytes_sum'] / $count);
	
	$types[] = array("internal_hosts_contacted", $avg_hosts);
	$types[] = array("ingress_bytes", $avg_ingress_bytes);
	$types[] = array("egress_bytes", $avg_egress_bytes);
}

foreach($types as $type)
{
	// near-average
	$ht = @pg_query($pg, "SELECT * FROM \"ASExposure\".\"".date('Y-m-d', $date)."\" WHERE asn != '0' and ".$type[0]." < '" . $type[1] . "' order by ".$type[0]." desc limit 5");

	if($ht)
	{
		while($ht_row = pg_fetch_assoc($ht))
		{
			$asn[] = $ht_row['asn'];
			$internal_hosts_contacted[] = $ht_row['internal_hosts_contacted'];
			$ingress[] = $ht_row['ingress_bytes'];
			$egress[] = $ht_row['egress_bytes'];
		}
	}

	$ht = @pg_query($pg, "SELECT * FROM \"ASExposure\".\"".date('Y-m-d', $date)."\" WHERE asn != '0' and ".$type[0]." >= '" . $type[1] . "' order by ".$type[0]." asc limit 5");

	if($ht)
	{
		while($ht_row = pg_fetch_assoc($ht))
		{
			if (isset($asn[$ht_row['asn']])) {
				continue;
			}
			$asn[] = $ht_row['asn'];
			$internal_hosts_contacted[] = $ht_row['internal_hosts_contacted'];
			$ingress[] = $ht_row['ingress_bytes'];
			$egress[] = $ht_row['egress_bytes'];
		}
	}

	// near-max
	$ht = @pg_query($pg, "SELECT * FROM \"ASExposure\".\"".date('Y-m-d', $date)."\" WHERE asn != '0' order by ".$type[0]." desc limit 5");

	if($ht)
	{
		while($ht_row = pg_fetch_assoc($ht))
		{
			if (isset($asn[$ht_row['asn']])) {
				continue;
			}
			$asn[] = $ht_row['asn'];
			$internal_hosts_contacted[] = $ht_row['internal_hosts_contacted'];
			$ingress[] = $ht_row['ingress_bytes'];
			$egress[] = $ht_row['egress_bytes'];
		}
	}

	// near-min
	$ht = @pg_query($pg, "SELECT * FROM \"ASExposure\".\"".date('Y-m-d', $date)."\" WHERE asn != '0' order by ".$type[0]." asc limit 5");

	if($ht)
	{
		while($ht_row = pg_fetch_assoc($ht))
		{
			if (isset($asn[$ht_row['asn']])) {
				continue;
			}
			$asn[] = $ht_row['asn'];
			$internal_hosts_contacted[] = $ht_row['internal_hosts_contacted'];
			$ingress[] = $ht_row['ingress_bytes'];
			$egress[] = $ht_row['egress_bytes'];
		}
	}
}
/*
$ht = @pg_query($pg, "SELECT * FROM \"ASExposure\".\"".date('Y-m-d', $date)."\" WHERE asn != '0' limit 10");

if($ht)
{
	while($ht_row = pg_fetch_assoc($ht))
	{
		if (isset($asn[$ht_row['asn']])) {
			continue;
		}
		$asn[] = $ht_row['asn'];
		$internal_hosts_contacted[] = $ht_row['internal_hosts_contacted'];
		$ingress[] = $ht_row['ingress_bytes'];
		$egress[] = $ht_row['egress_bytes'];
	}
}
*/

$exposure = array(
	"network_exposure" => array(
		"type" => "as",
		"entities" => array(
			"asn" => $asn,
			"internal_hosts_contacted" => $internal_hosts_contacted,
			"ingress" => $ingress,
			"egress" => $egress
		)
	)
);

echo json_encode($exposure);
?>
