<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  requirePrivilege($postgreSQL, ADMINISTRATOR_PRIVILEGE);
  $username = substr($_SERVER['PATH_INFO'], strrpos($_SERVER['PATH_INFO'], '/') + 1);
  $title = 'Modify User ' . $username;
  include('include/header.php');

  $result = @pg_query($postgreSQL, 'SELECT "password", "privileges", "active" FROM "AccessControl"."users" ' .
                                   'WHERE "name" = \'' . $username . '\'');
  $row = @pg_fetch_assoc($result);

  if ($_POST['modify']) {
    if (!strlen(trim($_POST['newUsername']))) {
      message('the username field is mandatory', true);
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
      if (!strlen(trim($_POST['newPassword']))) {
        $passwordHash = $row['password'];
      }
      else {
        $passwordHash = hash('sha256', $_POST['newPassword']);
      }
      if (!@pg_query($postgreSQL, 'UPDATE "AccessControl"."users" set "name" = \'' .
                                   pg_escape_string($postgreSQL, $_POST['newUsername']) . '\', ' .
                                   '"password" = \'' . $passwordHash . '\', ' .
                                   '"privileges" = \'' . $privileges . '\', ' .
                                   '"active" = \'' . $active . '\' WHERE "name" = \'' . $username . '\'')) {
        message('<b>PostgreSQL: </b>' . pg_last_error($postgreSQL), false);
      }
      else {
         message('Modified user <b>' . $_POST['newUsername'] . '</b>.', false);
      }
    }
  }

  echo '<div class="table">' .
         '<form method="post" action="./' . $username . '">' .
           '<table cellspacing="1">' .
             '<tr>' .
               '<td class="columnTitle center" colspan="2">' .
                 'Modify User' .
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
                       '<input type="text" name="newUsername" value="' .
                       fillFormField($_POST['modify'], $username, $_POST['newUsername']) . '" />' .
                     '</td>' .
                   '</tr>' .
                   '<tr>' .
                     '<td>' .
                       'Password:' .
                     '</td>' .
                     '<td>' .
                       '<input type="password" name="newPassword" value="' .
                       fillFormField($_POST['modify'], '', $_POST['newPassword']) . '" />' .
                     '</td>' .
                   '</tr>' .
                   '<tr>' .
                     '<td>' .
                     '</td>' .
                     '<td class="center">' .
                       '<input type="checkbox" name="active" ' .
                       fillCheckbox($_POST['modify'], $row['active'] == 't', $_POST['active']) . ' />' .
                       'Active' .
                     '</td>' .
                   '</tr>' .
                 '</table>' .
               '</td>' .
               '<td>' .
                 '<table>' .
                   '<tr>' .
                     '<td>' .
                       '<input type="checkbox" name="administrator" ' .
                       fillCheckbox($_POST['modify'], $row['privileges'] & ADMINISTRATOR_PRIVILEGE, $_POST['administrator']) . ' />' .
                       'Administrator' .
                     '</td>' .
                     '<td>' .
                       '<input type="checkbox" name="whitelist" ' .
                        fillCheckbox($_POST['modify'], $row['privileges'] & WHITELIST_PRIVILEGE, $_POST['whitelist']) . ' />' .
                       'Whitelist Write Access' .
                     '</td>' .
                   '</tr>' .
                   '<tr>' .
                     '<td>' .
                       '<input type="checkbox" name="ipOwnersList" ' .
                       fillCheckbox($_POST['modify'], $row['privileges'] & IP_OWNERS_PRIVILEGE, $_POST['ipOwnersList']) . ' />' .
                       'IP Owners List Write Access' .
                     '</td>' .
                     '<td>' .
                       '<input type="checkbox" name="infectedIPsList" ' .
                       fillCheckbox($_POST['modify'], $row['privileges'] & INFECTED_IPS_PRIVILEGE, $_POST['infectedIPsList']) . ' />' .
                       'Infected IPs List Write Access' .
                     '</td>' .
                   '</tr>' .
                   '<tr>' .
                     '<td>' .
                       '<input type="checkbox" name="hbfQueries" ' .
                       fillCheckbox($_POST['modify'], $row['privileges'] & HBF_QUERY_PRIVILEGE, $_POST['hbfQueries']) . ' />' .
                       'HBF Queries' .
                     '</td>' .
                   '</tr>' .
                 '</table>' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td class="top">' .
                 '<input type="submit" value="Modify" name="modify" />' .
               '</td>' .
             '</tr>' .
           '</table>' .
         '</form>' .
       '</div>';
?>
