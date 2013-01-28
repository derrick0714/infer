<?php
  function getBruteForcerServices(&$postgreSQL, &$ip, &$date) {
    $result = pg_query($postgreSQL, 'SELECT "destinationPort", COUNT("destinationIP") ' .
                                    ' FROM "BruteForcers"."' . $date . '" WHERE "sourceIP" = \'' .
                                    $ip . '\' GROUP BY "destinationPort"');
    if (pg_num_rows($result)) {
      while ($row = pg_fetch_assoc($result)) {
        $bruteForcedServices[$row['destinationPort']] = $row['count'];
      }
      return $bruteForcedServices;
    }
    return false;
  }

  function getBruteForcedServices(&$postgreSQL, &$ip, &$date) {
    $result = pg_query($postgreSQL, 'SELECT "destinationPort", COUNT("sourceIP") ' .
                                    ' FROM "BruteForcers"."' . $date . '" WHERE "destinationIP" = \'' .
                                    $ip . '\' GROUP BY "destinationPort"');
    if (pg_num_rows($result)) {
      while ($row = pg_fetch_assoc($result)) {
        $bruteForcedServices[$row['destinationPort']] = $row['count'];
      }
      return $bruteForcedServices;
    }
    return false;
  }

  function makeBruteForcerServicesLine(&$ip, &$date, &$bruteForcedServices) {
    foreach ($bruteForcedServices as $port => &$numHosts) {
      $line[] = 'the <a class="text" href="/role/bruteForcer/' . $date . '/' . $port . '/' . long2ip($ip) . '">' .
                       getServiceName(6, 0, $port, INTERNAL_INITIATOR) .
                     ' service on ' . number_format($numHosts) . ' host(s)</a>';
    }
    if (count($line) > 1) {
      $line[count($line) - 1] = 'and ' . $line[count($line) - 1];
    }
    return implode(', ', $line);
  }

  function makeBruteForcedServicesLine(&$ip, &$date, &$bruteForcedServices) {
    foreach ($bruteForcedServices as $port => &$numHosts) {
      $line[] = '<a class="text" href="/role/bruteForced/' . $date . '/' . $port . '/' . long2ip($ip) . '">' .
                  number_format($numHosts) . ' host(s) have attempted to brute force its ' .
                  getServiceName(6, 0, $port, INTERNAL_INITIATOR) . ' service' .
                '</a>';
    }
    if (count($line) > 1) {
      $line[count($line) - 1] = 'and ' . $line[count($line) - 1];
    }
    return implode(', ', $line);
  }

  function getBruteForcerStartTime(&$postgreSQL, &$ip, &$date) {
    $result = pg_query($postgreSQL, 'SELECT "startTime" FROM "BruteForcers"."' .
                                    $date . '" WHERE "sourceIP" = \'' . $ip .
                                    '\' ORDER BY "startTime" LIMIT 1');
    if (pg_num_rows($result)) {
      $row = pg_fetch_assoc($result);
      return $row['startTime'];
    }
    return false;
  }

  function getBruteForcedStartTime(&$postgreSQL, &$ip, &$date) {
    $result = pg_query($postgreSQL, 'SELECT "startTime" FROM "BruteForcers"."' .
                                    $date . '" WHERE "destinationIP" = \'' . $ip .
                                    '\' ORDER BY "startTime" LIMIT 1');
    if (pg_num_rows($result)) {
      $row = pg_fetch_assoc($result);
      return $row['startTime'];
    }
    return false;
  }

  function getBruteForcerEndTime(&$postgreSQL, &$ip, &$date) {
    $result = pg_query($postgreSQL, 'SELECT "endTime" FROM "BruteForcers"."' .
                                    $date . '" WHERE "sourceIP" = \'' . $ip .    
                                    '\' ORDER BY "endTime" DESC LIMIT 1');
    if (pg_num_rows($result)) {
      $row = pg_fetch_assoc($result);
      return $row['endTime'];
    }
    return false;
  }

  function getBruteForcedEndTime(&$postgreSQL, &$ip, &$date) {
    $result = pg_query($postgreSQL, 'SELECT "endTime" FROM "BruteForcers"."' .
                                    $date . '" WHERE "destinationIP" = \'' . $ip .
                                    '\' ORDER BY "endTime" DESC LIMIT 1');
    if (pg_num_rows($result)) {
      $row = pg_fetch_assoc($result);
      return $row['endTime'];
    }
    return false;
  }
?>
