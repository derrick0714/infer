<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');
  include('include/contacts.php');
  include('include/form.php');
  include('include/config.php');

  makeHeader($postgreSQL, makeNavMsg($url, false));
  message(makeNavMsg($url));
  /* Displays the main page if a query ID has not been specified. */
  if (!isContactsQueryID($postgreSQL, $url[2])) {
    $protocolNames = array(6 => 'TCP', 17 => 'UDP');

    echo
       '<div class="table">' .
           '<form method="post" action="./contacts" enctype="multipart/form-data">' .
             '<table width="100%" cellspacing="1">' .
              '<tr>' .
                 '<td align="center" class="columnTitle" colspan="5">' .
                   'New Query' .
                 '</td>' .
               '</tr>' .
               '<tr class="even">' .
                 '<td>' .
                   '<table width="100%">' .
                     '<tr>' .
                       '<td class="top" align="center" width="25%">' .
                         'Query Name' .
                       '</td>' .
                       '<td class="center" width="25%">' .
                         'IP Address' .
                       '</td>' .
                       '<td align="center" width="25%">' .
                         'Start Time (YYYY-MM-DD HH:MM:SS)' .
                       '</td>' .
                       '<td align="center" width="25%">' .
                         'End Time (YYYY-MM-DD HH:MM:SS)' .
                       '</td>' .
                     '</tr>' .
                     '<tr>' .
                       '<td class="center">' .
                         '<input type="text" name="queryName" value="' . fillFormField($_POST['submit'],
                                                                         getUsername($postgreSQL, $_COOKIE['imsSessionID']) . '\'s query',
                                                                         $_POST['queryName']) . '" />' .
                       '</td>' .
                       '<td class="center">' .
                         '<input type="text" name="queryIP" value="' . $_POST['queryIP'] . '" />' .
                       '</td>' .
                       '<td class="center">' .
                         '<input type="text" name="queryStartTime" value="' . fillFormField($_POST['submit'],
                                                                                       date('Y-m-d', time() - 86400) . ' ' . '08:00:00',
                                                                                       $_POST['queryStartTime']) . '" />' .
                       '</td>' .
                       '<td class="center">' .
                         '<input type="text" name="queryEndTime" value="' . fillFormField($_POST['submit'],
                                                                                     date('Y-m-d', time() - 86400) . ' ' . '09:00:00',
                                                                                     $_POST['queryEndTime']) . '" />' .
                       '</td>' .
                     '</tr>' .
                   '</table>' .
               '</td>' .
             '</tr>' .
             '<tr>' .
               '<td>' .
                 '<tr>' .
                 '<td class="top">' .
                   '<input type="submit" name="submit" value="Submit" />' .
               '</td>' .
             '</tr>' .
           '</table>' .
         '</form>' .
       '</div>';
    if ($_POST['submit']) {
      if (!$_POST['queryName'] || !$_POST['queryIP'] || !$_POST['queryStartTime'] || !$_POST['queryEndTime']) {
        message('All fields are mandatory.', true);
      }
      else {
        $queryID = prepareContactsQuery($postgreSQL, $_POST, getUsername($postgreSQL, $_COOKIE['imsSessionID']));
        if ($queryID === false) {
          message(@pg_last_error($postgreSQL), true);
        }
        else {
          $command = 'infer_findContacts-SQL ' . escapeshellcmd($queryID) . ' ' . escapeshellcmd($_POST['queryIP']) . ' ' . escapeshellcmd($_POST['queryStartTime']) . ' ' . escapeshellcmd($_POST['queryEndTime']) . ' ';
          $command .= ' > /dev/null 2>&1 &';
          exec($infer_install_prefix . '/bin/' . $command);
          message('Your query has been started. You may visit <a class="text" href="contacts/' . $queryID . '">this URL</a> to view results as they become available.');
        }
      }
    }
    if ($_POST['apply']) {  
      foreach ($_POST as $key => &$value) {
        $pause = false;
        $resume = false;
        $stop = false;
        $delete = false;
        if (substr($key, 0, 6) == 'pause_') {
          $pause = true;
          $queryID = substr($key, 6);
        }
        if (substr($key, 0, 7) == 'resume_') {
          $resume = true;
          $queryID = substr($key, 7);
        }
        if (substr($key, 0, 5) == 'stop_') {
          $stop = true;
          $queryID = substr($key, 5);
        }
        else {
          if (substr($key, 0, 7) == 'delete_') {
            $delete = true;
            $queryID = substr($key, 7);
          }
        }
        if ($pause || $resume || $stop || $delete) {
          $result = @pg_query($postgreSQL, 'SELECT * FROM "Indexes"."contactsQueries" WHERE "id" = \'' . $queryID . '\'');
          if (@pg_num_rows($result) > 0) {
            $row = @pg_fetch_assoc($result);
            /*
             * A query may only be paused, resumed, stopped, or deleted by an
             * administrator or the user who started it.
             */
            if ($row['username'] == getUsername($postgreSQL, $_COOKIE['imsSessionID']) ||
                getPrivileges($postgreSQL, $_COOKIE['imsSessionID']) & ADMINISTRATOR_PRIVILEGE) {
              if ($pause && $row['status'] == CONTACTS_RUNNING) {
                posix_kill($row['pid'], SIGSTOP);
                $time = time();
                if ($row['pauseTime'] == NULL) {
                  $duration = $time - $row['startTime'];
                }
                else {
                  $duration = $row['duration'] + $time - $row['pauseTime'];
                }
                @pg_query($postgreSQL, 'UPDATE "Indexes"."contactsQueries" SET "pauseTime" = \'' . $time . '\', ' .
                                       '"duration" = \'' . $duration . '\', ' .
                                       '"status" = \'' . CONTACTS_PAUSED . '\', ' .
                                       '"details" = \'' . getUsername($postgreSQL, $_COOKIE['imsSessionID']) . '\' ' .
                                       'WHERE "id" = \'' . $queryID . '\'');
                $pausedQueries[] = '"' . $row['name'] . '"';
              }
              if ($resume && $row['status'] == CONTACTS_PAUSED) {
                posix_kill($row['pid'], SIGCONT);
                @pg_query($postgreSQL, 'UPDATE "Indexes"."contactsQueries" SET "resumeTime" = \'' . time() . '\', ' .
                                       '"status" = \'' . CONTACTS_RUNNING . '\', ' .
                                       '"details" = NULL WHERE "id" = \'' . $queryID . '\'');

                $resumedQueries[] = '"' . $row['name'] . '"';
              }
              if ($stop && $row['status'] == CONTACTS_RUNNING) {
                posix_kill($row['pid'], SIGKILL);
                $time = time();
                if ($row['pauseTime'] == NULL) {
                  $duration = $time - $row['startTime'];
                }
                else {
                  $duration = $row['duration'] + $time - $row['resumeTime'];
                }
                @pg_query($postgreSQL, 'UPDATE "Indexes"."contactsQueries" SET "duration" = \'' . $duration . '\', ' .
                                       '"status" = \'' . CONTACTS_STOPPED . '\', ' .
                                       '"details" = \'' . getUsername($postgreSQL, $_COOKIE['imsSessionID']) . '\' ' .
                                       'WHERE "id" = \'' . $queryID . '\'');
                $stoppedQueries[] = '"' . $row['name'] . '"';
              }
              else {
                if ($delete && ($row['status'] == CONTACTS_COMPLETED || $row['status'] == CONTACTS_STOPPED)) {
                  @pg_query($postgreSQL, 'DELETE FROM "Indexes"."contactsQueries" WHERE "id" = \'' . $queryID . '\'');
                  @pg_query($postgreSQL, 'DROP TABLE "ContactsQueries"."' . $queryID . '"');
                  $deletedQueries[] = '"' . $row['name'] . '"';
                }
              }
            }
          }
        }
      }
      if (count($pausedQueries) > 0) {
        message('Paused ' . implode(', ', $pausedQueries) . '.');
      }
      if (count($resumedQueries) > 0) {
        message('Resumed ' . implode(', ', $resumedQueries) . '.');
      }
      if (count($stoppedQueries) > 0) {
        message('Stopped ' . implode(', ', $stoppedQueries) . '.');
      }
      if (count($deletedQueries) > 0) {
        message('Deleted ' . implode(', ', $deletedQueries) . '.');
      }
    }
    $result = @pg_query($postgreSQL, 'SELECT * FROM "Indexes"."contactsQueries" ORDER BY "startTime" DESC');
    if (@pg_num_rows($result) > 0) {
      echo '<div class="table">' .
             '<form method="post" action="./contacts">' .
               '<table width="100%" cellspacing="1">' .
                 '<tr>' .
                   '<td class="columnTitle center">' .
                     'Query Name' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Query IP' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Query Start Time' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Query End Time' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Started by' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Started at' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Ran for' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Progress' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Status' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Num. Results' .
                   '</td>' .
                   '<td align="center" class="columnTitle">' .
                     'Pause' .
                   '</td>' .
                   '<td align="center" class="columnTitle">' .
                     'Resume' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Stop' .
                   '</td>' .
                   '<td class="columnTitle center">' .
                     'Delete' .
                   '</td>' .
                 '</tr>';
      while ($row = @pg_fetch_assoc($result)) {
        $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
        echo '<tr class="' . $rowClass . '">' .
               '<td align="center">' .
                 '<a class="text" href="contacts/' . $row['id'] . '">' .
                   $row['name'] .
                 '</a>' .
               '</td>' .
	       '<td align="center">' .
	         long2ip($row['queryIP']) .
               '</td>' .
               '<td align="center">' .
                 date("Y-m-d H:i:s", $row['queryStartTime']) .
               '</td>' .
               '<td align="center">' .
                 date("Y-m-d H:i:s", $row['queryEndTime']) .
               '</td>' .
               '<td align="center">' .
                 $row['username'] .
               '</td>' .
               '<td align="center">' .
                 date("l, F j, Y \a\\t g:i:s A", $row['startTime']) .
               '</td>' .
               '<td align="center">';
        if ($row['status'] == CONTACTS_RUNNING) {
          $time = time();
          if ($row['pauseTime'] == NULL) {
            $duration = $time - $row['startTime'];
          }
          else {
            $duration = $time - $row['resumeTime'] + $row['duration'];
          }
        }
        else {
          $duration = $row['duration'];
        }
        echo duration($duration) .
           '</td>' .
           '<td align="center">' .
             $row['percentComplete'] . '%' .
           '</td>' .
           '<td align="center">';
        switch ($row['status']) {
          case CONTACTS_RUNNING:
            echo 'Running';
            break;
          case CONTACTS_COMPLETED:
            echo 'Completed';
            break;
          case CONTACTS_STOPPED:
            echo 'Stopped by ' . $row['details'];
            break;
          case CONTACTS_PAUSED;
            echo 'Paused by ' . $row['details'];
            break;
        }
        echo '</td>' .
             '<td align="center">' .
               number_format($row['numResults']) .
             '</td>' .
             '<td align="center">';
        /* The option to pause a query is only available if it is running. */
        if ($row['status'] == CONTACTS_RUNNING) {
          echo '<input type="checkbox" name="pause_' . $row['id'] . '" value="' . checked($_POST['pause_' . $row['id']], false) . '" />';
        }
        echo '</td>' .
             '<td align="center">';
        /* The option to resume a query is only available if it is paused. */
        if ($row['status'] == CONTACTS_PAUSED) {
          echo '<input type="checkbox" name="resume_' . $row['id'] . '" value="' . checked($_POST['resume_' . $row['id']], false) . '" />';
        }
        echo '</td>' .
             '<td align="center">';
        /* The option to stop a query is only available if it is running. */
        if ($row['status'] == CONTACTS_RUNNING) {
          echo '<input type="checkbox" name="stop_' . $row['id'] . '" value="' . checked($_POST['stop_' . $row['id']], false) . '" />';
        }
        echo '</td>' .
             '<td align="center">';
        /*
         * The option to delete a query is only available if it has completed
         * or has been stopped.
         */
        if ($row['status'] == CONTACTS_COMPLETED || $row['status'] == CONTACTS_STOPPED) {
          echo '<input type="checkbox" name="delete_' . $row['id'] . '" value="' . checked($_POST['delete_' . $row['id']], false) . '" />';
        }
        echo '</td>' .
           '</tr>';
         ++$rowNumber;
      }
      echo '<tr>' .
             '<td class="top">' .
               '<input type="submit" name="apply" value="Apply" />' .
             '</td>' .
           '</tr>' .
         '</table>' .
       '</form>' .
     '</div>';
    }
    include('include/footer.html');
  }
  else {
    $result = @pg_query($postgreSQL, 'SELECT "name" FROM "Indexes"."contactsQueries" WHERE "id" = \'' . $url[2] . '\'');
    $row = @pg_fetch_assoc($result);
    
    $result = @pg_query($postgreSQL, 'SELECT "queryIP", "queryStartTime", "queryEndTime" FROM "Indexes"."contactsQueries" WHERE "id" = \'' . $url[2] . '\'');
    $row = @pg_fetch_assoc($result);
/*
    echo 'Query:<br>' .
         '<div class="table">' .
	   '<table width="100%" cellspacing="1" cellpadding="2">' .
	     '<tr class="even">' .
	       '<td class="columnTitle center">' .
	         'IP Address' .
	       '</td>' .
	       '<td class="columnTitle center">' .
	         'Start Time' .
	       '</td>' .
	       '<td class="columnTitle center">' .
	         'End Time' .
	       '</td>' .
	     '</tr>' .
	     '<tr class="even">' .
	       '<td class="center">' .
	         long2ip($row['queryIP']) .
	       '</td>' .
	       '<td class="center">' .
                 date("Y-m-d H:i:s", $row['queryStartTime']) .
	       '</td>' .
	       '<td class="center">' .
                 date("Y-m-d H:i:s", $row['queryEndTime']) .
	       '</td>' .
	     '</tr>' .
	   '</table>' .
	 '</div>' .
	 'Results:<br>';
*/
    echo '<div id="reason">' .
	   'Host contacts for ' . 
           long2ip($row['queryIP']) . 
           ' between ' . 
           date("Y-m-d H:i:s", $row['queryStartTime']) . 
           ' and ' . 
           date("Y-m-d H:i:s", $row['queryEndTime']) . ':' .
	 '</div>';

    $result = @pg_query($postgreSQL, 'SELECT * FROM "ContactsQueries"."' . $url[2] . '"');
    if (@pg_num_rows($result) == 0) {
      message('There are no results for this query.');
    }
    else {
      /* Displays results for the specified query ID. */
      $rowNum = 0;
      echo '<div class="table">' .
             '<table width="100%" cellspacing="1" cellpadding="2">';
      while ($row = @pg_fetch_assoc($result)) {
        if ($rowNum % 10 == 0) {
          echo     '<tr class="even">' .
                     '<td class="columnTitle center">' .
                       'Protocol' .
                     '</td>' .
                     '<td class="columnTitle center">' .
                       'Source IP / Port' .
                     '</td>' .
                     '<td class="columnTitle center">' .
                       'Destination IP / Port' .
                     '</td>' .
                     '<td class="columnTitle center">' .
                       'Start Time' .
                     '</td>' .
                     '<td class="columnTitle center">' .
                       'End Time' .
                     '</td>' .
                   '</tr>';
	}
	echo '<tr class="even">' .
                '<td class="center">' .
                   $protocolNames[$row['protocol']] .
                 '</td>' .
                 '<td class="center">' .
                   long2ip($row['sourceIP']) . ':' . $row['sourcePort'] .
                 '</td>' .
                 '<td class="center">' .
                   long2ip($row['destinationIP']) . ':' . $row['destinationPort'] .
                 '</td>' .
                 '<td class="center">' .
                   date("l, F j, Y \a\\t g:i:s A", $row['startTime']) .
                 '</td>' .
                 '<td class="center">' .
                   date("l, F j, Y \a\\t g:i:s A", $row['endTime']) .
                 '</td>' .
               '</tr>';
	$rowNum = $rowNum + 1;
      }
      echo   '</table>' .
           '</div>';
    }
  }
  include('include/footer.html');
?>
