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
		'display' => 'External IP',
		'name' => 'external_ip',
		'width' => 150,
		'sortable' => true,
		'align' => 'center',
		'hide' => false
	),
	array(
		'display' => 'Relevance',
		'name' => 'relevance',
		'width' => 150,
		'sortable' => true,
		'align' => 'center',
		'hide' => false
	),
	array(
		'display' => 'AS',
		'name' => 'as',
		'width' => 150,
		'sortable' => true,
		'align' => 'center',
		'hide' => false
	),
	array(
		'display' => 'Country',
		'name' => 'country_name',
		'width' => 50,
		'sortable' => true,
		'align' => 'center',
		'hide' => false
	),
	array(
		'display' => 'First Occurence',
		'name' => 'start_time',
		'width' => 150,
		'sortable' => true,
		'align' => 'center',
		'hide' => false
	),
	array(
		'display' => 'Last Occurence',
		'name' => 'end_time',
		'width' => 150,
		'sortable' => true,
		'align' => 'center',
		'hide' => false
	),
	array(
		'display' => 'Number of Packets',
		'name' => 'num_packets',
		'width' => 150,
		'sortable' => true,
		'align' => 'center',
		'hide' => false
	),
	array(
		'display' => 'Number of Bytes',
		'name' => 'num_bytes',
		'width' => 150,
		'sortable' => true,
		'align' => 'center',
		'hide' => false
	),
	array(
		'display' => 'Min Packet Size',
		'name' => 'min_packet_size',
		'width' => 90,
		'sortable' => true,
		'align' => 'left',
		'hide' => false
	),
	array(
		'display' => 'Max Packet Size',
		'name' => 'max_packet_size',
		'width' => 90,
		'sortable' => true,
		'align' => 'left',
		'hide' => false
	)
);

$col_names = array();
foreach ($col_model as $col) {
	$col_names[] = $col['name'];
}
	
$sortname = $_GET['sortname'];
if (!in_array($sortname, $col_names)) {
	$sortname = 'relevance';
}

$sortorder = 'DESC';
if ($_GET['sortorder'] == 'asc') {
	$sortorder = 'ASC';
}

if ($page != 0 && $rp != 0) {
	$query =
		"SELECT foo.external_ip, " .
			   "foo.relevance, " .
			   "foo.as, " .
			   "cn.\"countryCode\" as country_code, " .
			   "cn.\"countryName\" as country_name, " .
			   "foo.start_time, " .
			   "foo.end_time, " .
			   "foo.num_packets, " .
			   "foo.num_bytes, " .
			   "foo.min_packet_size, " .
			   "foo.max_packet_size " .
		"FROM (" .
			"SELECT pi.\"externalIP\" as external_ip, " .
				   "pi.relevance, " .
				   "an.\"asDescription\" as as, " .
				   "pi.\"countryNumber\" as country, " .
				   "pi.\"startTime\" as start_time, " .
				   "pi.\"endTime\" as end_time, " .
				   "pi.\"numPackets\" as num_packets, " .
				   "pi.\"numBytes\" as num_bytes, " .
				   "pi.\"minPacketSize\" as min_packet_size, " .
				   "pi.\"maxPacketSize\" as max_packet_size " .
			"FROM \"PortIPs\".\"" . date('Y-m-d', $date) . "\" AS pi " .
			"LEFT JOIN \"Maps\".\"asNames\" AS an " .
			"ON pi.\"asNumber\" = an.\"asNumber\" " .
			"WHERE (\"internalIP\"= '" . $host . "' ) " .
				"AND (port = '443') " .
				"AND (initiator = '2') " .
		") AS foo " .
		"LEFT JOIN \"Maps\".\"countryNames\" AS cn " .
		"ON foo.country = cn.\"countryNumber\"" .
		"ORDER BY \"" . $sortname . "\" " . $sortorder . " " .
			($sortname != 'start_time'?", start_time ASC ":"");

	$total = intval(array_pop(@pg_fetch_assoc(@pg_query($pg, 
		"SELECT COUNT(*) FROM (" . $query . ") as foo"))));
	
	$query .= 'OFFSET ' . (($page - 1) * $rp) . ' ' .
			  'LIMIT ' . $rp;

	$result = pg_query($pg, $query);
	if ($result) {
		$row_number = 0;
		while ($row = @pg_fetch_assoc($result)) {
			$rows[] = array(
				"id" => $row_number++,
				"cell" => array(
					long2ip($row['external_ip']),
					number_format(floatval($row['relevance']) * 100, 2) . " %",
					$row['as'],
					$essentials->country_flag($row['country_name'],
											  $row['country_code']),
					$essentials->time_display($row['start_time']),
					$essentials->time_display($row['end_time']),
					number_format(intval($row['num_packets'])),
					$essentials->size_display(intval($row['num_bytes'])),
					$essentials->size_display(intval($row['min_packet_size'])),
					$essentials->size_display(intval($row['max_packet_size']))
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
