<?php
require_once('../essentials.class.php');
require_once('../pg.include.php');

$essentials = new essentials();

$date = $essentials->dateParser($argv[1], strtotime(date('Y-m-d', time() - 86400)));

$rows = @pg_query($pg, "SELECT * FROM \"BrowserVersionStats\".\"".date('Y-m-d', $date)."\"");
if($rows)
{
	$browsers = array();
	while($row = pg_fetch_assoc($rows))
	{
		$browser = array_search($row['browser'], $browsers);
		if($browser === false)
		{
			$browsers[] = $row['browser'];
			$browser = array_search($row['browser'], $browsers);
		}
		$versions[$browser][] = $row['version'];
		$vhc[$browser][] = (int) $row['internal_host_count'];
	}
	
	$rows = @pg_query($pg, "SELECT * FROM \"BrowserStats\".\"".date('Y-m-d', $date)."\"");
	if($rows)
	{
		while($row = pg_fetch_assoc($rows))
		{
			$browser = array_search($row['browser'], $browsers);
			$hc[] = (int) $row['internal_host_count'];
		}
	}
}

$app_inventory = array(
	"app_inventory" => array(
		"app_type" => "browser",
		"inventory" => array(
			"name" => $browsers,
			"host_count" => $hc,
			"version" => $versions,
			"version_host_count" => $vhc
		)
	)
);

echo json_encode($app_inventory);
?>