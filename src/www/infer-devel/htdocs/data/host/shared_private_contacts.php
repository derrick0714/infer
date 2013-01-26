<?php
require_once('../essentials.class.php');
require_once('../pg.include.php');

$essentials = new essentials();

$date = $essentials->dateParser($argv[2], strtotime(date('Y-m-d', time() - 86400)));

$date = strftime("%Y-%m-%d", $date);

$host = ip2long($argv[1]);
$related_host = ip2long($argv[3]);

$shared_private_contacts_schema = exec('/usr/local/bin/infer_config shared_private_contacts.shared-private-contacts-schema');
$privacy_graph_dir = exec('/usr/local/bin/infer_config privacy-graph-dir');
$pruned_network_graph_dir = exec('/usr/local/bin/infer_config pruned-network-graph-dir');

$query = 
	"SELECT foo.ip, " .
		   "foo.related_ip, " .
		   "foo.shared_contact_ip, " .
		   "foo.shared_contact_asn, " .
		   "foo.shared_contact_as_description, " .
		   "cn.\"countryCode\" as country_code, " .
		   "cn.\"countryName\" as country_name " .
	"FROM (" .
			 "SELECT t.ip, " .
					"t.related_ip, " .
					"t.shared_contact_ip, " .
					"t.shared_contact_asn, " .
					"an.\"asDescription\" as shared_contact_as_description, " .
					"t.shared_contact_country " .
			 "FROM " .
				"\"" . $shared_private_contacts_schema . "\".\"" . $date . "\" " .
				"AS t " .
			 "LEFT JOIN \"Maps\".\"asNames\" AS an " .
			 "ON t.shared_contact_asn = an.\"asNumber\" " .
			 "WHERE t.ip = '" . $host . "' AND " .
				"t.related_ip = '" . $related_host . "' " .
	") AS foo " .
	"LEFT JOIN \"Maps\".\"countryNames\" AS cn " .
	"ON foo.shared_contact_country = cn.\"countryNumber\" " .
	"ORDER BY shared_contact_ip";

/*
echo "<pre>";
echo $query . "\n";
*/

$json = array(
	"metadata" => array(
		"data_type" => "shared_private_contacts",
		"ip" => $argv[1],
		"related_ip" => $argv[3]
	),
	"table" => array(
		"caption" => "Private Contacts shared with " . $argv[3],
		"header" => array(
		/*
			array(
				"ip",
				"IP"
			),
			array(
				"related_ip",
				"Related IP"
			),
		*/
			array(
				"shared_contact",
				"Shared Contact"
			),
			array(
				"shared_contact_as_description",
				"AS Description"
			),
			array(
				"shared_contact_country_flag",
				"Country"
			)
		),
		"rows" => array()
	)
);

$result = @pg_query($pg, $query);

if (!$result) {
	pg_query($pg,
			 "CREATE TABLE " .
				"\"" . $shared_private_contacts_schema . "\".\"" . $date . "\" (" .
					"ip uint32, " .
					"related_ip uint32, " .
					"shared_contact_ip uint32, " .
					"shared_contact_asn uint16, " .
					"shared_contact_country smallint " .
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
		'/usr/local/bin/infer_shared_private_contacts --decimal ' .
			'-i "' . $pruned_network_graph_dir . '/' . $date . '" ' .
			$date . ' ' . $argv[1] . ' ' . $argv[3] . ' | ' .
		'/usr/local/bin/psql -q "' .
			$pg_port.' '.
			$pg_dbname.' '.
			$pg_user.' '.
			$pg_password . '" -c "\\copy \"' .
				$shared_private_contacts_schema . '\".\"' . $date . '\" from STDIN"';

	exec($cmd);
	$result = pg_query($pg, $query);
}

while ($row = @pg_fetch_assoc($result)) {
	$json['table']['rows'][] = array(
		//long2ip($row['ip']),
		//long2ip($row['related_ip']),
		long2ip($row['shared_contact_ip']),
		$row['shared_contact_as_description'],
		$essentials->country_flag($row['country_name'],
								  $row['country_code']),
	);
}

echo json_encode($json);

?>
