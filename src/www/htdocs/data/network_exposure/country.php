<?php
require_once('../essentials.class.php');
$essentials = new essentials();

$date = $essentials->dateParser($argv[1], strtotime(date('Y-m-d', time() - 86400)));

$exposure = array(
	"network_exposure" => array(
		"type" => "country",
		"entities" => array(
			array("USA", 5363, 45, 89983, 3003),
			array("Britain", 3533, 450, 899803, 22333),
			array("China", 29912, 352, 2442, 30032),
			array("Canada", 8211, 4, 8930, 9010),
			array("India", 282, 2, 883, 3453),
			array("Japan", 90912, 4, 8928, 988920)
		)
	)
);

echo json_encode($exposure);
?>