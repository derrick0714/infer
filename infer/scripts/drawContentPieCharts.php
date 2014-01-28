#!/usr/local/bin/php
<?php
  $imsHome = dirname(__FILE__) . '/..';

  include($imsHome . '/htdocs/include/postgreSQL.php');
  include('include/shared.php');
  include('/usr/local/share/jpgraph/jpgraph.php');
  include('/usr/local/share/jpgraph/jpgraph_pie.php');
  include('/usr/local/share/jpgraph/jpgraph_pie3d.php');

  $graphColors = array('#00b263', '#31ff00', '#ceff00', '#ffe700', '#ffcf00',
                       '#ff9a00', '#ff6500', '#e7006b', '#940094');

  if (!file_exists($imsHome . '/htdocs/images/graphs/content/' . $argv[1])) {
    mkdir($imsHome . '/htdocs/images/graphs/content/' . $argv[1]);
    mkdir($imsHome . '/htdocs/images/graphs/content/' . $argv[1] . '/inbound');
    mkdir($imsHome . '/htdocs/images/graphs/content/' . $argv[1] . '/outbound');
  }

  $result = pg_query($postgreSQL, 'SELECT "ip" FROM "InterestingIPs"."' . $argv[1] . '"');
  while ($row = pg_fetch_assoc($result)) {
    $interestingIPs[] = $row['ip'];
  }
  $result = pg_query($postgreSQL, 'SELECT "ip", "inboundBytes", "outboundBytes", ' .
                                  '"inboundContent", "outboundContent" FROM ' .
                                  '"HostTraffic"."' . $argv[1] . '"');
  while ($row = pg_fetch_assoc($result)) {
    if (array_search($row['ip'], $interestingIPs) !== false) {
      echo long2ip($row['ip']) . "\n";
      $inboundContent = explode(',' ,substr($row['inboundContent'], 1, -1));
      foreach ($inboundContent as &$_inboundContent) {
        $_inboundContent *= 16384;
      }
      $inboundContentSum = array_sum($inboundContent);
      $inboundContent[] = $row['inboundBytes'] - $inboundContentSum;
      if ($inboundContentSum || $row['inboundBytes']) {
        drawContentPieChart($inboundContent, $graphColors,
                            $imsHome . '/htdocs/images/graphs/content/' . $argv[1] .
                            '/inbound/' . long2ip($row['ip']) . '.png');
      }
      unset($inboundContent);
      $outboundContent = explode(',' ,substr($row['outboundContent'], 1, -1));
      foreach ($outboundContent as &$_outboundContent) {
        $_outboundContent *= 16384;
      }
      $outboundContentSum = array_sum($outboundContent);
      $outboundContent[] = $row['outboundBytes'] - $outboundContentSum;
      if ($outboundContentSum || $row['outboundBytes']) {
        drawContentPieChart($outboundContent, $graphColors,
                            $imsHome . '/htdocs/images/graphs/content/' . $argv[1] .
                            '/outbound/' . long2ip($row['ip']) . '.png');
      }
      unset($outboundContent);
    }
  }

  function drawContentPieChart(&$content, &$graphColors, $fileName) {
    $graph = new pieGraph(210, 140);
    $graph -> setFrame(false);
    $graph -> setMarginColor('#D9D9D9');
    $pie = new PiePlot3D($content);
    $pie -> SetSize(85);

    $pie -> SetCenter(0.5, 0.475);
    $pie -> SetStartAngle(20);
    $pie -> SetAngle(55);
    $pie -> SetSliceColors($graphColors);
    $pie -> value -> SetColor('#D9D9D9');
    $graph -> SetAntiAliasing();
    $graph -> Add($pie);
    $graph -> Stroke($fileName);
  }
?>
