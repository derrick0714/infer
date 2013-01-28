<?php

	function getSymptomDescription($symptomName, $conf) {
		if (!isset($conf[$symptomName])) {
			return false;
		}

		if (!isset($conf[$symptomName]['description'])) {
			return false;
		}

		return $conf[$symptomName]['description'];
	}
	
	$symptomsConf = array(
		"darkSpaceTargets" => array(
			"schema" => "DarkSpaceTargets",
			"hostColumn" => '"internalIP"',
			"description" => 'Hosts that have been contacted by a host that has attempted to contact unused address space.',
			1 => array(
				"heading" => array('Internal IP', 'Internal Port', 'Dark Space Sources'),
				"display" => array('"internalIP"', '"internalPort"', 'COUNT(DISTINCT "externalIP")'),
				"type" => array('ip', NULL, NULL),
				"group" => array('"internalIP"', '"internalPort"'),
				"order" => array('COUNT(DISTINCT "externalIP")'),
				"sort" => 'DESC'
			),
			2 => array(
				"heading" => array('Protocol', 'External IP', 'External Port', 'Internal Port', 'Service', 'Country', 'Start Time', 'Duration', 'Number of Packets', 'Number of Bytes', 'Min Packet Size', 'Max Packet Size'),
				"display" => array('"protocol"', '"externalIP"', '"externalPort"', '"internalPort"', '("protocol", "internalPort")', '"countryNumber"', '"startTime"', '"endTime" - "startTime"', '"numPackets"', '"numBytes"', '"minPacketSize"', '"maxPacketSize"'),
				"type" => array('protocol', 'ip', NULL, NULL, 'service', 'country', 'date', 'duration', NULL, 'bytes', 'bytes', 'bytes'),
				"url" => array('"internalIP"', '"internalPort"'),
				"urlType" => array('ip', NULL)
			)
		),
		"darkSpaceSources" => array(
			"schema" => "DarkSpaceSources",
			"hostColumn" => '"sourceIP"',
			"description" => 'Hosts that have attempted to contact unused address space.',
			1 => array(
				"heading" => array('Source IP', 'Country', 'Destination Port', 'Targets in Dark Space'),
				"display" => array('"sourceIP"', '"countryNumber"', '"destinationPort"', 'COUNT(DISTINCT "destinationIP")'),
				"type" => array('ip', 'country', NULL, NULL),
				"group" => array('"sourceIP"', '"countryNumber"', '"destinationPort"'),
				"order" => array('COUNT(DISTINCT "destinationIP") DESC')
			),
			2 => array(
				"heading" => array('Protocol', 'Source Port', 'Destination IP', 'Destination Port', 'Service', 'First Occurence', 'Duration', 'Number of Packets', 'Number of Bytes', 'Min Packet Size', 'Max Packet Size'),
				"display" => array('"protocol"', '"sourcePort"', '"destinationIP"', '"destinationPort"', '("protocol", "destinationPort")', '"startTime"', '"endTime" - "startTime"', '"numPackets"', '"numBytes"', '"minPacketSize"', '"maxPacketSize"'),
				"type" => array('protocol', NULL, 'ip', NULL, 'service', 'date', 'duration', NULL, 'bytes', 'bytes', 'bytes'),
				"url" => array('"sourceIP"', '"destinationPort"'),
				"urlType" => array('ip', NULL)
			)
		),
		"infectedContacts" => array(
			"schema" => "InfectedContacts",
			"hostColumn" => '"internalIP"',
			"description" => 'Hosts that have contacted IP addresses on a publically available "Infected Hosts" list.',
			1 => array(
				"heading" => array('Internal IP', 'External IP', 'Number of Bytes', 'Listed as Infected at', 'Autonomous System', 'Country', 'Number of Occurrences'),
				"display" => array('"internalIP"', '"externalIP"', 'SUM("numBytes")', '"sourceNumbers"', '"asNumber"', '"countryNumber"', 'COUNT(*)'),
				"type" => array('ip', 'ip', 'bytes', 'infectedSources', 'as', 'country', NULL),
				"group" => array('"internalIP"', '"externalIP"', '"sourceNumbers"', '"asNumber"', '"countryNumber"'),
				"order" => array('SUM("numBytes") DESC')
			),
			2 => array(
				"heading" => array('Protocol', 'Internal Port', 'External Port', 'Service', 'First Occurence', 'Duration', 'Number of Packets', 'Number of Bytes', 'Min Packet Size', 'Max Packet Size'),
				"display" => array('"protocol"', '"internalPort"', '"externalPort"', '("protocol", "externalPort")', '"startTime"', '"endTime" - "startTime"', '"numPackets"', '"numBytes"', '"minPacketSize"', '"maxPacketSize"'),
				"type" => array('protocol', NULL, NULL, 'service', 'date', 'duration', NULL, 'bytes', 'bytes', 'bytes'),
				"url" => array('"internalIP"', '"externalIP"'),
				"urlType" => array('ip', 'ip'),
				"order" => array('"numBytes"'),
				"sort" => 'DESC'
			)
		),
		"evasiveTraffic" => array(
			"schema" => "EvasiveTraffic",
			"hostColumn" => '"internalIP"',
			"description" => 'Traffic that resembles malware communications trying to avoid detection.',
			1 => array(
				"heading" => array('Internal IP', 'External IP', 'Number of Bytes', 'Number of Packets', 'Autonomous System', 'Country', 'Number of Occurrences'),
				"display" => array('"internalIP"', '"externalIP"', 'SUM("numBytes")', 'SUM("numPackets")', '"asNumber"', '"countryNumber"', 'COUNT(*)'),
				"type" => array('ip', 'ip', 'bytes', NULL, 'as', 'country', NULL),
				"group" => array('"internalIP"', '"externalIP"', '"asNumber"', '"countryNumber"'),
				"order" => array('SUM("numPackets") DESC')
			),
			2 => array(
				"heading" => array('Protocol', 'Internal Port', 'External Port', 'Service', 'First Occurence', 'Duration', 'Number of Frags', 'Min TTL', 'Max TTL', 'Number of Packets', 'Min Packet Size', 'Max Packet Size', 'Number of Bytes'),
				"display" => array('"protocol"', '"internalPort"', '"externalPort"', '("protocol", "externalPort")', '"startTime"', '"endTime" - "startTime"', '"numFrags"', '"minTTL"', '"maxTTL"', '"numPackets"', '"minPacketSize"', '"maxPacketSize"', '"numBytes"'),
				"type" => array('protocol', NULL, NULL, 'service', 'date', 'duration', NULL, NULL, NULL, NULL, 'bytes', 'bytes', 'bytes'),
				"url" => array('"internalIP"', '"externalIP"'),
				"urlType" => array('ip', 'ip'),
				"order" => array('"numPackets"'),
				"sort" => 'DESC'
			)
		),
		"nonDNSTraffic" => array(
			"schema" => "NonDNSTraffic",
			"hostColumn" => '"internalIP"',
			"description" => 'Traffic to a host whose IP address was not obtained through a DNS request.',
			1 => array(
				"heading" => array('Internal IP', 'External IP', 'Number of Bytes', 'Number of Packets', 'Autonomous System', 'Country', 'Number of Occurrences'),
				"display" => array('"internalIP"', '"externalIP"', 'SUM("numBytes")', 'SUM("numPackets")', '"asNumber"', '"countryNumber"', 'COUNT(*)'),
				"type" => array('ip', 'ip', 'bytes', NULL, 'as', 'country', NULL),
				"group" => array('"internalIP"', '"externalIP"', '"asNumber"', '"countryNumber"'),
				"order" => array('COUNT(*) DESC')
			),
			2 => array(
				"heading" => array('Protocol', 'Internal Port', 'External Port', 'Service', 'First Occurence', 'Duration', 'Number of Packets', 'Number of Bytes', 'Min Packet Size', 'Max Packet Size'),
				"display" => array('"protocol"', '"internalPort"', '"externalPort"', '("protocol", "externalPort")', '"startTime"', '"endTime" - "startTime"', '"numPackets"', '"numBytes"', '"minPacketSize"', '"maxPacketSize"'),
				"type" => array('protocol', NULL, NULL, 'service', 'date', 'duration', NULL, 'bytes', 'bytes', 'bytes'),
				"url" => array('"internalIP"', '"externalIP"'),
				"urlType" => array('ip', 'ip'),
				"order" => array('"numPackets"'),
				"sort" => 'DESC'
			)
		)
	);

?>
