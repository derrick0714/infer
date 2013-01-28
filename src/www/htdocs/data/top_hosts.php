<?php
require_once('essentials.class.php');
require_once('pg.include.php');

$essentials = new essentials();

$date = $essentials->dateParser($argv[1], strtotime(date('Y-m-d', time() - 86400)));

$date = strftime("%Y-%m-%d", $date);
$count = intval($argv[2]);
$limit = '';
if ($count > 0) {
	$limit = 'limit ' . $count;
}

$json = array(
	"date" => $date,
	"hosts" => array()
);

$query = "CREATE AGGREGATE array_accum (anyelement)
(
    sfunc = array_append,
	    stype = anyarray,
		    initcond = '{}'
			); ";
$result = @pg_query($pg, $query);

$query = "SELECT iip.ip," .
				"iip.names, " .
				"iip.\"infectedContactScore\" AS infected_contact_score, " .
				"iip.\"evasiveTrafficScore\" AS evasive_traffic_score, " .
				"iip.\"darkSpaceSourceScore\" AS dark_space_source_score, " .
				"iip.\"darkSpaceTargetScore\" AS dark_space_target_score, " .
				"iip.\"nonDNSTrafficScore\" AS protocol_violations_score, " .
				"iip.\"rebootScore\" AS reboots_score, " .
				"rn.role_names " .
		 "FROM \"InterestingIPs\".\"" . $date . "\" AS iip " .
		 "LEFT JOIN " .
		 	"(SELECT a.ip, array_accum(b.role_name) AS role_names " .
			 "FROM \"Roles\".\"" . $date . "\" AS a, " .
			 	  "\"Maps\".role_names AS b " .
			 "WHERE a.role = b.role_id " .
			 "GROUP BY a.ip) AS rn " .
		 "ON iip.ip = rn.ip " .
		 "ORDER BY iip.rank ASC " .
		 $limit;

/*
echo "<pre>";
echo $query . "\n";
*/
$result = @pg_query($pg, $query);

if ($result) {
	while ($row = @pg_fetch_assoc($result)) {
		$tmp_row = array(long2ip($row['ip']),
						 explode(",",
							   	 substr($row['names'],
									   	1,
										strlen($row['names']) - 2)),
						 explode("\",\"",
							     substr($row['role_names'],
									   	2,
										strlen($row['role_names']) - 4)));
		if ($tmp_row[2][0] == "") {
			$tmp_row[2] = array();
		}
		if ($row['infected_contact_score']) {
			$tmp_row[2][] = "Infected Contacts";
		}
		if ($row['evasive_traffic_score']) {
			$tmp_row[2][] = "Evasive Traffic";
		}
		if ($row['dark_space_source_score']) {
			$tmp_row[2][] = "Dark Space Source";
		}
		if ($row['dark_space_target_score']) {
			$tmp_row[2][] = "Dark Space Target";
		}
		if ($row['protocol_violations_score']) {
			$tmp_row[2][] = "Protocol Violations";
		}
		if ($row['reboots_score']) {
			$tmp_row[2][] = "Reboots";
		}
		$json[hosts][] = $tmp_row;
	}
}

echo json_encode($json);

?>
