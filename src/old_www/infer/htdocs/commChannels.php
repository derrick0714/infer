<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  $request = explode('/', substr($_SERVER['PATH_INFO'], 1));

  $schemaName = 'CommChannels';

  if (!$request[0]) {
    $title = 'Suspicious Communication Channels';
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
        drawMonth($postgreSQL, $year, $month, $schemaName, 'existsPGTable', '/commChannels');
        echo '</div>';
      }
    }
  }
  else {
    $date = $request[0];
    $title = 'Suspicious Communication Channels for ' . $date;
    if (existsPGTable($postgreSQL, 'InfectedIPs', $date)) {
      getInfectedIPs($postgreSQL, $date, $infectedIPs);
    }
    if ($request[1]) {
      $internalIP = $request[1];         
      $numericInternalIP = sprintf('%u', ip2long($internalIP));
      $title .= ' (' . $request[1] . ')';
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
                   echo '<a class="text" href="/commChannels/' . $date . '/' . $internalIP . '/' . long2ip($row[0]) . '">' .
                          '<font color="#FF0000">' .
                            long2ip($row[0]) .
                          '</font>' .
                        '</a>';
                 }
                 else {
                   echo '<a class="text" href="/commChannels/' . $date . '/' . $internalIP . '/' . long2ip($row[0]) . '">' .
                          long2ip($row[0]) .
                        '</a>';
                 }
                 echo '</td>' .
                      '<td class="center">';
                 if (!isInternal($row[0])) {
                   echo getASDescriptionByNumber($postgreSQL, $row[3]);
                 }
                 echo '</td>' .
                      '<td class="center">';
                 if (!isInternal($row[0]) && $row[4] != 0) {
                   echo getCountryPicture($row[4], $countryCodeMap, $countryNameMap);
                 }
                 echo '</td>' .
                      '<td class="center">' .
                        number_format($row[5]) .
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
      $title = 'Suspicious Communication Channels for ' . $date . ' (' . $internalIP . '/' . $externalIP . ')';
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
      $result = pg_query($postgreSQL, 'SELECT "internalPort", "externalPort", "initiator", "numBytes", "numPackets", ' .
                                      '"minPacketSize", "maxPacketSize", "startTime", "endTime", "asNumber", "content" FROM "' .
                                      $schemaName . '"."' . $date . '" WHERE "internalIP" = \'' . $numericInternalIP .
                                      '\' AND "externalIP" = \'' . $numericExternalIP . '\' ORDER BY "numPackets" DESC');
      while ($row = pg_fetch_row($result)) {
        $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
        echo '<tr class="' . $rowClass . '">' .
               '<td class="center">' .
                 'TCP' .
               '</td>' .
               '<td class="center">';
        if ($row[2] == 1) {
          echo '<b>' .
                 $row[0] .
               '</b>';
        }
        else {
          echo $row[0];
        }
        echo '</td>' .
             '<td class="center">';
        if ($row[2] == 2) {
          echo '<b>' .
                 $row[1] .
               '</b>';
        }
        else {
          echo $row[1];
        }
        echo '</td>' .
             '<td class="center">' .
               getServiceName(6, $row[0], $row[1], $row[2]) .
             '</td>' .
             '<td class="center">' .
               date('g:i:s A', $row[7]) .
             '</td>' .
             '<td class="center">' .
               duration($row[8] - $row[7]) .
             '</td>' .
             '<td class="center">' .
               number_format($row[4]) .
             '</td>' .
             '<td class="center">' .
               size($row[3]) .
             '</td>' .
             '<td class="center">' .
               size($row[5]) . ' / ' . size($row[6]) .
             '</td>' .
             '<td class="center">' .
                drawContentStripe($row[10], $row[3]) .
             '</td>' .
           '</tr>';
        ++$rowNumber;
      }
      echo '</table>' .
         '</div>';
    }
  }
?>
