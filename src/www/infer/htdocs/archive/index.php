<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/calendar.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  $title = 'Analytics Archive';
  include('include/header.php');
  $days = getPGTableRange($postgreSQL, 'InterestingIPs',
                          getFirstPGTable($postgreSQL, 'InterestingIPs'),
                          getLastPGTable($postgreSQL, 'InterestingIPs'));
  if (count($days) == 0) {
    include('include/footer.html');
    exit;
  }
  foreach ($days as &$day) {
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
      drawMonth($postgreSQL, $year, $month, 'InterestingIPs', 'existsPGTable', '../report');
      echo '</div>';
    }
  }
  include ('include/footer.html');
?>
