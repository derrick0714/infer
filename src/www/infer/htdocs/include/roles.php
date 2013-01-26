<?php
  define('BRUTE_FORCER', -1);
  define('BRUTE_FORCED', -2);
  define('DARK_SPACE_BOT', 0);
  define('SPAM_BOT', 1);
  define('MAIL_SERVER_BOT', 2);
  define('WEB_SERVER_BOT', 3);
  define('MULTIMEDIA_P2P_NODE', 4);
  define('FTP_BRUTE_FORCER', 5);
  define('FTP_BRUTE_FORCED', 6);
  define('SSH_BRUTE_FORCER', 7);
  define('SSH_BRUTE_FORCED', 8);
  define('TELNET_BRUTE_FORCER', 9);
  define('TELNET_BRUTE_FORCED', 10);
  define('SCAN_BOT', 11);
  define('UNCLASSIFIED_P2P_NODE', 12);
  define('ENCRYPTED_P2P_NODE', 13);
  define('MICROSOFT_SQL_BRUTE_FORCER', 14);
  define('MICROSOFT_SQL_BRUTE_FORCED', 15);
  define('ORACLE_SQL_BRUTE_FORCER', 16);
  define('ORACLE_SQL_BRUTE_FORCED', 17);
  define('MYSQL_BRUTE_FORCER', 18);
  define('MYSQL_BRUTE_FORCED', 19);
  define('POSTGRESQL_BRUTE_FORCER', 20);
  define('POSTGRESQL_BRUTE_FORCED', 21);

  define('PUBLISHER_TYPE', 0);
  define('CONSUMER_TYPE', 1);
  define('P2P_TYPE', 2);
  define('ATTACKER_TYPE', 3);
  define('VICTIM_TYPE', 4);

  $inboundServicePorts = array('Mail Server' => array(25, 110, 143),
                               'Secure Mail Server' => array(465, 993, 995),
                               'Web Server' => array(80),
                               'Secure Web Server' => array(443));

  $outboundServicePorts = array('Mail Client' => array(25, 110, 143),
                                'Secure Mail Client' => array(465, 993, 995),
                                'Web Client' => array(80),
                                'Secure Web Client' => array(443));

  $roleNames = array(BRUTE_FORCER => 'Brute Forcer',
                     BRUTE_FORCED => 'Brute Forced',
                     DARK_SPACE_BOT => 'Dark Space Bot',
                     SPAM_BOT => 'Spam Bot',
                     MAIL_SERVER_BOT => 'New Mail Server',
                     WEB_SERVER_BOT => 'New Web Server',
                     MULTIMEDIA_P2P_NODE => 'Multimedia P2P Node',
                     FTP_BRUTE_FORCER => 'FTP Brute Forcer',
                     FTP_BRUTE_FORCED => 'FTP Brute Forced',
                     SSH_BRUTE_FORCER => 'SSH Brute Forcer',
                     SSH_BRUTE_FORCED => 'SSH Brute Forced',
                     TELNET_BRUTE_FORCER => 'TELNET Brute Forcer',
                     TELNET_BRUTE_FORCED => 'TELNET Brute Forced',
                     SCAN_BOT => 'Scanner',
                     UNCLASSIFIED_P2P_NODE => 'Unclassified P2P Node',
                     ENCRYPTED_P2P_NODE => 'Encrypted P2P Node',
                     MICROSOFT_SQL_BRUTE_FORCER => 'Microsoft SQL Brute Forcer',
                     MICROSOFT_SQL_BRUTE_FORCED => 'Microsoft SQL Brute Forced',
                     ORACLE_SQL_BRUTE_FORCER => 'Oracle SQL Brute Forcer',
                     ORACLE_SQL_BRUTE_FORCED => 'Oracle SQL Brute Forced',
                     MYSQL_BRUTE_FORCER => 'MySQL Brute Forcer',
                     MYSQL_BRUTE_FORCED => 'MySQL Brute Forced',
                     POSTGRESQL_BRUTE_FORCER => 'PostgreSQL Brute Forcer',
                     POSTGRESQL_BRUTE_FORCED => 'PostgreSQL Brute Forced');

  $roleURLNames = array('DarkSpaceBot' => BRUTE_FORCER,
                        'SpamBot' => SPAM_BOT,
                        'MailServerBot' => MAIL_SERVER_BOT,
                        'WebServerBot' => WEB_SERVER_BOT,
                        'MultimediaP2PNode' => MULTIMEDIA_P2P_NODE,
                        'FTPBruteForcer' => FTP_BRUTE_FORCER,
                        'FTPBruteForced' => FTP_BRUTE_FORCED,
                        'SSHBruteForcer' => SSH_BRUTE_FORCER,
                        'SSHBruteForced' => SSH_BRUTE_FORCED,
                        'TELNETBruteForcer' => TELNET_BRUTE_FORCER,
                        'TELNETBruteForced' => TELNET_BRUTE_FORCED,
                        'ScanBot' => SCAN_BOT,
                        'UnclassifiedP2PNode' => UNCLASSIFIED_P2P_NODE,
                        'EncryptedP2PNode' => ENCRYPTED_P2P_NODE,
                        'MicrosoftSQLBruteForcer' => MICROSOFT_SQL_BRUTE_FORCER,
                        'MicrosoftSQLBruteForced' => MICROSOFT_SQL_BRUTE_FORCED,
                        'OracleSQLBruteForcer' => ORACLE_SQL_BRUTE_FORCER,
                        'OracleSQLBruteForced' => ORACLE_SQL_BRUTE_FORCED,
                        'MySQLBruteForcer' => MYSQL_BRUTE_FORCER,
                        'MySQLBruteForced' => MYSQL_BRUTE_FORCED,
                        'PostgreSQLBruteForcer' => POSTGRESQL_BRUTE_FORCER,
                        'PostgreSQLBruteForced' => POSTGRESQL_BRUTE_FORCED);

  $roleTypes = array(BRUTE_FORCER => ATTACKER_TYPE,
                     BRUTE_FORCED => VICTIM_TYPE,
                     DARK_SPACE_BOT => ATTACKER_TYPE,
                     SPAM_BOT => ATTACKER_TYPE,
                     MAIL_SERVER_BOT => ATTACKER_TYPE,
                     WEB_SERVER_BOT => ATTACKER_TYPE,
                     MULTIMEDIA_P2P_NODE => P2P_TYPE,
                     FTP_BRUTE_FORCER => ATTACKER_TYPE,
                     FTP_BRUTE_FORCED => VICTIM_TYPE,
                     SSH_BRUTE_FORCER => ATTACKER_TYPE,
                     SSH_BRUTE_FORCED => VICTIM_TYPE,
                     TELNET_BRUTE_FORCER => ATTACK_TYPE,
                     TELNET_BRUTE_FORCED => VICTIM_TYPE,
                     SCAN_BOT => ATTACKER_TYPE,
                     UNCLASSIFIED_P2P_NODE => P2P_TYPE,
                     ENCRYPTED_P2P_NODE => P2P_TYPE,
                     MICROSOFT_SQL_BRUTE_FORCER => ATTACKER_TYPE,
                     MICROSOFT_SQL_BRUTE_FORCED => VICTIM_TYPE,
                     ORACLE_SQL_BRUTE_FORCER => ATTACK_TYPE,
                     ORACLE_SQL_BRUTE_FORCED => VICTIM_TYPE,
                     MYSQL_BRUTE_FORCER => ATTACKER_TYPE,
                     MYSQL_BRUTE_FORCED => VICTIM_TYPE,
                     POSTGRESQL_BRUTE_FORCER => ATTACKER_TYPE,
                     POSTGRESQL_BRUTE_FORCED => VICTIM_TYPE);

  $roleTypeNames = array(PUBLISHER_TYPE => 'Publishers',
                         CONSUMER_TYPE => 'Consumers',
                         P2P_TYPE => 'Peer to Peer',
                         ATTACKER_TYPE => 'Attackers',
                         VICTIM_TYPE => 'Victims');

  $exclusiveRoles = array(0 => DARK_SPACE_BOT,
                          1 => SPAM_BOT,
                          2 => MAIL_SERVER_BOT,
                          3 => WEB_SERVER_BOT,
                          4 => P2P_NODE,
                          5 => MULTIMEDIA_P2P_NODE,
                          6 => FTP_BRUTE_FORCER,
                          7 => SSH_BRUTE_FORCER,
                          8 => TELNET_BRUTE_FORCER,
                          9 => SCAN_BOT,
                          10 => UNCLASSIFIED_P2P_NODE,
                          11 => ENCRYPTED_P2P_NODE);

  $bruteForcerPorts = array(FTP_BRUTE_FORCER => 21,
                            FTP_BRUTE_FORCED => 21,
                            SSH_BRUTE_FORCER => 22,
                            SSH_BRUTE_FORCED => 22,
                            TELNET_BRUTE_FORCER => 23,
                            TELNET_BRUTE_FORCED => 23,
                            MICROSOFT_SQL_BRUTE_FORCER => 1433,
                            MICROSOFT_SQL_BRUTE_FORCED => 1433,
                            ORACLE_SQL_BRUTE_FORCER => 1521,
                            ORACLE_SQL_BRUTE_FORCED => 1521,
                            MYSQL_BRUTE_FORCER => 3306,
                            MYSQL_BRUTE_FORCED => 3306,
                            POSTGRESQL_BRUTE_FORCER => 5432,
                            POSTGRESQL_BRUTE_FORCED => 5432);

  function compareLessThan(&$destination, &$source) {
    if ($source < $destination || !$destination) {
      $destination = $source;
    }
  }

  function compareGreaterThan(&$destination, &$source) {
    if ($source > $destination || !$destination) {
      $destination = $source;
    }
  }

  function roleURL(&$role, &$date, &$ip) {
    switch ($role) {
      case MULTIMEDIA_P2P_NODE:
        return '/role/multimediaP2PNode/' . $date . '/' . long2ip($ip);
        break;
      case BRUTE_FORCER:
        return '/role/bruteForcer/' . $date . '/' . long2ip($ip);
        break;
      case BRUTE_FORCED:
        return '/role/bruteForced/' . $date . '/' . long2ip($ip);
        break;
      case DARK_SPACE_BOT:
        return '/symptom/darkSpaceSources/' . $date . '/' . long2ip($ip);
        break;
      case SPAM_BOT:
        return '/role/spamBot/' . $date . '/' . long2ip($ip);
        break;
      case FTP_BRUTE_FORCER:
        return '/role/bruteForcer/' . $date . '/21' . '/' . long2ip($ip);
        break;
      case FTP_BRUTE_FORCED:
        return '/role/bruteForced/' . $date . '/21' . '/' . long2ip($ip);
        break;
      case SSH_BRUTE_FORCER:
        return '/role/bruteForcer/' . $date . '/22' . '/' . long2ip($ip);
        break;
      case SSH_BRUTE_FORCED:
        return '/role/bruteForced/' . $date . '/22' . '/' . long2ip($ip);
        break;
      case TELNET_BRUTE_FORCER:
        return '/role/bruteForcer/' . $date . '/23' . '/' . long2ip($ip);
        break;
      case TELNET_BRUTE_FORCED:
        return '/role/bruteForced/' . $date . '/23' . '/' . long2ip($ip);
        break;
      case SCAN_BOT:
        return '/scanners/' . $date . '/' . long2ip($ip);
        break;
      case UNCLASSIFIED_P2P_NODE:
        return '/role/unclassifiedP2PNode/' . $date . '/' . long2ip($ip);
        break;
      case ENCRYPTED_P2P_NODE:
        return '/role/encryptedP2PNode/' . $date . '/' . long2ip($ip);
        break;
      case MICROSOFT_SQL_BRUTE_FORCER:
        return '/role/bruteForcer/' . $date . '/1433' . '/' . long2ip($ip);
        break;
      case MICROSOFT_SQL_BRUTE_FORCED:
        return '/role/bruteForced/' . $date . '/1433' . '/' . long2ip($ip);
        break;
      case ORACLE_SQL_BRUTE_FORCER:
        return '/role/bruteForcer/' . $date . '/1521' . '/' . long2ip($ip);
        break;
      case ORACLE_SQL_BRUTE_FORCED:
        return '/role/bruteForced/' . $date . '/1521' . '/' . long2ip($ip);
        break;
      case MYSQL_BRUTE_FORCER:
        return '/role/bruteForcer/' . $date . '/3306' . '/' . long2ip($ip);
        break;
      case MYSQL_BRUTE_FORCED:
        return '/role/bruteForced/' . $date . '/3306' . '/' . long2ip($ip);
        break;
      case POSTGRESQL_BRUTE_FORCER:
        return '/role/bruteForcer/' . $date . '/5432' . '/' . long2ip($ip);
        break;              
      case POSTGRESQL_BRUTE_FORCED:
        return '/role/bruteForced/' . $date . '/5432' . '/' . long2ip($ip);
        break;
    }
  }

