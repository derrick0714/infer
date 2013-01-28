<?php

include('include/postgreSQL.php');
include('include/shared.php');
include('include/accessControl.php');
include('include/checkSession.php');
include('include/roles.php');

include('/usr/local/share/sparkline/lib/Sparkline_Bar.php');

$type = $url[2];
$img_date = $url[3];
$ip = ip2long($url[4]);
$subtype = $url[5];

switch ($type) {
  case 'virulence':
	$dates = array_slice(getPGTableRange($postgreSQL, 'InterestingIPs', getFirstPGTable($postgreSQL, 'InterestingIPs'), $img_date), -14);
	foreach ($dates as &$date) {
		$numInterestingIPs[$date] = getNumInterestingIPs($postgreSQL, $date);
	}

	drawVirulenceSparkline($postgreSQL, $dates, $numInterestingIPs, $ip);
	break;
  case 'rank':
	$dates = array_slice(getPGTableRange($postgreSQL, 'InterestingIPs', getFirstPGTable($postgreSQL, 'InterestingIPs'), $img_date), -14);
	foreach ($dates as &$date) {
		$numInterestingIPs[$date] = getNumInterestingIPs($postgreSQL, $date);
	}

	drawRankSparkline($postgreSQL, $dates, $numInterestingIPs, $ip);
	break;
  case 'service':
	$dates = array_slice(getPGTableRange($postgreSQL, 'LiveIPs', getFirstPGTable($postgreSQL, 'LiveIPs'), $img_date), -14);
	$monitoredServices = getMonitoredServices($postgreSQL);
	$interestingPortsIndex = getInterestingPortsIndex($postgreSQL, $img_date);
	drawServicesSparkline($postgreSQL, $monitoredServices, $interestingPortsIndex, $dates, $ip, $subtype);
	break;
  case 'role':
	$dates = array_slice(getPGTableRange($postgreSQL, 'LiveIPs', getFirstPGTable($postgreSQL, 'LiveIPs'), $img_date), -14);
	drawRoleSparkline($postgreSQL, $dates, $ip, $roleURLNames[$subtype]);
	break;
  default:
	exit;
}


function getMonitoredServices(&$postgreSQL) {
	$result = pg_query($postgreSQL, 'SELECT * FROM "Maps"."monitoredServices"');
	while ($row = pg_fetch_assoc($result)) {
		$row['name'] = str_replace(' ', '', $row['name']);
		$monitoredServices[$row['name']]['ports'] =
			explode(',', substr($row['ports'], 1, -1));
		$monitoredServices[$row['name']]['initiator'] = $row['initiator'];
	}
	return $monitoredServices;
}

function getInterestingPortsIndex(&$postgreSQL, &$date) {
	$result = pg_query($postgreSQL, 'SELECT "interestingPorts" FROM "Indexes"."interestingPorts" WHERE "date" = \'' . $date . '\'');
	if (pg_num_rows($result)) {
		$index = 0;
		$row = pg_fetch_assoc($result);
		foreach (explode(',', substr($row['interestingPorts'], 1, -1)) as $port)
		{
			$interestingPortsIndex[$port] = $index++;
		}
		return $interestingPortsIndex;
	}
	return false;
}

function arraySum(&$interestingPortsIndex, &$array, &$indexes) {
	foreach ($indexes as &$index) {
		$sum += $array[$interestingPortsIndex[$index]];
	}
	return $sum;
}

function drawVirulenceSparkline(&$postgreSQL,
								&$dates,
								&$numInterestingIPs,
								&$ip)
{
	$minVirulence['value'] = 1;
	$maxVirulence['value'] = 0;
	foreach ($dates as &$date) {
		$result = pg_query($postgreSQL, 'SELECT "currentVirulence" FROM "InterestingIPs"."' . $date . '" ' .
																		'WHERE "ip" = \'' . $ip . '\'');
		if (pg_num_rows($result) > 0) {
			$row = pg_fetch_assoc($result);
			$virulenceData[] = $row['currentVirulence'];
			if ($row['currentVirulence'] < $minVirulence['value']) {
				$minVirulence['value'] = $row['currentVirulence'];
				$minVirulence['position'] = count($virulenceData) - 1;
			}
			if ($row['currentVirulence'] > $maxVirulence['value']) {
				$maxVirulence['value'] = $row['currentVirulence'];
				$maxVirulence['position'] = count($virulenceData) - 1;
			}
		}
		else {
			$virulenceData[] = 0;
			$minVirulence['value'] = 0;
			$minVirulence['position'] = count($virulenceData) - 1;
		}
	}
	$virulenceGraph = new Sparkline_Bar();
	$virulenceGraph -> setBarWidth(3);
	$virulenceGraph -> setBarSpacing(1);
	$virulenceGraph -> setColorHtml('red', '#FF0000');
	$virulenceGraph -> setColorHtml('green', '#00FF00');
	$virulenceGraph -> setColorHtml('blue', '#0000FF');
	$virulenceGraph -> setColorHtml('purple', '#660000');
	foreach ($virulenceData as $key => &$value) {
		if ($key == $minVirulence['position']) {
			$virulenceGraph -> setData($key, $value, 'green');
		}
		else {
			if ($key == $maxVirulence['position']) {
				$virulenceGraph -> setData($key, $value, 'red');
			}
			else {
				if ($key == count($dates) - 1) {
					$virulenceGraph -> setData($key, $value, 'blue');
				}
				else {
					$virulenceGraph -> setData($key, $value, 'purple');
				}
			}
		}
	}
	$virulenceGraph -> render(10);
	$virulenceGraph -> output();
}


