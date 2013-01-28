#!/usr/local/bin/php
<?php
  $imsHome = dirname(__FILE__) . '/..';

  include($imsHome . '/htdocs/include/postgreSQL.php');
  include('include/shared.php');
  include('/usr/local/share/sparkline/lib/Sparkline_Bar.php');

  if (!file_exists($imsHome . '/htdocs/images/sparklines/virulence/' . $argv[1])) {
    mkdir($imsHome . '/htdocs/images/sparklines/virulence/' . $argv[1]);
  }
  if (!file_exists($imsHome . '/htdocs/images/sparklines/rank/' . $argv[1])) {
    mkdir($imsHome . '/htdocs/images/sparklines/rank/' . $argv[1]);
  }

  function getNumInterestingIPs(&$postgreSQL, &$date) {
    $result = pg_query($postgreSQL, 'SELECT COUNT(*) FROM "InterestingIPs"."' .
                                    $date . '"');
    $row = pg_fetch_row($result);
    return $row[0];
  }

  $dates = array_slice(getPGTableRange($postgreSQL, 'InterestingIPs', getFirstPGTable($postgreSQL, 'InterestingIPs'), $argv[1]), -14);
  foreach ($dates as &$date) {
    $numInterestingIPs[$date] = getNumInterestingIPs($postgreSQL, $date);
  }
  foreach (getInterestingIPs($postgreSQL, $argv[1]) as $ip) {
    echo long2ip($ip) . "\n";
    drawVirulenceSparkline($postgreSQL, $imsHome, $dates, $numInterestingIPs, $ip);
  }

  function drawVirulenceSparkline(&$postgreSQL, &$imsHome, &$dates, &$numInterestingIPs, &$ip) {
    $minVirulence['value'] = 1;
    $maxVirulence['value'] = 0;
    $minRank['value'] = 4294967295;
    $maxRank['value'] = 0;
    foreach ($dates as &$date) {
      $result = pg_query($postgreSQL, 'SELECT "currentVirulence", "rank" FROM "InterestingIPs"."' . $date . '" ' .
                                      'WHERE "ip" = \'' . $ip . '\'');
      if (pg_num_rows($result) > 0) {
        $row = pg_fetch_assoc($result);
        $virulenceData[] = $row['currentVirulence'];
        if ($row['currentVirulence'] < $minVirulence['value']) {
          $minVirulence['value'] = $row['currentVirulence'];
          $minVirulence['position'] = count($virulenceData) - 1;
        }
        if ($row['currentVirulence'] > $maxVirulence['value']) {
          $maxVirulence['value'] = $row['currentVirulence'];
          $maxVirulence['position'] = count($virulenceData) - 1;
        }
        $rank = ($numInterestingIPs[$date] - ($row['rank'] - 1)) / $numInterestingIPs[$date];
        $rankData[] = $rank;
        if ($rank < $minRank['value']) {
          $minRank['value'] = $rank;
          $minRank['position'] = count($rankData) - 1;
        }
        if ($rank > $maxRank['value']) {
          $maxRank['value'] = $rank;
          $maxRank['position'] = count($rankData) - 1;
        }
      }
      else {
        $virulenceData[] = 0;
        $minVirulence['value'] = 0;
        $minVirulence['position'] = count($virulenceData) - 1;
        $rankData[] = 0;
        $minRank['value'] = 0;
        $minRank['position'] = count($rankData) - 1;
      }
    }
    $virulenceGraph = new Sparkline_Bar();
    $virulenceGraph -> setBarWidth(3);
    $virulenceGraph -> setBarSpacing(1);
    $virulenceGraph -> setColorHtml('red', '#FF0000');
    $virulenceGraph -> setColorHtml('green', '#00FF00');
    $virulenceGraph -> setColorHtml('blue', '#0000FF');
    $virulenceGraph -> setColorHtml('purple', '#660000');
    foreach ($virulenceData as $key => &$value) {
      if ($key == $minVirulence['position']) {
        $virulenceGraph -> setData($key, $value, 'green');
      }
      else {
        if ($key == $maxVirulence['position']) {
          $virulenceGraph -> setData($key, $value, 'red');
        }
        else {
          if ($key == count($dates) - 1) {
            $virulenceGraph -> setData($key, $value, 'blue');
          }
          else {
            $virulenceGraph -> setData($key, $value, 'purple');
          }
        }
      }
    }
    $virulenceGraph -> render(10);
    $virulenceGraph -> output($imsHome . '/htdocs/images/sparklines/virulence/' . $dates[count($dates) - 1] . '/' . long2ip($ip) . '.png');

    $rankGraph = new Sparkline_Bar();
    $rankGraph -> setBarWidth(3);
    $rankGraph -> setBarSpacing(1);
    $rankGraph -> setColorHtml('red', '#FF0000');
    $rankGraph -> setColorHtml('green', '#00FF00');
    $rankGraph -> setColorHtml('blue', '#0000FF');
    $rankGraph -> setColorHtml('purple', '#660000');
    foreach ($rankData as $key => &$value) {
      if ($key == $minRank['position']) {
        $rankGraph -> setData($key, $value, 'green');
      }
      else {
        if ($key == $maxRank['position']) {
          $rankGraph -> setData($key, $value, 'red');
        }
        else {
          if ($key == count($dates) - 1) {
            $rankGraph -> setData($key, $value, 'blue');
          }
          else {
            $rankGraph -> setData($key, $value, 'purple');
          }
        }
      }
    }
    $rankGraph -> render(10);
    $rankGraph -> output($imsHome . '/htdocs/images/sparklines/rank/' . $dates[count($dates) - 1] . '/' . long2ip($ip) . '.png');
  }
?>
