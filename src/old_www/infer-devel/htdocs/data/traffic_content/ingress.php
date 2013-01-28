<?php
require_once('../essentials.class.php');
require_once('../pg.include.php');

$essentials = new essentials();

$ip = $argv[1] == "*" ? false : ip2long($argv[1]);
$time_begin = $essentials->dateParser($argv[2], strtotime(date('Y-m-d', time() - 86400)));
$time_end = $essentials->dateParser($argv[3], $time_begin + 86400);

for($i = $time_begin; $i <= $time_end; $i += 86400)
{
	$ht = @pg_fetch_assoc(@pg_query($pg, "SELECT SUM(\"inboundContent\"[1]) AS \"0\", SUM(\"inboundContent\"[2]) AS \"1\", SUM(\"inboundContent\"[3]) AS \"2\", SUM(\"inboundContent\"[4]) AS \"3\", SUM(\"inboundContent\"[5]) AS \"4\", SUM(\"inboundContent\"[6]) AS \"5\", SUM(\"inboundContent\"[7]) AS \"6\", SUM(\"inboundContent\"[8]) AS \"7\", SUM(\"inboundBytes\") AS \"8\" FROM \"HostTraffic\".\"".date('Y-m-d', $i)."\"".($ip ? " WHERE \"ip\" = '$ip'" : "")));
	if (!$ht) {
		continue;
	}

	for ($j = 0; $j < count($ht) - 1; $j++) {
		$values[$j] += $ht[$j] << 14;
	}
	$values[count($ht) - 1] += $ht[8];
	/*$values[count($ht) - 1] += $ht[count($ht) - 1];
	array_pop($ht);
	$values[count($values) - 1] -= array_sum($ht);*/
}

$ingress_traffic_content = array(
	"traffic_content" => array(
		"display_name" => "Ingress Traffic",
		"direction" => "ingress",
		"breakdown" => array(
			array("Plaintext", $values[0]), 
			array("BMP image", $values[1]), 
			array("WAV audio", $values[2]), 
			array("Compressed", $values[3]), 
			array("JPEG image", $values[4]), 
			array("MP3 audio", $values[5]), 
			array("MPEG video", $values[6]), 
			array("Encrypted", $values[7]), 
			array("Unknown", $values[8])
		)
	)
);

echo json_encode($ingress_traffic_content);

?>
