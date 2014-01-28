<?php
  include('include/postgreSQL.php');
  include('include/roles.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  
  $rolesSchema = 'Roles';

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

  /* Displays the calendar if no date was provided. */
  if (!$url[1]) {
    $title = 'Host Roles';
    $logo = $title;
    include('include/calendar.php');
    include('include/header.php');

    $days = getPGTableRange($postgreSQL, $rolesSchema,
                             getFirstPGTable($postgreSQL, $rolesSchema),
                             getLastPGTable($postgreSQL, $rolesSchema));
    if (count($days) == 0) {
      include('include/footer.html');
      exit;
    }

    foreach ($days as &$day) {
      $date = explode('-', $day);
      if ($year != $date[0]) {
        $year = $date[0];
        unset($month);
        echo '<div class="yearContainer">' .
               '<div class="columnTitle">' .
                 $year .
               '</div>' .
             '</div>';
      }
      if ($month != $date[1]) {
        $month = $date[1];
        echo '<div class="monthContainer">';
        drawMonth($postgreSQL, $year, $month, $rolesSchema, 'existsPGTable', '/roles');
        echo '</div>';
      }
    }
  }
  else {
    $date = $url[1];
    $title = 'Host Roles for ' . $date;
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
    if (!$url[2]) {
      include('include/header.php');
      /*
       * Retrieves the ports that were monitored for inbound and outbound
       * connections on the specified day. 
       */
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
                 '<td align="center" class="tableName">' .
                   'Role' .
                 '</td>' .
                 '<td align="center" class="tableName">' .
                   'Num. Hosts' .
                 '</td>' .
                 '<td align="center" class="tableName">' .
                   'Bytes Transferred' .
                 '</td>' .
               '</tr>';
      foreach ($roles as $type => &$_roles) {
        echo '<tr>' .
               '<td align="center" class="columnTitle" colspan="3">' .
                 '<i>' .
                   $roleTypeNames[$type] .
                 '</i>' .
               '</td>' .
             '</tr>';
        $rowNumber = 0;
        foreach ($_roles as $_role => &$properties) {
          $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
          echo '<tr class="' . $rowClass . '">' .
                 '<td align="center">' .
                  '<a class="text" href="/roles/' . $url[1] . '/' . str_replace(' ', '', $_role) . '">' .
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
        }
      }
    }
    else {
      if (array_key_exists($url[2], $roleURLNames) !== false) {
        $title = $roleNames[$roleURLNames[$url[2]]] . ' ' . $title;
        $logo = $title;
        include('include/header.php');
        echo '<div class="table">' .
               '<table cellspacing="1" width="100%">' .
                 '<tr>' .
                   '<td align="center" class="columnTitle">' .
                     'IP' .
                   '</td>' .
                   '<td align="center" class="columnTitle">' .
                     'DNS Name(s)' .
                   '</td>' .
                   '<td align="center" class="columnTitle">' .
                     'Num. Hosts' .
                   '</td>' .
                   '<td align="center" class="columnTitle">' .
                     'Bytes Transferred' .
                   '</td>' .
                 '</tr>';
        $result = pg_query($postgreSQL, 'SELECT "ip", "numHosts", "numBytes" FROM "Roles"."' . $url[1] . '" ' .
                                        'WHERE "role" = \'' . $roleURLNames[$url[2]] . '\' ORDER BY "numHosts" DESC');
        while ($row = pg_fetch_assoc($result)) {
          $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
          echo '<tr class="' . $rowClass . '">' .
                 '<td align="center">' .
                   '<a class="text" href="' . roleURL($roleURLNames[$url[2]], $url[1], $row['ip']) . '">' .
                     long2ip($row['ip']) .
                   '</a>'.
                 '</td>' .
                 '<td align="center">' .
                   '<a class="text" href="' . roleURL($roleURLNames[$url[2]], $url[1], $row['ip']) . '">' .
                     getDNSNames($postgreSQL, $url[1], $row['ip']) .
                   '</a>'.
                 '<td align="center">' .
                   number_format($row['numHosts']) .
                 '</td>' .
                 '<td align="center">' .
                   size($row['numBytes'], ',') .
                 '</td>' .
               '</tr>';
          ++$rowNumber;
        }
      }
      else {
        preg_match_all('/[A-Z][^A-Z]*/', $url[2], $serviceName);
        $serviceName = implode(' ', $serviceName[0]);
        $title = $serviceName . ' ' . $title;
        $logo = $title;
        include('include/header.php');
        echo '<div class="table">' .
               '<table cellspacing="1" width="100%">' .
                 '<tr>' .
                   '<td align="center" class="columnTitle">' .
                     'IP' .
                   '</td>' .
                   '<td align="center" class="columnTitle">' .
                     'DNS Name(s)' .
                   '</td>' .
                   '<td align="center" class="columnTitle">' .
                     'Num. Hosts' .
                   '</td>' .
                   '<td align="center" class="columnTitle">' .
                     'Bytes Transferred' .
                   '</td>' .
                 '</tr>';
        $result = @pg_query($postgreSQL, 'SELECT "interestingPorts" FROM "Indexes"."interestingPorts" WHERE "date" = \'' . $url[1] . '\'');
        $row = @pg_fetch_assoc($result);
        $interestingPorts = array_flip(explode(',', substr($row['interestingPorts'], 1, -1)));
        $result = pg_query($postgreSQL, 'SELECT * FROM "Maps"."monitoredServices" WHERE "name" = \'' . $serviceName . '\'');
        if (@pg_num_rows($result) == 0) {
          include('include/footer.html');
          exit;
        }
        $row = pg_fetch_assoc($result);
        $ports = explode(',', substr($row['ports'], 1, -1));
        $result = pg_query($postgreSQL, 'SELECT "ip", ' . ports($interestingPorts, $ports, $row['initiator'], 'PortIPs', ' + ') .
                                        ' AS "portIPs", ' . ports($interestingPorts, $ports, $row['initiator'], 'PortTraffic', ' + ') .
                                        ' AS "portTraffic" FROM "HostTraffic"."' . $url[1] . '" WHERE ' .
                                        ports($interestingPorts, $ports, $row['initiator'], 'PortTraffic', ' + ' ) . ' > \'0\' ORDER BY ' .
                                        ports($interestingPorts, $ports, $row['initiator'], 'PortIPs', ' + ') . ' DESC');
        while ($row = pg_fetch_assoc($result)) {
          $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
          echo '<tr class="' . $rowClass . '">' .
                 '<td align="center">' .
                   '<a class="text" href="/serviceIPs/' . $url[1] . '/' . long2ip($row['ip']) . '/' . $url[2] . '">' .
                     long2ip($row['ip']) .
                   '</a>'.
                 '</td>' .
                 '<td align="center">' .
                   '<a class="text" href="/serviceIPs/' . $url[1] . '/' . long2ip($row['ip']) . '/' . $url[2] . '">' .
                     getDNSNames($postgreSQL, $url[1], $row['ip']) .
                   '</a>'.
                 '<td align="center">' .
                   number_format($row['portIPs']) .
                 '</td>' .
                 '<td align="center">' .
                   size($row['portTraffic'], ',') .
                 '</td>' .
               '</tr>';
          ++$rowNumber;
        }
      }
    }
  }
  echo '</table>' .
     '</div>';
  include('include/footer.html');
?>
