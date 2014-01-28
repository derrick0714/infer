<?php
  define('INBOUND', 0);
  define('OUTBOUND', 1);

  function getProtocolName(&$postgreSQL, &$protocolNumber) {
    $result = pg_query($postgreSQL, 'SELECT "protocolName" FROM ' .
                                    '"Maps"."protocolnames" WHERE ' .
                                    '"protocolNumber" = \'' . $protocolNumber .
                                    '\'');
    $row = pg_fetch_row($result);
    return $row[0];
  }

  function getSnortMessage(&$mySQL, &$generatorID, &$signatureID) {
    $result = mysqli_query($mySQL, 'SELECT "message" FROM ' .
                                   '"Maps"."snortMessages" WHERE ' .
                                   '"generatorID" = \'' . $generatorID .
                                   '\' AND "signatureID" = \'' . $signatureID .
                                   '\'');
    $row = pg_fetch_row($result);
    return $row[0];
  }

  function numAlerts(&$postgreSQL, &$tableName, &$ip, $direction) {
    if ($direction == INBOUND) {
      $ipField = 'destinationIP';
    }
    else {
      $ipField = 'sourceIP';
    }
    $result = mysqli_query($mySQL, 'SELECT COUNT(*) FROM "SnortAlerts"."' .
                                   $tableName . '" WHERE "' . $ipField .
                                   '" = \'' . $ip . '\'');
    $row = mysqli_fetch_row($result);
    return $row[0];
  }

  function alertDescriptionURL(&$generatorID, &$signatureID, &$text) {
    return '<a href="http://www.snort.org/pub-bin/sigs.cgi?sid=' .
           $generatorID . ':' . $signatureID . '">' . $text . '</a>';
  }
?>
