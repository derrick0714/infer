#!/usr/local/bin/php
<?php
  $imsHome = dirname(__FILE__) . '/..';
  $schemaName = 'Slowdown';
  include($imsHome . '/htdocs/include/postgreSQL.php');
  include('include/shared.php');
  include('/usr/local/share/jpgraph/jpgraph.php');
  include('/usr/local/share/jpgraph/jpgraph_line.php');

  $periodSize = 300;

  if (!file_exists($imsHome . '/htdocs/images/graphs/slowdown/' . $argv[1])) {
    mkdir($imsHome . '/htdocs/images/graphs/slowdown/' . $argv[1]);
  }

  $result = pg_query('SELECT * FROM "' . $schemaName . '"."' . $argv[1] . '"');
  while($row = pg_fetch_assoc($result)) {
    drawSlowdownGraph($argv[1], $row, $periodSize, $imsHome);
    echo long2ip($row['internalIP']) . "\n";
  }

  // The callback that converts timestamps to hours, minutes, and seconds
  function TimeCallback($aVal) {
    return date('g:i:s A', $aVal);
  }

  function drawSlowdownGraph(&$date, &$row, &$periodSize, &$imsHome) {
    $startTime = $row['firstBinTime'];
    $endTime = $row['lastBinTime'];
    $dayStartTime = $row['dayStartTime'];
    $alertStartTime = $row['alertStartTime'];
    $alertEndTime = $row['alertEndTime'];
    foreach (explode(',', substr($row['delta'], 1, -1)) as $del) {
      $delta[] = $del;
    }
    foreach (explode(',', substr($row['forecast'], 1, -1)) as $fore) {
      $forecast[] = $fore;
    }
    for ($index = 0; $index < count($delta); ++$index) {
      $datax[] = $startTime;
      $startTime += $periodSize;
    }
    if ($alertStartTime && $alertEndTime) {
      $anomalyCount = ($alertEndTime - $alertStartTime) / $periodSize;
      if ($alertStartTime == $dayStartTime) {
        $alertStartTime += $periodSize;
      }
      $anomalyStartDistance = ($alertStartTime - $dayStartTime) / $periodSize;
      $anomalyEndDistance = ($alertEndTime - $dayStartTime) / $periodSize;
      for ($index = 0; $index < $anomalyCount; ++$index) {
        $anomalyx[] = $alertStartTime;
        $alertStartTime += $periodSize;
      }
      for ($index = $anomalyStartDistance; $index < $anomalyEndDistance; ++$index) {
        $anomalyy[] = $delta[$index];
      }
    }
    // Set up the basic graph
    $graph = new Graph(600, 350, 'auto');
    $graph -> SetMargin(90, 7, 7, 70);
    // $graph->SetAlphaBlending();
    $graph -> SetScale('intlin', 0, 0, $dayStartTime, $endTime);
    $graph -> SetColor('#f0f0f0');
    $graph -> setFrame(true, '#bbaabb');
    $graph -> setMarginColor('#bbaabb');
    $graph -> xaxis -> SetColor('#660000', '#660000');
    $graph -> yaxis -> SetColor('#660000', '#660000');
    // Set up X-axis
    $graph -> xaxis -> SetFont(FF_ARIAL, FS_BOLD, 8);
    $graph -> xaxis -> SetLabelAngle(45);
    $graph -> xaxis -> SetPos('min');
    $graph -> xaxis -> SetLabelMargin(5);
    $graph -> xaxis -> SetLabelFormatCallback('TimeCallback');
    // Set up the Y-axis
    $graph -> yaxis -> SetFont(FF_ARIAL, FS_BOLD, 8);
    $graph -> yaxis -> scale -> SetGrace(20);
    $graph -> yaxis -> SetLabelFormat('%lf');
    $graph -> yaxis -> title -> SetFont(FF_ARIAL, FS_BOLD, 8);
    $graph -> yaxis -> title -> SetColor('#660000');
    $graph -> yaxis -> title -> SetMargin(40);
    $graph -> yaxis -> title -> Set('Response Times (Seconds)');
    $graph -> yaxis -> title -> SetAngle(90);
    // Create the line
    $p1 = new LinePlot($delta, $datax);
    $p1 -> SetColor('#ff0000');
    $p1 -> SetLegend('Observed');
    $graph->Add($p1);
    // Create the forecast line
    if (count($forecast) > 1) {
      $p2 = new LinePlot($forecast, $datax);
      $p2 -> SetColor('#0000ff');
      $p2 -> SetLegend('Forecasted');
      $graph -> Add($p2);
    }
    // Create the anomaly line
    if ($alertStartTime && $alertEndTime) {
      $p3 = new LinePlot($anomalyy, $anomalyx);
      $p3 -> SetColor('#ff0000@0.2');
      $p3 -> SetLegend('Slowdown');
      $p3 -> SetFillColor('#ff0000@0.5');
      $graph -> Add($p3);
    }
    $graph -> legend -> SetAbsPos(6, 7, 'right', 'top');
    $graph -> legend -> SetLayout(LEGEND_HOR);
    $graph -> legend -> SetShadow('#000000@0.9', 1);
    $graph -> Stroke($imsHome . '/htdocs/images/graphs/slowdown/' . $date . '/' . long2ip($row['internalIP']) . '.png');
  }
?>
