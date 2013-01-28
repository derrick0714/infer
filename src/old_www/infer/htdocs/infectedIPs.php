<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  $title = 'Infected IPs List';
  include('include/header.php');
  $result = pg_query('SELECT "ip", "time", "name", "reason", "lastSeenTime", "asNumber", "countryNumber" ' .
                     'FROM "Maps"."infectedIPs"');
  while ($row = pg_fetch_row($result)) {
    $infectedIPs[$row[0]]['time'] = $row[1];
    $infectedIPs[$row[0]]['name'] = $row[2];
    $infectedIPs[$row[0]]['reason'] = $row[3];
    $infectedIPs[$row[0]]['lastSeenTime'] = $row[4];
    $infectedIPs[$row[0]]['asNumber'] = $row[5];
    $infectedIPs[$row[0]]['countryNumber'] = $row[6];
    $infectedIPIndex[$row[0]][] = $row[4];
  }
  if ($_POST['add'] && $_POST['ips'] && (getPrivileges($postgreSQL, $_COOKIE['imsSessionID']) & INFECTED_IPS_PRIVILEGE ||
                                         getPrivileges($postgreSQL, $_COOKIE['imsSessionID']) & ADMINISTRATOR_PRIVILEGE)) {
    foreach (preg_split('(\s+)', trim($_POST['ips'])) as $ip) {
      $ip = sprintf('%u', ip2long($ip));
      if (!$infectedIPs || !array_key_exists($ip, $infectedIPs)) {
        $infectedIPs[$ip]['time'] = time();
        $infectedIPs[$ip]['name'] = getUsername($postgreSQL, $_COOKIE['imsSessionID']);
        $infectedIPs[$ip]['reason'] = $_POST['reason'];
        $infectedIPs[$ip]['lastSeenTime'] = NULL;
        $infectedIPs[$ip]['asNumber'] = getASNByIP($postgreSQL, $ip);
        $infectedIPs[$ip]['countryNumber'] = getCountryNumberByIP($postgreSQL, $ip);
        if ($_POST['reason']) {
          $reason = '\'' . pg_escape_string($postgreSQL, $_POST['reason']) . '\'';
        }
        else {
          $reason = 'NULL';
        }
        pg_query($postgreSQL, 'INSERT INTO "Maps"."infectedIPs" VALUES (\'' . $ip . '\', \'' .
                 $infectedIPs[$ip]['time'] . '\', \'' . pg_escape_string($postgreSQL, $infectedIPs[$ip]['name']) . '\', ' .
                 $reason . ', NULL, \'' . $infectedIPs[$ip]['asNumber'] . '\', ' . $infectedIPs[$ip]['countryNumber'] . ')');
        $infectedIPIndex[$ip][] = $infectedIPs[$ip]['lastSeenTime'];
        message('Added IP <b>' . long2ip($ip) . '</b>.', false);
      }
      else {
        message('IP ' . long2ip($ip) . ' exists', true);
      }
    }
  }
  if ($_POST['delete'] && (getPrivileges($postgreSQL, $_COOKIE['imsSessionID']) & INFECTED_IPS_PRIVILEGE ||
                           getPrivileges($postgreSQL, $_COOKIE['imsSessionID']) & ADMINISTRATOR_PRIVILEGE)) {
    foreach ($_POST as $postVariable => &$value) {
      $ip = substr($postVariable, 7);
      if (substr($postVariable, 0, 7) == 'delete_' && array_key_exists($ip, $infectedIPs)) {
        unset($infectedIPs[$ip]);
        pg_query('DELETE FROM "Maps"."infectedIPs" WHERE "ip" = \'' . $ip . '\'');
      }
    }
  }
  if ($infectedIPIndex) {
    arsort($infectedIPIndex);
  }
?>
<div class="table">
  <form method="post" action="/infectedIPs">
    <table>
      <tr>
        <td class="input_title">
          IPs
        </td>
        <td>
          <textarea name="ips" cols="50" rows="5" wrap="physical"><?php echo $_POST['ips'] ?></textarea>
        </td>
      </tr>
      <tr>
        <td class="input_title">
          Reason (optional)
        </td>
        <td>
          <input type="text" name="reason" size="52" value="<?php echo $_POST['reason'] ?>" />
        </td>
      </tr>
      <tr>
        <td>
          <input type="submit" value="Add" name="add" />
        </td>
      </tr>
    </table>
  </div>
  <?php
    if ($infectedIPs) {
      getCountryNameMap($postgreSQL, $countryCodeMap, $countryNameMap);
      echo '<div class="table">' .
             '<table width="100%" cellspacing="1">' .
               '<tr>' .
                 '<td class="columnTitle center">' .
                   'IP' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Autonomous System' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Region' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Added at' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Added by' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Reason' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Last Seen at' .
                 '</td>' .
                 '<td class="columnTitle center">' .
                   'Delete' .
                 '</td>' .
               '</tr>';
      foreach ($infectedIPIndex as $infectedIP => &$lastSeenTime) {
        $infectedIPStats = $infectedIPs[$infectedIP];
        $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
        echo '<tr class="' . $rowClass . '">' .
               '<td class="center">' .
                 '<a name="' . long2ip($infectedIP) . '"></a>' .
                   long2ip($infectedIP) .
                 '</td>' .
                 '<td class="center">';
          if (!isInternal($infectedIP)) {
            echo getASDescriptionByNumber($postgreSQL, $infectedIPStats['asNumber']);
          }
          echo '</td>' .
               '<td class="center">';
          if (!isInternal($infectedIP) && $infectedIPStats['countryNumber'] != 0) {
            echo getCountryPicture($infectedIPStats['countryNumber'], $countryCodeMap, $countryNameMap);
          }
          echo '</td>' .
               '<td class="center">' .
                 date("l, F j, Y \a\\t g:i:s A", $infectedIPStats['time']) .
                 ' (' . duration(time() - $infectedIPStats['time']) . ' ago)' .
               '</td>' .
               '<td class="center">' .
                 $infectedIPStats['name'] .
               '</td>' .
               '<td class="center">' .
                 $infectedIPStats['reason'] .
               '</td>' .
               '<td class="center">';
                 if ($infectedIPStats['lastSeenTime']) {
                   echo date("l, F j, Y \a\\t g:i:s A", $infectedIPStats['lastSeenTime']) .
                        ' (' . duration(time() - $infectedIPStats['lastSeenTime']) . ' ago)';
                 }
          echo '</td>' .
               '<td class="center">' .
                 '<input type="checkbox" name="delete_' . $infectedIP . '" />' .
               '</td>' .
             '</tr>';
          ++$rowNumber;
        }
        echo '<tr>' .
               '<td class="top">' .
                 '<input type="submit" value="Delete" name="delete" />' .
               '</td>' .
             '</tr>' .
           '</table>';
      }
    ?>
    </form>
  </body>
</html>
