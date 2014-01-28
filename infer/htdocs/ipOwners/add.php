<?php
  include('include/postgreSQL.php');
  include('include/openLDAP.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  requirePrivilege($postgreSQL, IP_OWNERS_PRIVILEGE);
  $title = 'IP Ownership Information';
  include('include/header.php');
  if (!$_POST['add']) {
    message('Required fields are in italics.', false);
  }
  else {
    $fieldTitles = array('cn' => 'IP', 'gn' => 'Owner\'s first name',
                         'sn' => 'Owner\'s last name');
    $allowedFields = array('cn', 'description', 'gn', 'sn', 'personalTitle',
                           'macAddress', 'mail', 'telephoneNumber',
                           'roomNumber', 'departmentNumber', 'buildingName');
    $requiredFields = array('cn', 'sn', 'gn');
    foreach ($requiredFields as &$requiredField) {
      if (!strlen($_POST[$requiredField])) {
        message('<b>' . $fieldTitles[$requiredField] . '</b>' . ' is a required field.', true);
        $error = true;
      }
      else {
        $newEntry['objectclass'] = 'imsIP';
        $newEntry['o'] = $organizationName;
        foreach ($allowedFields as &$allowedField) {
          if (strlen($_POST[$allowedField])) {
            $newEntry[$allowedField] = $_POST[$allowedField];
          }
        }
      }
    }
    if (!$error) {
      if (@ldap_add($openLDAP, 'cn=' . $newEntry['cn'] . ', o=' .
                                $organizationName, $newEntry)) {
        message('Added <b>' . $newEntry['cn'] . '</b>.', false);
      }
      else {
        message('<b>LDAP:</b> ' . ldap_error($openLDAP), false);
      }
    }
  }
  echo '<div class="table">' .
         '<form method="post" action="./add.php">' .
           '<table width="40%" cellspacing="1">' .
             '<tr>' .
               '<td>' .
                 '<b><i>IP: (<a class="text" href=".">seach for IP</a>)</i></b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="cn" value="' . $_POST['cn'] . '" />' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>MAC address:</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="macAddress" value="' . $_POST['macAddress'] . '" />' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>Description:</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="description" value="' . $_POST['description'] . '" />' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b><i>Owner\'s first name:</i></b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="gn" value="' . $_POST['gn'] . '" />' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b><i>Owner\'s last name:</i></b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="sn" value="' . $_POST['sn'] . '" />' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>Title:</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="personalTitle" value="' . $_POST['personalTitle'] . '" />' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>E-mail:</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="mail" value="' . $_POST['mail'] . '" />' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>Phone:</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="telephoneNumber" value="' . $_POST['telephoneNumber'] . '" />' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>Room:</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="roomNumber" value="' . $_POST['roomNumber'] . '" />' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>Department:</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="departmentNumber" value="' . $_POST['departmentNumber'] . '" />' .
               '</td>' .
             '</td>' .
             '<tr>' .
               '<td>' .
                 '<b>Building:</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="buildingName" value="' . $_POST['buildingName'] . '" />' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<input type="submit" value="Add" name="add" />' .
               '</td>' .
             '</tr>' .
           '</table>' .
         '</form>' .
       '</div>' .
     '</body>' .
   '</html>';
?>
