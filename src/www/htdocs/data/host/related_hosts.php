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
				"score " .
		 "FROM \"" . $mutual_contacts_schema . "\".\"" . $date . "\" " .
		 "WHERE ip = '" . $host . "' AND " .
		 	"related_ip != '" . $host . "'" .
		 "ORDER BY score DESC";

/*
echo "<pre>";
echo $query . "\n";
*/

$json = array(
	"metadata" => array(
		"data_type" => "related_hosts",
		"ip" => $argv[1]
	),
	"table" => array(
		"caption" => "Related Hosts",
		"header" => array(
			array(
				"related_ip",
				"Related IP"
			),
			array(
				"score",
				"Score"
			)
		),
		"rows" => array()
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

while ($row = @pg_fetch_assoc($result)) {
	$json['table']['rows'][] = array(
		long2ip($row['related_ip']),
		floatval($row['score'])
	);
}

echo json_encode($json);

exit;
echo <<<EOF
{
	"metadata":{
		"data_type":"related_hosts",
		"ip":"128.238.24.31"
	},
	"table":{
		"caption":"Related Hosts",
		"header":[
			["related_ip","Related IP"],
			["score","Score"]
		],
		"rows":[
			["128.238.24.41",0.0752154],
			["128.238.2.92",0.0605226],
			["128.238.52.67",0.0584037],
			["128.238.161.149",0.0578287],
			["128.238.2.12",0.0564743],
			["128.238.243.101",0.0558575],
			["128.238.49.11",0.054828],
			["128.238.52.146",0.0482986],
			["128.238.197.33",0.0461941],
			["128.238.140.99",0.0451565],
			["128.238.197.239",0.0431926],
			["128.238.140.62",0.0429404],
			["128.238.32.22",0.0114561],
			["128.238.1.68",0.0094954],
			["128.238.2.38",0.00946019],
			["128.238.53.114",0.00756876],
			["128.238.1.69",0.00578578],
			["128.238.2.83",0.00542922],
			["128.238.2.250",0.00517013]
		]
	}
}
EOF

?>
