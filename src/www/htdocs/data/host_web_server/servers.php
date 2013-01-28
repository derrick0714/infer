<?php
require_once('../essentials.class.php');
require_once('../pg.include.php');

$essentials = new essentials();

$host = ip2long($argv[1]);
$date = $essentials->dateParser($argv[2], strtotime(date('Y-m-d', time() - 86400)));

$page = is_numeric($_GET['page']) && $_GET['rp'] >= 0 ? intval($_GET['page']) : 1;
$rp = is_numeric($_GET['rp']) && $_GET['rp'] >= 0 ? intval($_GET['rp']) : 10;

$total = 0;
$rows = array();

$col_model = array(
	array(
		'display' => 'Server',
		'name' => 'server',
		'width' => 150,
		'sortable' => true,
		'align' => 'center',
		'hide' => false
	),
	array(
		'display' => 'Response Count',
		'name' => 'response_count',
		'width' => 150,
		'sortable' => true,
		'align' => 'center',
		'hide' => false
	),
	array(
		'display' => 'First Response Time',
		'name' => 'first_response_time_seconds',
		'width' => 150,
		'sortable' => true,
		'align' => 'center',
		'hide' => false
	),
	array(
		'display' => 'Last Response Time',
		'name' => 'last_response_time_seconds',
		'width' => 150,
		'sortable' => true,
		'align' => 'center',
		'hide' => false
	)
);

$col_names = array();
foreach ($col_model as $col) {
	$col_names[] = $col['name'];
}
	
$sortname = $_GET['sortname'];
if (!in_array($sortname, $col_names)) {
	$sortname = 'response_count';
}

$sortorder = 'DESC';
if ($_GET['sortorder'] == 'asc') {
	$sortorder = 'ASC';
}

if ($page != 0 && $rp != 0) {
	$query =
		"SELECT server, response_count, first_response_time_seconds, " .
			"last_response_time_seconds " .
		"FROM \"WebServerServers\".\"" . date('Y-m-d', $date) . "\" " .
		"WHERE server_ip = '" . $host . "' " .
		"ORDER BY \"" . $sortname . "\" " . $sortorder . " ";

	$total = intval(array_pop(@pg_fetch_assoc(@pg_query($pg, 
		"SELECT COUNT(*) FROM (" . $query . ") as foo"))));
	
	$query .= 'OFFSET ' . (($page - 1) * $rp) . ' ' .
			  'LIMIT ' . $rp;

	$result = pg_query($pg, $query);
	if ($result) {
		while ($row = @pg_fetch_assoc($result)) {
			$rows[] = array(
				"id" => '_' . strtolower($row['server']),
				"cell" => array(
					$row['server'],
					intval($row['response_count']),
					$essentials->time_display(
						$row['first_response_time_seconds']),
					$essentials->time_display(
						$row['last_response_time_seconds'])
				)
			);
		}
	}
}

$role = array(
	"col_model" => $col_model,
	"sort_name" => $sortname,
	"sort_order" => strtolower($sortorder),
	"page" => $page,
	"total" => $total,
	"rows" => $rows
);

echo json_encode($role);
?>
