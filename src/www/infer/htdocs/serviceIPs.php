<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');

  function sqlOR(&$_terms, $columnName) {
    if (count($_terms) == 1) {
      return '"' . $columnName . '" = \'' . $_terms[0] . '\'';
    }
    foreach ($_terms as &$term) {
      $terms[] = $columnName . ' = \'' . $term . '\'';
    }
    return '(' . implode(' OR ', $terms) . ')';
  }

  if (!$url[1]) {
    $title = 'IPs per Service';
    include('include/calendar.php');
    include('include/header.php');
    foreach (getPGTableRange($postgreSQL, 'PortIPs',
                             getFirstPGTable($postgreSQL, 'PortIPs'),
                             getLastPGTable($postgreSQL, 'PortIPs')) as $day) {
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
        drawMonth($postgreSQL, $year, $month, 'PortIPs', 'existsPGTable', '/serviceIPs');
        echo '</div>';
      }
    }
  }
  else {
    if (!$url[3]) {
      exit;
    }
    else {
      $date = $url[1];
      $numericIP = sprintf('%u', ip2long($url[2]));
      if (existsPGTable($postgreSQL, 'InfectedIPs', $date)) {
        getInfectedIPs($postgreSQL, $date, $infectedIPs);
      }
      getCountryNameMap($postgreSQL, $countryCodeMap, $countryNameMap);
      preg_match_all('/[A-Z][^A-Z]*/', $url[3], $serviceName);
      $serviceName = implode(' ', $serviceName[0]);
      $result = pg_query($postgreSQL, 'SELECT "ports", "initiator" FROM "Maps"."monitoredServices" ' .
                                      'WHERE "name" = \'' . pg_escape_string($postgreSQL, $serviceName) . '\'');
      if (!@pg_num_rows($result)) {
        exit;
      }
      $row = pg_fetch_assoc($result);
      $title = 'IPs per Service for ' . $date . '/' . $url[2] . '/' . $serviceName;
      include('include/header.php');
      $result = pg_query($postgreSQL, 'SELECT "externalIP", "port", "numBytes", "numPackets", "minPacketSize", "maxPacketSize", "startTime", ' .
                                      '"endTime", "asNumber", "countryNumber", "content" FROM "PortIPs"."' . $date . '" ' .
                                      'WHERE "internalIP" = \'' . $numericIP . '\' AND "initiator" = ' . $row['initiator'] . ' AND ' .
                                      sqlOR(explode(',', substr($row['ports'], 1, -1)), 'port') . ' ORDER BY "numPackets" DESC');
      
      if ($row['initiator'] == 1) {
        $portColumnName = 'External Port';
      }
      else {
        $portColumnName = 'Internal Port';
      }
      echo '<div class="table">' .
             '<table width="100%" cellspacing="1">' .
               '<tr>' .
                 '<td class="columnTitle center">' .
                   'External IP' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Autonomous System' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Region' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   $portColumnName .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'First Occurence' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Duration' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Number of Packets' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Number of Bytes' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Min. / Max. Packet Size' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Content Distribution' .
                 '</td>' .
               '</tr>';
      while ($row = pg_fetch_assoc($result)) {
        $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
        echo '<tr class="' . $rowClass . '">' .
               '<td class="center">' .
                 styleIP($infectedIPs, $row['externalIP']) .
               '</td>' .
               '<td class="center">' .
                 getASDescriptionByNumber($postgreSQL, $row['asNumber']) .
               '</td>' .
               '<td class="center">' .
                 getCountryPicture($row['countryNumber'], $countryCodeMap, $countryNameMap) .
               '</td>' .
               '<td class="center">' .
                 $row['port'] .
               '</td>' .
               '<td class="center">' .
                 date('g:i:s A', $row['startTime']) .
               '</td>' .
               '<td class="center">' .
                 duration($row['endTime'] - $row['startTime']) .
               '</td>' .
               '<td class="center">' .
                 number_format($row['numPackets']) .
               '</td>' .
               '<td class="center">' .
                 size($row['numBytes']) .
               '</td>' .
               '<td class="center">' .
                 size($row['minPacketSize']) . ' / ' . size($row['maxPacketSize']) .
               '</td>' .
               '<td class="center">' .
                 drawContentStripe($row['content'], $row['numBytes']) .
               '</td>' .
             '</tr>';
        ++$rowNumber;
      }
      echo '</table>' .
         '</div>';
    }
  }
  include('include/footer.html');
?>
