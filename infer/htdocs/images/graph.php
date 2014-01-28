<?php

include('include/postgreSQL.php');
include('include/shared.php');
include('include/accessControl.php');
include('include/checkSession.php');
include('include/roles.php');

include('/usr/local/share/jpgraph/jpgraph.php');
include('/usr/local/share/jpgraph/jpgraph_pie.php');
include('/usr/local/share/jpgraph/jpgraph_pie3d.php');
include('/usr/local/share/jpgraph/jpgraph_bar.php');

$graphColors = array('#00b263', '#31ff00', '#ceff00', '#ffe700', '#ffcf00',
					 '#ff9a00', '#ff6500', '#e7006b', '#940094', '#63009c');

$type = $url[2];
$img_date = $url[3];
$ip = ip2long($url[4]);
$direction = $url[5];

switch ($type) {
  case 'content':
	$result = pg_query($postgreSQL,
					   'SELECT "' . $direction . 'Bytes", ' .
					   '"' . $direction . 'Content" FROM ' .
					   '"HostTraffic"."' . $img_date . '" ' .
					   'WHERE "ip" = \'' . $ip . '\'');
	$row = pg_fetch_assoc($result);
	if ($row[$direction . 'Bytes'] == 0) {
		exit;
	}
	$content = explode(',', substr($row[$direction . 'Content'], 1, -1));
	foreach ($content as &$amount) {
		$amount *= 16384;
	}
	$content[] = $row[$direction . 'Bytes'] - array_sum($content);
	drawContentPieChart($content, $graphColors);
	break;
  case 'activeports':
	drawHostGraph($postgreSQL, $graphColors, $img_date, $ip);
	break;
  default:
	exit;
}


function drawContentPieChart(&$content, &$graphColors) {
	$graph = new pieGraph(210, 140);
	$graph->img->SetTransparent("white");
	$graph -> setFrame(false);
	//$graph -> setMarginColor('#D9D9D9');
	$pie = new PiePlot3D($content);
	$pie -> SetSize(85);

	$pie -> SetCenter(0.5, 0.475);
	$pie -> SetStartAngle(20);
	$pie -> SetAngle(55);
	$pie -> SetSliceColors($graphColors);
	//$pie -> value -> SetColor('#D9D9D9');
	$pie -> value -> SetColor('white');
	$graph -> SetAntiAliasing();
	$graph -> Add($pie);
	$graph -> Stroke();
}

function drawHostGraph(&$postgreSQL, &$graphColors, &$date, &$ip) {
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
	$graph -> yaxis -> SetLabelFormatCallback('abssize');
	// If you want the labels at an angle other than 0 or 90
	// you need to use TTF fonts
	//$graph->yaxis->SetLabelAngle(0);

	// Now create a bar pot
	//$bplot->SetShadow();

	//You can change the width of the bars if you like
	$inboundPlot -> SetWidth(0.6);
	$graph -> yaxis -> SetLabelAngle(45);
	$outboundPlot -> SetWidth(0.6);

	$inboundPlot -> SetFillColor($graphColors);
	$outboundPlot -> SetFillColor($graphColors);

	$graph -> add($inboundPlot);
	$graph -> add($outboundPlot);

	$graph -> Stroke();
}
?>
