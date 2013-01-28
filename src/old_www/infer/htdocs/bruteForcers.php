<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  $request = explode('/', substr($_SERVER['PATH_INFO'], 1));

  $schemaName = 'BruteForcers';

  if (!$request[0]) {
    $title = 'Brute Forcers';
    $logo = $title;
    include('include/calendar.php');
    include('include/header.php');
    foreach (getPGTableRange($postgreSQL, $schemaName,
                             getFirstPGTable($postgreSQL, $schemaName),
                             getLastPGTable($postgreSQL, $schemaName)) as $day) {
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
        drawMonth($postgreSQL, $year, $month, $schemaName, 'existsPGTable', '/bruteForcers');
        echo '</div>';
      }
    }
  }
  else {
    $date = $request[0];
    $title = 'Brute Forcers for ' . $date;
    if (existsPGTable($postgreSQL, 'InfectedIPs', $date)) {
      getInfectedIPs($postgreSQL, $date, $infectedIPs);
    }
    if (!$request[1]) {
      include('include/header.php');
      echo '<div class="table">' .
             '<table width="100%" cellspacing="1">' .
               '<tr>' .
                 '<td class="columnTitle center">' .
                   'Source IP' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Number of Destination IPs' .
                 '</td>' .
               '</tr>';
      $result = pg_query($postgreSQL, 'SELECT "sourceIP", COUNT(DISTINCT "destinationIP") FROM "' . $schemaName . '"."' . $date .
                                      '" GROUP BY "sourceIP" ORDER BY COUNT(DISTINCT "destinationIP") DESC');
      while ($row = pg_fetch_assoc($result)) {
        $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
        echo '<tr class="' . $rowClass . '">' .
               '<td class="center">' .
                 '<a class="text" href="/bruteForcers/' . $date . '/' . long2ip($row['sourceIP']) . '">' .
                   long2ip($row['sourceIP']) .
                 '</a>' .
               '</td>' .
               '<td class="center">' .
                 $row['count'] .
               '</td>' .
             '</tr>';
        ++$rowNumber;
      }
      echo '</table>' .
         '</div>';
    }
    else {
      $internalIP = $request[1];
      $numericInternalIP = sprintf('%u', ip2long($internalIP));
      if (!$request[2]) {
        $title .= ' (' . $request[1] . ')';
        $logo = makeLogo($postgreSQL, $schemaName, $date, $title, '/bruteForcers', '/' . $internalIP); 
        include('include/header.php');
        include('include/services.php');
        echo '<div class="table">' .
               '<table width="100%" cellspacing="1">' .
                 '<tr>' .
                   '<td class="columnTitle center">' .
                     'Service' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Number of Hosts' .
                   '</td>' .
                 '</tr>';
        $result = pg_query($postgreSQL, 'SELECT "destinationPort", COUNT(*) FROM "' . $schemaName . '"."' . $date .
                                        '" WHERE "sourceIP" = \'' . $numericInternalIP . '\' ' .
                                        'GROUP BY "destinationPort" ORDER BY COUNT(*) DESC');
        while ($row = pg_fetch_assoc($result)) {
          $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
          echo '<tr class="' . $rowClass . '">' .
                 '<td class="center">' .
                   '<a class="text" href="/bruteForcers/' . $date . '/' . $internalIP . '/' . $row['destinationPort'] . '">' .
                     getServiceName(6, 0, $row['destinationPort'], INTERNAL_INITIATOR) .
                   '</a>' .
                 '</td>' .
                 '<td class="center">' .
                   number_format($row['count']) .
                 '</td>' .
               '</tr>';
          ++$rowNumber;
        }
        echo '</table>' .
          '</div>';
      }
      else {
        getCountryNameMap($postgreSQL, $countryCodeMap, $countryNameMap);  
        $destinationPort = $request[2];
        include('include/services.php');
        $title = 'Brute Forcers for ' . $date . ' (' . $internalIP . '/' . getServiceName(6, 0, $destinationPort, INTERNAL_INITIATOR) . ')';
        include('include/header.php');
        echo '<div class="table">' .
               '<table width="100%" cellspacing="1">' .
                 '<tr>' .
                   '<td class="columnTitle center">' .
                     'Destination IP' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Autonomous System' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Region' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'First Occurence' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Last Occurence' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Number of Attempts' .
                   '</td>' .
                 '</tr>';
        $result = pg_query($postgreSQL, 'SELECT "destinationIP", "asNumber", "countryNumber", "startTime", "endTime", "numAttempts" FROM "' .
                                        $schemaName . '"."' . $date . '" WHERE "sourceIP" = \'' . $numericInternalIP .
                                        '\' AND "destinationPort" = \'' . $destinationPort . '\' ORDER BY "numAttempts" DESC');
        while ($row = pg_fetch_assoc($result)) {
          $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
          echo '<tr class="' . $rowClass . '">' .
                 '<td class="center">' .
                   long2ip($row['destinationIP']) .
                 '</td>' .
                 '<td class="center">' .
                   getASDescriptionByNumber($postgreSQL, $row['asNumber']) .
                 '</td>' .
                 '<td class="center">' .
                   getCountryPicture($row['countryNumber'], $countryCodeMap, $countryNameMap) .
                 '</td>' .
                 '<td class="center">' .
                   date("g:i:s A", $row['startTime']) .
                 '</td>' .
                 '<td class="center">' .
                   date("g:i:s A", $row['endTime']) .
                 '</td>' .
                 '<td class="center">' .
                   number_format($row['numAttempts']) .
                 '</td>' .
               '</tr>';
          ++$rowNumber;
        }
      }
    }
  }
?>
