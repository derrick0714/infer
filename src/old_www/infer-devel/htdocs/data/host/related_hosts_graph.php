<?php
require_once('../essentials.class.php');
require_once('../pg.include.php');

$essentials = new essentials();

$date = $essentials->dateParser($argv[2], strtotime(date('Y-m-d', time() - 86400)));

$date = strftime("%Y-%m-%d", $date);

$host = ip2long($argv[1]);

$mutual_contacts_schema = exec('/usr/local/bin/infer_config mutual_contacts.mutual-contacts-schema');
$privacy_graph_dir = exec('/usr/local/bin/infer_config privacy-graph-dir');

$query = "SELECT related_ip, " .
				"score, " .
				"shared_contacts " .
		 "FROM \"" . $mutual_contacts_schema . "\".\"" . $date . "\" " .
		 "WHERE ip = '" . $host . "' AND " .
		 	"related_ip != '" . $host . "'" .
		 "ORDER BY score DESC";

/*
echo "<pre>";
echo $query . "\n";
*/

$json = array(
	"ip" => $argv[1],
	"related_hosts_graph" => array(
		"nodes" => array(),
		"edges" => array()
	)
);

$result = @pg_query($pg, $query);

if (!$result) {
	pg_query($pg,
			 "CREATE TABLE " .
				"\"" . $mutual_contacts_schema . "\".\"" . $date . "\" (" .
					"ip uint32, " .
					"related_ip uint32, " .
					"score DOUBLE PRECISION, " .
					"shared_contacts uint32" .
				");");
			
	// error
	/*
	echo "Error!<br>";
	echo "argv[0]: " . $argv[0] . '<br>';
	echo "argv[1]: " . $argv[1] . '<br>';
	die;
	*/
}

if (!$result || pg_num_rows($result) == 0) {
	// run background program to populate db, then try again
	$cmd =
		'/usr/local/bin/infer_related_hosts --decimal ' .
			'-i "' . $privacy_graph_dir . '/' . $date . '" ' .
			$date . ' ' . $argv[1] . ' | ' .
		'/usr/local/bin/psql -q "' .
			$pg_port.' '.
			$pg_dbname.' '.
			$pg_user.' '.
			$pg_password . '" -c "\\copy \"' .
				$mutual_contacts_schema . '\".\"' . $date . '\" from STDIN"';
	
	exec($cmd);
	$result = pg_query($pg, $query);
}

$json['related_hosts_graph']['nodes'][] = array(
	"ip" => $argv[1],
	"score" => 1
);
$i = 1;
while ($row = @pg_fetch_assoc($result)) {
	$json['related_hosts_graph']['nodes'][] = array(
		"ip" => long2ip($row['related_ip']),
		"score" => floatval($row['score'])
	);
	$json['related_hosts_graph']['edges'][] = array(
		"source" => 0, 
		"target" => $i,
		"value" => intval($row['shared_contacts'])
	);
	$i++;
}

echo json_encode($json);

?>
