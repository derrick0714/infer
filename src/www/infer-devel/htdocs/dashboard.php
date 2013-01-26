<?php

require_once('include/infer_session.php');
require_once('data/essentials.class.php');

$infer_session = new InferSession();
if (!$infer_session->logged_in()) {
	$infer_session->redirect_to_login();
}

$page_name = 'dashboard';
$page_display = 'Dashboard';
$default_hash_path = '/stats';

$mid_row_tabs = array(
	'dashboard' => 'Dashboard',
	'incidents' => 'Incidents',
	'search' => 'Search'
);

$bottom_row_tabs = array(
	'stats' => 'Stats',
	'indicators' => 'Indicators',
);

$tabs_date = <<<EOF
if (d != undefined) {
	$('#bottom_row_tab_stats').attr('href',
									'#/stats/' + d);
	$('#bottom_row_tab_indicators').attr('href',
										 '#/indicators/' + d);
}
else {
	$('#bottom_row_tab_stats').removeAttr('href');
	$('#bottom_row_tab_indicators').removeAttr('href');
}
EOF;

require('page_template.php');

?>
