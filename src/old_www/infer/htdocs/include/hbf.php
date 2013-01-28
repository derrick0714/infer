<?php
  define('HBF_RUNNING', 0);
  define('HBF_COMPLETED', 1);
  define('HBF_STOPPED', 2);
  define('HBF_PAUSED', 3);

  /*
   * Signal definitions. PHP used to provide these, but no longer does, so we
   * define them and hope they don't change.
   */
  define('SIGKILL', 9);
  define('SIGSTOP', 17);
  define('SIGCONT', 19);

  /*
   * Returns true if the specified HBF query ID exists, or false if it
   * doesn't.
   */
  function isQueryID(&$postgreSQL, &$queryID) {
    $result = @pg_query($postgreSQL, 'SELECT * FROM "Indexes"."hbfQueries" ' .
                                     'WHERE "id" = \'' . $queryID . '\'');
    return (@pg_num_rows($result) > 0);
  }

  function prepareHBFquery(&$postgreSQL, &$_POST, &$queryString, &$username) {
    /*
     * Computes a random MD5 hash that is not currently a query ID. It will be
     * the query ID of the new query.
     */
    do {
      $queryID = hash('md5', rand());
    } while (isQueryID($postgreSQL, $queryID));
    /* Adds the HBF query about to be started to the HBF query index. */
    if (!insertPGRow($postgreSQL, 'Indexes', 'hbfQueries', $queryID, $_POST['queryName'],
                     $queryString, strtotime($_POST['startTime']), strtotime($_POST['endTime']),
                     $_POST['queryStringOffset'], $_POST['queryStringLength'], $_POST['matchLength'],
                     $_POST['protocol'], $_POST['minimumSourceIP'], $_POST['maximumSourceIP'],
                     $_POST['minimumDestinationIP'], $_POST['maximumDestinationIP'], $_POST['minimumSourcePort'],
                     $_POST['maximumSourcePort'], $_POST['minimumDestinationPort'], $_POST['maximumDestinationPort'],
                     false, $username, time(), false, false, false, false, 0, 0, HBF_RUNNING, false)) {
      return false;
    }
    /*        
     * Creates a table in the "hbfQueries" schema for the queryHBF-SQL program
     * to store results in.
     */
    if (!@pg_query($postgreSQL, 'CREATE TABLE "HBFQueries"."' . $queryID . '" ' .
                                '("protocol" uint16, "sourceIP" uint32, "destinationIP" uint32, ' .
                                '"sourcePort" uint16, "destinationPort" uint16, ' .
                                '"startTime" uint32, "endTime" uint32, "country" SMALLINT, ' .
                                '"asn" uint16, "matchingString" TEXT)')) {
      return false;
    }
    return $queryID;
  }
      
  /* Returns the ASCII part of a binary string. */
  function ascii(&$string) {
    for ($index = 0; $index < strlen($string); ++$index) {
      if (ctype_print($string[$index])) {
        switch ($string[$index]) {
          case '<':
            $printableString .= '&lt;';
            break;
          case '>':
            $printableString .= '&gt;';
            break;
          default:
            $printableString .= $string[$index];
            break;
        }
      }
    }
    return $printableString;
  }
?>
