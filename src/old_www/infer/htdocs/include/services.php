<?php
  define('UNKNOWN_INITIATOR', 0);
  define('INTERNAL_INITIATOR', 1);
  define('EXTERNAL_INITIATOR', 2);

  $result = pg_query($postgreSQL, 'SELECT * FROM "Maps"."serviceNames"');
  while ($row = pg_fetch_assoc($result)) {
    $services[$row['protocol']][$row['port']] = $row['name'];
  }

  function getServiceNameByPort($protocol, $port) {
  	global $services;
	return $services[$protocol][$port];
  }

  function getServiceName($protocol, $internalPort, $externalPort,
                          $initiator) {
    global $services;
    switch ($initiator) {
      case INTERNAL_INITIATOR:
        return $services[$protocol][$externalPort];
        break;
      case EXTERNAL_INITIATOR:
        return $services[$protocol][$internalPort];
        break;
      case UNKNOWN_INITIATOR:
        if ($services[$protocol][$internalPort]) {
          return $services[$protocol][$internalPort];
        }
        else {
          if ($services[$protocol][$externalPort]) {
            return $services[$protocol][$externalPort];
          }
        }
        break;
    }
  }
?>
