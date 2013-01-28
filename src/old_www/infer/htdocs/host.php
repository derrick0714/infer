<?php
  include('include/postgreSQL.php');
  include('include/ldap.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  include('include/roles.php');
  include('include/snort.php');
  include('include/services.php');
  include('include/bruteForcers.php');
  include('include/role.php');
  include('include/symptom.php');
  include('include/rankWhiteList.php');

  $imsHome = dirname(__FILE__) . '/..';
  $request = explode('/', substr($_SERVER['PATH_INFO'], 1));
  $ip = $request[0];
  $numericIP = sprintf('%u', ip2long($ip));
  if ($request[1]) {
    $date = $request[1];
  }
  else {
    $date = getLastInterestingDay($postgreSQL, $numericIP);
    if (!$date) {
      exit;
    }
    header('Location: /host/' . $ip . '/' . $date);
    exit;
  }

  if (!existsPGTable($postgreSQL, 'InterestingIPs', $date)) {
    exit;
  }

  if (!existsPGTable($postgreSQL, 'TopPorts', $date)) {
    header('Location: /host2/' . $ip . '/' . $date);
    exit;
  }

  if (!isInteresting($postgreSQL, $date, $numericIP)) {
    header('Location: /');
    exit;
  }

  if (existsPGTable($postgreSQL, 'InfectedIPs', $date)) {
    getInfectedIPs($postgreSQL, $date, $infectedIPs);
  }

  $symptomDescriptions = array('infectedContactScore' => 'It was in contact with known infected hosts',
                               'evasiveTrafficScore' => 'It has sent or received evasive traffic',
                               'darkSpaceSourceScore' => 'It has attempted to access unused address space',
                               'darkSpaceTargetScore' => 'It was in contact with hosts who have attempted to access unused address space',
                               'nonDNSTrafficScore' => 'It has contacted hosts without using DNS',
                               'rebootScore' => 'It has rebooted multiple times');

  $symptomNames = array('infectedContactScore' => 'Contact with Infected Hosts',
                        'evasiveTrafficScore' => 'Evasive Traffic',
                        'darkSpaceSourceScore' => 'Dark Space Source',
                        'darkSpaceTargetScore' => 'Dark Space Target',
                        'nonDNSTrafficScore' => 'Protocol Violations',
                        'rebootScore' => 'Reboots');

  $symptomURLs = array('infectedContactScore' => 'symptom/infectedContacts',
                       'evasiveTrafficScore' => 'symptom/evasiveTraffic',
                       'darkSpaceSourceScore' => 'symptom/darkSpaceSources',
                       'darkSpaceTargetScore' => 'symptom/darkSpaceTargets',
                       'nonDNSTrafficScore' => 'symptom/nonDNSTraffic',
                       'rebootScore' => 'reboots');

  $contentNames = array('Plaintext', 'BMP image', 'WAV audio', 'Compressed',
                        'JPEG image', 'MP3 audio', 'MPEG video', 'Encrypted',
                        'Unclassified');

  function makeReasons(&$postgreSQL, &$ip, &$date, &$infectedIPs, &$roles, &$interestingIPRow, &$symptomDescriptions) {
    $displayedRoles = array(0 => BRUTE_FORCER,
                            1 => BRUTE_FORCED,
                            2 => DARK_SPACE_BOT,
                            3 => SPAM_BOT,
                            4 => MAIL_SERVER_BOT,
                            5 => WEB_SERVER_BOT,
                            6 => SCAN_BOT);
    if (count($roles)) {
      foreach ($roles as $role => &$properties) {
        if (array_search($role, $displayedRoles) !== false) {
          switch ($role) {
            case BRUTE_FORCER:
              $reasons .= 'It has attempted to brute force ' . makeBruteForcerServicesLine($ip, $date, $properties['ports']) .
                          ' in ' . duration($properties['endTime'] - $properties['startTime']) . ' starting at ' .
                          date('g:i:s A', $properties['startTime']);
              if (count($properties['ports']) > 1 || $properties['ports'][0] != 21) {
                if (count($properties['commChannels'])) {
                  $reasons .= '<li type="square" class="detail">' .
                                '<span>' .
                                  'It is a brute forcer bot and is possibly being controlled by ' . get_comm_channels($postgreSQL, $ip, $date, $infectedIPs);
                }
                else {
                  $reasons .= '<li type="square" class="detail">' .
                                '<span>' .
                                  'It is a brute forcer bot';
                }
			    $isWhiteListed = isRankWhitelisted($postgreSQL, $date, $interestingIPRow['ip'], 'bruteForcerBot');
	  		    if ($isWhiteListed === true) {
				  $reasons .= 
								   ' (' .
								   '<a class="tooltip" href="/rankWhiteList/remove/' . long2ip($interestingIPRow['ip']) . '/' . 'bruteForcerBot' . '">' .
									 'W-' .
									 '<span>Remove "Brute Forcer Bot" from whitelist for this host</span>' .
								   '</a>)';
			    }
			    else if ($isWhiteListed === false) {
				  $reasons .= 
								   ' (' .
								   '<a class="tooltip" href="/rankWhiteList/add/' . long2ip($interestingIPRow['ip']) . '/' . 'bruteForcerBot' . '">' .
									 'W+' .
									 '<span>Add "Brute Forcer Bot" to whitelist for this host</span>' .
								   '</a>)';
			    }
			    $reasons .= '</span>' .
                              '</li>';
              }
              break;
            case BRUTE_FORCED:
              $reasons .= makeBruteForcedServicesLine($ip, $date, $properties['ports']) .
                          ' in ' . duration($properties['endTime'] - $properties['startTime']) . ' starting at ' .
                          date('g:i:s A', $properties['startTime']);
			  /*
			  $isWhiteListed = isRankWhitelisted($postgreSQL, $date, $interestingIPRow['ip'], 'bruteForced');
	  		  if ($isWhiteListed === true) {
				$reasons .= 
								   ' (' .
								   '<a class="tooltip" href="/rankWhiteList/remove/' . long2ip($interestingIPRow['ip']) . '/' . 'bruteForced' . '">' .
									 'W-' .
									 '<span>Remove "Brute Forced" from whitelist for this host</span>' .
								   '</a>)';
			  }
			  else if ($isWhiteListed === false) {
				$reasons .= 
								   ' (' .
								   '<a class="tooltip" href="/rankWhiteList/add/' . long2ip($interestingIPRow['ip']) . '/' . 'bruteForced' . '">' .
									 'W+' .
									 '<span>Add "Brute Forced" to whitelist for this host</span>' .
								   '</a>)';
			  }
			  */
			  $reasons .= '<br />';
              break;
            case DARK_SPACE_BOT:
              $reasons .= 'It has attempted to contact <a class="text" href="/symptom/darkSpaceSources/' . $date . '/' . long2ip($ip) . '">' .
                          number_format($properties['numHosts']) . ' inactive hosts</a> in ' .
                          duration($properties['endTime'] - $properties['startTime']) . ' starting at ' .
                          date('g:i:s A', $properties['startTime']);
              if (count($properties['commChannels'])) {
                $reasons .= '<li type="square" class="detail">' .
                              '<span>' .
                                'It is a dark space bot and is possibly being controlled by ' . get_comm_channels($postgreSQL, $ip, $date, $infectedIPs);
              }
              else {
                $reasons .= '<li type="square" class="detail">' .
                              '<span>' .
                                'It is a dark space bot';
              }
			  $isWhiteListed = isRankWhitelisted($postgreSQL, $date, $interestingIPRow['ip'], 'darkSpaceBot');
	  		  if ($isWhiteListed === true) {
				$reasons .= 
								   ' (' .
								   '<a class="tooltip" href="/rankWhiteList/remove/' . long2ip($interestingIPRow['ip']) . '/' . 'darkSpaceBot' . '">' .
									 'W-' .
									 '<span>Remove "Dark Space Bot" from whitelist for this host</span>' .
								   '</a>)';
			  }
			  else if ($isWhiteListed === false) {
				$reasons .= 
								   ' (' .
								   '<a class="tooltip" href="/rankWhiteList/add/' . long2ip($interestingIPRow['ip']) . '/' . 'darkSpaceBot' . '">' .
									 'W+' .
									 '<span>Add "Dark Space Bot" to whitelist for this host</span>' .
								   '</a>)';
			  }
			  $reasons .= '</span>' .
                            '</li>';
              break;
            case SPAM_BOT:
              $reasons .= 'It has sent <a class="text" href="/role/mailClient/' . $date . '/' . long2ip($ip) . '">' .
                          ' e-mail to ' . number_format($properties['numHosts']) . ' hosts</a> in ' .
                          duration($properties['endTime'] - $properties['startTime']) . ' starting at ' .
                          date('g:i:s A', $properties['startTime']);
              if (count($properties['commChannels'])) {
                $reasons .= '<li type="square" class="detail">' .
                              '<span>' .
                                'It is a spam bot and is possibly being controlled by ' . get_comm_channels($postgreSQL, $ip, $date, $infectedIPs);
              }
              else {
                $reasons .= '<li type="square" class="detail">' .
                              '<span>' .
                                'It is a spam bot';
              }
			  $isWhiteListed = isRankWhitelisted($postgreSQL, $date, $interestingIPRow['ip'], 'spamBot');
	  		  if ($isWhiteListed === true) {
				$reasons .= 
								   ' (' .
								   '<a class="tooltip" href="/rankWhiteList/remove/' . long2ip($interestingIPRow['ip']) . '/' . 'spamBot' . '">' .
									 'W-' .
									 '<span>Remove "Spam Bot" from whitelist for this host</span>' .
								   '</a>)';
			  }
			  else if ($isWhiteListed === false) {
				$reasons .= 
								   ' (' .
								   '<a class="tooltip" href="/rankWhiteList/add/' . long2ip($interestingIPRow['ip']) . '/' . 'spamBot' . '">' .
									 'W+' .
									 '<span>Add "Spam Bot" to whitelist for this host</span>' .
								   '</a>)';
			  }
			  $reasons .= '</span>' .
                            '</li>';
              break;
            case SCAN_BOT:
              $reasons .= 'It has scanned <a class="text" href="/scanners/' . $date . '/' . long2ip($ip) . '">' .
                          number_format($properties['numHosts']) . ' hosts</a> in ' .
                          duration($properties['endTime'] - $properties['startTime']) . ' starting at ' .
                          date('g:i:s A', $properties['startTime']);
              if (count($properties['commChannels'])) {
                $reasons .= '<li type="square" class="detail">' .
                              '<span>' .
                                'It is a scan bot and is possibly being controlled by ' . get_comm_channels($postgreSQL, $ip, $date, $infectedIPs);
              }
              else {
                $reasons .= '<li type="square" class="detail">' .
                              '<span>' .
                                'It is a scan bot';
              }
			  $isWhiteListed = isRankWhitelisted($postgreSQL, $date, $interestingIPRow['ip'], 'scanBot');
	  		  if ($isWhiteListed === true) {
				$reasons .= 
								   ' (' .
								   '<a class="tooltip" href="/rankWhiteList/remove/' . long2ip($interestingIPRow['ip']) . '/' . 'scanBot' . '">' .
									 'W-' .
									 '<span>Remove "Scan Bot" from whitelist for this host</span>' .
								   '</a>)';
			  }
			  else if ($isWhiteListed === false) {
				$reasons .= 
								   ' (' .
								   '<a class="tooltip" href="/rankWhiteList/add/' . long2ip($interestingIPRow['ip']) . '/' . 'scanBot' . '">' .
									 'W+' .
									 '<span>Add "Scan Bot" to whitelist for this host</span>' .
								   '</a>)';
			  }
			  $reasons .= '</span>' .
                            '</li>';
              break;
          }
        }
      }
    }
    foreach ($symptomDescriptions as $columnName => &$description) {
      if ($interestingIPRow[$columnName]) {
        $reasons .= $description;
		$isWhiteListed = isRankWhitelisted($postgreSQL, $date, $interestingIPRow['ip'], substr($columnName, 0, -5));
		if ($isWhiteListed === true) {
			$reasons .= 
							   ' (' .
							   '<a class="tooltip" href="/rankWhiteList/remove/' . long2ip($interestingIPRow['ip']) . '/' . substr($columnName, 0, -5) . '">' .
							     'W-' .
								 '<span>Remove symptom from whitelist for this host</span>' .
							   '</a>)';
		}
		else if ($isWhiteListed === false) {
			$reasons .= 
							   ' (' .
							   '<a class="tooltip" href="/rankWhiteList/add/' . long2ip($interestingIPRow['ip']) . '/' . substr($columnName, 0, -5) . '">' .
							     'W+' .
								 '<span>Add symptom to whitelist for this host</span>' .
							   '</a>)';
		}
		$reasons .= '<br />';
      }
    }
    return $reasons;
  }

  function arraySum(&$array, &$indexes) {
    foreach ($indexes as &$index) {
      $sum += $array[$index];
    }
    return $sum;
  }

  function makeHostRoles(&$postgreSQL, &$imsHome, &$date, &$ip,
                         &$roles, &$hostTrafficRow,
                         &$inboundPortActivityTimes, &$outboundPortActivityTimes,
                         &$inboundPortTraffic, &$outboundPortTraffic,
                         &$inboundServicePorts, &$outboundServicePorts,
                         &$inboundPortFlows, &$outboundPortFlows) {
  global $rolesConf;
    global $roleNames;
    $displayedRoles = array(0 => MULTIMEDIA_P2P_NODE,
                            1 => TELNET_BRUTE_FORCER,
                            2 => TELNET_BRUTE_FORCED,
                            3 => FTP_BRUTE_FORCER,
                            4 => FTP_BRUTE_FORCED,
                            5 => SSH_BRUTE_FORCER,
                            6 => SSH_BRUTE_FORCED,
                            7 => SCAN_BOT,
                            8 => UNCLASSIFIED_P2P_NODE,
                            9 => ENCRYPTED_P2P_NODE,
                            10 => MICROSOFT_SQL_BRUTE_FORCER,
                            11 => MICROSOFT_SQL_BRUTE_FORCED,
                            12 => ORACLE_SQL_BRUTE_FORCER,
                            13 => ORACLE_SQL_BRUTE_FORCED,
                            14 => MYSQL_BRUTE_FORCER,
                            15 => MYSQL_BRUTE_FORCED,
                            16 => POSTGRESQL_BRUTE_FORCER,
                            17 => POSTGRESQL_BRUTE_FORCED);
    if ($hostTrafficRow['outboundBytes']) {
      $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
      $portServices .= '<tr class="' . $rowClass . '">' .
                         '<td class="center">' .
                           '<b>' .
                             '<i>' .
                               'Publisher' .
                             '</i>' .
                           '</b>' .
                         '</td>' .
                         '<td>' .
                         '</td>' .
                         '<td>' .
                         '<td class="center">' .
                           size($hostTrafficRow['outboundBytes']) .
                         '</td>' .
                         '<td class="center">' .
                           date('g:i:s A', $hostTrafficRow['firstOutboundTime']) .
                         '</td>' .
                         '<td class="center">' .
                           duration($hostTrafficRow['lastOutboundTime'] - $hostTrafficRow['firstOutboundTime']) .
                         '</td>' .
                       '</tr>';
      ++$rowNumber;
    }
    foreach($inboundServicePorts as $name => &$ports) {
      $sum = arraySum($inboundPortTraffic, $ports);
      $flowSum = arraySum($inboundPortFlows, $ports);
      if ($sum) {
        $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
        $startTime = getRoleStartTime($ports, $inboundPortActivityTimes);
		$roleName = str_replace(' ', '', $name);
		$roleName = strtolower(substr($roleName, 0, 1)) . substr($roleName, 1);
		$roleDescription = getRoleDescription($roleName, $rolesConf);
        $portServices .= '<tr class="' . $rowClass . '">' .
                           '<td class="center">' .
                             '<a class="text" href="/role/' . $roleName . '/' . $date . '/' . long2ip($ip) . '">' .
                               $name .
                             '</a>' .
							 ' ' .
							 '<a class="tooltip" href="#">' .
							   '?' .
							   '<span>' . $roleDescription . '</span>' .
							 '</a>' .
                           '</td>' .
                           '<td class="center">' .
                             number_format($flowSum) .
                           '</td>' .
                           '<td class="center">';
        $portServices .= '<img src="/images/sparkline/service/' . $date . '/' . long2ip($ip) . '/' . str_replace(' ', '', $name) . '">';
        $portServices .= '</td>' .
                         '<td class="center">' .
                           size($sum) .
                         '</td>' .
                         '<td class="center">' .
                           date('g:i:s A', $startTime) .
                         '</td>' .
                         '<td class="center">' .
                           duration(getRoleEndTime($ports, $inboundPortActivityTimes) - $startTime) .
                         '</td>' .
                       '</tr>';
        ++$rowNumber;
      }
    }
    if ($hostTrafficRow['inboundBytes']) {
      $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
      $portServices .= '<tr class="' . $rowClass . '">' .
                         '<td class="center">' .
                           '<b>' .
                             '<i>' .
                               'Consumer' .
                             '</i>' .
                           '</b>' .
                         '</td>' .
                         '<td>' .
                         '</td>' .
                         '<td>' .
                         '<td class="center">' .
                           size($hostTrafficRow['inboundBytes']) .
                         '</td>' .
                         '<td class="center">' .
                           date('g:i:s A', $hostTrafficRow['firstInboundTime']) .
                         '</td>' .
                         '<td class="center">' .
                           duration($hostTrafficRow['lastInboundTime'] - $hostTrafficRow['firstInboundTime']) .
                         '</td>' .
                       '</tr>';
      ++$rowNumber;
    }
    foreach($outboundServicePorts as $name => &$ports) {
      $sum = arraySum($outboundPortTraffic, $ports);
      $flowSum = arraySum($outboundPortFlows, $ports);
      if ($sum) {
        $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
        $startTime = getRoleStartTime($ports, $outboundPortActivityTimes);
		$roleName = str_replace(' ', '', $name);
		$roleName = strtolower(substr($roleName, 0, 1)) . substr($roleName, 1);
		$roleDescription = getRoleDescription($roleName, $rolesConf);
        $portServices .= '<tr class="' . $rowClass . '">' .
                           '<td class="center">' .
                             '<a class="text" href="/role/' . $roleName . '/' . $date . '/' . long2ip($ip) . '">' .
                               $name .
                             '</a>' .
							 ' ' .
							 '<a class="tooltip" href="#">' .
							   '?' .
							   '<span>' . $roleDescription . '</span>' .
							 '</a>' .
                           '</td>' .
                           '<td class="center">' .
                             number_format($flowSum) .
                           '</td>' .
                           '<td class="center">';
        $portServices .= '<img src="/images/sparkline/service/' . $date . '/' . long2ip($ip) . '/' . str_replace(' ', '', $name) . '">';
        $portServices .= '</td>' .
                         '<td class="center">' .
                           size($sum) .
                         '</td>' .
                         '<td class="center">' .
                           date('g:i:s A', $startTime) .
                         '</td>' .
                         '<td class="center">' .
                           duration(getRoleEndTime($ports, $outboundPortActivityTimes) - $startTime) .
                         '</td>' .
                       '</tr>';
        ++$rowNumber;
      }
    }
    if (count($roles)) {
      foreach ($roles as $role => &$properties) {
        if (array_search($role, $displayedRoles) !== false) {
          $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
		  $roleName = str_replace(' ', '', $roleNames[$role]);
		  $roleName = strtolower(substr($roleName, 0, 1)) . substr($roleName, 1);
		  if (substr($roleName, -11, 11) == 'BruteForcer') {
		  	$roleName = 'bruteForcer';
		  } else if (substr($roleName, -11, 11) == 'BruteForced') {
		    $roleName = 'bruteForced';
		  }
		  $roleDescription = getRoleDescription($roleName, $rolesConf);
          $portServices .= '<tr class="' . $rowClass . '">' .
                             '<td class="center">' .
                               '<a class="text" href="' . roleURL($role, $date, $ip) . '">' .
                                 $roleNames[$role] .
                               '</a>';
		  if ($roleDescription !== false) {
		  	$portServices .= 
							   ' ' .
							   '<a class="tooltip" href="#">' .
							     '?' .
							     '<span>' . $roleDescription . '</span>' .
							   '</a>';
		  }
		  $portServices .=
                             '</td>' .
                             '<td class="center">' .
                               number_format($properties['numHosts']) .
                             '</td>' .
                             '<td class="center">' .
                               '<img src="/images/sparkline/role/' . $date . '/' . long2ip($ip) . '/' . str_replace(' ', '', $roleNames[$role]) . '">' .
                             '</td>' .
                             '<td class="center">' .
                               size($properties['numBytes']) .
                             '</td>' .
                             '<td class="center">' .
                               date('g:i:s A', $properties['startTime']) .
                            '</td>' .
                            '<td class="center">' .
                              duration($properties['endTime'] - $properties['startTime']) .
                            '</td>' .
                          '</tr>';
          ++$rowNumber;
        }
      }
    }
    return $portServices;
  }

  function makeSymptomNumbers(&$postgreSQL, &$interestingIPRow, &$date, &$symptomNames, &$symptomURLs) {
    global $symptomsConf;
    foreach ($interestingIPRow as $columnName => &$value) {
      if (array_key_exists($columnName, $symptomNames) && $value) {
        $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
		$tmp = explode('/', $symptomURLs[$columnName]);
		$symptomDescription = getSymptomDescription($tmp[1], $symptomsConf);
        $symptomNumbers .= '<tr class="' . $rowClass . '">' .
                             '<td class="center">' .
                               '<a class="text" href="/' . $symptomURLs[$columnName] . '/' . $date . '/' . long2ip($interestingIPRow['ip']) . '">' .
                                 $symptomNames[$columnName] .
                               '</a>';
		if ($symptomDescription !== false) {
			$symptomNumbers .= 
							   ' ' .
							   '<a class="tooltip" href="#">' .
							     '?' .
							     '<span>' . $symptomDescription . '</span>' .
							   '</a>';
		}
		$isWhiteListed = isRankWhitelisted($postgreSQL, $date, $interestingIPRow['ip'], substr($columnName, 0, -5));
		if ($isWhiteListed === true) {
			$symptomNumbers .= 
							   ' ' .
							   '<a class="tooltip" href="/rankWhiteList/remove/' . long2ip($interestingIPRow['ip']) . '/' . substr($columnName, 0, -5) . '">' .
							     'W-' .
								 '<span>Remove symptom from whitelist for this host</span>' .
							   '</a>';
		}
		else if ($isWhiteListed === false) {
			$symptomNumbers .= 
							   ' ' .
							   '<a class="tooltip" href="/rankWhiteList/add/' . long2ip($interestingIPRow['ip']) . '/' . substr($columnName, 0, -5) . '">' .
							     'W+' .
								 '<span>Add symptom to whitelist for this host</span>' .
							   '</a>';
		}
			
		$symptomNumbers .=   '</td>' .
                             '<td class="center">' .
                               number_format($value) .
                             '</td>' .
                           '</tr>';
        ++$rowNumber;
      }
    }
    return $symptomNumbers;
  }

  function getRoleStartTime(&$ports, &$activityTimes) {
    $startTime = 0;
    foreach ($ports as &$port) {
      if ($port && (!$startTime || $activityTimes[$port]['first'] < $startTime)) {
        $startTime = $activityTimes[$port]['first'];
      }
    }
    return $startTime;
  }

  function getRoleEndTime(&$ports, &$activityTimes) {
    $endTime = 0;
    foreach ($ports as &$port) {
      if ($port && $activityTimes[$port]['last'] > $endTime) {
        $endTime = $activityTimes[$port]['last'];
      }
    }
    return $endTime;
  }

  function makeContentLegend(&$ip, &$date, &$barColors, &$contentNames, &$content, &$bytes, $directory) {
  if ($date == '2008-09-28') {
    foreach (explode('},{', substr($content, 2, -2)) as $hourlyContentTypes) {
      $contentType = 0;
      foreach (explode(',', $hourlyContentTypes) as $hourlyContentType) {
        $totalContent[$contentType++] += $hourlyContentType * 16384;
      }
    }
    $contentType = 0;
    $totalBytes = array_sum(explode(',', substr($bytes, 1, -1)));
    $totalContent[] = $totalBytes - array_sum($totalContent);
    foreach ($totalContent as &$totalContentType) {
      $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
      $contentLegend .= '<tr class="' . $rowClass . '">' .
                          '<td width="16">' .
                            '<div class="box" style="background-color: ' . $barColors[$contentType] . ';" />' .
                          '</td>' .
                          '<td>' .
                            $contentNames[$contentType] .
                          '</td>' .
                          '<td class="right">' .
                            size($totalContentType) .
                          '</td>' .
                          '<td class="right">' .
                            @number_format($totalContentType / $totalBytes * 100, 1) . '%' .
                          '</td>' .
                        '</tr>';
      ++$contentType;
      ++$rowNumber;
    }
    $contentLegend .= '<tr class="odd">' .
                        '<td>' .
                        '</td>' .
                        '<td>' .
                          'Total' .
                        '</td>' .
                        '<td class="right">' .
                          size($totalBytes) .
                        '</td>' .
                        '<td class="right">' .
                          '100.0%' .
                        '</td>' .
                      '</tr>' .
                      '<tr class="even">' .
                        '<td colspan="4" class="center">' .
                          '<img src="/images/graph/content/' . $date . '/' . long2ip($ip) . '/' . $directory . '" />' .
                        '</td>' .
                      '</tr>' .
                    '</tfoot>';
    return $contentLegend;
  }
  else {
    $content = explode(',', substr($content, 1, -1));
    foreach ($content as &$_content) {
      $_content *= 16384;
    }
    $content[] = $bytes - array_sum($content);
    $contentType = 0;
    foreach ($content as &$_content) {
      $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
      $contentLegend .= '<tr class="' . $rowClass . '">' .
                          '<td width="16">' .
                            '<div class="box" style="background-color: ' . $barColors[$contentType] . ';" />' .
                          '</td>' .
                          '<td>' .
                            $contentNames[$contentType] .
                          '</td>' .
                          '<td class="right">' .
                            size($_content) .
                          '</td>' .
                          '<td class="right">' .
                            @number_format($_content / $bytes * 100, 1) . '%' .
                          '</td>' .
                        '</tr>';
      ++$contentType;
      ++$rowNumber;
    }
    $contentLegend .= '<tr class="odd">' .
                        '<td>' .
                        '</td>' .
                        '<td>' .
                          'Total' .
                        '</td>' .
                        '<td class="right">' .
                          size($bytes) .
                        '</td>' .
                        '<td class="right">' .
                          '100.0%' .
                        '</td>' .
                      '</tr>' .
                      '<tr class="even">' .
                        '<td colspan="4" class="center">' .
                          '<img src="/images/graph/content/' . $date . '/' . long2ip($ip) . '/' . $directory . '" />' .
                        '</td>' .
                      '</tr>' .
                    '</tfoot>';
    return $contentLegend;
  }}

  if (!isset($_COOKIE['imsSessionID']) || !isSession($postgreSQL, $_COOKIE['imsSessionID'])) {
    if (!isInternal(ip2long($_SERVER['REMOTE_ADDR'])) || $_SERVER['REMOTE_ADDR'] != $ip) {
      header('Location: /login');
      exit;
    }
  }

  function getFirstRebootTime(&$postgreSQL, &$date, &$numericIP) {
    if (existsPGTable($postgreSQL, 'Reboots', $date)) {
      $result = pg_query($postgreSQL, 'SELECT "applicationTimes"[1] FROM "Reboots"."' . $date . '" ' .
                                      ' WHERE "ip" = \'' . $numericIP . '\'');
      if (pg_num_rows($result)) {
        $row = pg_fetch_row($result);
        return date('g:i:s A', $row[0]);
      }
    }
    return '-';
  }

  function getNumReboots(&$postgreSQL, &$date, &$numericIP) {
    if (existsPGTable($postgreSQL, 'Reboots', $date)) {
      $result = pg_query($postgreSQL, 'SELECT COUNT(*) FROM "Reboots"."' . $date . '" ' .
                                      'WHERE "ip" = \'' . $numericIP . '\'');
      $row = pg_fetch_row($result);
      return $row[0];
    }
    return 0;
  }

  function getInboundPortActivityTimes(&$hostTrafficRow, &$interestingPortIndex) {
    $firstActivityTimes = explode(',', substr($hostTrafficRow['inboundPortFirstActivityTimes'], 1, -1));
    $lastActivityTimes = explode(',', substr($hostTrafficRow['inboundPortLastActivityTimes'], 1, -1));
    $interestingPortNumber = 0;
    foreach ($interestingPortIndex as &$interestingPort) {
      $activityTimes[$interestingPort]['first'] = $firstActivityTimes[$interestingPortNumber];
      $activityTimes[$interestingPort]['last'] = $lastActivityTimes[$interestingPortNumber++];
    }
    return $activityTimes;
  }

  function getOutboundPortActivityTimes(&$hostTrafficRow, &$interestingPortIndex) {
    $firstActivityTimes = explode(',', substr($hostTrafficRow['outboundPortFirstActivityTimes'], 1, -1));
    $lastActivityTimes = explode(',', substr($hostTrafficRow['outboundPortLastActivityTimes'], 1, -1));
    $interestingPortNumber = 0;
    foreach ($interestingPortIndex as &$interestingPort) {
      $activityTimes[$interestingPort]['first'] = $firstActivityTimes[$interestingPortNumber];
      $activityTimes[$interestingPort]['last'] = $lastActivityTimes[$interestingPortNumber++];
    }
    return $activityTimes;
  }

  function physicalIdentity($field) {
    if ($field && $field != ' ') {
      return $field;
    }
    return '<i>' .
             'Unknown' .
           '</i>';
  }

  if (isInteresting($postgreSQL, $date, $numericIP)) {
    $previousDay = getPreviousHostDay($postgreSQL, $date, $numericIP);
    $interestingPortIndex = getInterestingPortIndex($postgreSQL, $date);
    $hostTrafficRow = pg_fetch_assoc(pg_query($postgreSQL, 'SELECT * FROM "HostTraffic"."' . $date .
                                                           '" WHERE "ip" = \'' . $numericIP . '\''));
    if ($interestingPortIndex) {
      $inboundPortTraffic = @getInboundPortTraffic($postgreSQL, $date, $interestingPortIndex, $numericIP);
      $inboundPortActivityTimes = getInboundPortActivityTimes($hostTrafficRow, $interestingPortIndex);
      $outboundPortTraffic = @getOutboundPortTraffic($postgreSQL, $date, $interestingPortIndex, $numericIP);
      $outboundPortActivityTimes = getOutboundPortActivityTimes($hostTrafficRow, $interestingPortIndex);
      $inboundPortFlows = getInboundPortFlows($postgreSQL, $date, $interestingPortIndex, $numericIP);
      $outboundPortFlows = getOutboundPortFlows($postgreSQL, $date, $interestingPortIndex, $numericIP);
    }
    $imsHome = dirname(__FILE__) . '/..';
    $interestingIPRow = pg_fetch_assoc(pg_query($postgreSQL, 'SELECT * FROM "InterestingIPs"."' . $date .
                                                             '" WHERE "ip" = \'' . $numericIP . '\''));
    $names = explode(',', substr($interestingIPRow['names'], 1, -1));
    if (strlen($names[0])) {
      $names = implode(', ', $names);
    }
    else {
      $names = '-';
    }
    $roles = getRoles($postgreSQL, $numericIP, $date);
    $ldapData = getLDAPInformationByIP($postgreSQL, $ip);
    $title = 'Infection Report for ' . $ip;
    include('include/header.php');
    echo '<div class="identity" align="center">' .
           '<table width="100%">' .
             '<tr>' .
               '<td align="center">' .
                 '<table cellspacing="1" width="80%">' .
                   '<tr>' .
                     '<td class="columnTitle center" colspan="2">' .
                       'Network Identity' .
                     '</td>' .
                   '</tr>' .
                   '<tr class="even">' .
                     '<td class="center">' .
                       'IP Address' .
                     '</td>' .
                     '<td class="center">' .
                       $ip .
                     '</td>' .
                   '</tr>' .
                   '<tr class="odd">' .
                     '<td class="center">' .
                       'DNS Name(s)' .
                     '</td>' .
                     '<td class="center">' .
                       $names .
                     '</td>' .
                    '</tr>' .
                    '<tr class="even">' .
                     '<td class="center">' .
                       'MAC Address' .
                     '</td>' .
                     '<td class="center">' .
                       getMACAddress($postgreSQL, $date, $numericIP) .
                     '</td>' .
                   '</tr>' .
                   '<tr class="odd">' .
                     '<td class="center">' .
                       'First Activity' .
                     '</td>' .
                     '<td class="center">' .
                       date('g:i:s A', $hostTrafficRow['firstOutboundTime']) .
                     '</td>' .
                   '</tr>' .
                   '<tr class="even">' .
                     '<td class="center">' .
                       'First Reboot' .
                     '</td>' .
                     '<td class="center">' .
                       getFirstRebootTime($postgreSQL, $date, $numericIP) .
                     '</td>' .
                   '</tr>' .
                   '<tr class="odd">' .
                     '<td class="center">' .
                       'Last Activity' .
                     '</td>' .
                     '<td class="center">' .
                       date('Y-m-d \a\\t g:i:s A', $hostTrafficRow['lastOutboundTime']) .
                     '</td>' .
                   '</tr>' .
                   '<tr class="even">' .
                     '<td class="center">' .
                       'Total Bandwidth Used' .
                     '</td>' .
                     '<td class="center">' .
                       size(getNumBytes($postgreSQL, $date, $numericIP)) .
                     '</td>' .
                   '</tr>' .
                   '<tr class="odd">' .
                     '<td class="center">' .
                       'Hosts Contacted' .
                     '</td>' .
                     '<td class="center">' .
                       number_format(getFanout($postgreSQL, $date, $numericIP)) .
                     '</td>' .
                   '</tr>' .
                 '</table>' .
               '</td>' .
               '<td align="center">' .
                 '<table cellspacing="1" width="80%">' .
                   '<tr class="even">' .
                     '<td class="columnTitle center" colspan="2">' .
                       'Physical Identity' .
                     '</td>' .
                   '</tr>' .
                   '<tr class="even">' .
                     '<td class="center">' .
                       'Owner' .
                     '</td>' .
                     '<td class="center">' .
                       physicalIdentity($ldapData['firstName'] . ' ' . $ldapData['lastName']) .
                     '</td>' .
                   '</tr>' .
                   '<tr class="odd">' .
                     '<td class="center">' .
                       'Title' .
                     '</td>' .
                     '<td class="center">' .
                       physicalIdentity($ldapData['title']) .
                     '</td>' .
                   '</tr>' .
                   '<tr class="even">' .
                     '<td class="center">' .
                       'E-mail' .
                     '</td>' .
                     '<td class="center">' .
                       physicalIdentity($ldapData['email']) .
                     '</td>' .
                   '</tr>' .
                   '<tr class="odd">' .
                     '<td class="center">' .
                       'Phone Number' .
                     '</th>' .
                     '<td class="center">' .
                       physicalIdentity($ldapData['phoneNumber']) .
                     '</td>' .
                   '</tr>' .
                   '<tr class="even">' .
                     '<td class="center">' .
                       'Location' .
                     '</td>' .
                     '<td class="center">' .
                       physicalIdentity($ldapData['location']) .
                     '</td>' .
                   '</tr>' .
                   '<tr class="odd">' .
                     '<td class="center">' .
                       'Department' .
                     '</td>' .
                     '<td class="center">' .
                       physicalIdentity($ldapData['department']) .
                    '</td>' .
                   '</tr>' .
                 '</table>' .
               '</td>' .
             '</tr>' .
           '</table>' .
         '</div>' .
         '<div id="report_title">' .
           'Infection Report' .
         '</div>' .
         '<div id="infection">' .
           '<div class="metrics">' .
             '<table id="centered">' .
               '<tr>' .
                 '<td>' .
                   '<h1>' .
                     'Host Rank: ' .
                     '<span id="hotnumber">' .
                       number_format($interestingIPRow['rank']) .
                     '</span>' .
                     '/' . number_format(getNumInterestingIPs($postgreSQL, $date)) .
                   '</h1>' .
                 '</td>' .
                 '<td>' .
                   '<img src="/images/sparkline/virulence/' . $date . '/' . $ip . '">' .
                 '</td>' .
                 '<td>' .
                   '<h1>' .
                     'Virulence: ' . round($interestingIPRow['currentVirulence'], 3) . ' (' .
                     '<span id="coldnumber">' .
                       round($interestingIPRow['minVirulence'], 3) .
                     '</span>' . ', ' .
                     '<span id="hotnumber">' .
                       round($interestingIPRow['maxVirulence'], 3) .
                     '</span>' . ')' .
                   '</h1>' .
                 '</td>' .
               '</tr>' .
             '</table>' .
           '</div>' .
           '<div id="reason">' .
             'This host is believed to be infected because:' .
             '<blockquote id="reason_block">' .
               makeReasons($postgreSQL, $numericIP, $date, $infectedIPs, $roles, $interestingIPRow, $symptomDescriptions) .
             '</blockquote>' .
           '</div>' .
		   display_related_hosts($postgreSQL, $numericIP, $date) .
         '</div>' .
         '<div id="environment_title">' .
           'Host Environment' .
         '</div>' .
         '<div id="environment" class="top">' .
           '<table valign="top" width="100%">' .
             '<tr>' .
               '<td valign="top" align="center">' .
                 '<table cellspacing="1" width="100%">' .
                   '<tr>' .
                     '<td class="tableName center" colspan="6">' .
                       'Host Roles' .
                     '</td>' .
                   '</tr>' .
                   '<tr>' .
                     '<td class="columnTitle center">' .
                       'Role' .
                     '</td>' .
                     '<td class="columnTitle center">' .
                       'IPs' .
                     '</td>' .
                     '<td class="columnTitle center">' .
                       'Trend' .
                     '</td>' .
                     '<td class="columnTitle center">' .
                       'Amount' .
                     '</td>' .
                     '<td class="columnTitle center">' .
                       'First Activity' .
                     '</td>' .
                     '<td class="columnTitle center">' .
                       'Duration' .
                     '</td>' .
                   '</tr>' .
                     makeHostRoles($postgreSQL, $imsHome, $date, $numericIP,
                                   $roles, $hostTrafficRow,
                                   $inboundPortActivityTimes, $outboundPortActivityTimes,
                                   $inboundPortTraffic, $outboundPortTraffic,
                                   $inboundServicePorts, $outboundServicePorts,
                                   $inboundPortFlows, $outboundPortFlows) .
                 '</table>' .
               '</td>' .
               '<td valign="top" align="center">' .
                 '<table cellspacing="1" width="100%">' .
                   '<tr>' .
                     '<td class="tableName center" colspan="2">' .
                       'Symptoms' .
                     '</td>' .
                   '</tr>' .
                   '<tr>' .
                     '<td class="columnTitle center">' .
                       'Name' .
                     '</td>' .
                     '<td class="columnTitle center">' .
                       'Count' .
                     '</td>' .
                   '</tr>' .
                   makeSymptomNumbers($postgreSQL, $interestingIPRow, $date, $symptomNames,
                                      $symptomURLs) .
                   '</tr>' .
                 '</table>' .
               '</td>' .
             '</tr>' .
           '</table>' .
           '<table align="center" width="100%">' .
             '<tr>' .
               '<td align="center" width="50%">' .
                 '<table cellspacing="1" width="100%">' .
                   '<tr>' .
                     '<td class="tableName center" colspan="4">' .
                       'Inbound Content Distribution' .
                     '</td>' .
                   '</tr>' .
                   '<tr>' .
                     '<td class="columnTitle" colspan="2">' .
                       'Content Type' .
                     '</td>' .
                     '<td class="columnTitle right">' .
                       'Amount Received' .
                     '</td>' .
                       '<td class="columnTitle right">' .
                         '% of Total' .
                       '</td>' .
                     '</tr>' .
                     makeContentLegend($numericIP, $date, $barColors, $contentNames,
                                       $hostTrafficRow['inboundContent'],
                                       $hostTrafficRow['inboundBytes'], 'inbound') .
                  '</table>' .
                '</td>' .
              '<td align="center" width=50%">' .
              '<table cellspacing="1" width="100%">' .
                '<tr>' .
                  '<td class="tableName center" colspan="4">' .
                    'Outbound Content Distribution' .
                    '</td>' .
                  '</tr>' .
                  '<tr>' .
                    '<td class="columnTitle" colspan="2">' .
                      'Content Type' .
                    '</td>' .
                    '<td class="columnTitle right">' .
                      'Amount Sent' .
                    '</td>' .
                    '<td class="columnTitle right">' .
                      '% of Total' .
                    '</td>' .
                  '</tr>' .
                  makeContentLegend($numericIP, $date, $barColors, $contentNames,
                                    $hostTrafficRow['outboundContent'],
                                    $hostTrafficRow['outboundBytes'], 'outbound') .
               '</table>' .
             '</td>' .
           '</tr>' .
         '</table>' .
         '<div class="table">' .
            '<table width="100%">' .
              '<tr>' .
                '<td align="center">' .
                  '<table cellspacing="1">' .
                    '<tr>' .
                      '<td class="tableName center">' .
                        'Top 10 Most Active Ports' .
                      '</td>' .
                    '</tr>' .
                    '<tr>' .
                      '<td class="columnTitle center">' .
                        '<img src="/images/graph/activeports/' . $date . '/' . $ip . '">' .
                      '</td>' .
                    '</tr>' .
                  '</table>' .
                '</td>';
  if (@file_exists($imsHome . '/htdocs/images/graphs/slowdown/' . $date . '/' . $ip . '.png')) {
    echo  '<td align="center">' .
            '<table cellspacing="1">' .
              '<tr>' .
                '<td class="tableName center">' .
                  'Slowdown' .
                '</td>' .
              '</tr>' .
              '<tr>' .
                '<td class="columnTitle center">' .
                  '<img src="/images/graphs/slowdown/' . $date . '/' . $ip . '.png">' .
                '</td>' .
              '</tr>' .
            '</table>' .
          '</td>';
  }
  echo '</tr>' .
     '</table>' .
   '</div>';
  }
  include('include/footer.html');
?>
