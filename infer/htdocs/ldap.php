<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  requirePrivilege($postgreSQL, ADMINISTRATOR_PRIVILEGE);
  $title = 'LDAP Configuration';
  include('include/header.php');

  $ldapFields = array(0 => 'serverURL',
                      1 => 'port',
                      2 => 'relativeDistinguishedName',
                      3 => 'password',
                      4 => 'distinguishedName',
                      5 => 'ipField',
                      6 => 'hostnameField',
                      7 => 'macField',
                      8 => 'emailField',
                      9 => 'firstNameField',
                      10 => 'lastNameField',
                      11 => 'titleField',
                      12 => 'departmentField',
                      13 => 'locationField',
                      14 => 'phoneNumberField',
                      15 => 'cacheTTL');

  function textInput(&$postValue, &$sqlValue, $defaultValue = NULL) {
    if ($postValue !== NULL) {
      return 'value="' . $postValue . '"';
    }
    else {
      if ($sqlValue !== NULL) {
        return 'value="' . $sqlValue . '"';
      }
      else {
        if ($defaultValue !== NULL) {
          return 'value="' . $defaultValue . '"';
        }
      }
    }
    return NULL;
  }

  if ($_POST['submit']) {
    foreach ($ldapFields as &$ldapField) {
      if (!strlen($_POST[$ldapField])) {
        message('all fields are mandatory.', true);
        $submit = false;
        break;
      }
    }
    if ($submit !== false) {
      if (!@pg_query($postgreSQL, 'DELETE FROM "Maps"."ldapConfiguration"') ||
          !@pg_query($postgreSQL, 'INSERT INTO "Maps"."ldapConfiguration" VALUES (\'' . $_POST['serverURL'] . '\', \'' .
                                                                                        $_POST['port'] . '\', \'' .
                                                                                        $_POST['relativeDistinguishedName'] . '\', \'' .
                                                                                        $_POST['password'] . '\', \'' .
                                                                                        $_POST['distinguishedName'] . '\', \'' .
                                                                                        $_POST['ipField'] . '\', \'' .
                                                                                        $_POST['macField'] . '\', \'' .
                                                                                        $_POST['hostnameField'] . '\', \'' .
                                                                                        $_POST['emailField'] . '\', \'' .
                                                                                        $_POST['departmentField'] . '\', \'' .
                                                                                        $_POST['firstNameField'] . '\', \'' .
                                                                                        $_POST['lastNameField'] . '\', \'' .
                                                                                        $_POST['titleField'] . '\', \'' .
                                                                                        $_POST['locationField'] . '\', \'' .
                                                                                        $_POST['phoneNumberField'] . '\', \'' .
                                                                                        $_POST['cacheTTL'] . '\')')) {
        message(@pg_last_error($postgreSQL), true);
      }
      else {
        if ($_POST['serverURL'] != 'ldap://' && $_POST['serverURL'] != 'ldaps://') {
          while (substr($_POST['serverURL'], -1, 1) == '/') {
            $_POST['serverURL'] = substr($_POST['serverURL'], 0, -1);
          }
        }
        $ldap = @ldap_connect($_POST['serverURL'] . ':' . $_POST['port']);
        if (!$ldap || !@ldap_set_option($ldap, LDAP_OPT_PROTOCOL_VERSION, 3) ||
            !@ldap_bind($ldap, $_POST['relativeDistinguishedName'], $_POST['password'])) {
          message('LDAP configuration saved, but a login attempt to the LDAP server failed (' . @ldap_error($ldap) . ').', true);
        }
        else {
          message('LDAP configuration saved, and a login attempt to the LDAP server was successful.');
        }
      }
    }
  }

  echo '<div class="table">' .
         '<form method="post" action="/ldap">' .
           '<table>' .
             '<tr>' .
               '<td>' .
                 '<b>' .
                   'Server URL' .
                 '</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="serverURL" ' . textInput($_POST['serverURL'],
                                                                              getPGValue($postgreSQL, 'Maps', 'ldapConfiguration', 'serverURL'), 'ldap://') . ' >' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>' .
                   'Port' .
                 '</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="port" ' . textInput($_POST['port'],
                                                                         getPGValue($postgreSQL, 'Maps', 'ldapConfiguration', 'port'), '389') . ' >' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>' .
                   'Relative Distinguished Name' .
                 '</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="relativeDistinguishedName" ' . textInput($_POST['relativeDistinguishedName'],
                                                                                              getPGValue($postgreSQL, 'Maps', 'ldapConfiguration',
                                                                                                                      'relativeDistinguishedName')) . ' >' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>' .
                   'Password' .
                 '</br>' .
               '</td>' .
               '<td>' .
                 '<input type="password" size="30" name="password" ' . textInput($_POST['password'],
                                                                                 getPGValue($postgreSQL, 'Maps', 'ldapConfiguration', 'password')) . ' >' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>' .
                   'Distinguished Name' .
                 '</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="distinguishedName" ' . textInput($_POST['distinguishedName'],
                                                                                      getPGValue($postgreSQL, 'Maps', 'ldapConfiguration',
                                                                                                 'distinguishedName')) . '>' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>' .
                   'IP Address Field' .
                 '</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="ipField" ' . textInput($_POST['ipField'],
                                                                            getPGValue($postgreSQL, 'Maps' ,'ldapConfiguration', 'ipField')) . ' >' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>' .
                   'Hostname Field' .
                 '</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="hostnameField" ' . textInput($_POST['hostnameField'],
                                                                                  getPGValue($postgreSQL, 'Maps', 'ldapConfiguration', 'hostnameField')) . ' >' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>' .
                   'MAC Address Field' .
                 '</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="macField" ' . textInput($_POST['macField'],
                                                                             getPGValue($postgreSQL, 'Maps', 'ldapConfiguration', 'macField')) . ' >' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>' .
                   'E-mail Address Field' .
                 '</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="emailField" ' . textInput($_POST['emailField'],
                                                                               getPGValue($postgreSQL, 'Maps', 'ldapConfiguration', 'emailField'),
                                                                               'mail') . ' >' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>' .
                   'First Name Field' .
                 '</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="firstNameField" ' . textInput($_POST['firstNameField'],
                                                                                   getPGValue($postgreSQL, 'Maps', 'ldapConfiguration', 'firstNameField'),
                                                                                   'givenname') . ' >' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>' .
                   'Last Name Field' .
                 '</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="lastNameField" ' . textInput($_POST['lastNameField'],
                                                                                  getPGValue($postgreSQL, 'Maps', 'ldapConfiguration', 'lastNameField'),
                                                                                  'sn') . ' >' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>' .
                   'Title Field' .
                 '</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="titleField" ' . textInput($_POST['titleField'],
                                                                               getPGValue($postgreSQL, 'Maps', 'ldapConfiguration', 'titleField'),
                                                                               'title') . ' >' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>' .
                   'Department Name Field' .
                 '</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="departmentField" ' . textInput($_POST['departmentField'],
                                                                                    getPGValue($postgreSQL, 'Maps', 'ldapConfiguration', 'departmentField'),
                                                                                    'department') . ' >' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>' .
                   'Location Field' .
                 '</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="locationField" ' . textInput($_POST['locationField'],
                                                                                  getPGValue($postgreSQL, 'Maps', 'ldapConfiguration', 'locationField'),
                                                                                  'physicaldeliveryofficeName') . ' >' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>' .
                   'Phone Number Field' .
                 '</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="phoneNumberField" ' . textInput($_POST['phoneNumberField'],
                                                                                     getPGValue($postgreSQL, 'Maps', 'ldapConfiguration',
                                                                                                'phoneNumberField'), 'telephonenumber') . ' >' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<b>' .
                   'Cache TTL' .
                 '</b>' .
               '</td>' .
               '<td>' .
                 '<input type="text" size="30" name="cacheTTL" ' . textInput($_POST['cacheTTL'],
                                                                             getPGValue($postgreSQL, 'Maps', 'ldapConfiguration', 'cacheTTL'), '300') . ' >' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<input type="submit" value="Submit" name="submit" />' .
               '</td>' .
             '</tr>' .
           '</table>' .
         '</form>' .
       '</div>';
  include('include/footer.html');
?>
