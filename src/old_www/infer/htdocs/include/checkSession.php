<?php
  $url = explode('/', substr($_SERVER['REQUEST_URI'], 1));

  foreach ($url as &$_url) {
    $_url = @pg_escape_string($postgreSQL, $_url);
  }

  if (($url[0] != 'host' || $url[1] != $_SERVER['REMOTE_ADDR']) &&
      (!isset($_COOKIE['imsSessionID']) || !isSession($postgreSQL, $_COOKIE['imsSessionID']))) {
    $numericIP = ip2long($_SERVER['REMOTE_ADDR']);
    if (isInternal($numericIP)) {
      $lastDay = getLastInterestingDay($postgreSQL, $numericIP);
      if ($lastDay) {
        header('Location: /host/' . $_SERVER['REMOTE_ADDR'] . '/' . $lastDay);
        exit;
      }
    }
    header('Location: /login');
    exit;
  }
?>
