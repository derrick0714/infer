<?php

require_once('data/essentials.class.php');
require_once('include/infer_session.php');

$infer_session = new InferSession();
if (!$infer_session->logged_in()) {
	$infer_session->redirect_to_login();
}

$page_name = 'host';
$page_display = 'Host';
$default_hash_path = '/events';

$mid_row_tabs = array(
	'dashboard' => 'Dashboard',
	'incidents' => 'Incidents',
	'search' => 'Search',
	null,
	'host' => 'Host'
);
$bottom_row_tabs = array(
	'events' => 'Events',
	'identity' => 'Identity',
	'related_hosts' => 'Related Hosts',
	'stats' => 'Stats',
	'indicators' => 'Indicators',
);

$tabs_date = <<<EOF
if (d != undefined) {
	$('#bottom_row_tab_events').attr('href',
									 '#/events/' + argv[1] + '/' + d);
	$('#bottom_row_tab_identity').attr('href',
									   '#/identity/' + argv[1] + '/' + d);
	$('#bottom_row_tab_related_hosts').attr('href',
									   '#/related_hosts/' + argv[1] + '/' + d);
	$('#bottom_row_tab_stats').attr('href',
									'#/stats/' + argv[1] + '/' + d);
	$('#bottom_row_tab_indicators').attr('href',
										 '#/indicators/' + argv[1] + '/' + d);
}
else {
	$('#bottom_row_tab_events').removeAttr('href');
	$('#bottom_row_tab_identity').removeAttr('href');
	$('#bottom_row_tab_related_hosts').removeAttr('href');
	$('#bottom_row_tab_stats').removeAttr('href');
	$('#bottom_row_tab_indicators').removeAttr('href');
}
EOF;

require('page_template.php');

?>
