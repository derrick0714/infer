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
//  select a.host, b.relevance from "HTTPRequests"."2010-06-30" as a, "HTTPHostRelevance"."2010-06-30" as b where a.host = b.host and a.source_ip = '2163094898' group by a.host, b.relevance order by b.relevance desc;

	"SELECT DISTINCT a.host AS host_name, " .
		   "b.relevance AS host_relevance " .
	'FROM "HTTPRequests"."' . $date . '" AS a, ' .
		 '"HTTPHostRelevance"."' . $date . '" AS b ' .
	"WHERE a.host = b.host AND " .
		  "a.source_ip = '" . $host . "' " .
	"ORDER BY host_relevance DESC " .
	"LIMIT 10";

/*
echo "<pre>";
echo $query . "\n";
*/

$json = array(
	"metadata" => array(
		"data_type" => "top_requested_hosts",
		"ip" => $argv[1],
	),
	"table" => array(
		"caption" => "Most Interesting Requested HTTP Hosts",
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
				"host_name",
				"Host Name"
			),
			array(
				"host_relevance",
				"Relevance"
			)
		),
		"rows" => array()
	)
);

$result = pg_query($pg, $query);

while ($row = @pg_fetch_assoc($result)) {
	$json['table']['rows'][] = array(
		$row['host_name'],
		number_format(floatval($row['host_relevance']) * 100) . '%'
	);
}

echo json_encode($json);

?>
