<?php
  define('CONTACTS_RUNNING', 0);
  define('CONTACTS_COMPLETED', 1);
  define('CONTACTS_STOPPED', 2);
  define('CONTACTS_PAUSED', 3);

  /*
   * Signal definitions. PHP used to provide these, but no longer does, so we
   * define them and hope they don't change.
   */
  define('SIGKILL', 9);
  define('SIGSTOP', 17);
  define('SIGCONT', 19);

  /*
   * Returns true if the specified CONTACTS query ID exists, or false if it
   * doesn't.
   */
  function isContactsQueryID(&$postgreSQL, &$queryID) {
    $result = @pg_query($postgreSQL, 'SELECT * FROM "Indexes"."contactsQueries" ' .
                                     'WHERE "id" = \'' . $queryID . '\'');
    return (@pg_num_rows($result) > 0);
  }

  function prepareContactsQuery(&$postgreSQL, &$_POST, &$username) {
    /*
     * Computes a random MD5 hash that is not currently a query ID. It will be
     * the query ID of the new query.
     */
    do {
      $queryID = hash('md5', rand());
    } while (isContactsQueryID($postgreSQL, $queryID));
    /* Adds the CONTACTS query about to be started to the CONTACTS query index. */
    if (!insertPGRow($postgreSQL, 'Indexes', 'contactsQueries', $queryID, stripslashes($_POST['queryName']),
		     sprintf('%u', ip2long($_POST['queryIP'])), strtotime($_POST['queryStartTime']), strtotime($_POST['queryEndTime']),
                     0, $username, time(), false, false, false, false, 0, 0, CONTACTS_RUNNING, false)) {
      return false;
    }
    /*        
     * Creates a table in the "hbfQueries" schema for the queryCONTACTS-SQL program
     * to store results in.
     */
    if (!@pg_query($postgreSQL, 'CREATE TABLE "ContactsQueries"."' . $queryID . '" ' .
                                '("protocol" uint16, "sourceIP" uint32, "destinationIP" uint32, ' .
                                '"sourcePort" uint16, "destinationPort" uint16, ' .
                                '"startTime" uint32, "endTime" uint32)')) {
      return false;
    }
    return $queryID;
  }
?>
