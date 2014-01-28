<?php
	function isRole($name, $conf) {
		foreach ($conf as $group) {
			if (array_key_exists($name, $group['elements'])) {
				return true;
			}
		}
		return false;
	}

	function getRoleGroup($roleName, $conf) {
		foreach ($conf as $group) {
			if (array_key_exists($roleName, $group['elements'])) {
				return $group['elements'];
			}
		}
		return false;
	}

	function getRoleDescription($roleName, $conf) {
		$group = getRoleGroup($roleName, $conf);
		if ($group === false) {
			return false;
		}
		return $group[$roleName]['description'];
	}
	
	$rolesConf = array(
		array(
			"group" => 'Servers',
			"elements" => array(
				"webServer" => array(
					"schema" => "PortIPs",
					"hostColumn" => '"internalIP"',
					"description" => 'Hosts that have received a connection on port 80 from an external host.',
					1 => array(
						"heading" => array('Internal IP', 'Number of Hosts', 'Bytes Transferred'),
						"display" => array('"internalIP"', 'COUNT(DISTINCT "externalIP")', 'SUM("numBytes")'),
						"type" => array('ip', NULL, 'bytes'),
						"where" => array(array(array('port', '=', '80')), array(array('initiator', '=', '2'))),
						"group" => array('"internalIP"'),
						"order" => array('COUNT(DISTINCT "externalIP")'),
						"sort" => 'DESC'
					),
					2 => array(
						"heading" => array('External IP', 'Autonomous System', 'Country', 'First Occurence', 'Duration', 'Number of Packets', 'Number of Bytes', 'Min Packet Size', 'Max Packet Size', 'Requests', 'Responses'),
						"display" => array('"externalIP"', '"asNumber"', '"countryNumber"', '"startTime"', '"endTime" - "startTime"', '"numPackets"', '"numBytes"', '"minPacketSize"', '"maxPacketSize"', '(\'/http_requests\',"startTime","externalIP","internalIP",\'Requests\')', '(\'/http_responses\',"startTime","externalIP","internalIP",\'Responses\')'),
						"type" => array('ip', 'as', 'country', 'date', 'duration', NULL, 'bytes', 'bytes', 'bytes', 'linkdateip2', 'linkdateip2'),
						"where" => array(array(array('port', '=', '80')), array(array('initiator', '=', '2'))),
						"url" => array('"internalIP"'),
						"urlType" => array('ip'),
						"order" => array('"numPackets"'),
						"sort" => 'DESC'
					)
				),
				"secureWebServer" => array(
					"schema" => "PortIPs",
					"hostColumn" => '"internalIP"',
					"description" => 'Hosts that have received a connection on port 443 from an external host.',
					1 => array(
						"heading" => array('Internal IP', 'Number of Hosts', 'Bytes Transferred'),
						"display" => array('"internalIP"', 'COUNT(DISTINCT "externalIP")', 'SUM("numBytes")'),
						"type" => array('ip', NULL, 'bytes'),
						"where" => array(array(array('port', '=', '443')), array(array('initiator', '=', '2'))),
						"group" => array('"internalIP"'),
						"order" => array('COUNT(DISTINCT "externalIP")'),
						"sort" => 'DESC'
					),
					2 => array(
						"heading" => array('External IP', 'Autonomous System', 'Country', 'First Occurence', 'Duration', 'Number of Packets', 'Number of Bytes', 'Min Packet Size', 'Max Packet Size'),
						"display" => array('"externalIP"', '"asNumber"', '"countryNumber"', '"startTime"', '"endTime" - "startTime"', '"numPackets"', '"numBytes"', '"minPacketSize"', '"maxPacketSize"'),
						"type" => array('ip', 'as', 'country', 'date', 'duration', NULL, 'bytes', 'bytes', 'bytes'),
						"where" => array(array(array('port', '=', '443')), array(array('initiator', '=', '2'))),
						"url" => array('"internalIP"'),
						"urlType" => array('ip'),
						"order" => array('"numPackets"'),
						"sort" => 'DESC'
					)
				),
				"mailServer" => array(
					"schema" => "PortIPs",
					"hostColumn" => '"internalIP"',
					"description" => 'Hosts that have received a connection on port 25, 110, or 143 from an external host.',
					1 => array(
						"heading" => array('Internal IP', 'Number of Hosts', 'Bytes Transferred'),
						"display" => array('"internalIP"', 'COUNT(DISTINCT "externalIP")', 'SUM("numBytes")'),
						"type" => array('ip', NULL, 'bytes'),
						"where" => array(array(array('port', '=', '25'), array('port', '=', '110'), array('port', '=', '143')), array(array('initiator', '=', '2'))),
						"group" => array('"internalIP"'),
						"order" => array('COUNT(DISTINCT "externalIP")'),
						"sort" => 'DESC'
					),
					2 => array(
						"heading" => array('External IP', 'Internal Port', 'Autonomous System', 'Country', 'First Occurence', 'Duration', 'Number of Packets', 'Number of Bytes', 'Min Packet Size', 'Max Packet Size'),
						"display" => array('"externalIP"', '"port"', '"asNumber"', '"countryNumber"', '"startTime"', '"endTime" - "startTime"', '"numPackets"', '"numBytes"', '"minPacketSize"', '"maxPacketSize"'),
						"type" => array('ip', NULL, 'as', 'country', 'date', 'duration', NULL, 'bytes', 'bytes', 'bytes'),
						"where" => array(array(array('port', '=', '25'), array('port', '=', '110'), array('port', '=', '143')), array(array('initiator', '=', '2'))),
						"url" => array('"internalIP"'),
						"urlType" => array('ip'),
						"order" => array('"numPackets"'),
						"sort" => 'DESC'
					)
				),
				"secureMailServer" => array(
					"schema" => "PortIPs",
					"hostColumn" => '"internalIP"',
					"description" => 'Hosts that have received a connection on port 465, 993, or 995 from an external host.',
					1 => array(
						"heading" => array('Internal IP', 'Number of Hosts', 'Bytes Transferred'),
						"display" => array('"internalIP"', 'COUNT(DISTINCT "externalIP")', 'SUM("numBytes")'),
						"type" => array('ip', NULL, 'bytes'),
						"where" => array(array(array('port', '=', '465'), array('port', '=', '993'), array('port', '=', '995')), array(array('initiator', '=', '2'))),
						"group" => array('"internalIP"'),
						"order" => array('COUNT(DISTINCT "externalIP")'),
						"sort" => 'DESC'
					),
					2 => array(
						"heading" => array('External IP', 'Internal Port', 'Autonomous System', 'Country', 'First Occurence', 'Duration', 'Number of Packets', 'Number of Bytes', 'Min Packet Size', 'Max Packet Size'),
						"display" => array('"externalIP"', '"port"', '"asNumber"', '"countryNumber"', '"startTime"', '"endTime" - "startTime"', '"numPackets"', '"numBytes"', '"minPacketSize"', '"maxPacketSize"'),
						"type" => array('ip', NULL, 'as', 'country', 'date', 'duration', NULL, 'bytes', 'bytes', 'bytes'),
						"where" => array(array(array('port', '=', '465'), array('port', '=', '993'), array('port', '=', '995')), array(array('initiator', '=', '2'))),
						"url" => array('"internalIP"'),
						"urlType" => array('ip'),
						"order" => array('"numPackets"'),
						"sort" => 'DESC'
					)
				)
			)
		),
		array(
			"group" => 'Clients',
			"elements" => array(
				"webClient" => array(
					"schema" => "PortIPs",
					"hostColumn" => '"internalIP"',
					"description" => 'Hosts that have initiated a connection to port 80 on an external host.',
					1 => array(
						"heading" => array('Internal IP', 'Number of Hosts', 'Bytes Transferred'),
						"display" => array('"internalIP"', 'COUNT(DISTINCT "externalIP")', 'SUM("numBytes")'),
						"type" => array('ip', NULL, 'bytes'),
						"where" => array(array(array('port', '=', '80')), array(array('initiator', '=', '1'))),
						"group" => array('"internalIP"'),
						"order" => array('COUNT(DISTINCT "externalIP")'),
						"sort" => 'DESC'
					),
					2 => array(
						"heading" => array('External IP', 'Autonomous System', 'Country', 'First Occurence', 'Duration', 'Number of Packets', 'Number of Bytes', 'Min Packet Size', 'Max Packet Size', 'Requests', 'Responses'),
						"display" => array('"externalIP"', '"asNumber"', '"countryNumber"', '"startTime"', '"endTime" - "startTime"', '"numPackets"', '"numBytes"', '"minPacketSize"', '"maxPacketSize"', '(\'/http_requests\',"startTime","internalIP","externalIP",\'Requests\')', '(\'/http_responses\',"startTime","internalIP","externalIP",\'Responses\')'),
						"type" => array('ip', 'as', 'country', 'date', 'duration', NULL, 'bytes', 'bytes', 'bytes', 'linkdateip2', 'linkdateip2'),
						"where" => array(array(array('port', '=', '80')), array(array('initiator', '=', '1'))),
						"url" => array('"internalIP"'),
						"urlType" => array('ip'),
						"order" => array('"numPackets"'),
						"sort" => 'DESC'
					)
				),
				"secureWebClient" => array(
					"schema" => "PortIPs",
					"hostColumn" => '"internalIP"',
					"description" => 'Hosts that have initiated a connection to port 443 on an external host.',
					1 => array(
						"heading" => array('Internal IP', 'Number of Hosts', 'Bytes Transferred'),
						"display" => array('"internalIP"', 'COUNT(DISTINCT "externalIP")', 'SUM("numBytes")'),
						"type" => array('ip', NULL, 'bytes'),
						"where" => array(array(array('port', '=', '443')), array(array('initiator', '=', '1'))),
						"group" => array('"internalIP"'),
						"order" => array('COUNT(DISTINCT "externalIP")'),
						"sort" => 'DESC'
					),
					2 => array(
						"heading" => array('External IP', 'Autonomous System', 'Country', 'First Occurence', 'Duration', 'Number of Packets', 'Number of Bytes', 'Min Packet Size', 'Max Packet Size'),
						"display" => array('"externalIP"', '"asNumber"', '"countryNumber"', '"startTime"', '"endTime" - "startTime"', '"numPackets"', '"numBytes"', '"minPacketSize"', '"maxPacketSize"'),
						"type" => array('ip', 'as', 'country', 'date', 'duration', NULL, 'bytes', 'bytes', 'bytes'),
						"where" => array(array(array('port', '=', '443')), array(array('initiator', '=', '1'))),
						"url" => array('"internalIP"'),
						"urlType" => array('ip'),
						"order" => array('"numPackets"'),
						"sort" => 'DESC'
					)
				),
				"mailClient" => array(
					"schema" => "PortIPs",
					"hostColumn" => '"internalIP"',
					"description" => 'Hosts that have initiated a connection to port 25, 110, or 143 on an external host.',
					1 => array(
						"heading" => array('Internal IP', 'Number of Hosts', 'Bytes Transferred'),
						"display" => array('"internalIP"', 'COUNT(DISTINCT "externalIP")', 'SUM("numBytes")'),
						"type" => array('ip', NULL, 'bytes'),
						"where" => array(array(array('port', '=', '25'), array('port', '=', '110'), array('port', '=', '143')), array(array('initiator', '=', '1'))),
						"group" => array('"internalIP"'),
						"order" => array('COUNT(DISTINCT "externalIP")'),
						"sort" => 'DESC'
					),
					2 => array(
						"heading" => array('External IP', 'External Port', 'Autonomous System', 'Country', 'First Occurence', 'Duration', 'Number of Packets', 'Number of Bytes', 'Min Packet Size', 'Max Packet Size'),
						"display" => array('"externalIP"', '"port"', '"asNumber"', '"countryNumber"', '"startTime"', '"endTime" - "startTime"', '"numPackets"', '"numBytes"', '"minPacketSize"', '"maxPacketSize"'),
						"type" => array('ip', NULL, 'as', 'country', 'date', 'duration', NULL, 'bytes', 'bytes', 'bytes'),
						"where" => array(array(array('port', '=', '25'), array('port', '=', '110'), array('port', '=', '143')), array(array('initiator', '=', '1'))),
						"url" => array('"internalIP"'),
						"urlType" => array('ip'),
						"order" => array('"numPackets"'),
						"sort" => 'DESC'
					)
				),
				"secureMailClient" => array(
					"schema" => "PortIPs",
					"hostColumn" => '"internalIP"',
					"description" => 'Hosts that have initiated a connection to port 465, 993, or 995 on an external host.',
					1 => array(
						"heading" => array('Internal IP', 'Number of Hosts', 'Bytes Transferred'),
						"display" => array('"internalIP"', 'COUNT(DISTINCT "externalIP")', 'SUM("numBytes")'),
						"type" => array('ip', NULL, 'bytes'),
						"where" => array(array(array('port', '=', '465'), array('port', '=', '993'), array('port', '=', '995')), array(array('initiator', '=', '1'))),
						"group" => array('"internalIP"'),
						"order" => array('COUNT(DISTINCT "externalIP")'),
						"sort" => 'DESC'
					),
					2 => array(
						"heading" => array('External IP', 'External Port', 'Autonomous System', 'Country', 'First Occurence', 'Duration', 'Number of Packets', 'Number of Bytes', 'Min Packet Size', 'Max Packet Size'),
						"display" => array('"externalIP"', '"port"', '"asNumber"', '"countryNumber"', '"startTime"', '"endTime" - "startTime"', '"numPackets"', '"numBytes"', '"minPacketSize"', '"maxPacketSize"'),
						"type" => array('ip', NULL, 'as', 'country', 'date', 'duration', NULL, 'bytes', 'bytes', 'bytes'),
						"where" => array(array(array('port', '=', '465'), array('port', '=', '993'), array('port', '=', '995')), array(array('initiator', '=', '1'))),
						"url" => array('"internalIP"'),
						"urlType" => array('ip'),
						"order" => array('"numPackets"'),
						"sort" => 'DESC'
					)
				)
			)
		),
		array(
			"group" => 'Others',
			"elements" => array(
				"spamBot" => array(
					"schema" => "Roles",
					"hostColumn" => '"ip"',
					"description" => 'Spam Bots.',
					1 => array(
						"heading" => array('Internal IP', 'Number of Hosts', 'Bytes Transferred'),
						"display" => array('"ip"', '"numHosts"', '"numBytes"'),
						"type" => array('ip', NULL, 'bytes'),
						"where" => array(array(array('role', '=', '1'))),
						"order" => array('"numHosts"'),
						"sort" => 'DESC'
					),
					2 => array(
						"schema" =>  "PortIPs",
						"hostColumn" => '"internalIP"',
						"heading" => array('External IP', 'External Port', 'Autonomous System', 'Country', 'First Occurence', 'Duration', 'Number of Packets', 'Number of Bytes', 'Min Packet Size', 'Max Packet Size'),
						"display" => array('"externalIP"', '"port"', '"asNumber"', '"countryNumber"', '"startTime"', '"endTime" - "startTime"', '"numPackets"', '"numBytes"', '"minPacketSize"', '"maxPacketSize"'),
						"type" => array('ip', NULL, 'as', 'country', 'date', 'duration', NULL, 'bytes', 'bytes', 'bytes'),
						"where" => array(array(array('port', '=', '25'), array('port', '=', '110'), array('port', '=', '143')), array(array('initiator', '=', '1'))),
						"url" => array('"ip"'),
						"urlType" => array('ip'),
						"order" => array('"numPackets"'),
						"sort" => 'DESC'
					)
				),
				"multimediaP2PNode" => array(
					"schema" => "NonDNSTraffic",
					"hostColumn" => '"internalIP"',
					"description" => 'Peer to Peer nodes that have transferred multimedia content.',
					1 => array(
						"heading" => array('Internal IP', 'Number of Hosts', 'Bytes Transferred'),
						"display" => array('"internalIP"', 'COUNT(DISTINCT "externalIP")', 'SUM("numBytes")'),
						"type" => array('ip', NULL, 'bytes'),
						"where" => array(array(array('type', '=', '1'))),
						"group" => array('"internalIP"'),
						"order" => array('COUNT(DISTINCT "externalIP")'),
						"sort" => 'DESC'
					),
					2 => array(
						"heading" => array('External IP', 'Autonomous System', 'Country', 'Number of Occurences', 'Number of Packets', 'Number of Bytes'),
						"display" => array('"externalIP"', '"asNumber"', '"countryNumber"', 'COUNT(*)', 'SUM("numPackets")', 'SUM("numBytes")'),
						"type" => array('ip', 'as', 'country', NULL, NULL, 'bytes'),
						"where" => array(array(array('type', '=', '1'))),
						"group" => array('"externalIP"', '"asNumber"', '"countryNumber"'),
						"url" => array('"internalIP"'),
						"urlType" => array('ip'),
						"order" => array('SUM("numPackets")'),
						"sort" => 'DESC'
					),
					3 => array(
						"heading" => array('Protocol', 'Internal Port', 'External Port', 'Service', 'First Occurence', 'Duration', 'Number of Packets', 'Number of Bytes', 'Min Packet Size', 'Max Packet Size'),
						"display" => array('"protocol"', '"internalPort"', '"externalPort"', '("protocol", "externalPort")', '"startTime"', '"endTime" - "startTime"', '"numPackets"', '"numBytes"', '"minPacketSize"', '"maxPacketSize"'),
						"type" => array('protocol', NULL, NULL, 'service', 'date', 'duration', NULL, 'bytes', 'bytes', 'bytes'),
						"where" => array(array(array('type', '=', '1'))),
						"url" => array('"externalIP"'),
						"urlType" => array('ip'),
						"order" => array('"numPackets"'),
						"sort" => 'DESC'
					)
				),
				"unclassifiedP2PNode" => array(
					"schema" => "NonDNSTraffic",
					"hostColumn" => '"internalIP"',
					"description" => 'Peer to Peer nodes that have transferred content of an unknown type.',
					1 => array(
						"heading" => array('Internal IP', 'Number of Hosts', 'Bytes Transferred'),
						"display" => array('"internalIP"', 'COUNT(DISTINCT "externalIP")', 'SUM("numBytes")'),
						"type" => array('ip', NULL, 'bytes'),
						"where" => array(array(array('type', '=', '2'))),
						"group" => array('"internalIP"'),
						"order" => array('COUNT(DISTINCT "externalIP")'),
						"sort" => 'DESC'
					),
					2 => array(
						"heading" => array('External IP', 'Autonomous System', 'Country', 'Number of Occurences', 'Number of Packets', 'Number of Bytes'),
						"display" => array('"externalIP"', '"asNumber"', '"countryNumber"', 'COUNT(*)', 'SUM("numPackets")', 'SUM("numBytes")'),
						"type" => array('ip', 'as', 'country', NULL, NULL, 'bytes'),
						"where" => array(array(array('type', '=', '2'))),
						"group" => array('"externalIP"', '"asNumber"', '"countryNumber"'),
						"url" => array('"internalIP"'),
						"urlType" => array('ip'),
						"order" => array('SUM("numPackets")'),
						"sort" => 'DESC'
					),
					3 => array(
						"heading" => array('Protocol', 'Internal Port', 'External Port', 'Service', 'First Occurence', 'Duration', 'Number of Packets', 'Number of Bytes', 'Min Packet Size', 'Max Packet Size'),
						"display" => array('"protocol"', '"internalPort"', '"externalPort"', '("protocol", "externalPort")', '"startTime"', '"endTime" - "startTime"', '"numPackets"', '"numBytes"', '"minPacketSize"', '"maxPacketSize"'),
						"type" => array('protocol', NULL, NULL, 'service', 'date', 'duration', NULL, 'bytes', 'bytes', 'bytes'),
						"where" => array(array(array('type', '=', '2'))),
						"url" => array('"externalIP"'),
						"urlType" => array('ip'),
						"order" => array('"numPackets"'),
						"sort" => 'DESC'
					)
				),
				"encryptedP2PNode" => array(
					"schema" => "NonDNSTraffic",
					"hostColumn" => '"internalIP"',
					"description" => 'Peer to Peer nodes that have transferred encrypted content.',
					1 => array(
						"heading" => array('Internal IP', 'Number of Hosts', 'Bytes Transferred'),
						"display" => array('"internalIP"', 'COUNT(DISTINCT "externalIP")', 'SUM("numBytes")'),
						"type" => array('ip', NULL, 'bytes'),
						"where" => array(array(array('type', '=', '3'))),
						"group" => array('"internalIP"'),
						"order" => array('COUNT(DISTINCT "externalIP")'),
						"sort" => 'DESC'
					),
					2 => array(
						"heading" => array('External IP', 'Autonomous System', 'Country', 'Number of Occurences', 'Number of Packets', 'Number of Bytes'),
						"display" => array('"externalIP"', '"asNumber"', '"countryNumber"', 'COUNT(*)', 'SUM("numPackets")', 'SUM("numBytes")'),
						"type" => array('ip', 'as', 'country', NULL, NULL, 'bytes'),
						"where" => array(array(array('type', '=', '3'))),
						"group" => array('"externalIP"', '"asNumber"', '"countryNumber"'),
						"url" => array('"internalIP"'),
						"urlType" => array('ip'),
						"order" => array('SUM("numPackets")'),
						"sort" => 'DESC'
					),
					3 => array(
						"heading" => array('Protocol', 'Internal Port', 'External Port', 'Service', 'First Occurence', 'Duration', 'Number of Packets', 'Number of Bytes', 'Min Packet Size', 'Max Packet Size'),
						"display" => array('"protocol"', '"internalPort"', '"externalPort"', '("protocol", "externalPort")', '"startTime"', '"endTime" - "startTime"', '"numPackets"', '"numBytes"', '"minPacketSize"', '"maxPacketSize"'),
						"type" => array('protocol', NULL, NULL, 'service', 'date', 'duration', NULL, 'bytes', 'bytes', 'bytes'),
						"where" => array(array(array('type', '=', '3'))),
						"url" => array('"externalIP"'),
						"urlType" => array('ip'),
						"order" => array('"numPackets"'),
						"sort" => 'DESC'
					)
				),
				"bruteForcer" => array(
					"schema" => "BruteForcers",
					//"hostColumn" => '"sourceIP"',
					"description" => 'Hosts that have attempted to brute force access to a service of a different host.',
					1 => array(
						"heading" => array('Service', 'Destination Port', 'Number of Hosts'),
						"display" => array('(6, "destinationPort")', '"destinationPort"', 'COUNT(DISTINCT "sourceIP")'),
						"type" => array('service', NULL, NULL),
						"group" => array('"destinationPort"'),
						"order" => array('COUNT(DISTINCT "sourceIP")'),
						"sort" => 'DESC'
					),
					2 => array(
						"heading" => array('Source IP', 'Number of Destination IPs', 'Bytes Transferred'),
						"display" => array('"sourceIP"', 'COUNT(DISTINCT "destinationIP")', 'SUM("numBytes")'),
						"type" => array('ip', NULL, 'bytes'),
						"group" => array('"sourceIP"'),
						"url" => array('"destinationPort"'),
						"urlType" => array(NULL),
						"order" => array('COUNT(DISTINCT "destinationIP")'),
						"sort" => 'DESC'
					),
					3 => array(
						"heading" => array('Destination IP', 'Autonomous System', 'Country', 'First Occurence', 'Duration', 'Number of Attempts'),
						"display" => array('"destinationIP"', '"asNumber"', '"countryNumber"', '"startTime"', '"endTime" - "startTime"', '"numAttempts"'),
						"type" => array('ip', 'as', 'country', 'date', 'duration', NULL),
						"url" => array('"sourceIP"'),
						"urlType" => array('ip'),
						"order" => array('"numAttempts"'),
						"sort" => 'DESC'
					)
				),
				"bruteForced" => array(
					"schema" => "BruteForcers",
					//"hostColumn" => '"destinationIP"',
					"description" => 'Hosts that have been the victim of a brute force attempt.',
					1 => array(
						"heading" => array('Service', 'Destination Port', 'Number of Hosts'),
						"display" => array('(6, "destinationPort")', '"destinationPort"', 'COUNT(DISTINCT "destinationIP")'),
						"type" => array('service', NULL, NULL),
						"group" => array('"destinationPort"'),
						"order" => array('COUNT(DISTINCT "destinationIP")'),
						"sort" => 'DESC'
					),
					2 => array(
						"heading" => array('Destination IP', 'Number of Source IPs', 'Bytes Transferred'),
						"display" => array('"destinationIP"', 'COUNT(DISTINCT "sourceIP")', 'SUM("numBytes")'),
						"type" => array('ip', NULL, 'bytes'),
						"group" => array('"destinationIP"'),
						"url" => array('"destinationPort"'),
						"urlType" => array(NULL),
						"order" => array('COUNT(DISTINCT "sourceIP")'),
						"sort" => 'DESC'
					),
					3 => array(
						"heading" => array('Source IP', 'Autonomous System', 'Country', 'First Occurence', 'Duration', 'Number of Attempts'),
						"display" => array('"sourceIP"', '"sourceIP"', '"sourceIP"', '"startTime"', '"endTime" - "startTime"', '"numAttempts"'),
						"type" => array('ip', 'asLookup', 'countryLookup', 'date', 'duration', NULL),
						"url" => array('"destinationIP"'),
						"urlType" => array('ip'),
						"order" => array('"numAttempts"'),
						"sort" => 'DESC'
					)
				)
				/*
				"bruteForcer" => array(
					"schema" => "BruteForcers",
					"hostColumn" => '"sourceIP"',
					"description" => 'Hosts that have attempted to brute force access to a service of a different host.',
					1 => array(
						"heading" => array('Source IP', 'Number of Destination IPs', 'Bytes Transferred'),
						"display" => array('"sourceIP"', 'COUNT(DISTINCT "destinationIP")', 'SUM("numBytes")'),
						"type" => array('ip', NULL, 'bytes'),
						"group" => array('"sourceIP"'),
						"order" => array('COUNT(DISTINCT "destinationIP")'),
						"sort" => 'DESC'
					),
					2 => array(
						"heading" => array('Service', 'Destination Port', 'Number of Hosts'),
						"display" => array('(6, "destinationPort")', '"destinationPort"', 'COUNT(DISTINCT "destinationIP")'),
						"type" => array('service', NULL, NULL),
						"group" => array('"destinationPort"'),
						"url" => array('"sourceIP"'),
						"urlType" => array('ip'),
						"order" => array('COUNT(DISTINCT "destinationIP")'),
						"sort" => 'DESC'
					),
					3 => array(
						"heading" => array('Destination IP', 'Autonomous System', 'Country', 'First Occurence', 'Duration', 'Number of Attempts'),
						"display" => array('"destinationIP"', '"asNumber"', '"countryNumber"', '"startTime"', '"endTime" - "startTime"', '"numAttempts"'),
						"type" => array('ip', 'as', 'country', 'date', 'duration', NULL),
						"url" => array('"destinationPort"'),
						"urlType" => array(NULL),
						"order" => array('"numAttempts"'),
						"sort" => 'DESC'
					)
				),
				"bruteForced" => array(
					"schema" => "BruteForcers",
					"hostColumn" => '"destinationIP"',
					"description" => 'Hosts that have been the victim of a brute force attempt.',
					1 => array(
						"heading" => array('Destination IP', 'Number of Source IPs', 'Bytes Transferred'),
						"display" => array('"destinationIP"', 'COUNT(DISTINCT "sourceIP")', 'SUM("numBytes")'),
						"type" => array('ip', NULL, 'bytes'),
						"group" => array('"destinationIP"'),
						"order" => array('COUNT(DISTINCT "sourceIP")'),
						"sort" => 'DESC'
					),
					2 => array(
						"heading" => array('Service', 'Destination Port', 'Number of Hosts'),
						"display" => array('(6, "destinationPort")', '"destinationPort"', 'COUNT(DISTINCT "sourceIP")'),
						"type" => array('service', NULL, NULL),
						"group" => array('"destinationPort"'),
						"url" => array('"destinationIP"'),
						"urlType" => array('ip'),
						"order" => array('COUNT(DISTINCT "sourceIP")'),
						"sort" => 'DESC'
					),
					3 => array(
						"heading" => array('Source IP', 'Autonomous System', 'Country', 'First Occurence', 'Duration', 'Number of Attempts'),
						"display" => array('"sourceIP"', '"sourceIP"', '"sourceIP"', '"startTime"', '"endTime" - "startTime"', '"numAttempts"'),
						"type" => array('ip', 'asLookup', 'countryLookup', 'date', 'duration', NULL),
						"url" => array('"destinationPort"'),
						"urlType" => array(NULL),
						"order" => array('"numAttempts"'),
						"sort" => 'DESC'
					)
				)
				*/
			)
		)
	);
?>
