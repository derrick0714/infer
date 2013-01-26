<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  include('include/checkSession.php');

  makeHeader($postgreSQL, makeNavMsg($url, false));
  message(makeNavMsg($url));
  echo '<table align="center">' .
         '<tr>' .
           '<td align="center" width="20%">' .
             '<strong>' .
               '<a class="text" href="host">' .
                 'Host/IP' .
               '</a>' .
             '</strong>' .
           '</td>' .
           '<td align="center" width="20%">' .
             '<strong>' .
               '<a class="text" href="search">' .
                 'Flow' .
               '</a>' .
             '</strong>' .
           '</td>' .
           '<td align="center" width="20%">' .
             '<strong>' .
               '<a class="text" href="payload">' .
                 'Payload' .
               '</a>' .
             '</strong>' .
           '</td>' .
           '<td align="center" width="20%">' .
             '<strong>' .
               '<a class="text" href="contacts">' .
                 'Contacted Hosts' .
	           '</a>' .
             '</strong>' .
           '</td>' .
         '</tr>' .
       '</table>' .
     '</body>' .
   '</html>';
?>
