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

if (!isset($argv[3])) {
	$sortname = 'num_sources';
	if ($_GET['sortname'] && in_array($_GET['sortname'], array('source_port',
															   'num_sources')))
	{
		$sortname = $_GET['sortname'];
	}
	$sortorder = 'DESC';
	if ($_GET['sortorder'] == 'asc') {
		$sortorder = 'ASC';
	}

	if ($page != 0 && $rp != 0) {
		$query =
			'SELECT "sourcePort" as source_port, ' .
				   'COUNT(DISTINCT "destinationIP") as num_sources ' .
			'FROM "DarkSpaceSources"."' . date('Y-m-d', $date) . '"' .
			"WHERE \"sourceIP\" = '" . $host . "' " .
			'GROUP BY source_port ' .
			'ORDER BY ' . $sortname . ' ' . $sortorder . ' ';

		$total = intval(array_pop(@pg_fetch_assoc(@pg_query($pg, 
			"SELECT COUNT(*) FROM (" . $query . ") as foo"))));
		
		$query .= 'OFFSET ' . (($page - 1) * $rp) . ' ' .
				  'LIMIT ' . $rp;

		$result = pg_query($pg, $query);
		if ($result) {
			while ($row = @pg_fetch_assoc($result)) {
				$rows[] = array(
					"id" => '_url_' . $row['source_port'],
					"cell" => array(
						$row['source_port'],
						$row['num_sources']
					)
				);
			}
		}
	}

	$col_model = array(
		array(
			 'display'=> 'Source Port',
			 'name'=> 'source_port',
			 'width'=> 150,
			 'sortable'=> true,
			 'align'=> 'center',
			 'hide'=> false
		),
		array(
			 'display'=> 'Number of Targets',
			 'name'=> 'num_sources',
			 'width'=> 150,
			 'sortable'=> true,
			 'align'=> 'center',
			 'hide'=> false
		)
	);
}
else {
	$port = $argv[3];
	$sortname = 'num_packets';
	if ($_GET['sortname'] &&
		in_array($_GET['sortname'], array('protocol',
										  'destination_ip',
										  'destination_port',
										  'country_name',
										  'start_time',
										  'end_time',
										  'num_packets',
										  'num_bytes',
										  'min_packet_size',
										  'max_packet_size')))
	{
		$sortname = $_GET['sortname'];
	}
	$sortorder = 'DESC';
	if ($_GET['sortorder'] == 'asc') {
		$sortorder = 'ASC';
	}

	if ($page != 0 && $rp != 0) {
		$query =
			'SELECT dt.protocol, ' .
				   'dt."destinationIP" as destination_ip, ' .
				   'dt."destinationPort" as destination_port, ' .
				   'cn."countryCode" as country_code, ' .
				   'cn."countryName" as country_name, ' .
				   'dt."startTime" as start_time, ' .
				   'dt."endTime" as end_time, ' .
				   'dt."numPackets" as num_packets, ' .
				   'dt."numBytes" as num_bytes, ' .
				   'dt."minPacketSize" as min_packet_size, ' .
				   'dt."maxPacketSize" as max_packet_size ' .
			'FROM "DarkSpaceSources"."' . date('Y-m-d', $date) . '" AS dt ' .
			'LEFT JOIN "Maps"."countryNames" AS cn ' .
			'ON dt."countryNumber" = cn."countryNumber" ' .
			"WHERE \"sourceIP\" = '" . $host . "' " .
				"AND " .
				  "\"sourcePort\" = '" . $port . "' " .
			'ORDER BY ' . $sortname . ' ' . $sortorder . ' ';

		$total = intval(array_pop(@pg_fetch_assoc(@pg_query($pg, 
			"SELECT COUNT(*) FROM (" . $query . ") as foo"))));
		
		$query .= 'OFFSET ' . (($page - 1) * $rp) . ' ' .
				  'LIMIT ' . $rp;

		$result = pg_query($pg, $query);
		if ($result) {
			while ($row = @pg_fetch_assoc($result)) {
				$rows[] = array(
					"id" => $row['destination_ip'],
					"cell" => array(
						$essentials->protocol_name(intval($row['protocol'])),
						long2ip($row['destination_ip']),
						$row['destination_port'],
						$essentials->country_flag($row['country_name'],
												  $row['country_code']),
						$essentials->time_display($row['start_time']),
						$essentials->time_display($row['end_time']),
						intval($row['num_packets']),
						$essentials->size_display(intval($row['num_bytes'])),
						$essentials->size_display(intval($row['min_packet_size'])),
						$essentials->size_display(intval($row['max_packet_size'])),
					)
				);
			}
		}
	}

	$col_model = array(
		array(
			 'display'=> 'Protocol',
			 'name'=> 'protocol',
			 'width'=> 150,
			 'sortable'=> true,
			 'align'=> 'center',
			 'hide'=> false
		),
		array(
			 'display'=> 'Destination IP',
			 'name'=> 'destination_ip',
			 'width'=> 150,
			 'sortable'=> true,
			 'align'=> 'center',
			 'hide'=> false
		),
		array(
			 'display'=> 'Destination Port',
			 'name'=> 'destination_port',
			 'width'=> 150,
			 'sortable'=> true,
			 'align'=> 'center',
			 'hide'=> false
		),
		array(
			 'display'=> 'Country',
			 'name'=> 'country_name',
			 'width'=> 150,
			 'sortable'=> true,
			 'align'=> 'center',
			 'hide'=> false
		),
		array(
			 'display'=> 'First Activity',
			 'name'=> 'start_time',
			 'width'=> 150,
			 'sortable'=> true,
			 'align'=> 'center',
			 'hide'=> false
		),
		array(
			 'display'=> 'Last Activity',
			 'name'=> 'end_time',
			 'width'=> 150,
			 'sortable'=> true,
			 'align'=> 'center',
			 'hide'=> false
		),
		array(
			 'display'=> 'Number of Packets',
			 'name'=> 'num_packets',
			 'width'=> 150,
			 'sortable'=> true,
			 'align'=> 'center',
			 'hide'=> false
		),
		array(
			 'display'=> 'Amount',
			 'name'=> 'num_bytes',
			 'width'=> 150,
			 'sortable'=> true,
			 'align'=> 'center',
			 'hide'=> false
		),
		array(
			 'display'=> 'Min Packet Size',
			 'name'=> 'min_packet_size',
			 'width'=> 150,
			 'sortable'=> true,
			 'align'=> 'center',
			 'hide'=> false
		),
		array(
			 'display'=> 'Max Packet Size',
			 'name'=> 'max_packet_size',
			 'width'=> 150,
			 'sortable'=> true,
			 'align'=> 'center',
			 'hide'=> false
		)
	);
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
