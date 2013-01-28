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
	// give back a choice...
	$col_model = array(
		array(
			'display' => 'Display Method',
			'name' => 'display_method',
			'width' => 150,
			'sortable' => false,
			'align' => 'center',
			'hide' => false
		)
	);

	$rows = array(
		array(
			'id' => '_url_by_ip',
			'cell' => array(
				'View by IP address'
			)
		),
		array(
			'id' => '_url_by_host',
			'cell' => array(
				'View by host name'
			)
		)
	);

	$role = array(
		"col_model" => $col_model,
		"sort_name" => '',
		"sort_order" => '',
		"page" => 1,
		"total" => 1,
		"rows" => $rows
	);

	echo json_encode($role);
	exit;
}

switch ($argv[3]) {
  case 'by_ip':
	if (isset($argv[5])) {
		$external_ip = ip2long($argv[4]);
		$http_type = strtolower($argv[5]);

		switch ($http_type) {
		  case 'requests':
			$col_model = array(
				array(
					'display' => 'Time',
					'name' => 'time',
					'width' => 150,
					'sortable' => true,
					'align' => 'center',
					'hide' => false
				),
				array(
					'display' => 'Type',
					'name' => 'type',
					'width' => 50,
					'sortable' => true,
					'align' => 'center',
					'hide' => false
				),
				array(
					'display' => 'URI',
					'name' => 'uri',
					'width' => 150,
					'sortable' => true,
					'align' => 'center',
					'hide' => false
				),
				array(
					'display' => 'Version',
					'name' => 'version',
					'width' => 150,
					'sortable' => true,
					'align' => 'center',
					'hide' => false
				),
				array(
					'display' => 'Host',
					'name' => 'host',
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
					'display' => 'Referer',
					'name' => 'referer',
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
				$sortname = 'time';
			}

			$sortorder = 'ASC';
			if ($_GET['sortorder'] == 'desc') {
				$sortorder = 'DESC';
			}

			if ($page != 0 && $rp != 0) {
				$query =
					"SELECT time_seconds as time, " .
						   "type, " .
						   "uri, " .
						   "version, " .
						   "host, " .
						   "user_agent, " .
						   "referer " .
					"FROM \"HTTPRequests\".\"" . date('Y-m-d', $date) . "\" " .
					"WHERE (source_ip = '" . $host . "') " .
						"AND (destination_ip = '" . $external_ip . "') " .
					"ORDER BY \"" . $sortname . "\" " . $sortorder . " ";

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
								$essentials->time_display($row['time']),
								$row['type'],
								$row['uri'],
								$row['version'],
								$row['host'],
								$row['user_agent'],
								$row['referer']
							)
						);
					}
				}
			}
			break;
		  case 'responses':
			$col_model = array(
				array(
					'display' => 'Time',
					'name' => 'time',
					'width' => 150,
					'sortable' => true,
					'align' => 'center',
					'hide' => false
				),
				array(
					'display' => 'Version',
					'name' => 'version',
					'width' => 150,
					'sortable' => true,
					'align' => 'center',
					'hide' => false
				),
				array(
					'display' => 'Status',
					'name' => 'status',
					'width' => 50,
					'sortable' => true,
					'align' => 'center',
					'hide' => false
				),
				array(
					'display' => 'Reason',
					'name' => 'reason',
					'width' => 150,
					'sortable' => true,
					'align' => 'center',
					'hide' => false
				),
				array(
					'display' => 'Response',
					'name' => 'response',
					'width' => 150,
					'sortable' => true,
					'align' => 'center',
					'hide' => false
				),
				array(
					'display' => 'Content Type',
					'name' => 'content_type',
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
				$sortname = 'time';
			}

			$sortorder = 'ASC';
			if ($_GET['sortorder'] == 'desc') {
				$sortorder = 'DESC';
			}

			if ($page != 0 && $rp != 0) {
				$query =
					"SELECT time_seconds as time, " .
						   "version, " .
						   "status, " .
						   "reason, " .
						   "response, " .
						   "content_type " .
					"FROM \"HTTPResponses\".\"" . date('Y-m-d', $date) . "\" " .
					"WHERE (source_ip = '" . $external_ip . "') " .
						"AND (destination_ip = '" . $host. "') " .
					"ORDER BY \"" . $sortname . "\" " . $sortorder . " ";

				$total = intval(array_pop(@pg_fetch_assoc(@pg_query($pg, 
					"SELECT COUNT(*) FROM (" . $query . ") as foo"))));
				
				$query .= 'OFFSET ' . (($page - 1) * $rp) . ' ' .
						  'LIMIT ' . $rp;

				$result = pg_query($pg, $query);
				if ($result) {
					$row_number = 0;
					while ($row = @pg_fetch_assoc($result)) {
						$rows[] = array(
							"id" => sprintf("%09d", $row_number++),
							"cell" => array(
								$essentials->time_display($row['time']),
								$row['version'],
								$row['status'],
								$row['reason'],
								$row['response'],
								$row['content_type']
							)
						);
					}
				}
			}
			break;
		  default:
		};
	}
	else if (isset($argv[4])) {
		$external_ip = ip2long($argv[4]);

		$col_model = array(
			array(
				'display' => 'Type',
				'name' => 'type',
				'width' => 150,
				'sortable' => true,
				'align' => 'center',
				'hide' => false
			),
			array(
				'display' => 'Count',
				'name' => 'count',
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
			$sortname = 'type';
		}

		$sortorder = 'ASC';
		if ($_GET['sortorder'] == 'desc') {
			$sortorder = 'DESC';
		}
			
		if ($page != 0 && $rp != 0) {
			$query =
					"SELECT 'Requests' as type, " .
							"COUNT(*) " .
					"FROM \"HTTPRequests\".\"" . date('Y-m-d', $date) . "\" " .
					"WHERE (source_ip = '" . $host . "') " .
						"AND (destination_ip = '" . $external_ip . "') " .
				"UNION " .
					"SELECT 'Responses' as type, " .
							"COUNT(*) " .
					"FROM \"HTTPResponses\".\"" . date('Y-m-d', $date) . "\" " .
					"WHERE (source_ip = '" . $external_ip . "') " .
						"AND (destination_ip = '" . $host . "') " .
				"ORDER BY \"" . $sortname . "\" " . $sortorder . " ";

			$total = intval(array_pop(@pg_fetch_assoc(@pg_query($pg, 
				"SELECT COUNT(*) FROM (" . $query . ") as foo"))));
			
			$query .= 'OFFSET ' . (($page - 1) * $rp) . ' ' .
					  'LIMIT ' . $rp;

			$result = pg_query($pg, $query);
			if ($result) {
				while ($row = @pg_fetch_assoc($result)) {
					$rows[] = array(
						"id" => '_url_' . strtolower($row['type']),
						"cell" => array(
							$row['type'],
							intval($row['count'])
						)
					);
				}
			}
		}
	}
	else {
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
					"AND (port = '80') " .
					"AND (initiator = '1') " .
			") AS foo " .
			"LEFT JOIN \"Maps\".\"countryNames\" AS cn " .
			"ON foo.country = cn.\"countryNumber\"" .
			"ORDER BY \"" . $sortname . "\" " . $sortorder . " " .
				($sortname != 'start_time'?", start_time ASC ":"") .
			"LIMIT " . $rp . " " .
			"OFFSET " . (($page - 1) * $rp);
			
		$pi = pg_query($pg, $query);
		$total = intval(array_pop(@pg_fetch_assoc(@pg_query($pg, "SELECT COUNT(*) FROM \"PortIPs\".\"".date('Y-m-d', $date)."\" WHERE (\"internalIP\"= '".$host."' ) AND (port= '80' ) AND (initiator= '1' )"))));

		if($pi)
		{
			while($row = @pg_fetch_assoc($pi))
			{
				$rows[] = array(
					"id" => '_url_' . long2ip($row['external_ip']),
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
	break;
  case 'by_host':
	if (isset($argv[5])) {
		$server_name = pg_escape_string($argv[4]);
		$server_ip = $argv[5];

		$col_model = array(
			array(
				'display' => 'Time',
				'name' => 'time',
				'width' => 150,
				'sortable' => true,
				'align' => 'center',
				'hide' => false
			),
			array(
				'display' => 'Type',
				'name' => 'type',
				'width' => 50,
				'sortable' => true,
				'align' => 'center',
				'hide' => false
			),
			array(
				'display' => 'URI',
				'name' => 'uri',
				'width' => 150,
				'sortable' => true,
				'align' => 'center',
				'hide' => false
			),
			array(
				'display' => 'Version',
				'name' => 'version',
				'width' => 150,
				'sortable' => true,
				'align' => 'center',
				'hide' => false
			),
			array(
				'display' => 'Host',
				'name' => 'host',
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
				'display' => 'Referer',
				'name' => 'referer',
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
			$sortname = 'time';
		}

		$sortorder = 'ASC';
		if ($_GET['sortorder'] == 'desc') {
			$sortorder = 'DESC';
		}

		if ($page != 0 && $rp != 0) {
			$query =
				"SELECT time_seconds as time, " .
					   "type, " .
					   "uri, " .
					   "version, " .
					   "host, " .
					   "user_agent, " .
					   "referer " .
				"FROM \"HTTPRequests\".\"" . date('Y-m-d', $date) . "\" " .
				"WHERE source_ip = '" . $host . "' AND " .
					"host = '" . $server_name . "' AND " .
					"destination_ip = '" . ip2long($server_ip) . "' " .
				"ORDER BY \"" . $sortname . "\" " . $sortorder . " ";

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
							$essentials->time_display($row['time']),
							$row['type'],
							$row['uri'],
							$row['version'],
							$row['host'],
							$row['user_agent'],
							$row['referer']
						)
					);
				}
			}
		}
	}
	else if (isset($argv[4])) {
		$host_name = $argv[4];

		$col_model = array(
			array(
				'display' => 'IP Address',
				'name' => 'host_ip',
				'width' => 150,
				'sortable' => true,
				'align' => 'center',
				'hide' => false
			),
			array(
				'display' => 'Number of Requests',
				'name' => 'request_count',
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
				'SELECT ' .
					'destination_ip as host_ip, ' .
					'count(*) as request_count ' .
				'FROM ' .
					'"HTTPRequests"."' . date('Y-m-d', $date) . '" ' .
				"WHERE source_ip = '" . $host . "' AND " .
					"host = '" . pg_escape_string($host_name) . "' " .
				'GROUP BY destination_ip ' .
				"ORDER BY \"" . $sortname . "\" " . $sortorder . " ";
				
			$total = intval(array_pop(@pg_fetch_assoc(@pg_query($pg, 
				"SELECT COUNT(*) FROM (" . $query . ") as foo"))));
			
			$query .= 'OFFSET ' . (($page - 1) * $rp) . ' ' .
					  'LIMIT ' . $rp;

			$result = pg_query($pg, $query);
			if($result)
			{
				while($row = @pg_fetch_assoc($result))
				{
					$rows[] = array(
						"id" => '_url_' . long2ip($row['host_ip']),
						"cell" => array(
							long2ip($row['host_ip']),
							number_format(intval($row['request_count']))
						)
					);
				}
			}
		}
	}
	else {
		$col_model = array(
			array(
				'display' => 'Host',
				'name' => 'host',
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
				'display' => 'Host IP Count',
				'name' => 'host_ip_count',
				'width' => 150,
				'sortable' => true,
				'align' => 'center',
				'hide' => false
			),
			array(
				'display' => 'Number of Requests',
				'name' => 'request_count',
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
			$sortname = 'relevance';
		}

		$sortorder = 'DESC';
		if ($_GET['sortorder'] == 'asc') {
			$sortorder = 'ASC';
		}
		
		if ($page != 0 && $rp != 0) {
			$query =
				'SELECT ' .
					'a.host, ' .
					'b.relevance, ' .
					'count(DISTINCT a.destination_ip) as host_ip_count, ' .
					'count(a.*) as request_count ' .
				'FROM ' .
					'"HTTPRequests"."' . date('Y-m-d', $date) . '" AS a, ' .
					'"HTTPHostRelevance"."' . date('Y-m-d', $date) . '" AS b ' .
				'WHERE a.host = b.host AND ' .
					"source_ip = '" . $host . "' " .
				'GROUP BY a.host, b.relevance ' .
				"ORDER BY \"" . $sortname . "\" " . $sortorder . " " .
					($sortname != 'request_count'?", request_count DESC ":"");
				
			$total = intval(array_pop(@pg_fetch_assoc(@pg_query($pg, 
				"SELECT COUNT(*) FROM (" . $query . ") as foo"))));
			
			$query .= 'OFFSET ' . (($page - 1) * $rp) . ' ' .
					  'LIMIT ' . $rp;

			$result = pg_query($pg, $query);
			if($result)
			{
				while($row = @pg_fetch_assoc($result))
				{
					$rows[] = array(
						"id" => '_url_' . $row['host'],
						"cell" => array(
							$row['host'],
							number_format(floatval($row['relevance']) * 100, 2) . " %",
							number_format(intval($row['host_ip_count'])),
							number_format(intval($row['request_count']))
						)
					);
				}
			}
		}
	}
	break;
};

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
