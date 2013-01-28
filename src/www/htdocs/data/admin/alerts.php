<?php

$infer_config_cmd = '/usr/local/bin/infer_config';

switch ($_SERVER['REQUEST_METHOD']) {
  case 'POST':
	$smtp_notify_enable =
		$_POST['smtp_notify_enable'] === 'YES';

	$smtp_server =
		$_POST['smtp_server'];
	$smtp_user =
		$_POST['smtp_user'];
	$smtp_password =
		$_POST['smtp_password'];
	$smtp_displayname =
		$_POST['smtp_displayname'];
	$smtp_from =
		$_POST['smtp_from'];
	$subject_prefix =
		$_POST['subject_prefix'];
	$to =
		$_POST['to'];
	
	exec($infer_config_cmd . ' smtp_notify_enable ' .
		($smtp_notify_enable?"YES":"NO"));
	exec($infer_config_cmd . ' smtp_notify.smtp-server ' .
		escapeshellarg($smtp_server));
	exec($infer_config_cmd . ' smtp_notify.smtp-user ' .
		escapeshellarg($smtp_user));
	exec($infer_config_cmd . ' smtp_notify.smtp-password ' .
		escapeshellarg($smtp_password));
	exec($infer_config_cmd . ' smtp_notify.smtp-displayname ' .
		escapeshellarg($smtp_displayname));
	exec($infer_config_cmd . ' smtp_notify.smtp-from ' .
		escapeshellarg($smtp_from));
	exec($infer_config_cmd . ' smtp_notify.subject-prefix ' .
		escapeshellarg($subject_prefix));
	exec($infer_config_cmd . ' smtp_notify.to ' .
		escapeshellarg($to));

  case 'GET':
	$config = array(
		'smtp_notify_enable' =>
			exec('/usr/local/bin/infer_config smtp_notify_enable'),
		'smtp_server' =>
			exec('/usr/local/bin/infer_config smtp_notify.smtp-server'),
		'smtp_user' =>
			exec('/usr/local/bin/infer_config smtp_notify.smtp-user'),
		'smtp_password' =>
			exec('/usr/local/bin/infer_config smtp_notify.smtp-password'),
		'smtp_displayname' =>
			exec('/usr/local/bin/infer_config smtp_notify.smtp-displayname'),
		'smtp_from' =>
			exec('/usr/local/bin/infer_config smtp_notify.smtp-from'),
		'subject_prefix' =>
			exec('/usr/local/bin/infer_config smtp_notify.subject-prefix'),
		'to' =>
			exec('/usr/local/bin/infer_config smtp_notify.to')
	);

	//header('Content-type: application/json');
	print json_encode($config);
	
	break;

  default:
	header('HTTP/1.1 405 Method Not Allowed');
	header('Allow: GET, POST');
	break;
};
