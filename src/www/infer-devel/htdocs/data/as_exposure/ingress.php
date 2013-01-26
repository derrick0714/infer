<?php
require_once('../essentials.class.php');
require_once('../pg.include.php');

$essentials = new essentials();

$date = $essentials->dateParser($argv[1], strtotime(date('Y-m-d', time() - 86400)));
$ht = @pg_query($pg, "SELECT \"as\".\"asn\", \"asName\", \"asDescription\", \"as\".\"internal_hosts_contacted\", \"as\".\"ingress_bytes\", \"as\".\"egress_bytes\" FROM \"Maps\".\"asNames\" INNER JOIN (SELECT * FROM \"ASExposure\".\"".date('Y-m-d', $date)."\" WHERE asn != '0' ORDER BY \"ingress_bytes\" DESC LIMIT 10) AS \"as\" ON \"asNumber\" = \"as\".\"asn\"");

if($ht)
{
	while($ht_row = pg_fetch_assoc($ht))
	{
		$asn[] = $ht_row['asn'];
		$asn_name[] = $ht_row['asName'];
		$asn_desc[] = $ht_row['asDescription'];
		$internal_hosts_contacted[] = $ht_row['internal_hosts_contacted'];
		$ingress[] = $ht_row['ingress_bytes'];
		$egress[] = $ht_row['egress_bytes'];
	}
}

$exposure = array(
	"network_exposure" => array(
		"type" => "as",
		"direction" => "ingress",
		"entities" => array(
			"asn" => array_map('intval', $asn),
			"asn_name" => $asn_name,
			"asn_desc" => $asn_desc,
			"internal_hosts_contacted" => array_map('intval', $internal_hosts_contacted),
			"ingress" => array_map('intval', $ingress),
			"egress" => array_map('intval', $egress)
		)
	)
);

echo json_encode($exposure);
?>
