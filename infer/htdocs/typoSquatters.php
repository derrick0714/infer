<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  $request = explode('/', substr($_SERVER['PATH_INFO'], 1));

  $schemaName = 'TypoSquatters';

  if (!$request[0]) {
    $title = 'Typo Squatters';
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
        drawMonth($postgreSQL, $year, $month, $schemaName, 'existsPGTable', '/typoSquatters');
        echo '</div>';
      }
    }
  }
  else {
    $date = $request[0];
    $title = 'Typo Squatters for ' . $date;
    if (existsPGTable($postgreSQL, 'InfectedIPs', $date)) {
      getInfectedIPs($postgreSQL, $date, $infectedIPs);
    }
    if (!$request[1]) {
      include('include/header.php');
      echo '<div class="table">' .
             '<table width="100%" cellspacing="1">' .
               '<tr>' .
                 '<td class="columnTitle center">' .
                   'Squatter Domain' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Reputation' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Number of Victim Domains' .
                 '</td>' .
               '</tr>';
      $result = pg_query($postgreSQL, 'SELECT "squatter_domain", "squatter_reputation", COUNT(DISTINCT "victim_domain") FROM "' .
                                      $schemaName . '"."' . $date . '" GROUP BY "squatter_domain", "squatter_reputation" ' .
                                      ' ORDER BY COUNT(DISTINCT "victim_domain") DESC');
      while ($row = pg_fetch_assoc($result)) {
        $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
        echo '<tr class="' . $rowClass . '">' .
               '<td class="center">' .
                 '<a class="text" href="/typoSquatters/' . $date . '/' . $row['squatter_domain'] . '">' .
                   $row['squatter_domain'] .
                 '</a>' .
               '</td>' .
               '<td class="center">' .
                 $row['squatter_reputation'] .
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
      $squatterDomain = $request[1];
      $title .= ' (' . $request[1] . ')';
      include('include/header.php');
      $result = pg_query($postgreSQL, 'SELECT "squatter_ips" FROM "' . $schemaName . '"."' . $date .
                                      '" WHERE "squatter_domain" = \'' . $squatterDomain . '\' LIMIT 1');
      $row = pg_fetch_assoc($result);
      if ($row['squatter_ips'] != '{}') {
        echo '<div class="table">' .
               '<table width="100%" cellspacing="1">' .
                 '<th class="tableName" colspan="3">' .
                   'Squatter IPs' .
                 '</th>' .
                 '<tr>' .
                   '<td class="columnTitle center">' .
                     'IP' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Autonomous System' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Region' .
                   '</td>' .
                 '</tr>';
        foreach (explode(',', substr($row['squatter_ips'], 1, -1)) as $ip) {
          $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
          echo '<tr class="' . $rowClass . '">' .
                 '<td class="center">' .
                   long2ip($ip) .
                 '</td>' .
                 '<td class="center">' .
                   getASDescriptionByNumber($postgreSQL, getASNByIP($postgreSQL, $ip)) .
                 '</td>' .
                 '<td class="center">' .
                   getCountryPicture(getCountryNumberByIP($postgreSQL, $ip), $countryCodeMap, $countryNameMap) .
                 '</td>' .
               '</tr>';
          ++$rowNumber;
        }
        echo '</table>' .
           '</div>';
      }
      $result = pg_query($postgreSQL, 'SELECT * FROM "' . $schemaName . '"."' . $date .
                                      '" WHERE "squatter_domain" = \'' . $squatterDomain . '\'');
      while ($row = pg_fetch_assoc($result)) {
        echo '<div class="table">' .
               '<table width="100%" cellspacing="1">' .
                 '<th class="tableName" colspan="3">' .
                   'Victim (' . $row['victim_domain'] . ', Reputation: ' . $row['victim_reputation'] . ') IPs' .
                 '</th>' .
                 '<tr>' .
                   '<td class="columnTitle center">' .
                     'IP' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Autonomous System' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Region' .
                   '</td>' .
                 '</tr>';
        $rowNumber = 0;
        foreach (explode(',', substr($row['victim_ips'], 1, -1)) as $ip) {
          $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
          echo '<tr class="' . $rowClass . '">' .
                 '<td class="center">' .
                   long2ip($ip) .
                 '</td>' .
                 '<td class="center">' .
                   getASDescriptionByNumber($postgreSQL, getASNByIP($postgreSQL, $ip)) .
                 '</td>' .
                 '<td class="center">' .
                   getCountryPicture(getCountryNumberByIP($postgreSQL, $ip), $countryCodeMap, $countryNameMap) .
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
