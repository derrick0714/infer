<?php
$argv = array_merge(array($_SERVER['SCRIPT_NAME']), explode('/', trim($_SERVER['PATH_INFO'], '/')));
$argc = sizeof($argv);

$app_inventory_widget = array(
	"widget" => array(
		"window" => array(
			"name" => "app_inventory_window",
			"title" => "Application Inventory",
			"refresh" => 3600,
			"width" => 500,
			"height" => 300
		),
		"data" => array(
			"inventory_uris" => array(
				"/data/app_inventory/browser.php"
			)
		)
	)
);

echo json_encode($app_inventory_widget);
?>