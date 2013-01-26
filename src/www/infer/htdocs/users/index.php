<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  requirePrivilege($postgreSQL, ADMINISTRATOR_PRIVILEGE);
  $title = 'User Management';
  include('include/header.php');

  function active(&$active) {
    if ($active == 't') {
      return 'Yes';
    }
    return 'No';
  }

  function privileges(&$_privileges) {
    if ($_privileges & ADMINISTRATOR_PRIVILEGE) {
      $privileges[] = 'Administrator';
    }
    if ($_privileges & WHITELIST_PRIVILEGE) {
      $privileges[] = 'Whitelist Write Access';
    }
    if ($_privileges & IP_OWNERS_PRIVILEGE) {
      $privileges[] = 'IP Owners List Write Access';
    }
    if ($_privileges & INFECTED_IPS_PRIVILEGE) {
      $privileges[] = 'Infected IPs List Write Access';
    }
    if ($_privileges & HBF_QUERY_PRIVILEGE) {
      $privileges[] = 'HBF Queries';
    }
    if ($_privileges) {
      return implode(', ', $privileges);
    }
  }

  if ($_POST['add']) {
    if (!strlen(trim($_POST['newUsername'])) || !strlen(trim($_POST['newPassword']))) {
      message('the username and password fields are mandatory', true);
    }
    else {
      if (isUsername($postgreSQL, $_POST['newUsername'])) {
        message('user <b>' . $_POST['newUsername'] . '</b> already exists', true);
      }
      else {
        if ($_POST['active']) {
          $active = 't';
        }
        else {
          $active = 'f';
        }
        $privileges = 0;
        if ($_POST['administrator']) {
          $privileges |= ADMINISTRATOR_PRIVILEGE;
        }
        if ($_POST['whitelist']) {
          $privileges |= WHITELIST_PRIVILEGE;
        }
        if ($_POST['ipOwnersList']) {
          $privileges |= IP_OWNERS_PRIVILEGE;
        }
        if ($_POST['infectedIPsList']) {
          $privileges |= INFECTED_IPS_PRIVILEGE;
        }
        if ($_POST['hbfQueries']) {
          $privileges |= HBF_QUERY_PRIVILEGE;
        }
        if (!@pg_query($postgreSQL, 'INSERT INTO "AccessControl"."users" VALUES (\'' .
                                    pg_escape_string($postgreSQL, $_POST['newUsername']) . '\', \'' .
                                    hash('sha256', $_POST['newPassword']) . '\', \'' .
                                    $privileges . '\', \'' .
                                    $active . '\', NULL)')) {
          message('<b>PostgreSQL:</b> ' . pg_last_error($postgreSQL), false);
        }
        else {
          message('Added user <b>' . $_POST['newUsername'] . '</b>', false);
          unset($_POST);
        }
      }
    }
  }

  if ($_POST['delete']) {
    foreach ($_POST as $key => &$value) {
      if (substr($key, 0, 7) == 'delete_') {
        $usernames[] = substr($key, 7);
      }
    }
    if ($usernames) {
      foreach ($usernames as &$username) {
        if (!@pg_query($postgreSQL, 'DELETE FROM "AccessControl"."users" WHERE "name" = \'' .
                                    $username . '\'')) {
          message('<b>PostgreSQL: </b>' . pg_last_error($postgreSQL), false);
          break;
        }
        $username = '<b>' .
                      $username .
                    '</b>';
      }
      if (count($usernames) == 1) {
        message('Deleted user ' . $usernames[0], false);
      }
      else {
        message('Deleted users ' . implode(', ', $usernames), false);
      }
    }
  }

  echo '<div class="table">' .
         '<form method="post" action=".">' .
           '<table cellspacing="1">' .
             '<tr>' .
               '<td class="columnTitle center" colspan="2">' .
                 'New User' .
               '</td>' .
             '</tr>' .
             '<tr class="even">' .
               '<td>' .
                 '<table>' .
                   '<tr>' .
                     '<td>' .
                       'Username:' .
                     '</td>' .
                     '<td>' .
                       '<input type="text" name="newUsername" value="' . $_POST['newUsername'] . '" />' .
                     '</td>' .
                   '</tr>' .
                   '<tr>' .
                     '<td>' .
                       'Password:' .
                     '</td>' .
                     '<td>' .
                       '<input type="password" name="newPassword" value="' . $_POST['newPassword'] . '" />' .
                     '</td>' .
                   '</tr>' .
                   '<tr>' .
                     '<td>' .
                     '</td>' .
                     '<td class="center">' .
                       '<input type="checkbox" name="active" ' . checked($_POST['active'], true) . ' />' .
                       'Active' .
                     '</td>' .
                   '</tr>' .
                 '</table>' .
               '</td>' .
               '<td>' .
                 '<table>' .
                   '<tr>' .
                     '<td>' .
                       '<input type="checkbox" name="administrator" ' . checked($_POST['administrator'], false) . ' />' .
                       'Administrator' .
                     '</td>' .
                     '<td>' .
                       '<input type="checkbox" name="whitelist" ' . checked($_POST['whitelist'], false) . ' />' .
                       'Whitelist Write Access' .
                     '</td>' .
                   '</tr>' .
                   '<tr>' .
                     '<td>' .
                       '<input type="checkbox" name="ipOwnersList" ' . checked($_POST['ipOwnersList'], false) . ' />' .
                       'IP Owners List Write Access' .
                     '</td>' .
                     '<td>' .
                       '<input type="checkbox" name="infectedIPsList" ' . checked($_POST['infectedIPsList'], false) . ' />' .
                       'Infected IPs List Write Access' .
                     '</td>' .
                   '</tr>' .
                   '<tr>' .
                     '<td>' .
                       '<input type="checkbox" name="hbfQueries" ' . checked($_POST['hbfQueries'], false) . ' />' .
                       'HBF Queries' .
                     '</td>' .
                   '</tr>' .
                 '</table>' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td class="top">' .
                 '<input type="submit" value="Add" name="add" />' .
               '</td>' .
             '</tr>' .
           '</table>' .
         '</form>' .
       '</div>';

  echo '<div class="table">' .
         '<form method="post" action=".">' .
           '<table width="100%" cellspacing="1">' .
             '<tr>' .
               '<td class="columnTitle center">' .
                 'Username' .
               '</td>' .
               '<td class="columnTitle center">' .
                 'Last Login Time' .
               '</td>' .
               '<td class="columnTitle center">' .
                 'Privileges' .
               '</td>' .
               '<td class="columnTitle center">' .
                 'Active' .
               '</td>' .
               '<td class="columnTitle center">' .
                 'Delete' .
               '</td>' .
            '</tr>';
  $result = pg_query($postgreSQL, 'SELECT "name", "privileges", "active", "lastUsed" FROM "AccessControl"."users"');
  while ($row = pg_fetch_assoc($result)) {
    $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
    echo '<tr class="' . $rowClass . '">' .
           '<td class="center">' .
             '<a class="text" href="modify/' . $row['name'] . '">' .
               $row['name'] .
             '</a>' .
           '</td>' .
           '<td class="center">';
    if ($row['lastUsed']) {
      echo date("l, F j, Y \a\\t g:i:s A", $row['lastUsed']);
    };
    echo  '</td>' .
           '<td class="center">' .
             privileges($row['privileges']) .
           '</td>' .
           '<td class="center">' .
             active($row['active']) .
           '</td>' .             
           '<td class="center" width="10%">' .
             '<input type="checkbox" name="delete_' . $row['name'] . '" />' .
           '</td>' .
         '</tr>';
    ++$rowNumber;
  }
  echo '<tr>' .
         '<td class="top">' .
           '<input type="submit" value="Delete" name="delete" />';
         '</td>' .
       '</tr>' .
     '</table>' .
   '</form>' .
 '</div>';
?>
