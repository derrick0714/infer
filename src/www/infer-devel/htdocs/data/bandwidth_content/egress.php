<?php
require_once('../essentials.class.php');
require_once('../pg.include.php');

$essentials = new essentials();

$ip = $argv[1] == "*" ? false : ip2long($argv[1]);
$time_begin = $essentials->dateParser($argv[2], strtotime(date('Y-m-d', time() - 86400)));
$time_end = $essentials->dateParser($argv[3], $time_begin + 86400);

$ht = @pg_query($pg, "SELECT \"interval_start\", \"egress_bytes_per_second_plaintext\", \"egress_bytes_per_second_bmp_image\", \"egress_bytes_per_second_wav_audio\", \"egress_bytes_per_second_compressed\", \"egress_bytes_per_second_jpeg_image\", \"egress_bytes_per_second_mp3_audio\", \"egress_bytes_per_second_mpeg_video\", \"egress_bytes_per_second_encrypted\", \"egress_bytes_per_second_unknown\" FROM \"Stats\".\"".($ip ? "host_" : "")."bandwidth_content\" WHERE ".($ip ? "\"host\" = '$ip' AND " : "")."\"interval_start\" >= '".$time_begin."' AND \"interval_start\" < '".$time_end."' ORDER BY \"interval_start\" ASC");

if($ht)
{
	while($ht_row = pg_fetch_assoc($ht))
	{
		$history[] = array_map('intval', array_values($ht_row));
	}
}

$egress_bandwidth_content = array(
	"bandwidth_content" => array(
		"direction" => "egress",
		"history" => $history
	)
);

echo json_encode($egress_bandwidth_content);
?>