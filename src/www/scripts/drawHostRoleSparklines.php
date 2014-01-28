#!/usr/local/bin/php
<?php
  $imsHome = dirname(__FILE__) . '/..';

  include('include/shared.php');
  include($imsHome . '/htdocs/include/postgreSQL.php');
  include($imsHome . '/htdocs/include/roles.php');
  include('/usr/local/share/sparkline/lib/Sparkline_Bar.php');

  if (!file_exists($imsHome . '/htdocs/images/sparklines/roles/' . $argv[1])) {
    mkdir($imsHome . '/htdocs/images/sparklines/roles/' . $argv[1]);
  }

  function getMonitoredServices(&$postgreSQL) {
    $result = pg_query($postgreSQL, 'SELECT * FROM "Maps"."monitoredServices"');
    while ($row = pg_fetch_assoc($result)) {
      $monitoredServices[$row['name']]['ports'] = explode(',', substr($row['ports'], 1, -1));
      $monitoredServices[$row['name']]['initiator'] = $row['initiator'];
    }
    return $monitoredServices;
  }

  function getInterestingPortsIndex(&$postgreSQL, &$date) {
    $result = pg_query($postgreSQL, 'SELECT "interestingPorts" FROM "Indexes"."interestingPorts" WHERE "date" = \'' . $date . '\'');
    if (pg_num_rows($result)) {
      $index = 0;
      $row = pg_fetch_assoc($result);
      foreach (explode(',', substr($row['interestingPorts'], 1, -1)) as $port) {
        $interestingPortsIndex[$port] = $index++;
      }
      return $interestingPortsIndex;
    }
    return false;
  }

  function arraySum(&$interestingPortsIndex, &$array, &$indexes) {
    foreach ($indexes as &$index) {
      $sum += $array[$interestingPortsIndex[$index]];
    }
    return $sum;
  }

  $monitoredServices = getMonitoredServices($postgreSQL);
  $dates = array_slice(getPGTableRange($postgreSQL, 'LiveIPs', getFirstPGTable($postgreSQL, 'LiveIPs'), $argv[1]), -14);
  $interestingPortsIndex = getInterestingPortsIndex($postgreSQL, $argv[1]);
  foreach (getInterestingIPs($postgreSQL, $argv[1]) as $ip) {
    echo long2ip($ip) . "\n";
    if (!file_exists($imsHome . '/htdocs/images/sparklines/roles/' . $argv[1] . '/' . long2ip($ip))) {
      mkdir($imsHome . '/htdocs/images/sparklines/roles/' . $argv[1] . '/' . long2ip($ip));
    }
    drawServicesSparkline($postgreSQL, $imsHome, $monitoredServices, $interestingPortsIndex, $dates, $ip);
  }

  if (existsPGTable($postgreSQL, 'Roles', $argv[1])) {
    $result = pg_query($postgreSQL, 'SELECT DISTINCT "role" FROM "Roles"."' . $argv[1] . '"');
    while ($row = pg_fetch_assoc($result)) {
      $roles[] = $row['role'];
    }
    $result = pg_query($postgreSQL, 'SELECT DISTINCT "ip" FROM "Roles"."' . $argv[1] . '"');
    while ($row = pg_fetch_assoc($result)) {
       echo long2ip($row['ip']) . "\n";
       drawRoleSparkLine($postgreSQL, $imsHome, $dates, $row['ip'], $roles, $roleNames);
    }
  }

  function drawServicesSparkline(&$postgreSQL, &$imsHome, &$monitoredServices, &$interestingPortsIndex, &$dates, &$ip) {
    foreach ($dates as &$date) {
      $result = pg_query($postgreSQL, 'SELECT "inboundPortIPs", "outboundPortIPs" FROM "HostTraffic"."' . $date . '" ' .
                                      'WHERE "ip" = \'' . $ip . '\'');
      if (pg_num_rows($result) > 0) {
        $row = pg_fetch_assoc($result);
        foreach ($monitoredServices as $monitoredServiceName => &$monitoredService) {
          if ($monitoredService['initiator'] == 1) {
            $data[$monitoredServiceName][] = arraySum($interestingPortsIndex, explode(',', substr($row['outboundPortIPs'], 1, -1)), $monitoredService['ports']);
          }
          else {
            $data[$monitoredServiceName][] = arraySum($interestingPortsIndex, explode(',', substr($row['inboundPortIPs'], 1, -1)), $monitoredService['ports']);
          }
        }
      }
      else {
        foreach ($monitoredServices as $monitoredServiceName => &$monitoredService) {
          $data[$monitoredServiceName][] = 0;
        }
      }
    }
    foreach ($data as $monitoredServiceName => &$ports) {
      if (array_sum($ports)) {
        $graph = new Sparkline_Bar();
        $graph -> setBarWidth(3);
        $graph -> setBarSpacing(1);
        $graph -> setColorHtml('purple', '#660000');
        foreach ($ports as $key => &$value) {
          $graph -> setData($key, $value, 'purple');
        }
        $graph -> render(10);
        $graph -> output($imsHome . '/htdocs/images/sparklines/roles/' .
                         $dates[count($dates) - 1] . '/' . long2ip($ip) . '/' . str_replace(' ', '', $monitoredServiceName) . '.png');
      }
    }
  }

  function drawRoleSparkLine(&$postgreSQL, &$imsHome, &$dates, &$ip, &$roles, &$roleNames) {
    foreach ($dates as &$date) {
      $result = pg_query($postgreSQL, 'SELECT "role", "numHosts" FROM "Roles"."' . $date . '" ' .
                                      'WHERE "ip" = \'' . $ip . '\'');
      while ($row = pg_fetch_assoc($result)) {
        $data[$row['role']][] = $row['numHosts'];
      }
      foreach ($roles as &$role) {
        if (!$data[$role][count($data[$role]) - 1]) {
        $data[$role][] = 0;
      }
    }
  }
  foreach ($data as $role => &$numHosts) {
    if (array_sum($numHosts)) {
      $graph = new Sparkline_Bar();
      $graph -> setBarWidth(3);
      $graph -> setBarSpacing(1);
      $graph -> setColorHtml('purple', '#660000');
      foreach ($numHosts as $key => &$value) {
        $graph -> setData($key, $value, 'purple');
      }
      $graph -> render(10);
      $graph -> output($imsHome . '/htdocs/images/sparklines/roles/' .
                       $dates[count($dates) - 1] . '/' . long2ip($ip) . '/' . str_replace(' ', '', $roleNames[$role]) . '.png');
      }
    }
  }
?>
