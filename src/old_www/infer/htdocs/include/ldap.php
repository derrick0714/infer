<?php
  require_once('postgreSQL.php');

  function checkForNull(&$postgreSQL, &$text) {
    if ($text === NULL) {
      return 'NULL';
    }
    return '\'' . @pg_escape_string($postgreSQL, $text) . '\'';
  }

  function getLDAPInformationByIP(&$postgreSQL, &$ip) {
    /* Gets LDAP configuration from PostgreSQL. */
    $ldapConfiguration = getPGRow($postgreSQL, 'Maps', 'ldapConfiguration');
    if (!$ldapConfiguration) {
      return false;
    }
    $time = time();
    /* Deletes any cached LDAP records whose TTL has expired. */
    if (!pg_query($postgreSQL, 'DELETE FROM "Maps"."ldapCache" WHERE "time" <= \'' . ($time - $ldapConfiguration['cacheTTL']) . '\'')) {
      return false;
    }
    /*
     * Checks whether the record we're looking for is cached, and returns it
     * if it is.
     */
    $result = pg_query($postgreSQL, 'SELECT * FROM "Maps"."ldapCache" WHERE "ip" = \'' . $ip . '\'');
    if (pg_num_rows($result)) {
      $row = pg_fetch_assoc($result);
      return $row;
    }
    /*
     * Otherwise, connects to the LDAP server, searches for the record, and, if
     * it exists, caches it and returns it.
     */
    $ldap = @ldap_connect($ldapConfiguration['serverURL'] . ':' . $ldapConfiguration['port']);
    if (!$ldap) {
      return false;
    }
    if (!@ldap_set_option($ldap, LDAP_OPT_PROTOCOL_VERSION, 3)) {
      return false;
    }
    if (!@ldap_bind($ldap, $ldapConfiguration['relativeDistinguishedName'], $ldapConfiguration['password'])) {
      return false;
    }
    $result = ldap_get_entries($ldap, ldap_search($ldap, $ldapConfiguration['distinguishedName'],
                                             $ldapConfiguration['ipField'] . '=' . $ip));
    if (!$result['count']) {
      return false;
    }
    $ldapFields = array('ip' => $ldapConfiguration['ipField'],
                        'hostname' => $ldapConfiguration['hostnameField'],
                        'mac' => $ldapConfiguration['macField'],
                        'email' => $ldapConfiguration['emailField'],
                        'firstName' => $ldapConfiguration['firstNameField'],
                        'lastName' => $ldapConfiguration['lastNameField'],
                        'title' => $ldapConfiguration['titleField'],
                        'department' => $ldapConfiguration['departmentField'],
                        'location' => $ldapConfiguration['locationField'],
                        'phoneNumber' => $ldapConfiguration['phoneNumberField']);
    foreach ($ldapFields as $internalField => &$externalField) {
      if ($result[0][$externalField][0] !== NULL) {
        $ldapInformation[$internalField] = $result[0][$externalField][0];
      }
    }
    pg_query($postgreSQL, 'INSERT INTO "Maps"."ldapCache" VALUES (\'' . @pg_escape_string($postgreSQL, $ldapInformation['ip']) . '\', ' .
                                                                        checkForNull($postgreSQL, $ldapInformation['hostname']) . ', ' .
                                                                        checkForNull($postgreSQL, $ldapInformation['mac']) . ', ' .
                                                                        checkForNull($postgreSQL, $ldapInformation['email']) . ', ' .
                                                                        checkForNull($postgreSQL, $ldapInformation['firstName']) . ', ' .
                                                                        checkForNull($postgreSQL, $ldapInformation['lastName']) . ', ' .
                                                                        checkForNull($postgreSQL, $ldapInformation['title']) . ', ' .
                                                                        checkForNull($postgreSQL, $ldapInformation['location']) . ', ' .
                                                                        checkForNull($postgreSQL, $ldapInformation['department']) . ', ' .
                                                                        checkForNull($postgreSQL, $ldapInformation['phoneNumber']) . ', \'' .
                                                                        @pg_escape_string($postgreSQL, $time) . '\')');
    return $ldapInformation;
  }
?>
