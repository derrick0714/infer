<?php
  $symptoms = array('report' => 'InterestingIPs',
                    'serviceIPs' => 'PortIPs',
                    'commChannels' => 'CommChannels',
                    'steppingStones' => 'SteppingStones',
                    'muleContacts' => 'MuleContacts',
                    'infectedContacts' => 'InfectedContacts',
                    'evasiveTraffic' => 'EvasiveTraffic',
                    'nonDNSTraffic' => 'NonDNSTraffic',
                    'darkSpaceSources' => 'DarkSpaceSources',
                    'darkSpaceTargets' => 'DarkSpaceTargets',
                    'malwareSources' => 'MalwareSources',
                    'malwareTargets' => 'MalwareTargets',
                    'bruteForcer' => 'BruteForcers',
                    'bruteForced' => 'BruteForcers',
                    'roles' => 'Roles');

  function appendSymptomURL(&$_url) {
    if ($_url[2]) {
      $url[] = $_url[2];
    }
    if (isset($_url[3])) {
      $url[] = $_url[3];
    }
    if ($url) {
      return '/' . implode('/', $url);
    }
  }

  $isSession = isSession($postgreSQL, $_COOKIE['imsSessionID']);
    $previousDay = "";
	$nextDay = "";
  if (array_key_exists($url[0], $symptoms) && $url[1]) {
    if ($previousTable = getPreviousPGTable($postgreSQL, $symptoms[$url[0]], $url[1])) {
      $previousDay = '<td>' .
                       '<a class="text" href="/' . $url[0] . '/' . $previousTable . appendSymptomURL($url) . '">' .
                         '« Previous Day' .
                       '</a>' .
                     '</td>';
    }
    else {
      $previousDay = '<td>' .
                       '« Previous Day' .
                     '</td>';
    }
    $previousDay .= '<td class="center">' .
                      '|' .
                    '</td>';
    $nextDay = '<td class="center">' .
                 '|' .
               '</td>';
    if ($nextTable = getNextPGTable($postgreSQL, $symptoms[$url[0]], $url[1])) {
      $nextDay .= '<td class="right">' .
                    '<a class="text" href="/' . $url[0] . '/' . $nextTable . appendSymptomURL($url) . '">' .
                      'Next Day »' .
                    '</a>' .
                  '</td>';
    }
    else {
      $nextDay .= '<td class="right">' .
                    'Next Day »' .
                  '</td>';
    }
  }

  if ($url[0] == 'host') {
    $result = pg_query($postgreSQL, 'SELECT "dates" FROM ' .
                       '"Indexes"."interestingIPDates" WHERE "ip" = \'' . ip2long($url[1]) .
                       '\'');
    $row = pg_fetch_assoc($result);
    $dates = explode(',', substr($row['dates'], 1, -1));
    $index = array_search($url[2], $dates);
    if ($index) {
      $previousDay = '<td>' .
                       '<a class="text" href="/host/' . $url[1] . '/' . $dates[$index - 1] . '">' .
                         '« Previous Day' .
                       '</a>' .
                     '</td>';
    }
    else {
      $previousDay = '<td>' .
                       '« Previous Day' .
                     '</td>';
    }
    if ($isSession) {
      $previousDay .= '<td class="center">' .
                        '|' .
                      '</td>';
      $nextDay = '<td class="center">' .
                  '|' .
                 '</td>';
    }
    if ($index < count($dates) - 1) {
      $nextDay .= '<td class="right">' .
                    '<a class="text" href="/host/' . $url[1] . '/' . $dates[$index + 1] . '">' .
                      'Next Day »' .
                    '</a>' .
                  '</td>';
    }
    else {
      $nextDay .= '<td class="right">' .
                    'Next Day »' .
                  '</td>';
    }
  }

  echo '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">'. 
        '<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">'.
         '<head>' .
           '<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">' .
           '<link rel="stylesheet" type="text/css" href="/css/ims.css" />' .
          '<link rel="stylesheet" type="text/css" href="/css/menu.css.boris" />' .
           '<link rel="stylesheet" type="text/css" href="/css/host-page-style.css" />' .
           '<link rel="stylesheet" type="text/css" href="/css/host-page-identity-style.css" />' .
           '<link rel="stylesheet" type="text/css" href="/css/host-page-infection-style.css" />' .
		   '<link rel="stylesheet" type="text/css" href="/css/toggling.css" />' .
           '<link rel="stylesheet" type="text/css" href="/css/host-page-environment-style.css" />' .
           '<link rel="stylesheet" type="text/css" href="/css/tooltip.css" />';
		   if(isset($javascriptInclude))
		     echo $javascriptInclude;
          echo '<title>' .
             $title .
           '</title>' .
         '</head>';
		 if(isset($onLoad))
		     echo '<body onload="'.$onLoad.'"'.'>';
		 else
		     echo '<body>';
		 
		echo 
           '<div id="navbar">' .
             '<table width="100%">' .
               '<tr>' .
                 $previousDay .
                 '</td>';
  if ($isSession) {
    echo '<td class="center">' .
           '<a class="text" href="/">' .
             'Home' .
           '</a>' .
         '</td>' .
         '<td class="center">' .
           '|' .
         '</td>' .
         '<td width="150">' .
           '<div id="menu">' .
             '<ul>' .
               '<li>' .
                 '<div class="root_menu">' .
                   'Settings' .
                 '</div>' .
                 '<ul>';
    if (getPrivileges($postgreSQL, $_COOKIE['imsSessionID']) & ADMINISTRATOR_PRIVILEGE) {
      echo '<li>' .
             '<a href="/users/">' .
               'User Management' .
             '</a>' .
           '</li>';
    }
    echo    '<li>' .
              '<a href="/ldap">' .
                'LDAP Management' .
              '</a>' .
            '</li>' .
            '<li>' .
              '<a href="/nonDNSTrafficWhiteList">' .
                'DNS White List' .
               '</a>' .
            '</li>' .
            '<li>' .
              '<a href="/whiteList">' .
                'White List' .
              '</a>' .
            '</li>' .
            '<li>' .
              '<a href="/rankWhiteList">' .
                'Rank White List' .
              '</a>' .
            '</li>' .
            '<li>' .
              '<a href="/infectedIPs">' .
                'Infected IPs List' .
              '</a>' .
            '</li>' .
          '</ul>' .
        '</li>' .
      '</ul>' .
    '</div>' .
  '</td>' .
  '<td class="center">' .
    '|' .
  '</td>' .
  '<td class="center">' .
    '<a class="text" href="/archive/">' .
      'Archive' .
    '</a>' .
  '</td>' .
  '<td class="center">' .
    '|' .
  '</td>' .
  '<td class="center">' .
    '<a class="text" href="/processStats">' .
      'Analytics Statistics' .
    '</a>' .
  '</td>' .
  '<td class="center">' .
    '|' .
  '</td>' .
  '<td class="center">' .
    '<a class="text" href="/search/">' .
      'Search' .
    '</a>' .
  '</td>' .
  '<td class="center">' .
    '|' .
  '</td>' .
  '<td class="center">' .
    '<a class="text" href="/logout">' .
      'Log Out' .
    '</a>' .
  '</td>';
  }
  echo $nextDay .
    '</tr>' .
  '</table>' .
'</div>';
include('checkJavascript.php');
?>
