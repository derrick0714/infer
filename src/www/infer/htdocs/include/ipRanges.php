<?php
  class ScannedIPRange {
        var $protocol;
        var $sourcePort;
        var $firstIP;
        var $lastIP;
        var $destinationPort;
        var $startTime;
        var $endTime;
        var $numPackets;
        var $minPacketSize;
        var $maxPacketSize;
        var $numBytes;
        var $content;
        var $asNumber;
  }

  function makeIPPair(&$firstIP, &$lastIP) {
    if ($firstIP == $lastIP) {
      return long2ip($firstIP);
    }
    else {
      return long2ip($firstIP) . ' - ' . long2ip($lastIP);
    }
  }

?>
