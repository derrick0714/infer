<?php
require_once('../essentials.class.php');
require_once('../pg.include.php');

$essentials = new essentials();

$date = $essentials->dateParser($argv[1], strtotime(date('Y-m-d', time() - 86400)));

$page = is_numeric($_GET['page']) && $_GET['rp'] >= 0 ? intval($_GET['page']) : 1;
$rp = is_numeric($_GET['rp']) && $_GET['rp'] >= 0 ? intval($_GET['rp']) : 10;

$total = 0;
$rows = array();

/*
if (isset($argv[1])) {
}
else {
*/
	$col_model = array(
		array(
			'display' => 'Internal IP',
			'name' => 'internal_ip',
			'width' => 150,
			'sortable' => true,
			'align' => 'center',
			'hide' => false
		),
		array(
			'display' => 'Hosts Contacted',
			'name' => 'num_hosts',
			'width' => 150,
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
		$sortname = 'num_packets';
	}

	$sortorder = 'DESC';
	if ($_GET['sortorder'] == 'asc') {
		$sortorder = 'ASC';
	}
	
	$query =
		"SELECT pi.\"internalIP\" as internal_ip, " .
			   "COUNT(DISTINCT pi.\"externalIP\") as num_hosts, " .
			   "MIN(pi.\"startTime\") as start_time, " .
			   "MAX(pi.\"endTime\") as end_time, " .
			   "SUM(pi.\"numPackets\") as num_packets, " .
			   "SUM(pi.\"numBytes\") as num_bytes, " .
			   "MIN(pi.\"minPacketSize\") as min_packet_size, " .
			   "MAX(pi.\"maxPacketSize\") as max_packet_size " .
		"FROM \"PortIPs\".\"" . date('Y-m-d', $date) . "\" AS pi " .
		"WHERE (ARRAY[port] <@ " .
				"ARRAY['465'::uint16, '993'::uint16, '995'::uint16]) " .
			"AND (initiator = '2') " .
		"GROUP BY internal_ip " .
		"ORDER BY \"" . $sortname . "\" " . $sortorder . " ";

	$total = intval(array_pop(@pg_fetch_assoc(@pg_query($pg, 
		"SELECT COUNT(*) FROM (" . $query . ") as foo"))));
	
	$query .= 'OFFSET ' . (($page - 1) * $rp) . ' ' .
			  'LIMIT ' . $rp;
		
	$pi = pg_query($pg, $query);

	if($pi)
	{
		while($row = @pg_fetch_assoc($pi))
		{
			$rows[] = array(
				"id" => '_url_' . long2ip($row['internal_ip']),
				"cell" => array(
					long2ip($row['internal_ip']),
					number_format(intval($row['num_hosts'])),
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
//}

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
