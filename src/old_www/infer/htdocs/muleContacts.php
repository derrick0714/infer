<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  $request = explode('/', substr($_SERVER['PATH_INFO'], 1));

  $schemaName = 'MuleContacts';

  if (!$request[0]) {
    include('include/calendar.php');
    $title = 'Contact with Mules';
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
        drawMonth($postgreSQL, $year, $month, $schemaName, 'existsPGTable', '/muleContacts');
        echo '</div>';
      }
    }
  }
  else {
    $date = $request[0];
    $title = 'Contact with mules for ' . $date;
    $logo = '<table width="100%">' .
              '<tr>' .
                '<td class="sub_title" width="15%" align="left">' .
                  getPreviousPGDay($postgreSQL, $schemaName, $date, '/muleContacts') .
               '</td>' .
              '<td width="60%" align="center">' .
                $title .
              '</td>' .
              '<td class="sub_title" width="15%" align="right">' .
                getNextPGDay($postgreSQL, $schemaName, $date, '/muleContacts') .
              '</td>' .
            '</tr>' .
          '</table>';
    if (existsPGTable($postgreSQL, 'InfectedIPs', $date)) {
      getInfectedIPs($postgreSQL, $date, $infectedIPs);
    }
    if ($request[1]) {
      $title .= ' (' . $request[1] . ')';
      $logo = makeLogo($postgreSQL, $schemaName, $date, $title, '/muleContacts', '/' . $request[1]);
      $extra = ' WHERE "ip" = \'' . sprintf('%u', ip2long($request[1])) . '\'';
    }

    class SteppingStoneStats {
      var $sourceIP;
      var $sourceSourcePort;
      var $sourceDestinationPort;
      var $initiator;
      var $destinationIP;
      var $destinationDestinationPort;
      var $numBytes;
      var $startTime;
      var $endTime;
      var $sourceCountryNumber;
      var $destinationCountryNumber;
    }

    getCountryNameMap($postgreSQL, $countryCodeMap, $countryNameMap);

    $result = @pg_query($postgreSQL, 'SELECT "ip", "sourceIP", "sourceSourcePort", "sourceDestinationPort", ' .
                                    '"initiator", "destinationIP", "destinationDestinationPort", "numBytes", ' .
                                    '"startTime", "endTime", "sourceCountryNumber", ' .
                                    '"destinationCountryNumber" FROM "' . $schemaName . '"."'
                                    . $date . '"' . $extra . ' ORDER BY "startTime"');
    while ($row = @pg_fetch_assoc($result)) {
      $steppingStoneStats = new SteppingStoneStats();
      $steppingStoneStats -> sourceIP = $row['sourceIP'];
      $steppingStoneStats -> sourceSourcePort = $row['sourceSourcePort'];
      $steppingStoneStats -> sourceDestinationPort = $row['sourceDestinationPort'];
      $steppingStoneStats -> initiator = $row['initiator'];
      $steppingStoneStats -> destinationIP = $row['destinationIP'];
      $steppingStoneStats -> destinationDestinationPort = $row['destinationDestinationPort'];
      $steppingStoneStats -> numBytes = $row['numBytes'];
      $steppingStoneStats -> startTime = $row['startTime'];
      $steppingStoneStats -> endTime = $row['endTime'];
      $steppingStoneStats -> sourceCountryNumber = $row['sourceCountryNumber'];
      $steppingStoneStats -> destinationCountryNumber = $row['destinationCountryNumber'];
      $steppingStones[$row['ip']][] = clone($steppingStoneStats);
    }

    function bold(&$string, $condition) {
      if ($condition) {
        return '<b>' .
                 $string .
               '</b>';
      }
      return $string;
    }

    include('include/header.php');
    if ($steppingStones) {
      foreach ($steppingStones as $steppingStone => &$steppingStoneChannels) {
        echo '<div class="table">' .
               '<table width="100%" cellspacing="1">' .
                 '<tr>' .
                   '<th class="columnTitle center" colspan="8">' .
                     long2ip($steppingStone) .
                   '</th>' .
                 '</tr>' .
                 '<tr>' .
                   '<td class="columnTitle center">' .
                     'External IP / Port / Internal Port' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Source Country' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Destination IP / Port' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Destination Country' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Bytes Transferred' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Start Time' .
                  '<td class="columnTitle center">' .
                     'Duration' .
                   '</td>' .
                 '</tr>';
        foreach ($steppingStoneChannels as &$steppingStoneChannel) {
          $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
          echo '<tr class="' . $rowClass . '">' .
                 '<td class="center">' .
                   styleIP($infectedIPs, $steppingStoneChannel -> sourceIP) . ':' .
                   bold($steppingStoneChannel -> sourceDestinationPort, $steppingStoneChannel -> initiator == 2) . ' <-> ' .
                   bold($steppingStoneChannel -> sourceSourcePort, $steppingStoneChannel -> initiator == 1) .
               '</td>' .
               '<td class="center">' .
                 getCountryPicture($steppingStoneChannel -> sourceCountryNumber,
                                   $countryCodeMap, $countryNameMap) .
               '</td>' .
               '<td class="center">' .
                 styleIP($infectedIPs, $steppingStoneChannel -> destinationIP) . ':' .
                 $steppingStoneChannel -> destinationDestinationPort .
               '</td>' .
               '<td class="center">' .
                 getCountryPicture($steppingStoneChannel -> destinationCountryNumber,
                                   $countryCodeMap, $countryNameMap) .
               '</td>' .
               '<td class="center">' .
                 size($steppingStoneChannel -> numBytes) .
               '</td>' .
               '<td class="center">' .
                 date('H:i:s', $steppingStoneChannel -> startTime) .
               '</td>' .
               '<td class="center">' .
                 duration($steppingStoneChannel -> endTime - $steppingStoneChannel -> startTime) .
               '</td>' .
             '</tr>';
          ++$rowNumber;
        }
        echo '</table>' .
           '</div>';
      }
    }
  }
?>
  </body>
</html>
