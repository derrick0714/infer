<?php
  function size($bytes) {
    $bytes = abs($bytes);
    if ($bytes < 1000) {
      return $bytes . ' B';
    }
    else {
      $bytes /= 1000;
      if ($bytes < 1000) {
        return number_format($bytes, 2) . ' KB';
      }
      else {
        $bytes /= 1000;
        if ($bytes < 1000) {
          return number_format($bytes, 2) . ' MB';
        }
        else {
          $bytes /= 1000;
          if ($bytes < 1000) {
            return number_format($bytes, 2) . ' GB';
          }
          else {
            $bytes /= 1000;
            return number_format($bytes, 2) . ' TB';
          }   
        }
      }
    }
  }

  function existsPGTable(&$postgreSQL, $schemaName, &$tableName) {
    $result = pg_query($postgreSQL, 'SELECT "table_name" FROM ' .
                                    '"information_schema"."tables" WHERE ' .
                                    '"table_schema" = \'' . $schemaName .
                                    '\' AND "table_name" = \'' . $tableName .
                                    '\'');
    return (pg_num_rows($result) > 0);
  }

  function getFirstPGTable(&$postgreSQL, $schemaName) {
    $result = pg_query($postgreSQL, 'SELECT "table_name" FROM ' .
                                    '"information_schema"."tables" WHERE ' .
                                    '"table_schema" = \'' . $schemaName .
                                    '\' ORDER BY "table_name" LIMIT 1');
    $row = pg_fetch_row($result);
    return $row[0];
  }
    
  function getLastPGTable(&$postgreSQL, $schemaName) {
    $result = pg_query($postgreSQL, 'SELECT "table_name" FROM ' .
                                    '"information_schema"."tables" WHERE ' .
                                    '"table_schema" = \'' . $schemaName .
                                    '\' ORDER BY "table_name" DESC LIMIT 1');
    $row = pg_fetch_row($result);
    return $row[0];
  }
        
  function getPGTableRange(&$postgreSQL, $schemaName, &$firstTable,
                           &$lastTable, $reverse = false) {
    $result = pg_query($postgreSQL, 'SELECT "table_name" FROM ' .
                                    '"information_schema"."tables" WHERE ' .
                                    '"table_schema" = \'' . $schemaName .
                                    '\' AND "table_name" >= \'' . $firstTable .
                                    '\' AND "table_name" <= \'' . $lastTable .
                                    '\' ORDER BY "table_name"');
    while ($row = pg_fetch_row($result)) {
      $tables[] = $row[0];
    }
    if ($reverse) {
      $tables = array_reverse($tables);
    }
    return $tables;
  }

  function getInterestingIPs(&$postgreSQL, &$date) {
    $result = pg_query('SELECT "ip" FROM "InterestingIPs"."' . $date . '"');
    while ($row = pg_fetch_assoc($result)) {
      $interestingIPs[] = $row['ip'];
    }
    return $interestingIPs;
  }

  function getNumLiveIPs(&$postgreSQL, &$tableName) {
    $result = pg_query($postgreSQL, 'SELECT COUNT(*) FROM "LiveIPs"."' .
                                    $tableName . '"');
    $row = pg_fetch_row($result);
    return $row[0];
  }

  function getNumFlows(&$postgreSQL, &$tableName) {
    $result = pg_query($postgreSQL, 'SELECT SUM("inboundFlows") + ' .
                                    'SUM("outboundFlows") FROM ' .
                                    '"NetworkStats"."' . $tableName . '"');
    $row = pg_fetch_row($result);
    return $row[0];
  }

  function getNumBytes(&$postgreSQL, &$tableName) {
    $result = pg_query($postgreSQL, 'SELECT SUM("inboundBytes") + ' .
                                    'SUM("outboundBytes") FROM ' .
                                    '"NetworkStats"."' . $tableName . '"');
    $row = pg_fetch_row($result);
    return $row[0];
  }

  function getNumPackets(&$postgreSQL, &$tableName) {
    $result = pg_query($postgreSQL, 'SELECT SUM("inboundPackets") + ' .
                                    'SUM("outboundPackets") FROM ' .
                                    '"NetworkStats"."' . $tableName . '"');
    $row = pg_fetch_row($result);
    return $row[0];
  }
?>
