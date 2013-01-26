<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  $request = explode('/', substr($_SERVER['PATH_INFO'], 1));

  $schemaName = 'DarkSpaceTargets';

  if (!$request[0]) {
    include('include/calendar.php');
    $title = 'Dark Space Targets';
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
        drawMonth($postgreSQL, $year, $month, $schemaName, 'existsPGTable', '/darkSpaceTargets');
        echo '</div>';
      }
    }
  }
  else {
    $date = $request[0];
    $title = 'Dark Space Targets for ' . $date;
    $logo = '<table width="100%">' .
              '<tr>' .
                '<td class="sub_title" width="15%" align="left">' .
                  getPreviousPGDay($postgreSQL, $schemaName, $date, '/darkSpaceTargets') .
               '</td>' .
              '<td width="60%" align="center">' .
                $title .
              '</td>' .
              '<td class="sub_title" width="15%" align="right">' .
                getNextPGDay($postgreSQL, $schemaName, $date, '/darkSpaceTargets') .
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
      $logo = makeLogo($postgreSQL, $schemaName, $date, $title, '/darkSpaceTargets', '/' . $ip);
      //$logo = $title;
      $extra = ' WHERE "internalIP" = \'' . sprintf('%u', ip2long($request[1])) . '\'';
    }
    if (!$request[2]) {
      $result = pg_query($postgreSQL, 'SELECT "internalIP", "internalPort", COUNT(DISTINCT "externalIP") FROM "' . $schemaName . '"."' . $date .
                                      '" ' . $extra . ' GROUP BY "internalIP", "internalPort" ORDER BY COUNT(DISTINCT "externalIP") DESC LIMIT 100');
      while ($row = pg_fetch_row($result)) {
        $darkSpaceTargets[$row[0] . ' ' . $row[1]]['numDarkSources'] = $row[2];
      }
      include('include/header.php');
      echo '<div class="table">' .
             '<table width="100%" cellspacing="1">' .
               '<tr>' .
                 '<td class="columnTitle center">' .
                   'Internal IP' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Internal Port' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Dark Space Sources' .
                 '</td>' .
              '</tr>';
      if ($darkSpaceTargets) {
        foreach ($darkSpaceTargets as $flowID => &$stats) {
          $flowID = explode(' ', $flowID);
          $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
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
               '<td class="center">' .
                 $flowID[1] .
               '</td>' .
               '<td class="center">' .
                 '<a class="text" href="/darkSpaceTargets/' . $date . '/' . long2ip($flowID[0]) . '/' . $flowID[1] . '">' .
                   number_format($stats['numDarkSources']) .
                 '</a>' .
               '</td>' .
             '</tr>';
          ++$rowNumber;
        }
      }
    }
    else {
      $port = $request[2];
      $title = 'Dark space sources targeting ' . $ip . ' on port ' . $port;
      $logo = makeLogo($postgreSQL, $schemaName, $date, $title, '/darkSpaceTargets', '/' . $ip . '/' . $port);
      getCountryNameMap($postgreSQL, $countryCodeMap, $countryNameMap);
      $result = pg_query($postgreSQL, 'SELECT "protocol", "externalIP", "externalPort", "numPackets", ' .
                         '"numBytes", "minPacketSize", "maxPacketSize", "content", "startTime", "endTime", "asNumber" FROM "' .
                         $schemaName . '"."' . $date . '"WHERE "internalIP" = \'' . $numericIP .
                        '\' AND "internalPort" = \'' . $port . '\'');
      while ($row = pg_fetch_assoc($result)) {
        $darkSpaceSources[$row['externalIP']]['protocol'] = $row['protocol'];
        $darkSpaceSources[$row['externalIP']]['externalPort'] = $row['externalPort'];
        $darkSpaceSources[$row['externalIP']]['numPackets'] = $row['numPackets'];
        $darkSpaceSources[$row['externalIP']]['numBytes'] = $row['numBytes'];
        $darkSpaceSources[$row['externalIP']]['minPacketSize'] = $row['minPacketSize'];
        $darkSpaceSources[$row['externalIP']]['maxPacketSize'] = $row['maxPacketSize'];
        $darkSpaceSources[$row['externalIP']]['content'] = $row['content'];
        $darkSpaceSources[$row['externalIP']]['asNumber'] = $row['asNumber'];
        if (!$darkSpaceSources[$row['internalIP']]['startTime'] ||
            $row['startTime'] < $darkSpaceSources[$row['externalIP']]['startTime']) {
          $darkSpaceSources[$row['externalIP']]['startTime'] = $row['startTime'];
        }
        if ($row['endTime'] > $darkSpaceSources[$row['externalIP']]['endTime']) {
          $darkSpaceSources[$row['externalIP']]['endTime'] = $row['endTime'];
        }
      }
      include('include/header.php');
      include('include/services.php');
      echo '<div class="table">' .
             '<table width="100%" cellspacing="1">' .
               '<tr>' .
                 '<td class="columnTitle center">' .
                   'Protocol' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'External IP' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'External Port' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Internal Port' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Service' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Country' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Start Time' .
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
      if ($darkSpaceSources) {
        foreach ($darkSpaceSources as $sourceIP => &$stats) {
          $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
          if (!isInternal($sourceIP)) {
            $sourceCountryNumber = getCountryNumberByIP($postgreSQL, $sourceIP);
          }
          echo '<tr class="' . $rowClass . '">' .
                 '<td class="center">' .
                   $protocolNames[$stats['protocol']] .
                 '</td>' .
                 '<td class="center">';
          if ($infectedIPs && array_search($sourceIP, $infectedIPs) !== false) {
            echo '<font color="#FF0000">' .
                   long2ip($sourceIP) .
                 '</font>';
          }
          else {
            echo long2ip($sourceIP);
          }
          echo '</td>' .
               '<td class="center">';
          if ($stats['externalPort']) {
            echo styleExternalPort($stats['externalPort'], $stats['initiator']);
          }
          echo '</td>' .
               '<td class="center">' .
                 styleInternalPort($port, $stats['initiator']) .
               '</td>' .
               '<td class="center">' .
                 getServiceName($stats['protocol'], $port, 0, EXTERNAL_INITIATOR) .
               '</td>' .
               '<td class="center">';
          if ($sourceCountryNumber != 0) {
            echo '<img src="' . '/images/flags/' . $countryCodeMap[$sourceCountryNumber] . '.png" ' .
                 'title="' . $countryNameMap[$sourceCountryNumber] . '" ' .
                 'alt="' . $countryNameMap[$sourceCountryNumber] . '">';
          }
          echo '</td>' .
               '<td class="center">' .
                 date('g:i:s A', $stats['startTime']) .
               '</td>' .
               '<td class="center">' .
                 duration($stats['endTime'] - $stats['startTime']) .
               '</td>' .
               '<td class="center">';
          if ($stats['numPackets']) {
            echo number_format($stats['numPackets']);
          }
          echo '</td>' .
               '<td class="center">';
          if ($stats['numBytes']) {
            echo size($stats['numBytes']);
          }
          echo '</td>' .
               '<td class="center">';
          if ($stats['minPacketSize']) {
            echo size($stats['minPacketSize']) . ' / ' . size($stats['maxPacketSize']);
          }
          echo '</td>' .
               '<td class="center">' .
                 drawContentStripe($stats['content'], $stats['numBytes']) .
               '</td>' .
             '</tr>';
          ++$rowNumber;
        }
        echo '</table>';
      }
    }
  }
?>
