<?php
function day_of_Week($month, $year) {
  $firstofthemonth = strtotime($month . '/01/' . $year);
  $firstofthemonthArray = getdate($firstofthemonth);
  $startday = $firstofthemonthArray['wday'];
  return $startday;
}

function days_in_month($month, $year) {
  for ($i = 31; $i > 0; $i--) {
    if (checkdate($month, $i, $year)) {
      return $i;
    }
  }
  return 0;
}

function drawMonth(&$postgreSQL, $year, $month, $schemaName, $interesting_function, $documentName) {
  $months = array('01' => 'January',
                  '02' => 'February',
                  '03' => 'March',
                  '04' => 'April',
                  '05' => 'May',
                  '06' => 'June',
                  '07' => 'July',
                  '08' => 'August',
                  '09' => 'September',
                  '10' => 'October',
                  '11' => 'November',
                  '12' => 'December');
  $DateArray = getdate(strtotime($month . '/01/' . $year));
  echo '<table cellspacing="1" cellpadding="2">' .
         '<tr>' .
           '<th class="tableName" colspan="7">' .
             $months[$month] .
           '</th>' .
         '</tr>' .
       '<tr>' .
         '<td class="columnTitle center">Su</td>' .
         '<td class="columnTitle center">M</td>' .
         '<td class="columnTitle center">T</td>' .
         '<td class="columnTitle center">W</td>' .
         '<td class="columnTitle center">Th</td>' .
         '<td class="columnTitle center">F</td>' .
         '<td class="columnTitle center">Sa</td>' .
       '</tr>';
  $dayofweek = day_of_week($month, $year);
  $daysinmonth = days_in_month($month, $year);
  $lastcell = (ceil(($daysinmonth + $dayofweek) / 7 ) * 7);
  for($i = 0; $i < $lastcell; $i = $i + 1) {
    if ($i % 7 == 0) {
      echo '<tr>';
    }
    if($i < $dayofweek || $i > $daysinmonth + $dayofweek -1) {
      echo '<td class="even"></td>';
    }
    else {
      $date = $i - $dayofweek + 1;
      $paddedDay = str_pad($date, 2, '0', STR_PAD_LEFT);
      $tableName = $year . '-' . $month . '-' . $paddedDay;
      if ($interesting_function($postgreSQL, $schemaName, $tableName)) {
        $calendar_day = '<a class="text" href="' . $documentName . '/' . $year . '-' . $month . '-' .  $paddedDay . '">' . $date . '</a>';
      }
      else {
        $calendar_day = $date;
      }
      echo '<td class="even" align="right">' . $calendar_day . '</td>';
    }
    if ((($i + 1) % 7) == 0)
      echo '</tr>';
  }
  echo '</table>';
}
