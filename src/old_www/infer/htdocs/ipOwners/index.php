<?php
  include('include/postgreSQL.php');
  include('include/openLDAP.php');  
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  $title = 'IP Ownership Information';
  include('include/header.php');
  message('Asterisks may be used as wildcards. Leaving all fields blank will display all entries.', false);
  echo '<div class="table">' .
         '<form method="post" action="./index.php">' .
           '<table width="40%">' .
             '<tr>' .
               '<td>' .
                 '<b>IP: (<a class="text" href="add.php">add new IP</a>)</b>' .
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
                 '<b>Owner\'s first name:</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="gn" value="' . $_POST['gn'] . '" />' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>Owner\'s last name:</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="sn" value="' . $_POST['sn'] . '" />' .
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
             '</tr>' .
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
                 '<input type="submit" value="Search" name="search" />' .
               '</td>' .
             '</tr>' .
           '</table>' .
         '</div>';
  $searchableFields = array('cn', 'macAddress', 'gn', 'sn', 'roomNumber',
                            'departmentNumber', 'buildingName');
  if ($_POST['search'] || $_POST['delete']) {
    if ($_POST['delete']) {
      foreach($_POST as $postVariable => &$value) {
        if (substr($postVariable, 0, 7) == 'delete_') {
          $delete = true;
          $ip = str_replace('_', '.', substr($postVariable, 7));
          if (@ldap_delete($openLDAP, 'cn=' .$ip . ', o=' .
                                        $organizationName)) {
            echo '<b>' .
                   '<i>' .
                     'Deleted ' . $ip .
                   '</i>' .
                 '</b>';
          }
          else {
            echo '<b>' .
                   '<i>' .
                     'Error deleting ' . $ip . ' (' . ldap_error($openLDAP) .
                     ')' .
                   '</i>' .
                 '</b>';
          }
          echo '<br />';
        }
      }
      if ($delete) {
        echo '<br />';
      }
    }
    foreach ($searchableFields as &$searchableField) {
      if (strlen($_POST[$searchableField])) {
        $searchFilter .= '(' . $searchableField . '=' .
                         $_POST[$searchableField] . ')';
      }
    }
    if ($searchFilter) {
      $searchFilter = '(|' . $searchFilter . ')';
    }
    else {
      $searchFilter = 'cn=*';
    }
    $result = @ldap_get_entries($openLDAP, @ldap_search($openLDAP,
                                                        'o=' .
                                                          $organizationName,
                                                        $searchFilter));
    if (!$result['count']) {
      message('No matches found.', false);
    }
    else {
      echo '<div class="table">' .
             '<table width="100%" cellspacing="1">' .
               '<tr>' .
                 '<td class="columnTitle center">' .
                   'IP' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'MAC address' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Description' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Owner' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Title' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'E-mail' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Phone' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Room' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Department' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Building' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Delete' .
                 '</td>' .
               '</tr>';
      for ($entryNumber = 0; $entryNumber < $result['count']; ++$entryNumber) {
        $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
        echo '<tr class="' . $rowClass . '">' .
               '<td class="center">' .
                 $result[$entryNumber]['cn'][0] .
               '</td>' .
               '<td class="center">' .
                 $result[$entryNumber]['macaddress'][0] .
               '</td>' .
               '<td class="center">' .
                 $result[$entryNumber]['description'][0] .
               '</td>' .
               '<td class="center">' .
                 $result[$entryNumber]['givenname'][0] . ' ' .
                 $result[$entryNumber]['sn'][0] .
               '</td>' .
               '<td class="center">' .
                 $result[$entryNumber]['personaltitle'][0] .
               '</td>' .
               '<td class="center">' .
                 $result[$entryNumber]['mail'][0] .
               '</td>' .
               '<td class="center">' .
                 $result[$entryNumber]['telephonenumber'][0] .
               '</td>' .
               '<td class="center">' .
                 $result[$entryNumber]['roomnumber'][0] .
               '</td>' .
               '<td class="center">' .
                 $result[$entryNumber]['departmentnumber'][0] .
               '</td>' .
               '<td class="center">' .
                 $result[$entryNumber]['buildingname'][0] .
               '</td>' .
               '<td class="center">' .
                 '<input type="checkbox" name="delete_' .
                 str_replace('.', '_', $result[$entryNumber]['cn'][0]) . '" />' .
               '</td>' .
             '</tr>';
        ++$rowNumber;
      }
      echo '<tr>' .
             '<td class="top">' .
               '<input type="submit" value="Delete" name="delete" />' .
             '</td>' .
           '</tr>' .
         '</table>' .
       '</div>';
    }
  }
?>
</form>
</body>
</html>
