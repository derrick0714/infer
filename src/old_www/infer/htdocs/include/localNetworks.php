<?php
  include('include/config.php');

  foreach (explode(' ', $localNetworks) as $localNetwork) {
    $firstIP = sprintf('%u', ip2long(substr($localNetwork, 0, strpos($localNetwork, '/'))));
    $lastIP = $firstIP + pow(2, 32 - substr($localNetwork, strpos($localNetwork, '/') + 1)) - 1;
    $_localNetworks[$firstIP] = $lastIP;
  }

  function isInternal(&$ip) {
    global $_localNetworks;
    foreach ($_localNetworks as $firstIP => &$lastIP) {
      if ($ip >= $firstIP && $ip <= $lastIP) {
        return true;
      }
    }
    return false;
  }
?>
