<?php
$argv = array_merge(array($_SERVER['SCRIPT_NAME']), explode('/', trim($_SERVER['PATH_INFO'], '/')));
$argc = sizeof($argv);

$network_traffic_widget = array(
	"widget" => array(
		"window" => array(
			"name" => "network_traffic_window",
			"title" => "Network traffic summary",
			"refresh" => 3600,
			"width" => 500,
			"height" => 300
		),
		"data" => array(
			"traffic_content" => array(
				"ingress_uri" => "/data/traffic_content/ingress.php",
				"egress_uri" => "/data/traffic_content/egress.php"
			),
			"bandwidth_utilization" => array(
				"ingress_uri" => "/data/bandwidth_utilization/ingress.php",
				"egress_uri" => "/data/bandwidth_utilization/egress.php"
			)
		)
	)
);

echo json_encode($network_traffic_widget);
?>