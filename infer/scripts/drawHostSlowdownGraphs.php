#!/usr/local/bin/php
<?php
  $imsHome = dirname(__FILE__) . '/..';
  $periodSize = 300;
  $schemaName = 'Slowdown';
  include($imsHome . '/htdocs/include/postgreSQL.php');
  include('include/shared.php');
  include('/usr/local/share/jpgraph/jpgraph.php');
  include('/usr/local/share/jpgraph/jpgraph_line.php');
  include('/usr/local/share/jpgraph/jpgraph_bar.php');

  if (!file_exists($imsHome . '/htdocs/images/graphs/slowdown/' . $argv[1])) {
    mkdir($imsHome . '/htdocs/images/graphs/slowdown/' . $argv[1]);
  }
  $numericInternalIP = sprintf('%u', ip2long($argv[2]));
  $result = pg_query('SELECT * FROM "' .$schemaName . '"."' . $argv[1] .
                         '" WHERE "internalIP" = \'' . $numericInternalIP . '\'');
  while( $row = pg_fetch_assoc($result)){
    drawSlowdownGraphs($argv[1], $argv[2], $row);
  }
  function drawSlowdownGraphs(&$date, &$ip, &$row) {
    global $imsHome;
    global $periodSize;
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

    //print_r($delta);

    for( $index = 0; $index < count($delta); ++$index ) {
      $datax[] = $startTime;
      $startTime += $periodSize;
    }

    if($alertStartTime && $alertEndTime){
      $anomalyCount = ($alertEndTime - $alertStartTime) / $periodSize;
      if($alertStartTime == $dayStartTime){
        $alertStartTime += $periodSize;
      }
      $anomalyStartDistance = ($alertStartTime - $dayStartTime) / $periodSize;
      $anomalyEndDistance = ($alertEndTime - $dayStartTime) / $periodSize;
      for( $index = 0; $index < $anomalyCount; ++$index ) {
        $anomalyx[] = $alertStartTime;
        $alertStartTime += $periodSize;
      }
      for( $index = $anomalyStartDistance; $index < $anomalyEndDistance; ++$index ) {
        $anomalyy[] = $delta[$index];
      }
    }

    // Setup the basic graph
    $graph = new Graph(600,350,"auto");
    $graph->SetMargin(90, 7, 7, 70);
    //$graph->SetAlphaBlending();
    $graph->SetScale("intlin",0,0,$dayStartTime,$endTime);
    $graph -> SetColor('#F0F0F0');
    $graph->setFrame(true, '#BBAABB');
    $graph->setMarginColor('#BBAABB');
    $graph->xaxis -> SetColor('#660000', '#660000');
    $graph->yaxis -> SetColor('#660000', '#660000');

    // Setup X-axis
    $graph->xaxis->SetFont(FF_ARIAL, FS_BOLD, 8);
    $graph->xaxis->SetLabelAngle(45);
    $graph->xaxis->SetPos('min');
    $graph->xaxis->SetLabelMargin(5);
    $graph->xaxis->SetLabelFormatCallback('TimeCallback');
    $graph->xaxis->title->SetFont(FF_ARIAL, FS_BOLD, 8);
    $graph->xaxis->title->SetColor('#660000');
    $graph->xaxis->title->SetMargin(35);
    $graph->xaxis->title->Set('Hours <0-23>');

    // Setup the Y-axis
    $graph->yaxis->SetFont(FF_ARIAL, FS_BOLD, 8);
    $graph->yaxis->scale->SetGrace(0);
    $graph->yaxis->SetLabelFormat('%lf');
    $graph->yaxis->title->SetFont(FF_ARIAL, FS_BOLD, 8);
    $graph->yaxis->title->SetColor('#660000');
    $graph->yaxis->title->SetMargin(35);
    $graph->yaxis->title->Set('Response Times');
    $graph->yaxis->title->SetAngle(90);
    
    // Create the line
    $p1 = new LinePlot($delta,$datax);
    $p1->SetColor('red');
    $p1->SetLegend("Observed");
    $graph->Add($p1);

    // Create the forecast line
    if(count($forecast) > 1){
      $p2 = new LinePlot($forecast, $datax);
      $p2->SetColor('blue');
      $p2->SetLegend("Forecasted");
      $graph->Add($p2);
    }
    // Create the anomaly line
    if($alertStartTime && $alertEndTime){
      $p3 = new LinePlot($anomalyy, $anomalyx);
      $p3->SetColor('red@0.2');
      $p3->SetLegend("Slowdown");
      $p3->SetFillColor("red@0.5");
      $graph->Add($p3);
    }

    $graph->legend->Pos(0.18,0.94,"left","top");
    $graph->legend->SetLayout(LEGEND_HOR);
    $graph->legend->SetShadow("black@0.9",1);

    $graph -> Stroke($imsHome . '/htdocs/images/graphs/slowdown/' . $date . '/' . $ip . '.png');
  }

  // The callback that converts timestamp to minutes and seconds
  function TimeCallback($aVal) {
    return Date('H:i:s',$aVal);
  }
?>
