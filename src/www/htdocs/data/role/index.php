<?php
require_once('../essentials.class.php');
require_once('../pg.include.php');

$essentials = new essentials();

$date = $essentials->dateParser($argv[1], strtotime(date('Y-m-d', time() - 86400)));
$page = is_numeric($_GET['page']) && $_GET['rp'] >= 0 ? intval($_GET['page']) : 1;
$rp = is_numeric($_GET['rp']) && $_GET['rp'] >= 0 ? intval($_GET['rp']) : 10;
$sortname = $_GET['sortname'] && in_array($_GET['sortname'], array('name', 'num_hosts', 'num_bytes', 'start_time', 'end_time')) ? $_GET['sortname'] : 'num_hosts';
$sortorder = $_GET['sortorder'] == 'asc' ? 'ASC' : 'DESC';

$total = 0;
$rows = array();

if ($page != 0 && $rp != 0) {
	function ports(&$interestingPorts, &$ports, &$initiator, $column, $delimiter) {
		if ($initiator == 1) {
			$column = '"outbound' . $column . '"';
		}   
		else {
			$column = '"inbound' . $column . '"';
		}   
		foreach ($ports as &$port) {
			$query[] = $column . '[' . ($interestingPorts[$port] + 1) . ']';
		}   
		return implode($delimiter, $query);
	}


	$query = 'SELECT "interestingPorts" FROM "Indexes"."interestingPorts" WHERE "date" = \'' . date('Y-m-d', $date) . '\'';
	$result = pg_query($pg, $query);
	$row = pg_fetch_assoc($result);
	$interesting_ports = array_flip(explode(',', substr($row['interestingPorts'], 1, -1)));

	$query = 'SELECT * FROM "Maps"."monitoredServices"';
	$result = pg_query($pg, $query);
	while ($row = pg_fetch_assoc($result)) {
		$monitored_service = array(
			'name' => $row['name'],
			'ports' => explode(',', substr($row['ports'], 1, -1)),
			'initiator' => $row['initiator']
		);

		$subquery .=
				"SELECT COUNT(*) as num_hosts, " .
					   "'" . $monitored_service['name'] . "' as name, " .

					   "SUM(" . ports($interesting_ports,
					   				  $monitored_service['ports'],
									  $monitored_service['initiator'],
									  'PortTraffic',
									  '+') . ') as num_bytes, ' .
					   "MIN(LEAST(" . ports($interesting_ports,
					   						$monitored_service['ports'],
											$monitored_service['initiator'],
											'PortFirstActivityTimes',
											',') . ')) as start_time, ' .
					   "MAX(GREATEST(" . ports($interesting_ports,
					   						   $monitored_service['ports'],
											   $monitored_service['initiator'],
											   'PortLastActivityTimes',
											   ',') . ')) as end_time ' .
				"FROM \"HostTraffic\".\"" . date('Y-m-d', $date) . '" ' .
				"WHERE " . ports($interesting_ports,
					   				  $monitored_service['ports'],
									  $monitored_service['initiator'],
									  'PortTraffic',
									  '+') . " > '0' " .
			"UNION ";
	}

	$subquery .=
			"SELECT COUNT(DISTINCT ip) as num_hosts, " .
				   "rn.role_name as name, " .
				   "SUM(\"numBytes\") AS num_bytes, " .
				   "MIN(\"startTime\") AS start_time, " .
				   "MAX(\"endTime\") AS end_time " .
			"FROM \"Roles\".\"" . date('Y-m-d', $date) . "\" AS r " .
			"INNER JOIN \"Maps\".role_names AS rn " .
			"ON r.role = rn.role_id " .
			"GROUP BY name " .
		"";
		/*
		"UNION " .
			"SELECT COUNT(DISTINCT \"internalIP\") AS num_hosts, " .
				   "'Infected Contacts' AS name, " .
				   "CAST(CAST(SUM(\"numBytes\") AS text) AS uint64) AS num_bytes, " .
				   "MIN(\"startTime\") AS start_time, " .
				   "MAX(\"endTime\") AS end_time " .
			"FROM \"InfectedContacts\".\"" . date('Y-m-d', $date) . "\" " .
		"UNION " .
			"SELECT COUNT(DISTINCT \"internalIP\") AS num_hosts, " .
				   "'Evasive Traffic' AS name, " .
				   "CAST(CAST(SUM(\"numBytes\") AS text) AS uint64) AS num_bytes, " .
				   "MIN(\"startTime\") AS start_time, " .
				   "MAX(\"endTime\") AS end_time " .
			"FROM \"EvasiveTraffic\".\"" . date('Y-m-d', $date) . "\" " .
		"UNION " .
			"SELECT \"sourceIP\" AS internal_ip, " .
				   "'Dark Space Source' AS name, " .
				   "CAST(CAST(COUNT(DISTINCT \"destinationIP\") AS text) AS uint64) AS num_contacted, " .
				   "CAST(CAST(SUM(\"numBytes\") AS text) AS uint64) AS num_bytes, " .
				   "MIN(\"startTime\") AS start_time, " .
				   "MAX(\"endTime\") AS end_time " .
			"FROM \"DarkSpaceSources\".\"" . date('Y-m-d', $date) . "\" " .
			"WHERE \"sourceIP\" = '" . $host . "' " .
			"GROUP BY \"sourceIP\" " .
		"UNION " .
			"SELECT \"internalIP\" AS internal_ip, " .
				   "'Dark Space Target' AS name, " .
				   "CAST(CAST(COUNT(DISTINCT \"externalIP\") AS text) AS uint64) AS num_contacted, " .
				   "CAST(CAST(SUM(\"numBytes\") AS text) AS uint64) AS num_bytes, " .
				   "MIN(\"startTime\") AS start_time, " .
				   "MAX(\"endTime\") AS end_time " .
			"FROM \"DarkSpaceTargets\".\"" . date('Y-m-d', $date) . "\" " .
			"WHERE \"internalIP\" = '" . $host . "' " .
			"GROUP BY \"internalIP\" " .
		"UNION " .
			"SELECT \"internalIP\" AS internal_ip, " .
				   "'Protocol Violations' AS name, " .
				   "CAST(CAST(COUNT(DISTINCT \"externalIP\") AS text) AS uint64) AS num_contacted, " .
				   "CAST(CAST(SUM(\"numBytes\") AS text) AS uint64) AS num_bytes, " .
				   "MIN(\"startTime\") AS start_time, " .
				   "MAX(\"endTime\") AS end_time " .
			"FROM \"NonDNSTraffic\".\"" . date('Y-m-d', $date) . "\" " .
			"WHERE \"internalIP\" = '" . $host . "' " .
			"GROUP BY \"internalIP\" ";
		*/

	$query =
		"SELECT * from (" . $subquery . ") as foo " .
		"ORDER BY \"" . $sortname . "\" " . $sortorder . " " .
		"LIMIT " . $rp . " " .
		"OFFSET " . (($page - 1) * $rp);
	/*
	echo "<pre>";
	echo $query . "\n";
	*/
	$pi = pg_query($query);
	$total = @pg_num_rows(@pg_query($pg, $subquery));
	if($pi)
	{
		while($pi_row = @pg_fetch_assoc($pi))
		{
			$rows[] = array(
				"id" => "_url_".str_replace(" ", "_", strtolower($pi_row['name'])),
				"cell" => array(
					$pi_row['name'],
					number_format(intval($pi_row['num_hosts'])),
					$essentials->size_display(intval($pi_row['num_bytes'])),
					$essentials->time_display($pi_row['start_time']),
					$essentials->time_display($pi_row['end_time'])));
		}
	}
}

$col_model = array(
	array(
		 'display'=> 'Indicator',
		 'name'=> 'name',
		 'width'=> 150,
		 'sortable'=> true,
		 'align'=> 'center',
		 'hide'=> false
	),
	array(
		 'display'=> 'Internal IPs',
		 'name'=> 'num_hosts',
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
	)
);

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
