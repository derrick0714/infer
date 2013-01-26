<?php
if($_SERVER['HTTP_REFERER'] == '') header("HTTP/1.0 404 Not Found");
?>
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html><head>
<title>404 Not Found</title>
</head><body>
<h1>Not Found</h1>
<p>The requested URL <?=$_SERVER['HTTP_REFERER'] != '' ? substr($_SERVER['HTTP_REFERER'], strpos($_SERVER['HTTP_REFERER'], $_SERVER['HTTP_HOST']) + strlen($_SERVER['HTTP_HOST'])) : $_SERVER['REQUEST_URI']?> was not found on this server.</p>
</body></html>
<?php
exit();
?>