function drawRankSparkline(&$postgreSQL,
								&$dates,
								&$numInterestingIPs,
								&$ip)
{
	$minRank['value'] = 4294967295;
	$maxRank['value'] = 0;
	foreach ($dates as &$date) {
		$result = pg_query($postgreSQL, 'SELECT "rank" FROM "InterestingIPs"."' . $date . '" ' .
																		'WHERE "ip" = \'' . $ip . '\'');
		if (pg_num_rows($result) > 0) {
			$row = pg_fetch_assoc($result);
			$rank = ($numInterestingIPs[$date] - ($row['rank'] - 1)) / $numInterestingIPs[$date];
			$rankData[] = $rank;
			if ($rank < $minRank['value']) {
				$minRank['value'] = $rank;
				$minRank['position'] = count($rankData) - 1;
			}
			if ($rank > $maxRank['value']) {
				$maxRank['value'] = $rank;
				$maxRank['position'] = count($rankData) - 1;
			}
		}
		else {
			$rankData[] = 0;
			$minRank['value'] = 0;
			$minRank['position'] = count($rankData) - 1;
		}
	}
	$rankGraph = new Sparkline_Bar();
	$rankGraph -> setBarWidth(3);
	$rankGraph -> setBarSpacing(1);
	$rankGraph -> setColorHtml('red', '#FF0000');
	$rankGraph -> setColorHtml('green', '#00FF00');
	$rankGraph -> setColorHtml('blue', '#0000FF');
	$rankGraph -> setColorHtml('purple', '#660000');
	foreach ($rankData as $key => &$value) {
		if ($key == $minRank['position']) {
			$rankGraph -> setData($key, $value, 'green');
		}
		else {
			if ($key == $maxRank['position']) {
				$rankGraph -> setData($key, $value, 'red');
			}
			else {
				if ($key == count($dates) - 1) {
					$rankGraph -> setData($key, $value, 'blue');
				}
				else {
					$rankGraph -> setData($key, $value, 'purple');
				}
			}
		}
	}
	$rankGraph -> render(10);
	$rankGraph -> output();
}

function drawServicesSparkline(&$postgreSQL,
							   &$monitoredServices,
							   &$interestingPortsIndex,
							   &$dates,
							   &$ip,
							   &$monitoredServiceName)
{
	$graph = new Sparkline_Bar();
	$graph -> setBarWidth(3);
	$graph -> setBarSpacing(1);
	$graph -> setColorHtml('purple', '#660000');
	
	$i = 0;
	foreach ($dates as &$date) {
		$result = pg_query($postgreSQL, 'SELECT "inboundPortIPs", "outboundPortIPs" FROM "HostTraffic"."' . $date . '" ' .
																		'WHERE "ip" = \'' . $ip . '\'');
		if (pg_num_rows($result) > 0) {
			$row = pg_fetch_assoc($result);
			$monitoredService = $monitoredServices[$monitoredServiceName];
			if ($monitoredService['initiator'] == 1) {
				$graph -> setData($i, arraySum($interestingPortsIndex, explode(',', substr($row['outboundPortIPs'], 1, -1)), $monitoredService['ports']), "purple");
			}
			else {
				$graph -> setData($i, arraySum($interestingPortsIndex, explode(',', substr($row['inboundPortIPs'], 1, -1)), $monitoredService['ports']), "purple");
			}
		}
		else {
			$graph->setData($i, 0);
		}
		$i++;
	}

	$graph -> render(10);
	$graph -> output();
}

function drawRoleSparkLine(&$postgreSQL,
						   &$dates,
						   &$ip,
						   &$roleNum)
{
	$graph = new Sparkline_Bar();
	$graph -> setBarWidth(3);
	$graph -> setBarSpacing(1);
	$graph -> setColorHtml('purple', '#660000');

	$i = 0;
	foreach ($dates as &$date) {
		$result = pg_query($postgreSQL, 'SELECT "numHosts" FROM "Roles"."' . $date . '" WHERE "ip" = \'' . $ip . '\' AND "role" = \'' . $roleNum . '\'');
		if (pg_num_rows($result) > 0) {
			$row = pg_fetch_assoc($result);
			$graph->setData($i, $row['numHosts'], 'purple');
		}
		else {
			$graph->setData($i, 0, 'purple');
		}
		foreach ($roles as &$role) {
			if (!$data[$role][count($data[$role]) - 1]) {
				$data[$role][] = 0;
			}
		}
		$i++;
	}

	$graph -> render(10);
	$graph -> output();
}

?>
