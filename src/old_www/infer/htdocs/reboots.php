<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  $request = explode('/', substr($_SERVER['PATH_INFO'], 1));

  $schemaName = 'Reboots';

  if (!$request[0]) {
    $title = 'Reboots';
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
        drawMonth($postgreSQL, $year, $month, $schemaName, 'existsPGTable', '/reboots');
        echo '</div>';
      }
    }
  }
  else {
    $date = $request[0];
    $title = 'Reboots for ' . $date;
    $logo = '<table width="100%">' .
              '<tr>' .
                '<td class="sub_title" width="15%" align="left">' .
                  getPreviousPGDay($postgreSQL, $schemaName, $date, '/reboots') .
               '</td>' .
              '<td width="60%" align="center">' .
                $title .
              '</td>' .
              '<td class="sub_title" width="15%" align="right">' .
                getNextPGDay($postgreSQL, $schemaName, $date, '/reboots') .
              '</td>' .
            '</tr>' .
          '</table>';
    if (existsPGTable($postgreSQL, 'InfectedIPs', $date)) {
      getInfectedIPs($postgreSQL, $date, $infectedIPs);
    }
    if (!$request[1]) {
      $logo = makeLogo($postgreSQL, $schemaName, $date, $title, '/reboots', '/' . $internalIP);
      include('include/header.php');
      echo '<div class="table">' .
             '<table width="100%" cellspacing="1">' .
               '<tr>' .
                 '<td class="columnTitle center">' .
                   'IP' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Number of Reboots' .
                 '</td>' .
               '</tr>';
      $result = pg_query($postgreSQL, 'SELECT "ip", COUNT(*) FROM "' . $schemaName . '"."' . $date .
                                      '" GROUP BY "ip" ORDER BY COUNT(*) DESC');
      while ($row = pg_fetch_assoc($result)) {
        $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
        echo '<tr class="' . $rowClass . '">' .
               '<td class="center">' .
                 '<a class="text" href="/reboots/' . $date . '/' . long2ip($row['ip']) . '">' .
                   long2ip($row['ip']) .
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
      $title .= ' (' . $request[1] . ')';
      $logo = makeLogo($postgreSQL, $schemaName, $date, $title, '/reboots', '/' . $internalIP);
      include('include/header.php');
      $result = pg_query($postgreSQL, 'SELECT "applicationNames", "applicationTimes" FROM "' . $schemaName . '"."' . $date .
                                      '" WHERE "ip" = \'' . $numericInternalIP . '\'');
      while ($row = pg_fetch_assoc($result)) {
        echo '<div class="table">' .
               '<table width="100%" cellspacing="1">' .
                 '<tr>' .
                   '<td class="columnTitle center">' .
                     'Application' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Time of Activity' .
                   '</td>'  .
                 '</tr>';
        $applicationNames = explode(',', substr($row['applicationNames'], 1, -1));
        $applicationTimes = explode(',', substr($row['applicationTimes'], 1, -1));
        for ($index = 0; $index < count($applicationNames); $index++) {
          $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
          echo '<tr class="' . $rowClass . '">' .
                 '<td class="center">' .
                   $applicationNames[$index] .
                 '</td>' .
                 '<td class="center">' .
                   date("l, F j, Y \a\\t g:i:s A", $applicationTimes[$index]) .
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
