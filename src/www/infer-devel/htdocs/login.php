<?php
require_once("include/infer_session.php");

$infer_session = new InferSession();

if ($infer_session->logged_in()) {
	header('Location: /dashboard/');
}
else
{
	if($_POST) {
		if ($infer_session->authenticate($_POST['username'],
										 $_POST['password']))
		{
			if($_GET['redir'])
				header('Location: ' . $_GET['redir']);
			else
				header('Location: /dashboard/');
		}
	}
	else
	{
		$failed = true;
	}
}
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd"> 
<html xmlns="http://www.w3.org/1999/xhtml"> 
 
<head> 
	<title>INFER - Login</title>
	<meta content="text/html; charset=utf-8" http-equiv="Content-Type" /> 
	<link rel="stylesheet" type="text/css" href="/new_ui/css/style.css" />
	<script type="text/javascript" src="/js/jQuery/jquery.js"></script> 
	<script type="text/javascript" src="/js/jQuery/jquery.address.js"></script>

	<script type="text/javascript">
	$(document).ready(function()
	{
		$("#body_content").load("/site/login/index.html");
	});
	</script>
</head> 
 
<body>
	<div id="wrapper">
		<?php require_once('include/header.php');?>
		
		<?php require_once('include/body.php');?>
		
		<?php require_once('include/footer.php');?>
	</div>
</body>

</html>
