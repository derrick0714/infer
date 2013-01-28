<?php
  $psql_dbname =
    exec('/usr/local/bin/infer_config postgresql.dbname');
  $psql_user =
    exec('/usr/local/bin/infer_config postgresql.user');
  $psql_password =
    exec('/usr/local/bin/infer_config postgresql.password');
    
  @$postgreSQL = pg_connect('dbname = ' . $psql_dbname . ' user = ' . $psql_user . ' password = ' . $psql_password);

  foreach ($_COOKIE as &$cookie) {
    $cookie = @pg_escape_string($postgreSQL, $cookie);
  }

  /*
   * Row insertion wrapper for PostgreSQL. Returns true upon success, or false
   * upon failure, at which point the pg_last_error() function may be used to
   * examine what went wrong.
   */
  function insertPGRow(&$postgreSQL, $schema, $table) {
    $query = 'INSERT INTO "' . $schema . '"."' . $table . '" VALUES (';
    for ($arg = 3; $arg < func_num_args(); ++$arg) {
      $_arg = func_get_arg($arg);
      /* NULL values are represented by PHP's false boolean literal. */
      if ($_arg === false || strlen($_arg) == 0) {
        $query .= 'NULL';
      }
      else {
        /* Non-NULL values are quoted and escaped. */
        $query .= '\'' . pg_escape_string($postgreSQL, $_arg) . '\'';
      }
      if ($arg < (func_num_args() - 1)) {
        $query .= ', ';
      }
    }
    $query .= ')';
    return (pg_query($postgreSQL, $query) !== false) ;
  }

  function getPGRow(&$postgreSQL, $schemaName, $tableName) {
    $result = @pg_query($postgreSQL, 'SELECT * FROM "' . $schemaName . '"."' .
                                     $tableName . '"');
    if (@pg_num_rows($result)) {
      $row = @pg_fetch_assoc($result);
      return $row;
    }
    return false;
  }

  function getPGValue(&$postgreSQL, $schemaName, $tableName, $columnName) {
    $result = @pg_query($postgreSQL, 'SELECT "' . $columnName . '" FROM "' .
                                     $schemaName . '"."' . $tableName . '"');
    if (@pg_num_rows($result)) {
      $row = @pg_fetch_assoc($result);
      return $row[$columnName];
    }
    return NULL;
  }
?>
