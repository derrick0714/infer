<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');

  if ($_POST['ip']) {
    $schemaName = 'InterestingIPs';
    foreach (getPGTableRange($postgreSQL, $schemaName, getFirstPGTable($postgreSQL, $schemaName), getLastPGTable($postgreSQL, $schemaName), 1) as $tableName) {
      $result = pg_query($postgreSQL, 'SELECT * FROM "' . $schemaName . '"."' . $tableName .
                                     '" WHERE "ip" = \'' . sprintf('%u', ip2long($_POST['ip'])) . '\'');
      if (pg_fetch_row($result)) {
        header('Location: /host/' . $_POST['ip'] . '/' . $tableName);
        exit(0);
      }
    }
  }
  makeHeader($postgreSQL, makeNavMsg($url, false));
  message(makeNavMsg($url));
  echo '<center>' .
         '<form method="post" action="/search/host">' .
           '<input type="text" maxlength="128" size="55" name="ip" title="Host/IP Search" value="' . $_POST['ip'] . '" />' .
           '<br />' .
           '<input type="submit" value="IP Search" name="submit" />' .
        '</form>' .
        '<br />';
  if ($_POST['ip']) {
    message('No results found.', false);
  }
  echo '</center>' .
     '</body>' .
   '</html>';
?>
