<?php
require_once('../essentials.class.php');
require_once('../pg.include.php');

$essentials = new essentials();

$date = $essentials->dateParser($argv[2], strtotime(date('Y-m-d', time() - 86400)));

$date = strftime("%Y-%m-%d", $date);

$host = ip2long($argv[1]);

$query = "SELECT a.ip, " .
				"a.mac, " .
				"b.names, " .
				"b.ingress_bytes, " .
				"b.egress_bytes, " .
				"b.first_activity, " .
				"b.last_activity " .
		 "FROM \"LiveIPs\".\"" . $date . "\" AS a " .
		 "LEFT JOIN " .
		 	"(SELECT c.ip, " .
					"c.\"inboundBytes\" AS ingress_bytes, " .
					"c.\"outboundBytes\" AS egress_bytes, " .
					"c.\"firstOutboundTime\" AS first_activity, " .
					"c.\"lastOutboundTime\" AS last_activity, " .
					"d.names " .
			 "FROM \"HostTraffic\".\"" . $date . "\" AS c " .
			 "LEFT JOIN \"InterestingIPs\".\"" . $date . "\" AS d " .
			 "ON c.ip = d.ip " .
			 "WHERE c.ip = '" . $host . "') AS b " .
		 "ON a.ip = b.ip " .
		 "WHERE a.ip = '" . $host . "';";

/*
echo "<pre>";
echo $query . "\n";
*/

$json = array(
	"host_physical_identity" => array(
		"names" => array(
			"owner" => "Owner",
			"title" => "Title",
			"email" => "E-Mail",
			"phone" => "Phone Number",
			"location" => "Location",
			"department" => "Department"
		),
		"values" => array()
	)
);

$result = @pg_query($pg, $query);

if ($result) {
	$row = @pg_fetch_assoc($result);
	if ($row) {
		$row['ip'] = long2ip($row['ip']);
		$row['names'] = explode(',', substr($row['names'],
											1,
											strlen($row['names']) - 2));
		$row['names'] = implode(', ', $row['names']);
		$row['ingress_bytes'] =
			$essentials->size_display($row['ingress_bytes']);
		$row['egress_bytes'] =
			$essentials->size_display($row['egress_bytes']);
		$row['first_activity'] =
			$essentials->time_display($row['first_activity']);
		$row['last_activity'] =
			$essentials->time_display($row['last_activity']);

		$json['host_network_identity']['values'] = $row;
	}
};

echo json_encode($json);

?>
