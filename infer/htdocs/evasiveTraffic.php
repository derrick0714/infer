<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  $request = explode('/', substr($_SERVER['PATH_INFO'], 1));

  $schemaName = 'EvasiveTraffic';

  if (!$request[0]) {
    $title = 'Evasive Traffic';
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
        drawMonth($postgreSQL, $year, $month, $schemaName, 'existsPGTable', '/evasiveTraffic');
        echo '</div>';
      }
    }
  }
  else {
    $date = $request[0];
    $title = 'Evasive Traffic for ' . $date;
    $logo = '<table width="100%">' .
              '<tr>' .
                '<td class="sub_title" width="15%" align="left">' .
                  getPreviousPGDay($postgreSQL, $schemaName, $date, '/evasiveTraffic') .
               '</td>' .
              '<td width="60%" align="center">' .
                $title .
              '</td>' .
              '<td class="sub_title" width="15%" align="right">' .
                getNextPGDay($postgreSQL, $schemaName, $date, '/evasiveTraffic') .
              '</td>' .
            '</tr>' .
          '</table>';
    if (existsPGTable($postgreSQL, 'InfectedIPs', $date)) {
      getInfectedIPs($postgreSQL, $date, $infectedIPs);
    }
    if ($request[1]) {
      $internalIP = $request[1];         
      $numericInternalIP = sprintf('%u', ip2long($internalIP));
      $title .= ' (' . $request[1] . ')';
      $logo = makeLogo($postgreSQL, $schemaName, $date, $title, '/evasiveTraffic', '/' . $internalIP);
    }
    else {
      exit;
    }

    getCountryNameMap($postgreSQL, $countryCodeMap, $countryNameMap);

    if (!$request[2]) {
      include('include/header.php');
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
                 '</td>'  .
                 '<td class="columnTitle center">' .
                   'Number of Occurences' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Number of Packets' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Number of Bytes' .
                 '</td>' .
               '</tr>';
      $result = pg_query($postgreSQL, 'SELECT "externalIP", SUM("numBytes"), SUM("numPackets"), "asNumber", "countryNumber", COUNT(*) ' .
                                      'FROM "' . $schemaName . '"."' . $date . '" WHERE "internalIP" = \'' . $numericInternalIP . '\' ' .
                                      'GROUP BY "externalIP", "asNumber", "countryNumber" ORDER BY SUM("numPackets") DESC');
      while ($row = pg_fetch_row($result)) {
        $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
        echo '<tr class="' . $rowClass . '">' .
               '<td class="center">';
                 if ($infectedIPs && array_search($row[0], $infectedIPs) !== false) {
                   echo '<a class="text" href="/evasiveTraffic/' . $date . '/' . $internalIP . '/' . long2ip($row[0]) . '">' .
                          '<font color="#FF0000">' .
                            long2ip($row[0]) .
                          '</font>' .
                        '</a>';
                 }
                 else {
                   echo '<a class="text" href="/evasiveTraffic/' . $date . '/' . $internalIP . '/' . long2ip($row[0]) . '">' .
                          long2ip($row[0]) .
                        '</a>';
                 }
                 echo '</td>' .
                      '<td class="center">';
                 echo getASDescriptionByNumber($postgreSQL, $row[3]);
                 echo '</td>' .
                      '<td class="center">';
                 if ($row[4] != 0) {
                   echo getCountryPicture($row[4], $countryCodeMap, $countryNameMap);
                 }
                 echo '</td>' .
                      '<td class="center">' .
                        $row[5] .
                      '</td>' .
                      '<td class="center">' .
                        number_format($row[2]) .
                      '</td>' .
                      '<td class="center">' .
                        size($row[1]) .
                      '</td>' .
                    '</tr>';
        ++$rowNumber;
      }
      echo '</table>' .
         '</div>';
    }
    else {
      $externalIP = $request[2];
      $numericExternalIP = sprintf('%u', ip2long($externalIP));
      $title = 'Evasive Traffic for ' . $date . ' (' . $internalIP . '/' . $externalIP . ')';
      $logo = makeLogo($postgreSQL, $schemaName, $date,
                       'Evasive Traffic for ' . $date . ' (' . $internalIP . '/' . styleIP($infectedIPs, $numericExternalIP) . ')',
                       '/evasiveTraffic', '/' . $internalIP . '/' . $externalIP);
      include('include/header.php');
      include('include/services.php');
      echo '<div class="table">' .
             '<table width="100%" cellspacing="1">' .
               '<tr>' .
                 '<td class="columnTitle center">' .
                   'Protocol' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Internal Port' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'External Port' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Service' .
                 '</td>'  .
                 '<td class="columnTitle center">' .
                   'First Occurence' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Duration' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Number of Frags' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Min. / Max. TTL' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Number of Packets' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Min. / Max. Packet Size' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Number of Bytes' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Content Distribution' .
                 '</td>' .
               '</tr>';
      $result = pg_query($postgreSQL, 'SELECT "protocol", "internalPort", "externalPort", "initiator", "numBytes", "numPackets", ' .
                                      '"minPacketSize", "maxPacketSize", "numFrags", "minTTL", "maxTTL", "startTime", "endTime", ' .
                                      '"asNumber", "content" FROM "' . $schemaName . '"."' . $date . '" WHERE "internalIP" = \'' .
                                      $numericInternalIP . '\' AND "externalIP" = \'' . $numericExternalIP . '\' ORDER BY "numPackets" DESC');
      while ($row = pg_fetch_assoc($result)) {
        $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
        echo '<tr class="' . $rowClass . '">' .
               '<td class="center">' .
                 $protocolNames[$row['protocol']] .
               '</td>' .
               '<td class="center">' .
                 styleInternalPort($row['internalPort'], $row['initiator']) .
               '</td>' .
               '<td class="center">' .
                 styleExternalPort($row['externalPort'], $row['initiator']) .
               '</td>' .
               '<td class="center">' .
                 getServiceName($row['protocol'], $row['internalPort'], $row['externalPort'], $row['initiator']) .
               '</td>' .
               '<td class="center">' .
                 date('g:i:s A', $row['startTime']) .
               '</td>' .
               '<td class="center">' .
                 duration($row['endTime'] - $row['startTime']) .
               '</td>' .
               '<td class="center">' .
                 $row['numFrags'] .
               '</td>' .
               '<td class="center">' .
                 $row['minTTL'] . ' / ' . $row['maxTTL'] .
               '</td>' .
               '<td class="center">' .
                 number_format($row['numPackets']) .
               '</td>' .
               '<td class="center">';
        if ($row['minPacketSize']) {
          echo size($row['minPacketSize']) . ' / ' . size($row['maxPacketSize']);
        }
        echo '</td>' .
             '<td class="center">' .
               size($row['numBytes']) .
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
?>
