<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  $title = 'White List';
  include('include/header.php');

  $result = pg_query('SELECT * FROM "Maps"."whiteList"');
  while ($row = pg_fetch_assoc($result)) {
    $whiteList[$row['ip']] = $row;
  }
  if ($_POST['add'] && $_POST['ips']) {
    $privileges = getPrivileges($postgreSQL, $_COOKIE['imsSessionID']);
    if ($privileges & ADMINISTRATOR_PRIVILEGE || $privileges & WHITELIST_PRIVILEGE) {
      foreach (preg_split('(\s+)', trim($_POST['ips'])) as $ip) {
        $ip = sprintf('%u', ip2long($ip));
        if (!$whiteList || !array_key_exists($ip, $whiteList)) {
          $whiteList[$ip]['time'] = time();
          $whiteList[$ip]['name'] = getUsername($postgreSQL, $_COOKIE['imsSessionID']);
          $whiteList[$ip]['comments'] = $_POST['comments'];
          if ($_POST['comments']) {
            $comments = '\'' . pg_escape_string($postgreSQL, $_POST['comments']) . '\'';
          }
          else {
            $comments = 'NULL';
          }
          pg_query($postgreSQL, 'INSERT INTO "Maps"."whiteList" VALUES (\'' . $ip . '\', \'' .
                   $whiteList[$ip]['time'] . '\', \'' . pg_escape_string($postgreSQL, $whiteList[$ip]['name']) . '\', ' .
                   $comments . ')');
          message('Added IP <b>' . long2ip($ip) . '</b>.', false);
        }
        else {
          message('IP ' . long2ip($ip) . ' exists', true);
        }
      }
    }
  }
  if (count($whiteList) && $_POST['delete']) {
    $privileges = getPrivileges($postgreSQL, $_COOKIE['imsSessionID']);
    if ($privileges & ADMINISTRATOR_PRIVILEGE || $privileges & WHITELIST_PRIVILEGE) {
      foreach ($_POST as $postVariable => &$value) {
        $ip = substr($postVariable, 7);
        if (substr($postVariable, 0, 7) == 'delete_' &&
            array_key_exists($ip, $whiteList)) {
          if (pg_query('DELETE FROM "Maps"."whiteList" WHERE "ip" = \'' . $ip . '\'')) {
            unset($whiteList[$ip]);
            message('Deleted <b>' . long2ip($ip) . '</b>.', false);
          }
        }
      }
    }
  }
  echo '<div class="table">' .
         '<form method="post" action="/whiteList">' .
           '<table>' .
             '<tr>' .
               '<td class="bold">' .
                 'IPs' .
               '</td>' .
               '<td>' .
                 '<textarea name="ips" cols="50" rows="5" wrap="physical">';
  if ($url[1]) {
    echo $url[1];
  }
  else {
    echo $_POST['ips'];
  }
  echo           '</textarea>' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td class="bold">' .
                 'Comments (optional)' .
               '</td>' .
               '<td>' .
                 '<input type="text" name="comments" size="57" value="' . $_POST['comments'] . '" />' .
               '</td>' .
             '</tr>' .
             '<tr>' .
           '<td>' .
             '<input type="submit" value="Add" name="add" />' .
           '</td>' .
         '</tr>' .
       '</table>' .
     '</div>' .
     '<div class="table">' .
       '<table width="100%" cellspacing="1">' .
         '<tr>' .
           '<td class="columnTitle center">' .
             'IP' .
           '</td>' .
           '<td class="columnTitle center">' .
             'Added at' .
           '</td>' .
           '<td class="columnTitle center">' .
             'Added by' .
           '</td>' .
           '<td class="columnTitle center">' .
             'Comments' .
           '</td>' .
           '<td class="columnTitle center">' .
             'Delete' .
           '</td>' .
         '</tr>';
  if (count($whiteList)) {
    foreach ($whiteList as $ip => &$row) {
      $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
      ++$rowNumber;
      echo '<tr class="' . $rowClass . '">' .
             '<td class="center">' .
               long2ip($ip) .
             '</td>' .
             '<td class="center">' .
               date("l, F j, Y \a\\t g:i:s A", $row['time']) .
               ' (' . duration(time() - $row['time']) . ' ago)' .
             '</td>' .
             '<td class="center">' .
               $row['name'] .
             '</td>' .
             '<td class="center">' .
               $row['comments'] .
             '</td>' .
             '<td class="center">' .
               '<input type="checkbox" name="delete_' . $ip . '" />' .
             '</td>' .
           '</tr>';
    }
    echo '<tr>' .
           '<td class="top">' .
             '<input type="submit" value="Delete" name="delete" />';
           '</td>' .
         '</tr>' .
       '</table>';
  }
?>
