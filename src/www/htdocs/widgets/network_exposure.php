<?php
$argv = array_merge(array($_SERVER['SCRIPT_NAME']), explode('/', trim($_SERVER['PATH_INFO'], '/')));
$argc = sizeof($argv);

$network_exposure_widget = array(
	"widget" => array(
		"window" => array(
			"name" => "network_exposure_window",
			"title" => "Network exposure",
			"refresh" => 3600,
			"width" => 500,
			"height" => 300
		),
		"data" => array(
			"exposure_uri" => "/data/network_exposure/as.php"
		)
	)
);

echo json_encode($network_exposure_widget);
?>