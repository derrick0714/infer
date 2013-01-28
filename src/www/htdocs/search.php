<?php
require_once('include/infer_session.php');
require_once('data/essentials.class.php');
require_once('data/pg.include.php');

$infer_session = new InferSession();
if (!$infer_session->logged_in()) {
	$infer_session->redirect_to_login();
}


$page_name =trim($argv[0],'/'); 
$page_display =ucfirst(trim($argv[0],'/')); 
$default_hash_path = '/queries';

$mid_row_tabs = array(
	'dashboard' => 'Dashboard',
	'incidents' => 'Incidents',
	'search' => 'Search'
	);
$bottom_row_tabs = array(
	'new' => 'New',
	'queries' => 'Queries'
	);
require('page_template.php');
?>

