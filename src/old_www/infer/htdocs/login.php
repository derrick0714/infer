<?php
  include('include/postgreSQL.php');
  include('include/shared.php');
  include('include/accessControl.php');
  if ($_COOKIE['imsSessionID'] && isSession($postgreSQL, $_COOKIE['imsSessionID'])) {
    header('Location: /');
    exit;
  }
  if ($_POST['login'] && checkCredentials($postgreSQL, $_POST['username'], $_POST['password'])) {
    do {
      $sessionID = hash('sha256', rand());
    } while (isSession($postgreSQL, $sessionID));
    setcookie('imsSessionID', $sessionID, 0, '/', false, true);
    @pg_query($postgreSQL, 'UPDATE "AccessControl"."users" SET "lastUsed" = \'' . time() .
                           '\' WHERE "name" = \'' . $_POST['username'] . '\'');
    @pg_query($postgreSQL, 'DELETE FROM "AccessControl"."sessions" WHERE "name" = \'' . $_POST['username'] . '\'');
    @pg_query($postgreSQL, 'INSERT INTO "AccessControl"."sessions" VALUES(\'' . $_POST['username'] . '\', \'' .
             $sessionID . '\')');
    header('Location: /');
    exit;
  }
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
    <title>IMS Login</title>
    <link rel="stylesheet" type="text/css" href="/css/ims.css" />
  </head>
  <body>
    <div class="table top">
      <form method="post" action="/login">
        <table width="100%" align="center">
          <tr>
            <td class="columnTitle center">
              <table align="center">
                <tr>
                  <td class="left">
                    <b>
                      Username
                    </b>
                  </td>
                  <td class="right">
                    <b>
                      <input type="text" name="username" value="<?php echo $_POST['username'] ?>" />
                    </b>
                  </td>
                </tr>
                <tr>
                  <td class="left">
                    <b>
                      Password
                   </b>
                  </td>
                  <td class="right">
                    <input type="password" name="password" value="<?php echo $_POST['password'] ?>" />
                  </td>
                </tr>
                <tr>
                  <td>
                  </td>
                  <td class="left top">
                   <input type="submit" name="login" value="Log in" />
                  </td>
                </tr>
              </table>
            </td>
          </tr>
        </table>
      </form>
    </div>
    <?php
      if ($_POST['login']) {
        message('<center>Invalid username or password.</center>', false);
      }
    ?>
  </body>
</html>
