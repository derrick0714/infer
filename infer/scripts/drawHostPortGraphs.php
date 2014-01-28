#!/usr/local/bin/php
<?php
  $imsHome = dirname(__FILE__) . '/..';

  include($imsHome . '/htdocs/include/postgreSQL.php');
  include('include/shared.php');
  include('/usr/local/share/jpgraph/jpgraph.php');
  include('/usr/local/share/jpgraph/jpgraph_bar.php');

  if (!file_exists($imsHome . '/htdocs/images/graphs/' . $argv[1])) {
    mkdir($imsHome . '/htdocs/images/graphs/' . $argv[1]);
  }

  $barColors = array('#00b263', '#31ff00', '#ceff00', '#ffe700', '#ffcf00', '#ff9a00', '#ff6500', '#e7006b', '#940094', '#63009c');

  $interestingIP_Count = 0;
  $result = pg_query($postgreSQL, 'SELECT "ip" FROM "InterestingIPs"."' . $argv[1] . '"');
  while ($row = pg_fetch_row($result)) {
    drawHostGraph($argv[1], $row[0]);
    echo long2ip($row[0]) . "\n";
  }

  function drawHostGraph(&$date, &$ip) {
    global $postgreSQL, $imsHome, $barColors;
    $graph = new Graph(600, 350, 'auto');
    $graph -> setFrame(true, '#BBAABB');
    $graph -> setMarginColor('#BBAABB');
    $graph -> SetScale('textlin');
    $graph -> SetColor('#F0F0F0');
    $graph -> xaxis -> SetColor('#660000', '#660000');
    $graph -> yaxis -> SetColor('#660000', '#660000');
    $result = pg_query($postgreSQL, 'SELECT * FROM "TopPorts"."' . $date .
                                    '" WHERE "ip" = \'' . $ip . '\'');
    $row = pg_fetch_assoc($result);
    $inboundTraffic = explode(',', substr($row['topInboundTraffic'], 1, -1));
    $outboundTraffic = explode(',', substr($row['topOutboundTraffic'], 1, -1));
    $graphedInboundPort = 0;
    foreach (explode(',', substr($row['topInboundPorts'], 1, -1)) as $inboundPort) {
      $inboundPorts[] = $inboundPort;
      $xLabels[] = $inboundPort;
      $graphedInboundPorts[$inboundPort] -= $inboundTraffic[$graphedInboundPort++];
    }
    $inboundPlot = new BarPlot(array_values($graphedInboundPorts));
    $graphColor = 0;
    $xLabel = 0;
    $graphedOutboundPort = 0;
    foreach (explode(',', substr($row['topOutboundPorts'], 1, -1)) as $outboundPort) {
      $outboundPorts[] = $outboundPort;
      $xLabels[$xLabel++] .= ' / ' . $outboundPort;
      $graphedOutboundPorts[$outboundPort] += $outboundTraffic[$graphedOutboundPort++];
    }
    $outboundPlot = new BarPlot(array_values($graphedOutboundPorts));

    $graph->Set90AndMargin(90, 7, 7, 70);

    $graph->xaxis->SetPos('min');

    // Setup X-axis
    $graph->xaxis->SetTickLabels($xLabels);
    $graph -> xaxis -> SetFont(FF_ARIAL, FS_BOLD, 10);

    // Some extra margin looks nicer
    $graph->xaxis->SetLabelMargin(5);

    // Label align for X-axis
    $graph->xaxis->SetLabelAlign('right','center');
    //$graph -> xscale -> ticks -> SupressMinorTickMarks();

    // Add some grace to y-axis so the bars doesn't go
    // all the way to the end of the plot area
    $graph->yaxis->scale->SetGrace(20);

    // Setup the Y-axis to be displayed in the bottom of the
    // graph. We also finetune the exact layout of the title,
    // ticks and labels to make them look nice.
    $graph->yaxis->SetPos('max');

    // First make the labels look right
    $graph->yaxis->SetLabelAlign('right', 'top');
    $graph->yaxis->SetLabelFormat('%s');
    $graph->yaxis->SetLabelSide(SIDE_RIGHT);

    // The fix the tick marks
    $graph->yaxis->SetTickSide(SIDE_LEFT);

$graph->yaxis->SetTitleSide(SIDE_RIGHT);
$graph->yaxis->title->SetFont(FF_ARIAL, FS_BOLD, 9);
$graph->yaxis->title->SetColor('#660000');
$graph -> yaxis -> title -> SetMargin(35);
$graph -> yaxis -> title -> Set('Inbound < Traffic > Outbound');
$graph->yaxis->title->SetAngle(0);
$graph->yaxis->title->Align('right');


    // To center the title use :
    //$graph->yaxis->SetTitle('Turnaround 2002','center');
    //$graph->yaxis->title->Align('center');

    $graph->yaxis->title->SetFont(FF_ARIAL,FS_BOLD,8);

    $graph->yaxis->SetFont(FF_ARIAL, FS_BOLD, 8);
    $graph -> yaxis -> SetLabelFormatCallback('size');
    // If you want the labels at an angle other than 0 or 90
    // you need to use TTF fonts
    //$graph->yaxis->SetLabelAngle(0);

    // Now create a bar pot
    //$bplot->SetShadow();

    //You can change the width of the bars if you like
    $inboundPlot -> SetWidth(0.6);
    $graph -> yaxis -> SetLabelAngle(45);
    $outboundPlot -> SetWidth(0.6);

    $inboundPlot -> SetFillColor($barColors);
    $outboundPlot -> SetFillColor($barColors);

    $graph -> add($inboundPlot);
    $graph -> add($outboundPlot);

    $graph -> Stroke($imsHome . '/htdocs/images/graphs/' . $date . '/' . long2ip($ip) . '.png');
  }
?>
