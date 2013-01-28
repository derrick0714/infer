<?php
/// The main PHP file to fetch data for the executive dashboard.
/// Author: Aktarer Zaman
/// Date: 05/22/2010

//**********************//
// Data parsing methods
//**********************//

function parse_traffic($data)
{
	// Window portion of json
	$window = array("name" => "bandwidth_window", "title" => "Title to be shown to user on widget's title bar", "refresh" => 30, "width" => 500, "height" => 300);
	
	// Data portion of json
	$data_set = array();
	
	// Ingress Traffic
	preg_match("/Content Type	: Ingress Traffic in Bytes(.+?)\n\n/s", $data, $ingress_content);
	preg_match_all("/([A-Za-z ]+)	: ([0-9]+)/s", trim($ingress_content[1]), $ingress_table_raw);
	for($i = 0; $i < count($ingress_table_raw[2]); $i++) $ingress_table_raw[2][$i] = intval($ingress_table_raw[2][$i]);
	$types = array();
	$values = array();
	array_push($types, $ingress_table_raw[1][0]);
	array_push($values, $ingress_table_raw[2][0]);
	for($i = 1; $i < count($ingress_table_raw[1]); $i++) {
		array_push($types, $ingress_table_raw[1][$i]);
		array_push($values, $ingress_table_raw[2][$i]);
	}
	array_push($data_set, array("ingress" => array("name" => "ingress_traffic", "title" => "Ingress Traffic", "container" => "table", "columns" => array(array("string", "Type of Content"), array("bytes", "Bytes")), "table" => array("types" => $types, "values" => $values))));

	// Egress Traffic
	preg_match("/Content Type	: Egress Traffic in Bytes(.+?)\n\n/s", $data, $egress_content);
	preg_match_all("/([A-Za-z ]+)	: ([0-9]+)/s", trim($egress_content[1]), $egress_table_raw);
	for($i = 0; $i < count($egress_table_raw[2]); $i++) $egress_table_raw[2][$i] = intval($egress_table_raw[2][$i]);
	$egress_table = array();
	array_push($egress_table, $egress_table_raw[2][0]);
	for($i = 1; $i < count($egress_table_raw[1]); $i++) array_push($egress_table, $egress_table_raw[2][$i]);
	array_push($data_set, array("egress" => array("name" => "egress_traffic", "title" => "Egress Traffic", "container" => "table", "columns" => array(array("string", "Type of Content"), array("bytes", "Bytes")), "table" => $egress_table)));
	
	// Network Throughput (24 hours)
	preg_match("/Hour:Minutes	:Throughput in Mbps(.+?)\n\n/s", $data, $throughput_content);
	preg_match_all("/([0-9:]+)	: ([0-9.]+)/s", trim($throughput_content[1]), $throughput_table_raw);
	for($i = 0; $i < count($throughput_table_raw[2]); $i++) $throughput_table_raw[2][$i] = floatval($throughput_table_raw[2][$i]);
	$throughput_table = array();
	array_push($throughput_table, array('x' => 0, 'y' => $throughput_table_raw[2][0]));
	for($i = 1; $i < count($throughput_table_raw[2]); $i++) array_push($throughput_table, array('x' => $i, 'y' => $throughput_table_raw[2][$i]));
	array_push($data_set, array("throughput" => array("name" => "network_throughput", "title" => "Network Throughput (24 hours)", "container" => "table", "columns" => array(array("string", "Time in GMT"), array("rate", "Throughput in Mbps")), "table" => $throughput_table)));
	
	return json_encode(array("widget" => array("window" => $window, "data" => array("dataset" => $data_set))));
}

