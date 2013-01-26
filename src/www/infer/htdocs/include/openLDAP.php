<?php
  $organizationName = 'Polytechnic University';
  $openLDAPRootCN = 'Manager';
  $openLDAPRootPassword = 'infection';
  $openLDAP = ldap_connect('127.0.0.1');
  ldap_set_option($openLDAP, LDAP_OPT_PROTOCOL_VERSION, 3);
  ldap_bind($openLDAP, 'cn=' . $openLDAPRootCN . ', o=' . $organizationName,
            $openLDAPRootPassword);
?>
