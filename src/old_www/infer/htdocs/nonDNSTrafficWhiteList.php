<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  $title = 'Non-DNS Traffic White List';
  include('include/header.php');

  $result = pg_query('SELECT * FROM "Maps"."nonDNSTrafficWhiteList"');
  while ($row = pg_fetch_assoc($result)) {
    $whiteList[$row['ipBlock']] = $row;
  }
  if ($_POST['add'] && $_POST['ipBlocks']) {
    $privileges = getPrivileges($postgreSQL, $_COOKIE['imsSessionID']);
    if ($privileges & ADMINISTRATOR_PRIVILEGE || $privileges & WHITELIST_PRIVILEGE) {
      foreach (preg_split('(\s+)', trim($_POST['ipBlocks'])) as $ipBlock) {
        $slash = strpos($ipBlock, '/');
        if ($slash === false) {
          $ipBlock .= '/32';
        }
        if (!$whiteList || !array_key_exists($ipBlock, $whiteList)) {
          $whiteList[$ipBlock]['time'] = time();
          $whiteList[$ipBlock]['name'] = getUsername($postgreSQL, $_COOKIE['imsSessionID']);
          $whiteList[$ipBlock]['comments'] = $_POST['comments'];
          if ($_POST['comments']) {
            $comments = $_POST['comments'];
          }
          else {
            $comments = false;
          }
          insertPGRow($postgreSQL, 'Maps', 'nonDNSTrafficWhiteList', $ipBlock,
                      $whiteList[$ipBlock]['time'], $whiteList[$ipBlock]['name'], $comments);
          message('Added IP block <b>' . $ipBlock . '</b>.', false);
        }
        else {
          message('IP block ' . $ipBlock . ' exists', true);
        }
      }
    }
  }
  if (count($whiteList) > 0 && $_POST['delete']) {
    $privileges = getPrivileges($postgreSQL, $_COOKIE['imsSessionID']);
    if ($privileges & ADMINISTRATOR_PRIVILEGE || $privileges & WHITELIST_PRIVILEGE) {
      foreach ($_POST as $postVariable => &$value) {
        $ipBlock = str_replace('_', '.', substr($postVariable, 7));
        if (substr($postVariable, 0, 7) == 'delete_' &&
            array_key_exists($ipBlock, $whiteList)) {
          if (pg_query('DELETE FROM "Maps"."nonDNSTrafficWhiteList" WHERE "ipBlock" = \'' . $ipBlock . '\'')) {
            unset($whiteList[$ipBlock]);
            message('Deleted <b>' . $ipBlock . '</b>.', false);
          }
        }
      }
    }
  }
  echo '<div class="table">' .
         '<form method="post" action="/nonDNSTrafficWhiteList">' .
           '<table>' .
             '<tr>' .
               '<td class="bold">' .
                 'IP Blocks' .
               '</td>' .
               '<td>' .
                 '<textarea name="ipBlocks" cols="50" rows="5" wrap="physical">';
  if ($url[1]) {
    echo $url[1];
  }
  else {
    echo $_POST['ipBlocks'];
  }
  echo           '</textarea>' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td class="bold">' .
                 'Comments (Optional)' .
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
     '</div>';
  if (count($whiteList) > 0) {
  echo '<div class="table">' .
         '<table width="100%" cellspacing="1">' .
           '<tr>' .
             '<td class="columnTitle center">' .
               'IP Block' .
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
    foreach ($whiteList as $ipBlock => &$row) {
      $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
      ++$rowNumber;
      echo '<tr class="' . $rowClass . '">' .
             '<td class="center">' .
               $ipBlock .
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
               '<input type="checkbox" name="delete_' . str_replace('.', '_', $ipBlock) . '" />' .
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
