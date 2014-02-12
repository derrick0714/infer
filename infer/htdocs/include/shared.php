<?php
  include('localNetworks.php');

  $imsHome = dirname(__FILE__) . '/../../';

  $protocolNames = array(6 => 'TCP', 17 => 'UDP');

  $barColors = array('#00b263', '#31ff00', '#ceff00', '#ffe700', '#ffcf00',
                     '#ff9a00', '#ff6500', '#e7006b', '#940094');

  function fillFormField(&$submit, $default, &$current) {
    if ($submit) {
      if (strlen($current)) {
        return $current;
      }
    }
    else {
      if (strlen($default)) {
        return $default;
      }
    }
  }

  function fillCheckbox(&$submit, $default, &$current) {
    if ($submit) {
      if ($current) {
        return 'checked';
      }
    }
    else {
      if ($default != false) {
        return 'checked';
      }
    }
  }

  function message($message, $error = false) {
    echo '<div class="message" cellspacing="3">';
    if ($error) {
      echo '<b>' .
             'Error: ' .
           '</b>';
    }
    echo $message .
       '</div>';
  }

  function checked(&$checked, $default) {
    if ($_POST['add']) {
      if ($checked) {
        return 'checked';
      }
    }
    else {
      if ($default) {
        return 'checked';
      }
    }
  }

  function getLastInterestingDay(&$postgreSQL, &$ip) {
    $result = pg_query($postgreSQL, 'SELECT "dates"[array_upper("dates", 1)] FROM ' .
                                    '"Indexes"."interestingIPDates" WHERE "ip" = \'' . $ip . '\'');
    if (pg_num_rows($result)) {
      $row = pg_fetch_row($result);
      return $row[0];
    }
  }

  function getMACAddress(&$postgreSQL, &$date, &$ip) {
    $result = pg_query($postgreSQL, 'SELECT "mac" FROM "LiveIPs"."' . $date .
                                    '" WHERE "ip" = \'' . $ip . '\'');
    $row = pg_fetch_row($result);
    return $row[0];
  }

  function getRank(&$postgreSQL, &$date, &$ip) {
    $result = pg_query($postgreSQL, 'SELECT "rank" FROM "InterestingIPs"."' .
                                    $date . '" WHERE "ip" = \'' . $ip . '\'');
    $row = pg_fetch_row($result);
    return $row[0];
  }

  function getNumInterestingIPs(&$postgreSQL, &$date) {
    $result = pg_query($postgreSQL, 'SELECT COUNT(*) FROM "InterestingIPs"."' .
                                    $date . '"');
    $row = pg_fetch_row($result);
    return $row[0];
  }

  function loadServices(&$postgreSQL, &$serviceMap) {
    foreach (file('services.csv') as $serviceLine) {
      $serviceArray = explode(',', $serviceLine);
      foreach (explode(' ', $serviceArray[0]) as $asName) {
        foreach (explode(' ', $serviceArray[1]) as $portNumber) {
          if ($serviceArray[0] == '*') {
            $serviceMap['*'][$portNumber] = $serviceArray[2];
          }
          else {
            $serviceMap[getASNumberByName($postgreSQL, $asName)][$portNumber] = $serviceArray[2];
          }
        }
      }
    }
  }

  function serviceName(&$serviceMap, &$asn, &$port) {
    if ($serviceMap['*'][$port]) {
      return $serviceMap['*'][$port];
    }
    elseif ($serviceMap[$asn][$port]) {
      return $serviceMap[$asn][$port];
    }
  }

  function drawContentStripe(&$content, &$bytes) {
    global $barColors;
    $stripe = '<table width="100" height="8" align="center" border="0" cellpadding="4" cellspacing="0">' .
                '<tr>';
    $content = explode(',', substr($content, 1, -1));
    if ($bytes) {
      foreach ($content as $contentNumber => &$contentType) {
        $contentType = round((($contentType * 16384) / $bytes) * 100);
        $sum += $contentType;
        if ($contentType > 0) {
          $stripe .= '<td width="' . $contentType . '" style="background-color: ' . $barColors[$contentNumber] . ';"></td>';
        }
      }
    }
    if ($sum < 100) {
      $stripe .= '<td width="' . (100 - $sum) . '" style="background-color: ' . $barColors[8] . ';"></td>';
    }
    $stripe .= '</tr>' .
             '</table>';
    return $stripe;
  }

  function pad_number($number) {
    if (strlen($number) > 1) {
      return $number;
    }
    else {
      return '0' . $number;
    }
  }

  function size($bytes) {
    if ($bytes < 1024) {
      return $bytes . ' B';
    }
    else {
      $bytes /= 1024;
      if ($bytes < 1024) {
        return number_format($bytes, 2, '.', '') . ' KiB';
      }
      else {
        $bytes /= 1024;
        if ($bytes < 1024) {
          return number_format($bytes, 2, '.', '') . ' MiB';
        }
        else {
          $bytes /= 1024;
          if ($bytes < 1024) {
            return number_format($bytes, 2, '.', '') . ' GiB';
          }
          else {
            $bytes /= 1024;
            return number_format($bytes, 2, '.', '') . ' TiB';
          }
        }
      }
    }
  }
  
  function abssize($bytes) {
    $bytes = abs($bytes);
    if ($bytes < 1000) {
      return $bytes . ' B';
    }
    else {
      $bytes /= 1000;
      if ($bytes < 1000) {
        return number_format($bytes, 2) . ' KB';
      }
      else {
        $bytes /= 1000;
        if ($bytes < 1000) {
          return number_format($bytes, 2) . ' MB';
        }
        else {
          $bytes /= 1000;
          if ($bytes < 1000) {
            return number_format($bytes, 2) . ' GB';
          }
          else {
            $bytes /= 1000;
            return number_format($bytes, 2) . ' TB';
          }   
        }
      }
    }
  }

  function makeGetFromDate($date, $noHTML = false) {
    $date = explode('-', $date);
    if (!$noHTML) {
      $ampersand = '&amp;';
    }
    else {
      $ampersand = '&';
    }
    return '?year=' . $date[0] . $ampersand . 'month=' . $date[1] . $ampersand . 'day=' . $date[2];
  }

  function makeGet($exclude = NULL) {
    if ($_GET) {
      foreach ($_GET as $key => &$value) {
        if ($key != $exclude) {
          $get[] = $key . '=' . $value;
        }
      }
      return implode('&amp;', $get) . '&amp;';
    }
  }

  function makeUnion(&$dbName, &$tableNames) {
    foreach ($tableNames as &$tableName) {  
      $tableName = '`' . $dbName . '`.`' . $tableName . '`';
    }
    return implode(', ', $tableNames);
  }

  function existsPGSchema(&$postgreSQL, &$schemaName) {
    $result = pg_query($postgreSQL, 'SELECT "table_schema" FROM ' .
                                    '"information_schema"."tables" WHERE ' .
                                    '"table_schema" = \'' . $schemaName . '\'');
    return (pg_num_rows($result) > 0);
  }    

  function existsPGTable(&$postgreSQL, $schemaName, $tableName) {
    $result = pg_query($postgreSQL, 'SELECT "table_name" FROM ' .
                                    '"information_schema"."tables" WHERE ' .
                                    '"table_schema" = \'' . $schemaName . 
                                    '\' AND "table_name" = \'' . $tableName .
                                    '\'');
    return (pg_num_rows($result) > 0);
  }

  function getFirstPGTable(&$postgreSQL, $schemaName) {
    $result = pg_query($postgreSQL, 'SELECT "table_name" FROM ' .
                                    '"information_schema"."tables" WHERE ' .
                                    '"table_schema" = \'' . $schemaName .
                                    '\' ORDER BY "table_name" LIMIT 1');
    $row = pg_fetch_row($result);
    return $row[0];
  }

  function getLastPGTable(&$postgreSQL, $schemaName) {
    $result = pg_query($postgreSQL, 'SELECT "table_name" FROM ' .
                                    '"information_schema"."tables" WHERE ' .
                                    '"table_schema" = \'' . $schemaName .
                                    '\' ORDER BY "table_name" DESC LIMIT 1');
    $row = pg_fetch_row($result);
    return $row[0];
  }

  function getPGTableRange(&$postgreSQL, $schemaName, &$firstTable,
                           &$lastTable, $reverse = false) {
    $result = pg_query($postgreSQL, 'SELECT "table_name" FROM ' .
                                    '"information_schema"."tables" WHERE ' .
                                    '"table_schema" = \'' . $schemaName .
                                    '\' AND "table_name" >= \'' . $firstTable .
                                    '\' AND "table_name" <= \'' . $lastTable .
                                    '\' ORDER BY "table_name"');
    while ($row = pg_fetch_row($result)) {
      $tables[] = $row[0];
    }
    if ($reverse && count($tables) > 0) {
      $tables = array_reverse($tables);
    }
    return $tables;
  }

  function getPreviousPGTable(&$postgreSQL, $schemaName, &$tableName) {
    $result = pg_query($postgreSQL, 'SELECT "table_name" FROM ' .
                                    '"information_schema"."tables" WHERE ' .
                                    '"table_schema" = \'' . $schemaName .
                                    '\' AND "table_name" < \'' . $tableName .
                                    '\' ORDER BY "table_name" DESC LIMIT 1');
    if ($row = pg_fetch_row($result)) {
      return $row[0];
    }
  }

  function getNextPGTable(&$postgreSQL, $schemaName, &$tableName) {
    $result = pg_query($postgreSQL, 'SELECT "table_name" FROM ' .
                                    '"information_schema"."tables" WHERE ' .
                                    '"table_schema" = \'' . $schemaName .
                                    '\' AND "table_name" > \'' . $tableName .
                                    '\' ORDER BY "table_name" LIMIT 1');
    if ($row = pg_fetch_row($result)) {
      return $row[0];
    }
  }

  function getPreviousPGDay(&$postgreSQL, &$schemaName, &$tableName,
                            $documentName = 'report.php') {
    if ($previousTableName = getPreviousPGTable($postgreSQL, $schemaName,
                         $tableName)) {
      return '<a href="' . $documentName . '/' . $previousTableName .
             '">« Previous Day</a>';
    }
  }

  function getNextPGDay(&$postgreSQL, &$schemaName, &$tableName,
                        $documentName = 'report.php') {
    if ($nextTableName = getNextPGTable($postgreSQL, $schemaName, $tableName)) {
      return '<a href="' . $documentName . '/' . $nextTableName .
             '">Next Day »</a>';
    }
  }

  function getNetworkStatistic(&$postgreSQL, &$date, $column) {
    if (existsPGTable($postgreSQL, 'NetworkTraffic', $date)) {
      $result = pg_query($postgreSQL, 'SELECT "' . $column . '" FROM ' .
                                      '"NetworkTraffic"."' . $date . '"');
      $row = pg_fetch_row($result);
      return $row[0];
    }
    else {
      return getCachedNetworkStatistic($postgreSQL, $date, $column);
    }
  }

  function getCachedNetworkStatistic(&$postgreSQL, &$tableName, $columnName) {
    $result = pg_query($postgreSQL, 'SELECT "' . $columnName . '" FROM ' .
                                    '"DisplayedNetworkStats"."' . $tableName .
                                    '"');
    $row = pg_fetch_row($result);
    return $row[0];
  }

  function getNumFlows(&$postgreSQL, $date, $ip) {
    if (existsPGTable($postgreSQL, 'HostTraffic', $date)) {
      $result = pg_query($postgreSQL, 'SELECT "inboundFlows", "outboundFlows" ' .
                                      ' FROM "HostTraffic"."' . $date . '" WHERE ' .
                                      '"ip" = \'' . $ip . '\'');
      $row = pg_fetch_row($result);
      return array_sum(explode(',', substr($row[0], 1, -1))) + array_sum(explode(',', substr($row[1], 1, -1)));
    }
    else {
      return getCachedHostStatistic($postgreSQL, $date, $ip, 'numFlows');
    }
  }
     
  function getNumPackets(&$postgreSQL, $date, $ip) {
    if (existsPGTable($postgreSQL, 'HostTraffic', $date)) {
      $result = pg_query($postgreSQL, 'SELECT "inboundPackets", "outboundPackets" ' .
                                      ' FROM "HostTraffic"."' . $date . '" WHERE ' .
                                      '"ip" = \'' . $ip . '\'');
      $row = pg_fetch_row($result);
      return array_sum(explode(',', substr($row[0], 1, -1))) + array_sum(explode(',', substr($row[1], 1, -1)));
    }
    else {
      return getCachedHostStatistic($postgreSQL, $date, $ip, 'numPackets');
    }
  }

  function getNumBytes(&$postgreSQL, $date, $ip) {
    $result = pg_query($postgreSQL, 'SELECT "inboundBytes", "outboundBytes" ' .
                                    ' FROM "HostTraffic"."' . $date . '" WHERE ' .
                                    '"ip" = \'' . $ip . '\'');
    $row = pg_fetch_assoc($result);
    return $row['inboundBytes'] + $row['outboundBytes'];
  }

  function getFanout(&$postgreSQL, $date, $ip) {
    $result = pg_query($postgreSQL, 'SELECT external_host_count FROM "InternalContacts"."' . $date . '" WHERE ' .
									'internal_ip = \'' . $ip . '\'');
    $row = pg_fetch_row($result);
	return $row[0];
  }

  function getCachedHostStatistic(&$postgreSQL, $tableName, $ip,
                                  $columnName) {
    $result = pg_query($postgreSQL, 'SELECT "' . $columnName . '" FROM ' .
                                    '"DisplayedHostStats"."' . $tableName .
                                    '" WHERE "ip" = \'' . $ip . '\'');
    $row = pg_fetch_row($result);
    return $row[0];
  }

  function getASNameByNumber(&$postgreSQL, &$asNumber) {
    $result = pg_query($postgreSQL, 'SELECT "asName" FROM "Maps".' .
                                    '"asNames" WHERE "asNumber" = \'' .
                                    $asNumber . '\'');
    $row = pg_fetch_row($result);
    return $row[0];
  }

  function getASDescriptionByNumber(&$postgreSQL, &$asNumber) {
    $result = pg_query($postgreSQL, 'SELECT "asDescription" FROM "Maps".' .
                                    '"asNames" WHERE "asNumber" = \'' .
                                    $asNumber . '\'');
    $row = pg_fetch_row($result);
    return $row[0];
  }

  function getASNumberByName(&$postgreSQL, &$asName) {
    $result = pg_query($postgreSQL, 'SELECT "asNumber" FROM "Maps"."asNames" ' .
                                    'WHERE "asName" = \'' . $asName . '\'');
    $row = pg_fetch_row($result);
    return $row[0];
  }

  function getASNByIP(&$postgreSQL, &$ip) {
    $result = pg_query($postgreSQL, 'SELECT "asn" FROM ' .
                                    '"Maps"."asIPBlocks" WHERE ' .
                                    '"firstIP" <= \'' . $ip . '\' AND ' .
                                    '"lastIP" >= \'' . $ip . '\' ORDER BY ' .
                                    '"lastIP" - "firstIP" LIMIT 1');
    if ($row = pg_fetch_row($result)) {
      return $row[0];
    }
    return 0;
  }

  function getCountryNumberByIP(&$postgreSQL, &$ip) {
    $result = pg_query($postgreSQL, 'SELECT "countryNumber" FROM ' .
                                    '"Maps"."countryIPBlocks" WHERE ' .
                                    '"firstIP" <= \'' . $ip . '\' AND ' .
                                    '"lastIP" >= \'' . $ip . '\'');
    if ($row = pg_fetch_row($result)) {
      return $row[0];
    }
    return 0;
  }

  function getCountryNameMap(&$postgreSQL, &$countryCodeMap,
                             &$countryNameMap) {
    $result = pg_query($postgreSQL, 'SELECT "countryNumber", "countryCode", ' .
                                    '"countryName" FROM "Maps"."countryNames"');
    while ($row = pg_fetch_row($result)) {
      $countryCodeMap[$row[0]] = $row[1];
      $countryNameMap[$row[0]] = $row[2];
    }
  }

  function getCountryPicture(&$countryNumber, &$countryCodeMap,
                             &$countryNameMap) {
    global $imsHome;
    if ($countryNumber !== NULL) {

      $fileName = $countryCodeMap[$countryNumber];
      $name = $countryNameMap[$countryNumber];
      if (@file_exists($imsHome . '/htdocs/images/flags/' . $fileName . '.png')) {
        return '<img src="' . '/images/flags/' .
               $fileName . '.png" ' . 'title="' .
               $name . '" ' . 'alt="' . 
               $name . '" style="vertical-align:middle; border:none;">';
      }
    }
  }

  function getInfectedIPs(&$postgreSQL, &$tableName, &$infectedIPs) {
    $result = pg_query($postgreSQL, 'SELECT "ip" FROM "InfectedIPs"."' .
                                    $tableName . '"');
    while ($row = pg_fetch_row($result)) {
      $infectedIPs[] = $row[0];
    }
  }

  function isInteresting(&$postgreSQL, &$tableName, &$ip) {
    $result = pg_query($postgreSQL, 'SELECT "ip" FROM "InterestingIPs"."' .
                                    $tableName . '" WHERE "ip" = \'' . $ip .
                                    '\'');
    return (@pg_num_rows($result) > 0);
  }

  function isInfected(&$postgreSQL, &$tableName, &$ip) {
    $result = pg_query($postgreSQL, 'SELECT "ip" FROM "InfectedIPs"."' .
                                    $tableName . '" WHERE "ip" = \'' . $ip .
                                    '\'');
    return (pg_num_rows($result) > 0);
  }

  function styleIP(&$infectedIPs, &$ip, $linkToInternal = true) {
    $styledIP = long2ip($ip);
    if ($linkToInternal && isInternal($ip)) {
      $styledIP = '<a class="text" href="/host/' . $styledIP . '">' .
                    $styledIP .
                  '</a>';
    }
    if ($infectedIPs && array_search($ip, $infectedIPs) !== false) {
      $styledIP = '<font color="#FF0000">' .
                    $styledIP .
                  '</font>';
    }
    return $styledIP;
  }

  function styleInternalPort(&$port, &$initiator) {
    if ($initiator == 1) {
      return '<b>' .
               $port .
             '</b>';
    }
    return $port;
  }

  function styleExternalPort(&$port, &$initiator) {
    if ($initiator == 2) {
      return '<b>' .
               $port .
             '</b>';
    }
    return $port;
  }

  function duration($time) {
    if ($time < 1) {
      return '<1s';
    }
    $years = floor($time / (60 * 60 * 24 * 365));
    $time %= 60 * 60 * 24 * 365;
    $weeks = floor($time / (60 * 60 * 24 * 7));
    $time %= 60 * 60 * 24 * 7;
    $days = floor($time / (60 * 60 * 24));
    $time %= 60 * 60 * 24;
    $hours = floor($time / (60 * 60));
    $time %= 60 * 60;
    $minutes = floor($time / 60);
    $seconds = $time % 60;
    if ($years) {
      $duration .= $years . 'y ';
    }
    if ($weeks) {
      $duration .= $weeks . 'w ';
    }
    if ($days) {
      $duration .= $days . 'd ';
    }
    if ($hours) {
      $duration .= $hours . 'h ';
    }
    if ($minutes) {
      $duration .= $minutes . 'm ';
    }
    if ($seconds) {
      $duration .= $seconds . 's';
    }
    if (!$duration) {
      $duration = '<1s';
    }
    return $duration;
  }

  function makeLogo(&$postgreSQL, &$schemaName, &$tableName, $title, $prefix,
                    $postfix = '') {
    $previousPGTable = getPreviousPGTable($postgreSQL, $schemaName, $tableName);
    $nextPGTable = getNextPGTable($postgreSQL, $schemaName, $tableName);
    $logo = '<table width="100%">' .
              '<tr>' .
                '<td class="sub_title" width="15%" align="left">';
    if ($previousPGTable) {
      $logo .= '<a href="' . $prefix . '/' . $previousPGTable . $postfix . '">' .
                 '« Previous Day' .
               '</a>';
    }
    $logo .= '</td>' .
               '<td width="60%"align="center">' .
                 $title .
               '</td>' .
               '<td class="sub_title" width="15%" align="right">';
    if ($nextPGTable) {
      $logo .= '<a href="' . $prefix . '/' . $nextPGTable . $postfix . '">' .
                 'Next Day »' .
               '</a>';
    }
    $logo .= '</td>' .
           '</tr>' .
         '</table>';
    return $logo;
  }

  function getPreviousHostDay(&$postgreSQL, &$date, &$ip) {
    $result = pg_query($postgreSQL, 'SELECT "dates" FROM ' .
                       '"Indexes"."interestingIPDates" WHERE "ip" = \'' . $ip .
                       '\'');
    $row = pg_fetch_row($result);
    $dates = explode(',', substr($row[0], 1, -1));
    return $dates[array_search($date, $dates) - 1];
  }

  function makeHostLogo(&$postgreSQL, &$date, &$ip, &$title) {
    $result = pg_query($postgreSQL, 'SELECT "dates" FROM ' .
                       '"Indexes"."interestingIPDates" WHERE "ip" = \'' . $ip .
                       '\'');
    $row = pg_fetch_row($result);
    $dates = explode(',', substr($row[0], 1, -1));
    $index = array_search($date, $dates);
    $logo = '<table width="100%">' .
              '<tr>' .
                '<td class="sub_title" width="15%" align="left">';
    if ($index) {
      $logo .= '<a href="/host/' . long2ip($ip) . '/' . $dates[$index - 1] . '">' .
                 '« Previous Day' .
               '</a>';
    }
    $logo .= '</td>' .
             '<td class="sub_title" width="15%" align="right">';
    if ($index < count($dates) - 1) {
      $logo .= '<a href="/host/' . long2ip($ip) . '/' . $dates[$index + 1] . '">' .
                 'Next Day »' .
               '</a>';
    }
    $logo .= '</td>' .
           '</tr>' .
         '</table>';
    return $logo;
  }

  function getHostRoles(&$postgreSQL, &$tableName, &$ip) {
    if (existsPGTable($postgreSQL, 'HostTraffic', $tableName)) {
      $schemaName = 'HostTraffic';
    }
    else {
      $schemaName = 'HostTrafficStats';
    }
    $result = pg_query($postgreSQL, 'SELECT "roles" FROM "' . $schemaName . '"."' .
                       $tableName . '" WHERE "ip" = \'' . $ip . '\'');
    if (pg_num_rows($result) > 0) {
      $row = pg_fetch_row($result);
      return $row[0];
    }
  }

	function makeNavMsg(array $url, $links = true) {
		$navMessage = '';
		if ($links === true) {
			$navMessage .= '<a href="/" class="text">INFER</a>';
		} else {
			$navMessage .= 'INFER';
		}
		foreach ($url as $idx => $page) {
			if (!$page) {
				break;
			}
			$navMessage .= ' - ';
			if ($links === true) {
				$navMessage .= '<a href="/' . 
								implode('/', array_slice($url, 0, $idx + 1)) .
								'" class="text">' . $page . '</a>';
			} else {
				$navMessage .= $page;
			}
		}

		return $navMessage;
	}

	function makeHeader(&$postgreSQL, $title, $doPrevNext = NULL, $baseURL = NULL, $appendURL = NULL, $schema = NULL, $date = NULL) {
		$previousDay = '';
		$nextDay = '';

		$isSession = isSession($postgreSQL, $_COOKIE['imsSessionID']);
		
		if ($doPrevNext === true) {
			if ($previousTable = getPreviousPGTable($postgreSQL, $schema, $date)) {
				$previousDay =
					'<td>' .
						'<a class="text" href="' . $baseURL . '/' . 
						$previousTable . ($appendURL?'/' . $appendURL:'') .
						'">' .
							'« Previous Day' .
						'</a>' .
					'</td>';
			} else {
				$previousDay =
					'<td>' .
						'« Previous Day' .
					'</td>';
			}
			$previousDay .=
				'<td class="center">' .
					'|' .
				'</td>';
			$nextDay =
				'<td class="center">' .
					'|' .
				'</td>';

			if ($nextTable = getNextPGTable($postgreSQL, $schema, $date)) {
				$nextDay .=
					'<td>' .
						'<a class="text" href="' . $baseURL . '/' . 
						$nextTable . ($appendURL?'/' . $appendURL:'') .
						'">' .
							'Next Day »' .
						'</a>' .
					'</td>';
			} else {
				$nextDay .=
					'<td>' .
						'Next Day »' .
					'</td>';
			}
		}

		// now display the header...

		echo
			'<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">'. 
			'<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">'.
			'<head>' .
				'<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">' .
				'<link rel="stylesheet" type="text/css" href="/css/ims.css" />' .
				'<link rel="stylesheet" type="text/css" href="/css/menu.css.boris" />' .
				'<link rel="stylesheet" type="text/css" href="/css/host-page-style.css" />' .
				'<link rel="stylesheet" type="text/css" href="/css/host-page-identity-style.css" />' .
				'<link rel="stylesheet" type="text/css" href="/css/host-page-infection-style.css" />' .
				'<link rel="stylesheet" type="text/css" href="/css/toggling.css" />' .
				'<link rel="stylesheet" type="text/css" href="/css/host-page-environment-style.css" />' .
				'<link rel="stylesheet" type="text/css" href="/css/tooltip.css" />' .
				'<link rel="stylesheet" type="text/css" href="/css/search.css" />' .
				'<title>' .
					$title .
				'</title>' .
			'</head>' .
			'<script language="javascript" type="text/javascript">' .
				'/* <![CDATA[ */' .
				'function toggleTabs(turnOn, turnOff) {' .
					'var onTab = document.getElementsByClassName(turnOn);' .
					'var offTab= document.getElementsByClassName(turnOff);' .
					'onTab[0].style.display =\'none\';' .
					'offTab[0].style.display=\'block\';' .
				'}' .
				'/* ]]> */' .
			'</script>' .
			'<body>'; 
		echo 
			'<div id="navbar">' .
				'<table width="100%">' .
					'<tr>' .
						$previousDay;
		if ($isSession) {
			echo 
						'<td class="center">' .
							'<a class="text" href="/">' .
								'Home' .
							'</a>' .
						'</td>' .
						'<td class="center">' .
							'|' .
						'</td>' .
						'<td width="150">' .
							'<div id="menu">' .
								'<ul>' .
									'<li>' .
										'<div class="root_menu">' .
											'Settings' .
										'</div>' .
									 	'<ul>';
			if (getPrivileges($postgreSQL, $_COOKIE['imsSessionID']) & ADMINISTRATOR_PRIVILEGE) {
				echo 
											'<li>' .
												'<a href="/users/">' .
													'User Management' .
												'</a>' .
											'</li>' .
											'<li>' .
												'<a href="/config/">' .
													'Backend Configuration' .
												'</a>' .
											'</li>';
			}
			echo
											'<li>' .
												'<a href="/ldap">' .
													'LDAP Management' .
												'</a>' .
											'</li>' .
											'<li>' .
												'<a href="/nonDNSTrafficWhiteList">' .
													'DNS White List' .
												'</a>' .
											'</li>' .
											'<li>' .
												'<a href="/whiteList/">' .
													'White List' .
												'</a>' .
											'</li>' .
											'<li>' .
												'<a href="/rankWhiteList">' .
													'Rank White List' .
												'</a>' .
											'</li>' .
											'<li>' .
												'<a href="/infectedIPs">' .
													'Infected IPs List' .
												'</a>' .
											'</li>' .
										'</ul>' .
									'</li>' .
								'</ul>' .
							'</div>' .
						'</td>' .
						'<td class="center">' .
							'|' .
						'</td>' .
						'<td class="center">' .
							'<a class="text" href="/archive/">' .
								'Archive' .
							'</a>' .
						'</td>' .
						'<td class="center">' .
							'|' .
						'</td>' .
						'<td class="center">' .
							'<a class="text" href="/processStats">' .
								'Analytics Statistics' .
							'</a>' .
						'</td>' .
						'<td class="center">' .
							'|' .
						'</td>' .
						'<td class="center">' .
							'<a class="text" href="/search/">' .
								'Search' .
							'</a>' .
						'</td>' .
						'<td class="center">' .
							'|' .
						'</td>' .
						'<td class="center">' .
							'<a class="text" href="/logout">' .
								'Log Out' .
							'</a>' .
						'</td>';
		}
		echo
						$nextDay .
					'</tr>' .
				'</table>' .
			'</div>';
		include('checkJavascript.php');
	}

function display_related_hosts (&$postgreSQL, $numeric_ip, $date) {
	if (!existsPGTable($postgreSQL, "MutualContacts", $date)) {
		return ''; 
	}   

	$result = pg_query($postgreSQL, 'SELECT related_ip FROM ' .
									'"MutualContacts"."' . $date . '" WHERE ' .
									'ip = \'' . $numeric_ip . '\' ' .
									'ORDER BY score DESC LIMIT 4');
	if (pg_num_rows($result) <= 1) {
		return ''; 
	}

	pg_fetch_row($result); // skip self
	while ($row = pg_fetch_row($result)) {
		$related_hosts[] = '<a href="/host/' . long2ip($row[0]) . '/' . $date .
							'" class="text">' . long2ip($row[0]) . '</a>';
	}   
	
	$r = ''; 
	$r .=  '<div id="related_hosts">';
	$r .=   'The following hosts are believed to be related to this one:';
	$r .=   '<blockquote id="related_hosts_block">';
	$r .=	   implode(', ', $related_hosts);
	$r .=   '</blockquote>';
	$r .=  '</div>';
	
	return $r; 
}

?>
