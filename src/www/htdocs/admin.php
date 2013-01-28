<?php

require_once('data/essentials.class.php');
require_once('include/infer_session.php');

$infer_session = new InferSession();
if (!$infer_session->logged_in()) {
	$infer_session->redirect_to_login();
}

if ($_SESSION['privileges'] !== 0xffff) {
	header("HTTP/1.1 401 Unauthorized");
	echo "<http><head><title>401 Unauthorized</title></head>" .
		"<body><h1>401 Unauthorized</h1></body></http>";
	exit;
}

$page_name = 'admin';
$page_display = 'Admin Control Panel';
$default_hash_path = '/alerts';

$mid_row_tabs = array(
	'dashboard' => 'Dashboard',
	'incidents' => 'Incidents',
	'search' => 'Search'
);
$bottom_row_tabs = array(
	'alerts' => 'Alerts',
	'users' => 'Users'
);

require('page_template.php');

?>
