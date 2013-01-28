<?php
$argv = array_merge(array($_SERVER['SCRIPT_NAME']), explode('/', trim($_SERVER['PATH_INFO'], '/')));
$argc = sizeof($argv);

$host_inventory_widget = array(
	"widget" => array(
		"window" => array(
			"name" => "host_inventory_window",
			"title" => "Host roles in this network",
			"refresh" => 3600,
			"width" => 500,
			"height" => 300
		),
		"data" => array(
			"role_uris" => array(
				"/data/role/web_server.php",
				"/data/role/web_client.php",
				"/data/role/mail_server.php",
				"/data/role/mail_client.php",
				"/data/role/secure_web_server.php",
				"/data/role/secure_web_client.php",
				"/data/role/secure_mail_server.php",
				"/data/role/secure_mail_client.php"
			)
		)
	)
);

echo json_encode($host_inventory_widget);
?>