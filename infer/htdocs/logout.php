<?php
  include('include/postgreSQL.php');
  include('include/accessControl.php');
  if ($_COOKIE['imsSessionID'] && isSession($postgreSQL, $_COOKIE['imsSessionID'])) {
    pg_query('DELETE FROM "AccessControl"."sessions" WHERE "id" = \'' . $_COOKIE['imsSessionID'] . '\'');
  }
  header('Location: /login');
?>
