<?php
require_once('../essentials.class.php');
require_once('../pg.include.php');

$essentials = new essentials();

$host = ip2long($argv[1]);
$date = $essentials->dateParser($argv[2], strtotime(date('Y-m-d', time() - 86400)));
$page = is_numeric($_GET['page']) && $_GET['rp'] >= 0 ? intval($_GET['page']) : 1;
$rp = is_numeric($_GET['rp']) && $_GET['rp'] >= 0 ? intval($_GET['rp']) : 10;
$sortname = $_GET['sortname'] && in_array($_GET['sortname'], array('name', 'num_contacted', 'num_bytes', 'start_time', 'end_time')) ? $_GET['sortname'] : 'name';
$sortorder = $_GET['sortorder'] == 'desc' ? 'DESC' : 'ASC';

$total = 0;
$rows = array();

if ($page != 0 && $rp != 0) {
	$subquery =
			"SELECT pi.\"internalIP\" as internal_ip, " .
				   "ms.name, " .
				   "CAST(CAST(COUNT(pi.\"externalIP\") AS text) AS uint64) AS num_contacted, " .
				   "CAST(CAST(SUM(pi.\"numBytes\") AS text) AS uint64) AS num_bytes, " .
				   "MIN(pi.\"startTime\") AS start_time, " .
				   "MAX(pi.\"endTime\") AS end_time " .
			"FROM \"PortIPs\".\"" . date('Y-m-d', $date) . "\" AS pi " .
			"INNER JOIN \"Maps\".\"monitoredServices\" AS ms " .
			"ON pi.\"internalIP\" = '" . $host . "' " .
				"AND ms.ports @> ARRAY[pi.port] " .
				"AND ms.initiator = pi.initiator " .
			"GROUP BY pi.\"internalIP\", " .
					 "ms.name " .
		"UNION " .
			"SELECT ip as internal_ip, " .
				   "rn.role_name as name, " .
				   "CAST(CAST(\"numHosts\" AS text) AS uint64) as num_contacted, " .
				   "\"numBytes\" AS num_bytes, " .
				   "\"startTime\" AS start_time, " .
				   "\"endTime\" AS end_time " .
			"FROM \"Roles\".\"" . date('Y-m-d', $date) . "\" AS r " .
			"INNER JOIN \"Maps\".role_names AS rn " .
			"ON r.role = rn.role_id " .
			"WHERE ip = '" . $host . "' " .
		"UNION " .
			"SELECT \"internalIP\" AS internal_ip, " .
				   "'Infected Contacts' AS name, " .
				   "CAST(CAST(COUNT(DISTINCT \"externalIP\") AS text) AS uint64) AS num_contacted, " .
				   "CAST(CAST(SUM(\"numBytes\") AS text) AS uint64) AS num_bytes, " .
				   "MIN(\"startTime\") AS start_time, " .
				   "MAX(\"endTime\") AS end_time " .
			"FROM \"InfectedContacts\".\"" . date('Y-m-d', $date) . "\" " .
			"WHERE \"internalIP\" = '" . $host . "' " .
			"GROUP BY \"internalIP\" " .
		"UNION " .
			"SELECT \"internalIP\" AS internal_ip, " .
				   "'Evasive Traffic' AS name, " .
				   "CAST(CAST(COUNT(DISTINCT \"externalIP\") AS text) AS uint64) AS num_contacted, " .
				   "CAST(CAST(SUM(\"numBytes\") AS text) AS uint64) AS num_bytes, " .
				   "MIN(\"startTime\") AS start_time, " .
				   "MAX(\"endTime\") AS end_time " .
			"FROM \"EvasiveTraffic\".\"" . date('Y-m-d', $date) . "\" " .
			"WHERE \"internalIP\" = '" . $host . "' " .
			"GROUP BY \"internalIP\" " .
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
			$rows[] = array("id" => "_url_".str_replace(" ", "_", strtolower($pi_row['name'])), "cell" => array($pi_row['name'], intval($pi_row['num_contacted']), $essentials->size_display(intval($pi_row['num_bytes'])), $essentials->time_display($pi_row['start_time']), $essentials->time_display($pi_row['end_time'])));
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
		 'display'=> 'IPs',
		 'name'=> 'num_contacted',
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
