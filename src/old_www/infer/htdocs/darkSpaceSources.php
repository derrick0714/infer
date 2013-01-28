<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  $request = explode('/', substr($_SERVER['PATH_INFO'], 1));

  $schemaName = 'DarkSpaceSources';

  if (!$request[0]) {
    include('include/calendar.php');
    $title = 'Dark Space Sources';
    $logo = $title;
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
        drawMonth($postgreSQL, $year, $month, $schemaName, 'existsPGTable', '/darkSpaceSources');
        echo '</div>';
      }
    }
  }
  else {
    $date = $request[0];
    $title = 'Dark Space Sources for ' . $date;
    $logo = '<table width="100%">' .
              '<tr>' .
                '<td class="sub_title" width="15%" align="left">' .
                  getPreviousPGDay($postgreSQL, $schemaName, $date, '/darkSpaceSources') .
                '</td>' .
                '<td width="60%" align="center">' .
                  $title .
                '</td>' .
                '<td class="sub_title" width="15%" align="right">' .
                  getNextPGDay($postgreSQL, $schemaName, $date, '/darkSpaceSources') .
                '</td>' .
              '</tr>' .
            '</table>';
    if (existsPGTable($postgreSQL, 'InfectedIPs', $date)) {
      getInfectedIPs($postgreSQL, $date, $infectedIPs);
    }
    if ($request[1]) {
      $ip = $request[1];
      $numericIP = sprintf('%u', ip2long($ip));
      $title .= ' (' . $request[1] . ')';
      $logo = makeLogo($postgreSQL, $schemaName, $date, $title, '/darkSpaceSources', '/' . $ip);
      $extra = ' WHERE "sourceIP" = \'' . sprintf('%u', ip2long($request[1])) . '\'';
    }
    if (!$request[2]) {
      getCountryNameMap($postgreSQL, $countryCodeMap, $countryNameMap);
      $result = pg_query($postgreSQL, 'SELECT "sourceIP", "destinationPort", COUNT(DISTINCT "destinationIP") FROM "' . $schemaName . '"."' . $date .
                         '" ' . $extra . ' GROUP BY "sourceIP", "destinationPort" ORDER BY COUNT(DISTINCT "destinationIP") DESC');
      while ($row = pg_fetch_row($result)) {
        $darkSpaceSources[$row[0] . ' ' . $row[1]]['numDarkTargets'] = $row[2];
      }
      include('include/header.php');
      echo '<div class="table">' .
             '<table width="100%" cellspacing="1">' .
             '<tr>' .
               '<td class="columnTitle center">' .
                 'Source IP' .
               '</td>' .
               '<td class="columnTitle center">' .
                 'Country' .
               '</td>' .
               '<td class="columnTitle center">' .
                 'Destination Port' .
               '</td>' .
               '<td class="columnTitle center">' .
                 'Targets in Dark Space' .
               '</td>' .
             '</tr>';
      if ($darkSpaceSources) {
        foreach ($darkSpaceSources as $flowID => &$stats) {
          $flowID = explode(' ', $flowID);
          $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
          if (!isInternal($flowID[0])) {
            $sourceCountryNumber = getCountryNumberByIP($postgreSQL, $flowID[0]);
          }
          else {
            $sourceCountryNumber = NULL;
          }
          echo '<tr class="' . $rowClass . '">' .
               '<td class="center">';
          if ($infectedIPs && array_search($flowID[0], $infectedIPs) !== false) {
            echo '<font color="#FF0000">' .
                   long2ip($flowID[0]) .
                 '</font>';
          }
          else {
            echo long2ip($flowID[0]);
          }
          echo '</td>' .
               '<td class="center">';
          if ($sourceCountryNumber != 0) {
            echo '<img src="' . '/images/flags/' . $countryCodeMap[$sourceCountryNumber] . '.png" ' .
                 'title="' . $countryNameMap[$sourceCountryNumber] . '" ' .
                 'alt="' . $countryNameMap[$sourceCountryNumber] . '">';
          }
          echo '</td>' .
               '<td class="center">' .
                 $flowID[1] .
               '</td>' .
               '<td class="center">' .
                 '<a class="text" href="/darkSpaceSources/' . $date . '/' . long2ip($flowID[0]) . '/' . $flowID[1] . '">' .
                   number_format($stats['numDarkTargets']) .
                 '</a>' .
               '</td>' .
             '</tr>';
          ++$rowNumber;
        }
      }
    }
    else {
      $port = $request[2];
      $title = 'Dark IPs targeted by ' . $ip . ' on port ' . $port;
      $logo = makeLogo($postgreSQL, $schemaName, $date, $title, '/darkSpaceSources', '/' . $ip . '/' . $port);
      
      class ScannedIPRange {
        var $protocol;
        var $sourcePort;
        var $firstIP;
        var $lastIP;
        var $destinationPort;
        var $startTime;
        var $endTime;
        var $numPackets;
        var $minPacketSize;
        var $maxPacketSize;
        var $numBytes;
        var $content;
        var $asNumber;
      }

      function makeIPPair(&$firstIP, &$lastIP) {
        if ($firstIP == $lastIP) {
          return long2ip($firstIP);
        }
        else {
          return long2ip($firstIP) . ' - ' . long2ip($lastIP);
        }
      }
      $scannedIPRange = new ScannedIPRange;
      $result = pg_query($postgreSQL, 'SELECT "protocol", "sourcePort", "destinationIP", "destinationPort", "numPackets", "minPacketSize", "maxPacketSize", ' .
                         '"numBytes", "content", "startTime", "endTime", "asNumber" FROM "' .
                         $schemaName . '"."' . $date . '"WHERE "sourceIP" = \'' . $numericIP .
                         '\' AND "destinationPort" = \'' . $port . '\'');
      $row = pg_fetch_assoc($result);
      $scannedIPRange -> protocol = $row['protocol'];
      $scannedIPRange -> sourcePort = $row['sourcePort'];
      $scannedIPRange -> firstIP = $row['destinationIP'];
      $scannedIPRange -> lastIP = $row['destinationIP'];
      $scannedIPRange -> startTime = $row['startTime'];
      $scannedIPRange -> endTime = $row['endTime'];
      $scannedIPRange -> numPackets = $row['numPackets'];
      $scannedIPRange -> minPacketSize = $row['minPacketSize'];
      $scannedIPRange -> maxPacketSize = $row['maxPacketSize'];
      $scannedIPRange -> numBytes = $row['numBytes'];
      $scannedIPRange -> content = explode(',', substr($row['content'], 1, -1));
      $scannedIPRange -> asn = $row['asNumber'];
      $previousDestinationIP = $scannedIPRange -> firstIP;
      while ($row = pg_fetch_assoc($result)) {
        if ($previousDestinationIP == $row['destinationIP'] - 1) {
          $scannedIPRange -> lastIP = $row['destinationIP'];
          if ($row['startTime'] < $scannedIPRange -> startTime) {
            $scannedIPRange -> startTime = $row['startTime'];
          }
          if ($row['endTime'] > $scannedIPRange -> endTime) {
            $scannedIPRange -> endTime = $row['endTime'];
          }
        }
        else {
          $scannedIPRanges[] = clone($scannedIPRange);
          $scannedIPRange = new ScannedIPRange;
          $scannedIPRange -> protocol = $row['protocol'];
          $scannedIPRange -> sourcePort = $row['sourcePort'];
          $scannedIPRange -> firstIP = $row['destinationIP'];
          $scannedIPRange -> lastIP = $row['destinationIP'];
          $scannedIPRange -> startTime = $row['startTime'];
          $scannedIPRange -> endTime = $row['endTime'];
          $scannedIPRange -> numPackets = $row['numPackets'];
          $scannedIPRange -> minPacketSize = $row['minPacketSize'];
          $scannedIPRange -> maxPacketSize = $row['maxPacketSize'];
          $scannedIPRange -> numBytes = $row['numBytes'];
          $scannedIPRange -> content = explode(',', substr($row['content'], 1, -1));
          $scannedIPRange -> asn = $row['asNumber'];
        }
        $previousDestinationIP = $row['destinationIP'];
      }
      $scannedIPRanges[] = clone($scannedIPRange);
      include('include/header.php');
      include('include/services.php');
      echo '<div class="table">' .
             '<table width="100%" cellspacing="1">' .
             '<tr>' .
               '<td class="columnTitle center">' .
                 'Protocol' .
               '</td>' .
               '<td class="columnTitle center">' .
                 'Source Port' .
               '</td>' .
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
      if ($scannedIPRanges[0] -> firstIP) {
        foreach ($scannedIPRanges as &$scannedIPRange) {
          $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
          echo '<tr class="' . $rowClass . '">' .
                 '<td class="center">';
          if ($scannedIPRange -> protocol) {
            echo $protocolNames[$scannedIPRange -> protocol];
          }
          echo '</td>' .
               '<td class="center">';
          if ($scannedIPRange -> sourcePort) {
            if ($scannedIPRange -> protocol == 6) {
              echo '<b>' .
                     $scannedIPRange -> sourcePort .
                   '</b>';
            }
            else {
              echo $scannedIPRange -> sourcePort;
            }
          }
          echo '</td>' .
               '<td class="center">' .
                 makeIPPair($scannedIPRange -> firstIP, $scannedIPRange -> lastIP) .
               '</td>' .
               '<td class="center">' .
                 $port .
               '</td>' .
               '<td class="center">' .
                 getServiceName($scannedIPRange -> protocol, 0, $port, INTERNAL_INITIATOR) .
               '</td>' .
               '<td class="center">' .
                 date('g:i:s A', $scannedIPRange -> startTime) .
               '</td>' .
               '<td class="center">' .
                 duration($scannedIPRange -> endTime - $scannedIPRange -> startTime) .
               '</td>' .
               '<td class="center">';
          if ($scannedIPRange -> numPackets) {
            echo number_format($scannedIPRange -> numPackets);
          }
          echo '</td>' .
               '<td class="center">';
          if ($scannedIPRange -> numBytes) {
            echo size($scannedIPRange -> numBytes);
          }
          echo '</td>' .
               '<td class="center">';
          if ($scannedIPRange -> minPacketSize) {
            echo size($scannedIPRange -> minPacketSize) . ' / ' . size($scannedIPRange -> maxPacketSize);
          }
          echo '</td>' .
               '<td class="center">' .
                 drawContentStripe($scannedIPRange -> content, $scannedIPRange -> numBytes) .
               '</td>' .
             '</tr>';
          ++$rowNumber;
        }
      }
    }
  }
?>
