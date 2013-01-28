<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  $title = 'Analytics Statistics';
  include('include/header.php');

  function getDataSize(&$postgreSQL, &$tableName) {
    $result = pg_query($postgreSQL, 'SELECT * FROM "DataSize"."' . $tableName . '"');
    while ($row = pg_fetch_row($result)) {
      $dataSize[] = size($row[1]) . ' of <i>' . $row[0] . '</i> data';
    }
    return implode(', ', $dataSize);
  }

  $days = getPGTableRange($postgreSQL, 'ProcessStats',
                          getFirstPGTable($postgreSQL, 'ProcessStats'),
                          getLastPGTable($postgreSQL, 'ProcessStats'), true);
  if (count($days) == 0) {
    include('include/footer.html');
    exit;
  }
  foreach ($days as &$tableName) {
    echo '<div class="table">' .
           '<table width="100%" cellspacing="1">' .
             '<tr>' .
               '<th class="tableName center" colspan="8">' .
                 $tableName . ' (' . getDataSize($postgreSQL, $tableName) . ' analyzed)' .
               '</th>' .
             '</tr>' .
             '<tr>' .
               '<td class="columnTitle center">' .
                 'Process Name' .
               '</td>' .
               '<td class="columnTitle center">' .
                 'Time Started' .
               '</td>' .
               '<td class="columnTitle center">' .
                 'Percent Complete' .
               '</td>' .
               '<td class="columnTitle center">' .
                 'Real Time Used' .
               '</td>' .
               '<td class="columnTitle center">' .
                 'CPU Time Used' .
               '</td>' .
               '<td class="columnTitle center">' .
                 'Resident Memory Size' .
               '</td>' .
               '<td class="columnTitle center">' .
                 'Total Memory Size' .
               '</td>' .
               '<td class="columnTitle center">' .
                 'SQL Rows Inserted' .
               '</td>' .
             '</tr>';
    $result2 = pg_query($postgreSQL, 'SELECT * FROM "ProcessStats"."' . $tableName . '"');
      $rowNumber = 0;
      while ($row2 = pg_fetch_assoc($result2)) {
        $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
        if ($row2['crashed'] == 't') {
          $rowStyle = '<tr style="background: #FF0000;">';
        }
        else {
          if ($row2['percentComplete'] == 100) {
            $rowStyle = '<tr class="' . $rowClass . '">';
          }
          else {
            $rowStyle = '<tr style="background: #FFFFFF;">';
          }
        }
        echo $rowStyle .
               '<td class="center">' .
                 $row2['processName'] .
               '</td>' .
               '<td class="center">' .
                  date("l, F j, Y \a\\t g:i:s A", $row2['startTime']) .
               '</td>' .
               '<td class="center">' .
                 $row2['percentComplete'] . '%' .
               '</td>' .
               '<td class="center">' .
                 duration($row2['endTime'] - $row2['startTime']) .
               '</td>' .
               '<td align="center">' .
                 duration($row2['cpuTime']) .
               '</td>' .
               '<td align="center">' .
                 size($row2['residentSize'] * 1024) .
               '</td>' .
               '<td align="center">' .
                 ($row2['totalSize'] ? size($row2['totalSize'] * 1024) : '-') .
               '</td>' .
               '<td align="center">' .
                 ($row2['insertedRows'] !== NULL ? number_format($row2['insertedRows']) : '-') .
               '</td>' .
             '</td>';
      ++$rowNumber;
    }
    echo '</table>' .
       '</div>';
  }
  include('include/footer.html');
?>
