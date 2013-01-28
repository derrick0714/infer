<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  include('include/roles.php');

  function getDNSNames(&$postgreSQL, &$date, &$ip) {
    $result = pg_query($postgreSQL, 'SELECT "names" FROM "InterestingIPs"."' . $date .
                                     '" WHERE "ip" = \'' . $ip . '\'');
    if (@pg_num_rows($result) > 0) {
      $row = @pg_fetch_assoc($result);
      $names = explode(',', substr($row['names'], 1, -1));
      if (strlen($names[0]) > 0) {
        return implode(', ', $names);
      }
    }
    return false;
  }

  function ports(&$interestingPorts, &$ports, &$initiator, $column, $delimiter) {
    if ($initiator == 1) {
      $column = '"outbound' . $column . '"';
    }
    else {
      $column = '"inbound' . $column . '"';
    }
    foreach ($ports as &$port) {
      $query[] = $column . '[' . ($interestingPorts[$port] + 1) . ']';
    }
    return implode($delimiter, $query);
  }

  function getPortRoleStats(&$postgreSQL, &$interestingPorts, &$date, &$properties) {
    $query = 'SELECT COUNT(*), SUM(' .
             ports($interestingPorts, $properties['ports'], $properties['initiator'], 'PortTraffic', ' + ') .
             ') AS "numBytes" FROM "HostTraffic"."' . $date . '" WHERE ' .
             ports($interestingPorts, $properties['ports'], $properties['initiator'], 'PortTraffic', ' + ') . ' > \'0\'';
    $result = @pg_query($postgreSQL, $query);
    if (@pg_num_rows($result) > 0) {
      $row = pg_fetch_assoc($result);
      return $row;
    }
    return false;
  }

  function cmp($left, $right) {
    if ($left['count'] != $right['count']) {
      if ($left['count'] < $right['count']) {
        return 1;
      }
      return -1;
    }
    return 0;
  }

 	function getRolePageURL($roleName, $date) {
		$rolePageNames = array(
			'WebServer' => 'webServer',
			'MailServer' => 'mailServer',
			'SecureWebServer' => 'secureWebServer',
			'SecureMailServer' => 'secureMailServer',
			'WebClient' => 'webClient',
			'MailClient' => 'mailClient',
			'SecureWebClient' => 'secureWebClient',
			'SecureMailClient' => 'secureMailClient',
			'SpamBot' => 'spamBot',
			'EncryptedP2PNode' => 'encryptedP2PNode',
			'UnclassifiedP2PNode' => 'unclassifiedP2PNode',
			'MultimediaP2PNode' => 'multimediaP2PNode',
			'SSHBruteForced' => 'bruteForced',
			'MySQLBruteForced' => 'bruteForced',
			'MicrosoftSQLBruteForced' => 'bruteForced',
			'TELNETBruteForced' => 'bruteForced',
			'FTPBruteForced' => 'bruteForced',
			'SSHBruteForcer' => 'bruteForcer',
			'MySQLBruteForcer' => 'bruteForcer',
			'MicrosoftSQLBruteForcer' => 'bruteForcer',
			'TELNETBruteForcer' => 'bruteForcer',
			'FTPBruteForcer' => 'bruteForcer');

		if (substr($roleName, -12, 11) == 'Brute Force') {
			$ret = '/role/' . $rolePageNames[str_replace(' ', '', $roleName)] .
					'/' . $date . '/';
			switch ($roleName[0]) {
			  case 'S':
			  	$ret .= '22';
				break;
			  case 'M':
			    if ($roleName[1] == 'i') {
					$ret .= '1433';
				}
				else if ($roleName[1] == 'y') {
					$ret .= '3306';
				}
				break;
			  case 'T':
			  	$ret .= '23';
				break;
			  case 'F':
			  	$ret .= '21';
				break;
			}
		} else {
			$ret = '/role/' . $rolePageNames[str_replace(' ', '', $roleName)] .
							'/' . $date;
		}

		return $ret;
	}

  $request = explode('/', substr($_SERVER['PATH_INFO'], 1));

  $schemaName = 'InterestingIPs';
  $rolesSchema = 'Roles';

  if (!$request[0] || !existsPGTable($postgreSQL, $schemaName, $request[0])) {
    $lastDay = getLastPGTable($postgreSQL, $schemaName);
    if (!$lastDay) {
      header('Location: /users/');
      exit;
    }
    header('Location: /report/' . getLastPGTable($postgreSQL, $schemaName));
    exit;
  }

  $date = $request[0];
  if ($request[1] !== NULL && is_numeric($request[1])) {
    $page = $request[1];
  }

  if (!getFirstPGTable($postgreSQL, 'InterestingIPs')) {
    exit;
  }

  if (existsPGTable($postgreSQL, 'InfectedIPs', $date)) {
    getInfectedIPs($postgreSQL, $date, $infectedIPs);
  }

  if (existsPGTable($postgreSQL, 'Roles', $date)) {
    $result = pg_query($postgreSQL, 'SELECT * FROM "Roles"."' . $date . '"');
    while ($row = pg_fetch_assoc($result)) {
      $roles[$row['ip']][$row['role']]['set'] = true;
	  if ($row['dangerous'] == 1) {
	  	$roles[$row['ip']][$row['role']]['dangerous'] = true;
	  }
    }
  }

  if (array_search('contain', $request)) {
    $containedIP = sprintf('%u', ip2long($request[count($request) - 1]));
  }
  if (array_search('clear', $request)) {
    $clearedIP = sprintf('%u', ip2long($request[count($request) - 1]));
  }
  if (array_search('whiteList', $request)) {
    $whiteListedIP = sprintf('%u', ip2long($request[array_search('whiteList', $request) + 1]));
  }
  if (array_search('unWhiteList', $request)) {
    $unWhiteListedIP = sprintf('%u', ip2long($request[count($request) - 1]));
  }
  if (array_search('showWhiteList', $request)) {
    $showWhiteList = true;
  }

  if ($containedIP && !isContained($postgreSQL, $containedIP)) {
    containIP($postgreSQL, $containedIP);
  }
  if ($clearedIP && isContained($postgreSQL, $clearedIP)) {
    clearIP($postgreSQL, $clearedIP);
  }
  if ($whiteListedIP && !isWhiteListed($postgreSQL, $whiteListedIP)) {
    whiteListIP($postgreSQL, $whiteListedIP);
  }
  if ($unWhiteListedIP && isWhiteListed($postgreSQL, $unWhiteListedIP)) {
    unWhiteListIP($postgreSQL, $unWhiteListedIP);
  }

  function getPageURL(&$page) {
    if ($page !== NULL) {
      return $page . '/';
    }
  }

  function showWhiteList($showWhiteList) {
    if ($showWhiteList) {
      return '/showWhiteList';
    }
  }

  function containIP(&$postgreSQL, &$ip) {
    pg_query($postgreSQL, 'INSERT INTO "Maps"."containedHosts" VALUES (\'' . $ip . '\', \'' . time() . '\')');
  }
  function clearIP(&$postgreSQL, &$ip) {
    pg_query($postgreSQL, 'DELETE FROM "Maps"."containedHosts" WHERE "ip" = \'' . $ip . '\'');
  }
  function isContained(&$postgreSQL, &$ip) {
    $result = pg_query($postgreSQL, 'SELECT * FROM "Maps"."containedHosts" WHERE "ip" = \'' . $ip . '\'');
    return ($row = pg_fetch_row($result));
  }
  function whiteListIP(&$postgreSQL, &$ip) {
    pg_query($postgreSQL, 'INSERT INTO "Maps"."whiteList" VALUES (\'' . $ip . '\', \'' . time() . '\')');
  }
  function unWhiteListIP(&$postgreSQL, &$ip) {
    pg_query($postgreSQL, 'DELETE FROM "Maps"."whiteList" WHERE "ip" = \'' . $ip . '\'');
  }
  function isWhiteListed(&$postgreSQL, &$ip) {
    $result = pg_query($postgreSQL, 'SELECT * FROM "Maps"."whiteList" WHERE "ip" = \'' . $ip . '\'');
    return ($row = pg_fetch_row($result));
  }

  function getSymptoms(&$row, &$date, &$hostTrafficRow, &$roles) {
    global $roleNames, $exclusiveRoles;
    if ($roles) {
      foreach ($roleNames as $role => &$roleName) {
        if ($roles[$role]['set']) {
          if (array_search($role, $exclusiveRoles) !== false) {
            $exclusive = true;
          }
          if ($roles[$role]['dangerous']) {
            $symptoms[] = '<a class="text" href="' . roleURL($role, $date, $hostTrafficRow['ip']) . '">' .
                            $roleName .
                          '</a>' .
                          ' (possibly being controlled by ' . get_comm_channels($postgreSQL, $hostTrafficRow['ip'], $date, $infectedIPs) . ')';
          }
          else {
            $symptoms[] = '<a class="text" href="' . roleURL($role, $date, $hostTrafficRow['ip']) . '">' .
                            $roleName .
                          '</a>';
          }
        }
      }
    }
    if ($exclusive) {
      return @implode(', ', $symptoms);
    }
    else {
      if ($row['infectedContactScore']) {
        $symptoms[] = '<a class="text" href="/symptom/infectedContacts/' . $date . '/' . long2ip($row['ip']) . '">Contact with Infected Hosts</a>';
      }
      if ($row['evasiveTrafficScore']) {
        $symptoms[] = '<a class="text" href="/symptom/evasiveTraffic/' . $date . '/' . long2ip($row['ip']) . '">Evasive Traffic</a>';
      }
      if ($row['darkSpaceSourceScore']) {
        $symptoms[] = '<a class="text" href="/symptom/darkSpaceSources/' . $date . '/' . long2ip($row['ip']) . '">Dark Space Source</a>';
      }
      if ($row['darkSpaceTargetScore']) {
        $symptoms[] = '<a class="text" href="/symptom/darkSpaceTargets/' . $date . '/' . long2ip($row['ip']) . '">Dark Space Target</a>';
      }
      if ($row['nonDNSTrafficScore']) {
        $symptoms[] = '<a class="text" href="/symptom/nonDNSTraffic/' . $date . '/' . long2ip($row['ip']) . '">Protocol Violations</a>';
      }
      if ($row['rebootScore'] > 1) {
        $symptoms[] = '<a class="text" href="/reboots/' . $date . '/' . long2ip($row['ip']) . '">' .
                        'Multiple Reboots' .
                      '</a>';
      }
      return @implode(', ', $symptoms);
    }
  }

  include('include/header.php');

  echo '<div class="table">' .
         '<table width="100%" cellspacing="1">' .
           '<tr>' .
             '<th class="tableName row" colspan="6" align="center" width="100%">' .
               'Most Infected Hosts on ' . $date .
             '</th>' .
           '</tr>' .
           '<tr>' .
           '<td class="columnTitle center">' .
              'IP' .
           '</td>' .
           '<td class="columnTitle center">' .
             'Rank History' .
           '</td>' .
           '<td class="columnTitle center" width="50%">' .
             'Symptoms' .
           '</td>' .
           '<td class="columnTitle center">' .
             'Virulence History' .
           '</td>' .
           '<td class="columnTitle center">' .
             'Virulence (Current / Min. / Max.)' .
           '</td>' .
           '<td class="columnTitle center">' .
             'Add to Whitelist' .
           '</td>' .
         '</tr>';
       if ($page !== NULL) {
         $pageOptions = ' OFFSET ' . $page * 100 . ' LIMIT 100';
       }
       $result = pg_query($postgreSQL, 'SELECT * FROM "' . $schemaName . '"."' . $date . '" ORDER BY "currentVirulence" DESC' . $pageOptions);
       while ($row = pg_fetch_assoc($result)) {
         if (!isContained($postgreSQL, $row['ip']) && (!isWhiteListed($postgreSQL, $row['ip']) || $showWhiteList)) {
           $rows[] = $row;
           ++$numRows;
         }
         if ($numRows == 10 && $page == NULL) {
           break;
         }
       }
       foreach ($rows as &$row) {
         if (existsPGTable($postgreSQL, 'HostTraffic', $date)) {
           $result = pg_query($postgreSQL, 'SELECT * FROM "HostTraffic"."' . $date . '" WHERE "ip" = \'' . $row['ip'] . '\'');
           $hostTrafficRow = pg_fetch_assoc($result);
         }
         if (isWhiteListed($postgreSQL, $row['ip'])) {
           $rowClass = 'whiteList';
         }
         else {
           $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
         }
         ++$rowNumber;
         $names = explode(',', substr($row['names'], 1, -1));
         if (strlen($names[0])) {
           if (count($names) == 1) {
             $hostName = long2ip($row['ip']) . ' (' . $names[0] . ')';
           }
           else {
             $hostName = long2ip($row['ip']) . ' (' . $names[0] . ', ...)';
           }
         }
         else {
           $hostName = long2ip($row['ip']);
         }
         echo '<tr class="' . $rowClass . '">' .
                '<td class="center">' .
                  '<a class="text" href="/host/' . long2ip($row['ip']) . '/' . $date . '">' . $hostName . '</a>' .
                '</td>' .
                '<td class="center">' .
                  '<img src="/images/sparkline/rank/' . $date . '/' . long2ip($row['ip']) . '">' .
                '</td>' .
                '<td class="center">' .
                  getSymptoms($row, $date, $hostTrafficRow, $roles[$row['ip']]) .
                '</td>' .
                '<td class="center">' .
                  '<img style="margin-right: 3px;" src="/images/sparkline/virulence/' . $date .
                  '/' . long2ip($row['ip']) . '" / style="border: 0px;" title="14-Day Virulence Trend" />' .
                '</td>' .
                '<td class="center">' .
                  '<font color="#0000FF">' . round($row['currentVirulence'], 3) . '</font>' .
                  ' (<font color="#00FF00">' . round($row['minVirulence'], 3) . '</font> / ' .
                  '<font color="#FF0000">' . round($row['maxVirulence'], 3) . '</font>)' .
                '</td>' .
                '<td class="center">' .
				/*
                  '<a href="/report/' . $date . '/' . getPageURL($page) . 'contain/' . long2ip($row['ip']) . showWhiteList($showWhiteList) . '">' .
                    '<img src="/images/contain.png" title="Stop Clogging Pipes on the Internet" >' .
                  '</a>' .
                  '<img src="/images/inform.png" title="Inform user">' .
				*/
                  '<a href="/whiteList/' . long2ip($row['ip']) . showWhiteList($showWhiteList) . '">' .
                    '<img src="/images/exclude.png" title="Exclude host from analysis">' .
                  '</a>' .
                '</strong>' .
              '</td>' .
            '</tr>';
        }
        echo '</table><br /><center>';
        if (!$showWhiteList) {
          echo '<a href="/report/' . $date . '/' . getPageURL($page) . 'showWhiteList">' .
                 '<img src="/images/exclude.png" title="Show whitelisted hosts">' .
               '</a>';
        }
        else {
          echo '<a href="/report/' . $date . '/' . getPageURL($page) . '">' .
                 '<img src="/images/exclude.png" title="Hide whitelisted hosts">' .
               '</a>';
        }
        if ($page !== NULL) {
          if ($page > 0) {
            echo '<a href="/report/' . $date . '/' . ($page - 1) . showWhiteList($showWhiteList) . '">' .
                   '<img src="/images/nav_left.png" title="Previous Page">' .
                 '</a>';
          }
          echo '<a href="/report/' . $date . showWhiteList($showWhiteList) . '">' .
                 '<img src="/images/nav_up.png" title="Collapse Table">' .
               '</a>';
          $result = @pg_query($postgreSQL, 'SELECT COUNT(*) FROM "InterestingIPs"."' . $date . '"');
          $row = @pg_fetch_assoc($result);
          if ($row['count'] > ($page + 1) * 100) {
            echo '<a href="/report/' . $date . '/' . ($page + 1) . showWhiteList($showWhiteList) . '">' .
                   '<img src="/images/nav_right.png" title="Next Page">' .
                 '</a>';
          }
        }
        else {
          echo '<a href="/report/' . $date . '/0' . showWhiteList($showWhiteList) . '">' .
                 '<img src="/images/nav_down.png" title="Expand Table">' .
               '</a>';
        }
        echo '</center></div>';

  // display Roles
	unset($roles);
    $date = $url[1];
    $logo = '<table width="100%">' .
              '<tr>' .
                '<td class="sub_title" width="15%" align="left">' .
                  getPreviousPGDay($postgreSQL, $rolesSchema, $date, '/roles') .
               '</td>' .
              '<td width="60%" align="center">' .
                $title .
              '</td>' .
              '<td class="sub_title" width="15%" align="right">' .
                getNextPGDay($postgreSQL, $rolesSchema, $date, '/roles') .
              '</td>' .
            '</tr>' .
          '</table>';
      //
      // Retrieves the ports that were monitored for inbound and outbound
      // connections on the specified day. 
      //
      $result = @pg_query($postgreSQL, 'SELECT "interestingPorts" FROM "Indexes"."interestingPorts" WHERE "date" = \'' . $url[1] . '\'');
      $row = @pg_fetch_assoc($result);
      $interestingPorts = array_flip(explode(',', substr($row['interestingPorts'], 1, -1)));
      $result = @pg_query($postgreSQL, 'SELECT * FROM "Maps"."monitoredServices"');
      while ($row = @pg_fetch_assoc($result)) {
        if ($row['initiator'] == 1) {
          $roleType = CONSUMER_TYPE;
        }
        else {
          $roleType = PUBLISHER_TYPE;
        }
        $roles[$roleType][$row['name']]['ports'] = explode(',', substr($row['ports'], 1, -1));
        $roles[$roleType][$row['name']]['initiator'] = $row['initiator'];
      }
      foreach ($roles as $type => &$_roles) {
        foreach ($_roles as $_role => &$properties) {
          $stats = getPortRoleStats($postgreSQL, $interestingPorts, $date, $properties);
          $properties['count'] = $stats['count'];
          $properties['numBytes'] = $stats['numBytes'];
        }
      }
      $result = @pg_query($postgreSQL, 'SELECT "role", COUNT(*), SUM("numBytes") FROM "Roles"."' . $url[1] . '" GROUP BY "role"');
      while ($row = @pg_fetch_assoc($result)) {
        $roles[$roleTypes[$row['role']]][$roleNames[$row['role']]]['count'] = $row['count'];
        $roles[$roleTypes[$row['role']]][$roleNames[$row['role']]]['numBytes'] = $row['sum'];
      }
      ksort($roles);
      foreach ($roles as $type => &$_roles) {
        uasort($_roles, 'cmp');
      }
      echo '<div class="table">' .
             '<table cellspacing="1" width="100%">' .
			   '<tr>' .
			     '<td align="center" class="tableName" colspan="4">' .
				   'Roles' .
				 '</td>' .
			   '</tr>' .
               '<tr>' .
                 '<td align="center" class="columnTitle">' .
                   '&nbsp;' .
                 '</td>' .
                 '<td align="center" class="columnTitle">' .
                   'Role' .
                 '</td>' .
                 '<td align="center" class="columnTitle">' .
                   'Num. Hosts' .
                 '</td>' .
                 '<td align="center" class="columnTitle">' .
                   'Bytes Transferred' .
                 '</td>' .
               '</tr>';
		
	  $rowNumber = 0;
      foreach ($roles as $type => &$_roles) {
        $roleRowNumber = 0;
        foreach ($_roles as $_role => &$properties) {
          $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
          echo '<tr class="' . $rowClass . '" onmouseout="this.className=\'' . $rowClass . '\'" onmouseover="this.className=\'table_hover\'">';
		  if ($roleRowNumber == 0) {
		  	echo '<td align="center" class="columnTitle" rowspan="' . count($_roles) . '">' .
				   '<i>' . $roleTypeNames[$type] . '</i>' .
				 '</td>';
		  }
		  echo  '<td align="center">' .
		           '<a class="text" href="' . getRolePageURL($_role, $date) . '">' .
                     $_role .
                   '</a>' .
		         '</td>' .
                 '<td align="center">' .
                   number_format($properties['count']) .
                 '</td>' .
                 '<td align="center">' .
                   size($properties['numBytes']) .
                 '</a>' .
               '</tr>';
          ++$rowNumber;
		  ++$roleRowNumber;
        }
      }
	  echo '</table></div>';
  
  // network summary
  echo '<div class="table">' .
         '<table width="100%" cellspacing="1">' .
           '<th class="tableName center" colspan="4">' .
             'Network Summary' .
           '</th>' .
           '<tr>' .
             '<td class="columnTitle center">' .
               'Live IPs' .
             '</td>' .
             '<td class="columnTitle center">' .
               'Flows Seen' .
             '</td>' .
             '<td class="columnTitle center">' .
               'Bytes Transferred' .
             '</td>' .
             '<td class="columnTitle center">' .
               'Packets Transferred' .
             '</td>' .
           '</tr>' .
           '<tr class="even">' .
             '<td class="center">' .
               number_format(getNetworkStatistic($postgreSQL, $date, 'numLiveIPs')) .
             '</td>' .
             '<td class="center">' .
               number_format(getNetworkStatistic($postgreSQL, $date, 'numFlows')) .
             '</td>' .
             '<td class="center">' .
               size(getNetworkStatistic($postgreSQL, $date, 'numBytes')) .
             '</td>' .
             '<td class="center">' .
               number_format(getNetworkStatistic($postgreSQL, $date, 'numPackets')) .
             '</td>' .
           '</tr>' .
         '</table>' .
       '</div>';
  /*
  $result = pg_query($postgreSQL, 'SELECT COUNT(*) FROM "Maps"."containedHosts"');
  $row = pg_fetch_row($result);
  if ($row[0]) {
    echo '<div class="table">' .
           '<table width="100%" cellspacing="1">' .
             '<tr>' .
               '<th class="tableName center" colspan="5">' .
                 'Contained Hosts' .
               '</th>' .
             '</tr>' .
             '<tr>' .
               '<td class="columnTitle center">' .
                 'IP' .
               '</td>' .
               '<td class="columnTitle center">' .
                 'Contained at' .
               '</td>' .
               '<td class="columnTitle center">' .
                 'Elapsed Time' .
               '</td>' .
               '<td class="columnTitle center">' .
                 'Current Symptoms' .
               '</td>' .
               '<td class="columnTitle center">' .
                 'Clear Host' .
               '</td>' .
             '</tr>';
    $result = pg_query($postgreSQL, 'SELECT * FROM "Maps"."containedHosts"');
    $lastDay = getLastPGTable($postgreSQL, 'InterestingIPs');
    while ($row = pg_fetch_assoc($result)) {  
      if (existsPGTable($postgreSQL, 'HostTraffic', $date)) {
        $result3 = pg_query($postgreSQL, 'SELECT * FROM "HostTraffic"."' . $date . '" WHERE "ip" = \'' . $row['ip'] . '\'');
        $hostTrafficRow = pg_fetch_assoc($result3);
      }
      $result2 = pg_query($postgreSQL, 'SELECT * FROM "InterestingIPs"."' . $lastDay .
                                       '" WHERE "ip" = \'' . $row['ip'] . '\'');
      $row2 = pg_fetch_assoc($result2);
      $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
      ++$rowNumber;  
      echo '<tr class="' . $rowClass . '">' .
             '<td class="center">' .
               '<a class="text" href="/host/' . long2ip($row['ip']) . '">' . long2ip($row['ip']) . '</a>' .
             '</td>' .
             '<td class="center">' .
               date("l, F j, Y \a\\t g:i:s A", $row['time']) .
             '</td>' .
             '<td class="center">' .
               duration(time() - $row['time']) .
             '</td>' .
             '<td class="center">' .
               getSymptoms($row2, $lastDay, $hostTrafficRow, $roles) .
             '</td>' .
             '<td class="center">' .
               '<a href="/report/' . $date . '/' . getPageURL($page) . 'clear/' . long2ip($row['ip']) . '">' .
                 '<img src="/images/exclude.png" title="Clear host">' .
               '</a>' .
             '</td>' .
           '</tr>';
      }
    }
  */
?>
