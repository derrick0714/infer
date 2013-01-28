<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  $request = explode('/', substr($_SERVER['PATH_INFO'], 1));

  $schemaName = 'InfectedContacts';

  if (!$request[0]) {
    $title = 'Contact with Infected Hosts';
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
        drawMonth($postgreSQL, $year, $month, $schemaName, 'existsPGTable', '/infectedContacts');
        echo '</div>';
      }
    }
  }
  else {
    $date = $request[0];
    $title = 'Contact with infected hosts for ' . $date;
    $logo = '<table width="100%">' .
              '<tr>' .
                '<td class="sub_title" width="15%" align="left">' .
                  getPreviousPGDay($postgreSQL, $schemaName, $date, '/infectedContacts') .
               '</td>' .
              '<td width="60%" align="center">' .
                $title .
              '</td>' .
              '<td class="sub_title" width="15%" align="right">' .
                getNextPGDay($postgreSQL, $schemaName, $date, '/infectedContacts') .
              '</td>' .
            '</tr>' .
          '</table>';
    if ($request[1]) {
      $internalIP = $request[1];         
      $numericInternalIP = sprintf('%u', ip2long($internalIP));
      $title .= ' (' . $request[1] . ')';
      $logo = makeLogo($postgreSQL, $schemaName, $date, $title, '/infectedContacts', '/' . $internalIP);
    }
    else {
      exit;
    }

    getCountryNameMap($postgreSQL, $countryCodeMap, $countryNameMap);

    if (!$request[2]) {
      $sourceNames = array(0 => 'Master IMS infected IP list',
                           1 => 'Local IMS infected IP list',
                           2 => 'DShield',
                           3 => 'infiltrated.net blacklist',
                           4 => 'Bleeding-edge Threats Botnet Command and Control list',
                           5 => 'University of Waterloo Information Systems & Technology Security Trends',
                           6 => 'Bleeding-edge Threats known compromised host list',
                           7 => 'Malware Domain List',
                           8 => 'Emerging Threats Botnet Command and Control list',
                           9 => 'Emerging Threats known compromised host list');
      $sourceURLs = array(0 => '.',
                          1 => '/infectedIPs',
                          2 => 'http://www.dshield.org/ipsascii.html?limit=256',
                          3 => 'http://www.infiltrated.net/blacklisted',
                          4 => 'http://www.bleedingthreats.net/bleeding-botcc.rules',
                          5 => 'http://ist.uwaterloo.ca/security/trends/Blacklist-28.txt',
                          6 => 'http://www.bleedingthreats.net/rules/bleeding-compromised.rules',
                          7 => 'http://www.malwaredomainlist.com/mdl.php?search=&colsearch=All&quantity=All',
                          8 => 'http://www.emergingthreats.net/rules/bleeding-botcc.rules',
                          9 => 'http://www.emergingthreats.net/rules/bleeding-compromised.rules');

      function labelSourceNumbers(&$sourceNames, &$sourceURLs, $sourceNumbers, $numericIP) {
        $sourceLabel = array();
        foreach (explode(',', substr($sourceNumbers, 1, -1)) as $sourceNumber) {
          if (array_key_exists($sourceNumber, $sourceNames)) {
            if ($sourceNumber == 1) {
              $sourceLabel[] = '<a class="text" href="' . $sourceURLs[$sourceNumber] . '#' . long2ip($numericIP) . '">' .
                                 $sourceNames[$sourceNumber] .
                               '</a>';
            }
            else {
              $sourceLabel[] = '<a class="text" href="' . $sourceURLs[$sourceNumber] . '">' .
                                 $sourceNames[$sourceNumber] .
                               '</a>';
            }
          }
        }
        return implode(', ', $sourceLabel);
      }

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
                   'Listed as Infected at' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Number of Occurences' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Number of Bytes' .
                 '</td>' .
               '</tr>';
      $result = pg_query($postgreSQL, 'SELECT "externalIP", SUM("numBytes"), "sourceNumbers", "asNumber", "countryNumber", COUNT(*) ' .
                                      'FROM "' . $schemaName . '"."' . $date . '" WHERE "internalIP" = \'' . $numericInternalIP . '\' ' .
                                      'GROUP BY "externalIP", "sourceNumbers", "asNumber", "countryNumber" ORDER BY SUM("numBytes") DESC');
      while ($row = pg_fetch_row($result)) {
        $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
        echo '<tr class="' . $rowClass . '">' .
               '<td class="center">' .
                 '<a class="text" href="/infectedContacts/' . $date . '/' . $internalIP . '/' . long2ip($row[0]) . '">' .
                   '<font color="#FF0000">' .
                     long2ip($row[0]) .
                   '</font>' .
                 '</a>' .
               '</td>' .
               '<td class="center">' .
                 getASDescriptionByNumber($postgreSQL, $row[3]) .
               '</td>' .
               '<td class="center">' .
                 getCountryPicture($row[4], $countryCodeMap, $countryNameMap) .
               '</td>' .
               '<td class="center">' .
                 labelSourceNumbers($sourceNames, $sourceURLs, $row[2], $row[0]) .
               '</td>' .
               '<td class="center">' .
                 number_format($row[5]) .
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
      $title = 'Contact with Infected Hosts for ' . $date . ' (' . $internalIP . '/' . $externalIP . ')';
      $logo = makeLogo($postgreSQL, $schemaName, $date,
                       'Contact with Infected Hosts for ' . $date . ' (' . $internalIP . '/<font color="#FF0000">' . $externalIP . '</font>)',
                       '/infectedContacts', '/' . $internalIP . '/' . $externalIP);
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
      $result = pg_query($postgreSQL, 'SELECT "protocol", "internalPort", "externalPort", "initiator", "numPackets", ' .
                                      '"minPacketSize", "maxPacketSize", "numBytes", "startTime", "endTime", "content" ' .
                                      ' FROM "' . $schemaName . '"."' . $date . '" WHERE "internalIP" = \'' . $numericInternalIP .
                                      '\' AND "externalIP" = \'' . $numericExternalIP . '\' ORDER BY "numBytes" DESC');
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
                 $row['numPackets'] .
               '</td>' .
               '<td class="center">' .
                 size($row['numBytes']) .
               '</td>' .
               '<td class="center">';
        if ($row['minPacketSize']) {
          echo size($row['minPacketSize']) . ' / ' . size($row['maxPacketSize']);
        }
        echo '</td>' .
             '<td class="ceter">' .
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