function get_comm_channels(&$postgreSQL, &$ip, &$date, &$infectedIPs) {
	$result = pg_query("select distinct \"externalIP\", \"external_ip_fanin\"" .
					   " from \"CommChannels\".\"" . $date . "\" where " .
					   "\"internalIP\" = '" . $ip . "' order by " .
					   "\"external_ip_fanin\" limit 3");
	if (!pg_num_rows($result)) {
		return '';
	}

	while ($row = pg_fetch_assoc($result)) {
		$external_ip_dec = $row["externalIP"];
		$external_ip = long2ip($external_ip_dec);

		$comm_channels[] = '<a class="text" href="/commChannels/' . $date .
						   '/' . long2ip($ip) . '/' . $external_ip . '">' .
						   styleIP($infectedIPs, $external_ip_dec, false) .
						   '</a>';
	}	
	if (count($comm_channels) > 1) {
		$comm_channels[count($comm_channels) - 1] =
			'or ' . $comm_channels[count($comm_channels) - 1];
	}

	return implode(', ', $comm_channels);
}

  function getCommChannels(&$postgreSQL, &$ip, &$date, &$infectedIPs, &$commChannels, &$commChannelNames) {
    foreach ($commChannels as &$commChannel) {
      $_commChannel = long2ip($commChannel);
      $_commChannels[] = '<a class="text" href="/commChannels/' . $date . '/' .
                         long2ip($ip) . '/' . $_commChannel . '">' .
                           styleIP($infectedIPs, $commChannel, false) .
                         '</a>';
    }
    if (count($_commChannels) > 1) {
      $_commChannels[count($_commChannels) - 1] = 'or ' . $_commChannels[count($_commChannels) - 1];
    }
    return implode(', ', $_commChannels);
  }

  function getRoles(&$postgreSQL, &$ip, &$date) {
    global $bruteForcerPorts;
    $result = @pg_query($postgreSQL, 'SELECT * FROM "Roles"."' . $date .
                                     '" WHERE "ip" = \'' . $ip . '\'');
    if (@pg_num_rows($result)) {
      while ($row = @pg_fetch_assoc($result)) {
        switch ($row['role']) {
          case DARK_SPACE_BOT:
          case SPAM_BOT:
          case MAIL_SERVER_BOT:
          case WEB_SERVER_BOT:
          case MULTIMEDIA_P2P_NODE:
          case SCAN_BOT:
          case ENCRYPTED_P2P_NODE:
          case UNCLASSIFIED_P2P_NODE:
            $roles[$row['role']]['numHosts'] = $row['numHosts'];
            $roles[$row['role']]['numBytes'] = $row['numBytes'];
            compareLessThan($roles[$row['role']]['startTime'], $row['startTime']);
            compareGreaterThan($roles[$row['role']]['endTime'], $row['endTime']);
            if ($row['commChannels'] != '{}') {
              $roles[$row['role']]['commChannels'] = explode(',', substr($row['commChannels'], 1, -1));
              $roles[$row['role']]['commChannelNames'] = explode(',', substr($row['commChannelNames'], 1, -1));
              $roles[$row['role']]['commChannelASNs'] = explode(',', substr($row['commChannelASNs'], 1, -1));
              $roles[$row['role']]['commChannelCountries'] = explode(',', substr($row['commChannelCountries'], 1, -1));
            }
            break;
          case FTP_BRUTE_FORCER:
          case SSH_BRUTE_FORCER:
          case TELNET_BRUTE_FORCER:
          case MICROSOFT_SQL_BRUTE_FORCER:
          case ORACLE_SQL_BRUTE_FORCER:
          case MYSQL_BRUTE_FORCER:
          case POSTGRESQL_BRUTE_FORCER:
            $roles[BRUTE_FORCER]['numHosts'] += $row['numHosts'];
            $roles[BRUTE_FORCER]['numBytes'] += $row['numBytes'];
            compareLessThan($roles[BRUTE_FORCER]['startTime'], $row['startTime']);
            compareGreaterThan($roles[BRUTE_FORCER]['endTime'], $row['endTime']);
            $roles[BRUTE_FORCER]['ports'][$bruteForcerPorts[$row['role']]] = $row['numHosts'];
            if ($row['commChannels'] != '{}') {
              foreach (explode(',', substr($row['commChannels'], 1, -1)) as $commChannel) {
                $roles[BRUTE_FORCER]['commChannels'][] = $commChannel;
              }
            }
            $roles[$row['role']]['numHosts'] = $row['numHosts'];
            $roles[$row['role']]['numBytes'] = $row['numBytes'];
            $roles[$row['role']]['startTime'] = $row['startTime'];
            $roles[$row['role']]['endTime'] = $row['endTime'];
            break;
          case FTP_BRUTE_FORCED:
          case SSH_BRUTE_FORCED:
          case TELNET_BRUTE_FORCED:
          case MICROSOFT_SQL_BRUTE_FORCED:
          case ORACLE_SQL_BRUTE_FORCED:
          case MYSQL_BRUTE_FORCED:
          case POSTGRESQL_BRUTE_FORCED:
            $roles[BRUTE_FORCED]['numHosts'] += $row['numHosts'];
            $roles[BRUTE_FORCED]['numBytes'] += $row['numBytes'];
            compareLessThan($roles[BRUTE_FORCED]['startTime'], $row['startTime']);
            compareGreaterThan($roles[BRUTE_FORCED]['endTime'], $row['endTime']);
            $roles[BRUTE_FORCED]['ports'][$bruteForcerPorts[$row['role']]] = $row['numHosts'];
            $roles[$row['role']]['numHosts'] = $row['numHosts'];
            $roles[$row['role']]['numBytes'] = $row['numBytes'];
            $roles[$row['role']]['startTime'] = $row['startTime'];
            $roles[$row['role']]['endTime'] = $row['endTime'];
            break;
        }
      }
      return $roles;
    }
  }

  function getInterestingPortIndex(&$postgreSQL, &$date) {
    $result = pg_query($postgreSQL, 'SELECT "interestingPorts" FROM "Indexes"."interestingPorts" WHERE "date" = \'' . $date . '\'');
    if (pg_num_rows($result)) {
      $row = pg_fetch_row($result);
      return explode(',', substr($row[0], 1, -1));
    }
    return false;
  }

  function getInboundPortTraffic(&$postgreSQL, &$date, &$interestingPortIndex, &$ip) {
    $schemaName = 'HostTraffic';
    if (existsPGTable($postgreSQL, $schemaName, $date)) {
      $result = pg_query($postgreSQL, 'SELECT "inboundPortTraffic" FROM "' . $schemaName . '"."' . $date . '" WHERE "ip" = \'' . $ip . '\'');
      if (pg_num_rows($result)) {
        $row = pg_fetch_row($result);
        foreach (explode('},{', substr($row[0], 2, -2)) as $hour) {
          $index = 0;
          foreach (explode(',', $hour) as $bytes) {
            $inboundPortTraffic[$interestingPortIndex[$index++]] += $bytes;
          }
        }
        return $inboundPortTraffic;
      }
    }
    return false;
  }

  function getOutboundPortTraffic(&$postgreSQL, &$date, &$interestingPortIndex, &$ip) {
    $schemaName = 'HostTraffic';
    if (existsPGTable($postgreSQL, $schemaName, $date)) {
      $result = pg_query($postgreSQL, 'SELECT "outboundPortTraffic" FROM "' . $schemaName . '"."' . $date .'" WHERE "ip" = \'' . $ip . '\'');
      if (pg_num_rows($result)) {
        $row = pg_fetch_row($result);
        foreach (explode('},{', substr($row[0], 2, -2)) as $hour) {
          $index = 0;
          foreach (explode(',', $hour) as $bytes) {
            $outboundPortTraffic[$interestingPortIndex[$index++]] += $bytes;
          }
        }
        return $outboundPortTraffic;
      }
    }
    return false;
  }

  function getInboundPortFlows(&$postgreSQL, &$date, &$interestingPortIndex, &$ip) {
    $result = @pg_query($postgreSQL, 'SELECT "inboundPortIPs" FROM "HostTraffic"."' . $date .'" WHERE "ip" = \'' . $ip . '\'');
    if (@pg_num_rows($result)) {
      $row = pg_fetch_assoc($result);
      $index = 0;
      foreach (explode(',', substr($row['inboundPortIPs'], 1, -1)) as $bytesPerPort) {
        $inboundPortFlows[$interestingPortIndex[$index++]] = $bytesPerPort;
      }
      return $inboundPortFlows;
    }
    return false;
  }

  function getoutboundPortFlows(&$postgreSQL, &$date, &$interestingPortIndex, &$ip) {
    $result = @pg_query($postgreSQL, 'SELECT "outboundPortIPs" FROM "HostTraffic"."' . $date .'" WHERE "ip" = \'' . $ip . '\'');
    if (@pg_num_rows($result)) {
      $row = pg_fetch_assoc($result);
      $index = 0;
      foreach (explode(',', substr($row['outboundPortIPs'], 1, -1)) as $bytesPerPort) {
        $inboundPortFlows[$interestingPortIndex[$index++]] = $bytesPerPort;
      }
      return $inboundPortFlows;
    }
    return false;
  }
?>
