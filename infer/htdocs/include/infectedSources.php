<?php

$sourceNames = array(0 => 'Master IMS infected IP list',
				   1 => 'Local IMS infected IP list',
				   2 => 'DShield',
				   3 => 'infiltrated.net blacklist',
				   4 => 'Bleeding-edge Threats Botnet Command and Control list',
				   5 => 'University of Waterloo Information Systems & Technology Security Trends',
				   6 => 'Bleeding-edge Threats known compromised host list',
				   7 => 'Malware Domain List',
				   8 => 'Emerging Threats Botnet Command and Control list',
				   9 => 'Emerging Threats known compromised host list');
$sourceURLs = array(0 => '.',
				  1 => '/infectedIPs',
				  2 => 'http://www.dshield.org/ipsascii.html?limit=256',
				  3 => 'http://www.infiltrated.net/blacklisted',
				  4 => 'http://www.bleedingthreats.net/bleeding-botcc.rules',
				  5 => 'http://ist.uwaterloo.ca/security/trends/Blacklist-28.txt',
				  6 => 'http://www.bleedingthreats.net/rules/bleeding-compromised.rules',
				  7 => 'http://www.malwaredomainlist.com/mdl.php?search=&colsearch=All&quantity=All',
				  8 => 'http://www.emergingthreats.net/rules/bleeding-botcc.rules',
				  9 => 'http://www.emergingthreats.net/rules/bleeding-compromised.rules');

function getInfectedSourceLinks($sourceNumbers, $numericIP = NULL) {
	global $sourceNames, $sourceURLs;
	$sourceLabel = array();
	foreach (explode(',', substr($sourceNumbers, 1, -1)) as $sourceNumber) {
		if (array_key_exists($sourceNumber, $sourceNames)) {
			if ($sourceNumber == 1 && $numericIP != NULL) {
				$sourceLabel[] = '<a class="text" href="' .
									$sourceURLs[$sourceNumber] . '#' .
									long2ip($numericIP) . '">' .
									$sourceNames[$sourceNumber] .
									'</a>';
			} else {
				$sourceLabel[] = '<a class="text" href="' .
									$sourceURLs[$sourceNumber] . '">' .
									$sourceNames[$sourceNumber] .
									'</a>';
			}
		}
	}
	return implode(', ', $sourceLabel);
}
?>
