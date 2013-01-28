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
		'display' => 'Crawler IP',
		'name' => 'crawler_ip',
		'width' => 150,
		'sortable' => true,
		'align' => 'center',
		'hide' => false
	),
	array(
		'display' => 'User Agent',
		'name' => 'user_agent',
		'width' => 150,
		'sortable' => true,
		'align' => 'center',
		'hide' => false
	),
	array(
		'display' => 'Request Count',
		'name' => 'request_count',
		'width' => 150,
		'sortable' => true,
		'align' => 'center',
		'hide' => false
	),
	array(
		'display' => 'First Request Time',
		'name' => 'first_request_time_seconds',
		'width' => 150,
		'sortable' => true,
		'align' => 'center',
		'hide' => false
	),
	array(
		'display' => 'Last Request Time',
		'name' => 'last_request_time_seconds',
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
	$sortname = 'request_count';
}

$sortorder = 'DESC';
if ($_GET['sortorder'] == 'asc') {
	$sortorder = 'ASC';
}
	
if ($page != 0 && $rp != 0) {
	$query =
		"SELECT " .
			"client_ip, " .
			"user_agent, " .
			"request_count, " .
			"first_request_time_seconds, " .
			"last_request_time_seconds " .
		"FROM \"WebServerCrawlers\".\"" . date('Y-m-d', $date) . "\" " .
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
				"id" => '_' . strtolower(long2ip($row['client_ip']) . '_' .
										 $row['user_agent']),
				"cell" => array(
					long2ip($row['client_ip']),
					$row['user_agent'],
					intval($row['request_count']),
					$essentials->time_display(
						$row['first_request_time_seconds']),
					$essentials->time_display(
						$row['last_request_time_seconds'])
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
