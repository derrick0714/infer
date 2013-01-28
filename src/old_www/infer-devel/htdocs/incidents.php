<?php

require_once('data/essentials.class.php');
require_once('include/infer_session.php');

$infer_session = new InferSession();
if (!$infer_session->logged_in()) {
	$infer_session->redirect_to_login();
}

$page_name = 'incidents';
$page_display = 'Incidents';
$default_hash_path = '/top_incidents';

$mid_row_tabs = array(
	'dashboard' => 'Dashboard',
	'incidents' => 'Incidents',
	'search' => 'Search'
);
$bottom_row_tabs = array(
	'top_incidents' => 'Top Incidents'
);

require('page_template.php');

?>