function parse_host_inventory($data)
{
	// Window portion of json
	$window = array("name" => "host_inventory_window", "title" => "Host roles in this network", "refresh" => 3600, "width" => 500, "height" => 300);
	
	// Data portion of json
	$table = array();
	
	preg_match_all("/Host Role: (.+?): (.+?)\nWeek	Connections\n(.+?)\n\n/s", $data, $hosts);
	
	for($i = 0; $i < count($hosts[3]); $i++)
	{
		preg_match_all("/([0-9]+): ([0-9]+)/s", $hosts[3][$i], $t);
		for($j = 0; $j < count($t[2]); $j++) $t[2][$j] = intval($t[2][$j]);
		$hosts[3][$i] = array_combine($t[1], $t[2]);
		$host_table = array();
		array_push($host_table, array(0, $t[2][0]));
		for($j = 1; $j < count($t[2]); $j++) array_push($host_table, array($j, $t[2][$j]));

		array_push($table, array($hosts[1][$i], $hosts[2][$i], array("container" => "table", "columns" => array(array("string", "Day of Week"), array("count", "Number of Connections")), "table" => $host_table)));
	}
	
	return json_encode(array("widget" => array("window" => $window, "data" => array("dataset" => array("inventory" => array("name" => "host_inventory", "container" => "table", "columns" => array(array("string", "Roles"), array("count", "Active Hosts"), array("container", "Activity")), "table" => $table))))));
}

function parse_app_inventory($data)
{
	// Window portion of json
	$window = array("name" => "app_inventory_window", "title" => "Application Inventory", "refresh" => 3600, "width" => 500, "height" => 300);
	
	// Data portion of json
	$table = array();
	preg_match("/Activity	Application	Versions	Popular Version	Time Spent \(HH:MM\)(.+?)\n\n/s", $data, $app_content);
	preg_match_all("/\n(.+?)	(.+?)	(.+?)	(.+?)	([0-9]+):([0-9]+)/s", $app_content[1], $app_table_raw);
	
	for($i = 0; $i < count($app_table_raw[1]); $i++)
	{
		$t = array($app_table_raw[1][$i], $app_table_raw[2][$i], $app_table_raw[3][$i], $app_table_raw[4][$i], $app_table_raw[5][$i] == "0" ? $app_table_raw[6][$i]."mins" : $app_table_raw[5][$i]."hrs ".$app_table_raw[6][$i]."mins");
		array_push($table, $t);
	}
	
	return json_encode(array("widget" => array("window" => $window, "data" => array("dataset" => array("inventory" => array("name" => "app_inventory", "title" => "Application Inventory", "container" => "table", "columns" => array(array("string", "Activity"), array("string", "Application"), array("string", "Versions"), array("string", "Common Versions"), array("string", "Time Spent")), "table" => $table))))));
}

function parse_network_exposure($data)
{
	// Window portion of json
	$window = array("name" => "network_exposure_window", "title" => "Your Network's Exposure", "refresh" => 60, "width" => 500, "height" => 300);

	// Data portion of json
	$table = array();
	preg_match("/Autonomous System	AS Number	Distinct Contacts	Ingress Traffic in MB	Egress Traffic in MB(.+)/s", $data, $net_content);
	preg_match_all("/\n(.+?)	(.+?)	(.+?)	(.+?)	(.+?)/s", trim($net_content[1]), $net_table_raw);
	
	for($i = 0; $i < count($net_table_raw[1]); $i++)
	{
		//$t = array($net_table_raw[1][$i], intval($net_table_raw[2][$i]), intval($net_table_raw[3][$i]), intval($net_table_raw[4][$i]), intval($net_table_raw[5][$i]));
		$t = array('as' => $net_table_raw[1][$i], 'x' => intval($net_table_raw[4][$i]), 'y' => intval($net_table_raw[5][$i]), 'z' => intval($net_table_raw[3][$i]));
		array_push($table, $t);
	}
	
	return json_encode(array("widget" => array("window" => $window, "data" => array("dataset" => array("exposure" => array("name" => "network_exposure", "container" => "table", "columns" => array(array("string", "Autonomous System"), array("string", "AS Number"), array("count", "Distinct Contacts"), array("number", "Ingress Traffic in MB"), array("number", "Egress Traffic in MB")), "table" => $table))))));
}

//**********************//
// Execution starts here
//**********************//

$data = shell_exec("./data/generate");

if($_GET['w']) // Gets which widget data to show
	$widgets = array_unique(str_split($_GET['w'], 1));
else // Will show all widget data
	$widgets = array(1, 2, 3, 4);

foreach($widgets as $widget) // Calls the method of each requested widget and displays the json data
{
	switch($widget)
	{
		case 1:
			echo parse_traffic($data)."\n";
			break;
		case 2:
			echo parse_app_inventory($data)."\n";
			break;
		case 3:
			echo parse_host_inventory($data)."\n";
			break;
		case 4:
			echo parse_network_exposure($data)."\n";
			break;
	}
}
?>