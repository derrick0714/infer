<?php
require_once('../essentials.class.php');
require_once('../pg.include.php');

$essentials = new essentials();

$host = ip2long($argv[1]);
$date = $essentials->dateParser($argv[2], strtotime(date('Y-m-d', time() - 86400)));
$ht = @pg_query($pg, "SELECT \"as\".\"asn\", \"asName\", \"asDescription\", \"as\".\"ingress_bytes\", \"as\".\"egress_bytes\" FROM \"Maps\".\"asNames\" INNER JOIN (SELECT * FROM \"HostASExposure\".\"".date('Y-m-d', $date)."\" WHERE \"asn\" != '0' AND \"host\" = '$host' ORDER BY \"egress_bytes\" DESC LIMIT 10) AS \"as\" ON \"asNumber\" = \"as\".\"asn\"");

if($ht)
{
	while($ht_row = pg_fetch_assoc($ht))
	{
		$asn[] = $ht_row['asn'];
		$asn_name[] = ($ht_row['asName'] != null)?$ht_row['asName']:"";
		$asn_desc[] = ($ht_row['asDescription'] != null)?$ht_row['asDescription']:"";
		$ingress[] = $ht_row['ingress_bytes'];
		$egress[] = $ht_row['egress_bytes'];
	}
}

$exposure = array(
	"network_exposure" => array(
		"type" => "as",
		"direction" => "egress",
		"entities" => array(
			"asn" => array_map('intval', $asn),
			"asn_name" => $asn_name,
			"asn_desc" => $asn_desc,
			"ingress" => array_map('intval', $ingress),
			"egress" => array_map('intval', $egress)
		)
	)
);

echo json_encode($exposure);
?>
