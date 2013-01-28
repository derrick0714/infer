<?php
  define('ADMINISTRATOR_PRIVILEGE', 1);
  define('WHITELIST_PRIVILEGE', 2);
  define('IP_OWNERS_PRIVILEGE', 4);
  define('INFECTED_IPS_PRIVILEGE', 8);
  define('HBF_QUERY_PRIVILEGE', 16);

  function checkCredentials(&$postgreSQL, &$username, &$password) {
    $result = @pg_query('SELECT * FROM "AccessControl"."users" WHERE "name" = \'' .
                        $username . '\' AND "password" = \'' . hash('sha256', $password) . '\' ' .
                        'AND "active" = \'t\'');
    return (@pg_num_rows($result) > 0);
  }

  function isSession(&$postgreSQL, &$sessionID) {
    $result = @pg_query('SELECT * FROM "AccessControl"."sessions" WHERE "id" = \'' .
                        $sessionID . '\'');
    return (@pg_num_rows($result) > 0);
  }

  function getUsername(&$postgreSQL, &$sessionID) {
    $result = @pg_query('SELECT "name" FROM "AccessControl"."sessions" WHERE "id" = \'' .
                        $sessionID . '\'');
    $row = @pg_fetch_assoc($result);
    return $row['name'];
  }

  function isUsername(&$postgreSQL, &$username) {
    $result = @pg_query('SELECT * FROM "AccessControl"."users" WHERE "name" = \'' .
                        $username . '\'');
    return (@pg_num_rows($result) > 0);
  }

  function getPrivileges(&$postgreSQL, &$sessionID) {
    $result = @pg_query($postgreSQL, 'SELECT "privileges" FROM "AccessControl"."users" ' .
                                     'WHERE "name" = \'' . getUsername($postgreSQL, $sessionID) . '\'');
    $row = @pg_fetch_assoc($result);
    return $row['privileges'];
  }

  function requirePrivilege(&$postgreSQL, $privilege) {
    $privileges = getPrivileges($postgreSQL, $_COOKIE['imsSessionID']);
    if (!($privileges & $privilege) && !($privileges & ADMINISTRATOR_PRIVILEGE)) {
      echo '<script>' .
             'history.back(1);' .
           '</script>';
      exit;
    }
  }

  function getLastLoginTime(&$postgreSQL, &$username) {
    $result = @pg_query($postgreSQL, 'SELECT "lastUsed" FROM "AccessControl"."sessions" ' .
                                     'WHERE "name" = \'' . $username . '\'');
    if (@pg_num_rows($result) > 0) {
      $row = @pg_fetch_assoc($result);
      return date("l, F j, Y \a\\t g:i:s A", $row['lastUsed']);
    }
    return false;
  }
?>
