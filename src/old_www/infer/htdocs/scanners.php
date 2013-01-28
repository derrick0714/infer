<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  $request = explode('/', substr($_SERVER['PATH_INFO'], 1));

  $schemaName = 'Scanners';

  if (!$request[0]) {
    $title = 'Scanners';
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
        drawMonth($postgreSQL, $year, $month, $schemaName, 'existsPGTable', '/scanners');
        echo '</div>';
      }
    }
  }
  else {
    $date = $request[0];
    $title = 'Scanners for ' . $date;
    $logo = '<table width="100%">' .
              '<tr>' .
                '<td class="sub_title" width="15%" align="left">' .
                  getPreviousPGDay($postgreSQL, $schemaName, $date, '/scanners') .
               '</td>' .
              '<td width="60%" align="center">' .
                $title .
              '</td>' .
              '<td class="sub_title" width="15%" align="right">' .
                getNextPGDay($postgreSQL, $schemaName, $date, '/scanners') .
              '</td>' .
            '</tr>' .
          '</table>';
 
    if (existsPGTable($postgreSQL, 'InfectedIPs', $date)) {
      getInfectedIPs($postgreSQL, $date, $infectedIPs);
    }
    if (!$request[1]) {
      $logo = makeLogo($postgreSQL, $schemaName, $date, $title, '/scanners', '/' . $internalIP);
      getCountryNameMap($postgreSQL, $countryCodeMap, $countryNameMap);  
      include('include/header.php');
      echo '<div class="table">' .
             '<table width="100%" cellspacing="1">' .
               '<tr>' .
                 '<td class="columnTitle center">' .
                   'Source IP' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Autonomous System' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Country' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'First Occurence' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Last Occurence' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Scan Type' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Number of Packets' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Number of Bytes' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Number of Destination IPs' .
                 '</td>' .
               '</tr>';
      $result = pg_query($postgreSQL, 'SELECT "sourceIP", "startTime", "endTime", "numPackets", "scanType", "numBytes", COUNT(DISTINCT "destinationIP") FROM "' . $schemaName . '"."' . $date .
                                            '" GROUP BY "sourceIP", "startTime", "endTime", "numPackets", "numBytes", "scanType" ORDER BY COUNT(DISTINCT "destinationIP") DESC');
      while ($row = pg_fetch_assoc($result)) {
        $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
        $countryNumber = getCountryNumberByIP($postgreSQL, $row['sourceIP']);
        echo '<tr class="' . $rowClass . '">' .
               '<td class="center">' .
                 '<a class="text" href="/scanners/' . $date . '/' . long2ip($row['sourceIP']) . '">';
                  if ($infectedIPs && array_search($row['sourceIP'], $infectedIPs) !== false) {
                    echo '<font color="#FF0000">' .
                          long2ip($row['sourceIP']) .
                         '</font>';
                  }
                  else {
                    echo long2ip($row['sourceIP']);
                  }
                  echo '</a>';
          echo '</td>' .
               '<td class="center">' .
                 getASDescriptionByNumber($postgreSQL, getASNByIP($postgreSQL, $row['sourceIP'])) .
               '</td>' .
               '<td class="center">' .
                 getCountryPicture(getCountryNumberByIP($postgreSQL, $row['sourceIP']), $countryCodeMap, $countryNameMap) .
               '</td>' .
               '<td class="center">' .
                 date("g:i:s A", $row['startTime']) .
               '</td>' .
	       '<td class="center">' .
                 date("g:i:s A", $row['endTime']) .
               '</td>' .
               '<td class="center">' .
                 $row['scanType'] .
               '</td>' .
               '<td class="center">' .
                 $row['numPackets'] .
               '</td>' .
               '<td class="center">' .
                 size($row['numBytes']) .
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
      include('include/services.php');
      include('include/header.php');
      include('include/ipRanges.php');
      $title .= ' (' . $request[1] . ')';
      getCountryNameMap($postgreSQL, $countryCodeMap, $countryNameMap);  
      echo '<div class="table">' .
             '<table width="100%" cellspacing="1">' .
               '<tr>' .
                 '<td class="columnTitle center">' .
                   'Destination IP' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Destination Port' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Service' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Autonomous System' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Country' .
                 '</td>' .
               '</tr>';
      $result = pg_query($postgreSQL, 'SELECT "destinationIP", "destinationPort", "asNumber", "countryNumber" FROM "' .$schemaName . '"."' . $date . 
                                      '" WHERE "sourceIP" = \'' . $numericInternalIP . '\'');
      $scannedIPRange = new ScannedIPRange;
      $row = pg_fetch_assoc($result);
      $scannedIPRange -> firstIP =$row['destinationIP'];
      $scannedIPRange -> lastIP =$row['destinationIP'];
      $scannedIPRange -> destinationPort =$row['destinationPort'];
      $scannedIPRange -> asNumber =$row['asNumber'];     
      $scannedIPRange -> countryNumber =$row['countryNumber'];     
      $previousDestinationIP = $scannedIPRange -> firstIP;
      $previousDestinationPort = $scannedIPRange -> destinationPort;
      while ($row = pg_fetch_assoc($result)) {
        if ($previousDestinationIP == $row['destinationIP'] - 1) {
          if ($previousDestinationPort == $row['destinationPort']) {
            $scannedIPRange -> lastIP = $row['destinationIP'];
          }
        }
        else {
          $scannedIPRanges[] = clone($scannedIPRange);
          $scannedIPRange = new ScannedIPRange;
          $scannedIPRange -> firstIP = $row['destinationIP'];
          $scannedIPRange -> lastIP = $row['destinationIP'];
          $scannedIPRange -> destinationPort = $row['destinationPort'];
          $scannedIPRange -> asNumber =$row['asNumber'];     
          $scannedIPRange -> countryNumber =$row['countryNumber'];     
        }
        $previousDestinationIP = $row['destinationIP'];
        $previousDestinationPort = $scannedIPRange -> destinationPort;
      }
      $scannedIPRanges[] = clone($scannedIPRange);
      if ($scannedIPRanges[0] -> firstIP) {
        foreach ($scannedIPRanges as &$scannedIPRange) {
          $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
          echo '<tr class="' . $rowClass . '">' .
                 '<td class="center">' .
                   makeIPPair($scannedIPRange -> firstIP, $scannedIPRange -> lastIP) .
                 '</td>' .
                 '<td class="center">' .
                   $scannedIPRange -> destinationPort .
                 '</td>' .
                 '<td class="center">' .
                   getServiceName(6, 0, $scannedIPRange -> destinationPort, INTERNAL_INITIATOR) .
                 '</td>' .
                 '<td class="center">' .
                    getASDescriptionByNumber($postgreSQL, $scannedIPRange -> asNumber) .
                  '</td>' .
                 '<td class="center">' .
                   getCountryPicture($scannedIPRange -> countryNumber, $countryCodeMap, $countryNameMap) .
                 '</td>' .
               '</tr>';
          ++$rowNumber;
        }
      }
    }
  }
?>
