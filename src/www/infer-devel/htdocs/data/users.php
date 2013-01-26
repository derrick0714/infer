<?php

// GET /data/users
// GET /data/users/_uid_

// POST /data/users/new
// POST /data/users/update/_uid_
// POST /data/users/delete/_uid_

require_once(__DIR__ . '/essentials.class.php');
require_once(__DIR__ . '/../include/infer_session.php');

$infer_session = new InferSession();
if (!$infer_session->logged_in() || $_SESSION['privileges'] !== 0xffff) {
	header("HTTP/1.1 401 Unauthorized");
	echo "<http><head><title>401 Unauthorized</title></head>" .
		"<body><h1>401 Unauthorized</h1></body></http>";
	exit;
}

switch ($_SERVER['REQUEST_METHOD']) {
  case 'GET':
	// list user info...
	if ($argc > 2) {
		header("HTTP/1.1 400 Bad Request");
		break;
	}

	$uid = 0;
	if ($argc == 2) {
		if (!is_numeric($argv[1])) {
			header("HTTP/1.1 400 Bad Request");
			break;
		}
		$uid = intval($argv[1]);
	}

	$query =
		'SELECT * ' .
		'FROM "AccessControl".users ';
	if ($uid) {
		$query .=
			"WHERE uid = '" . $uid . "'";
	}

	$column_names = array(
		'uid',
		'user_name',
		'name',
		'privileges',
		'active'
	);
	$json = array(
		'sColumns' => implode(',', $column_names),
		'aaData' => array()
	);
	$result = pg_query($pg, $query);
	if ($uid && pg_num_rows($result) == 0) {
		header("HTTP/1.1 404 Not Found");
		break;
	}
	while ($row = pg_fetch_assoc($result)) {
		$json['aaData'][] = array(
			intval($row['uid']),
			$row['user_name'],
			$row['name'],
			intval($row['privileges']),
			($row['active'] == 't')
		);
	}

	echo json_encode($json);

	break;

  case 'POST':
	if ($argc < 2) {
		header("HTTP/1.1 400 Bad Request");
		break;
	}
	switch ($argv[1]) {
	  case 'new':
		if ($argc != 2) {
			header("HTTP/1.1 400 Bad Request");
			break;
		}
		// new user
		$user_name = pg_escape_string($_POST['user_name']);
		$sha256_password = hash("sha256", $_POST['password']);
		$name = pg_escape_string($_POST['name']);
		$privileges = 0;
		if ($_POST['privileges_admin'] == 'yes') {
			$privileges |= 0xffff;
		}
		$active = ( $_POST['active'] == 'yes' ? 't' : 'f' );

		$query =
			'INSERT INTO "AccessControl".users ' .
			'VALUES (' .
				"default, " . // uid
				"'" . $user_name . "', " .
				"'" . $sha256_password . "', " .
				"'" . $name . "', " .
				"'" . $privileges . "', " .
				"'" . $active . "'" .
			') ' .
			'RETURNING uid';

		$result = pg_query($pg, $query);
		if (!$result) {
			header("HTTP/1.1 500 Internal Server Error");
			break;
		}
		$row = pg_fetch_assoc($result);
		if (!$row) {
			header("HTTP/1.1 500 Internal Server Error");
			break;
		}
		$uid = $row['uid'];
		header("HTTP/1.1 303 See Other");
		header("Location: " . $argv[0] . '/' . $uid);

		break;

	  case 'delete':
		// delete user
		if ($argc != 3) {
			header("HTTP/1.1 400 Bad Request");
			break;
		}
		if (!is_numeric($argv[2])) {
			header("HTTP/1.1 400 Bad Request");
			break;
		}
		$uid = intval($argv[2]);
		$query =
			'DELETE FROM "AccessControl".users ' .
			"WHERE uid = '" . $uid . "'";

		$result = pg_query($pg, $query);
		if (!$result) {
			header("HTTP/1.1 500 Internal Server Error");
			break;
		}
		if (!pg_affected_rows($result)) {
			header("HTTP/1.1 404 Not Found");
			break;
		}

		break;

	  case 'update':
		// update user
		if ($argc != 3) {
			header("HTTP/1.1 400 Bad Request");
			break;
		}
		if (!is_numeric($argv[2])) {
			header("HTTP/1.1 400 Bad Request");
			break;
		}
		$uid = intval($argv[2]);

		if ($_POST['user_name']) {
			$user_name = pg_escape_string($_POST['user_name']);
		}
		if ($_POST['password']) {
			$sha256_password = hash("sha256", $_POST['password']);
		}
		if ($_POST['name']) {
			$name = pg_escape_string($_POST['name']);
		}
		$privileges = 0;
		if ($_POST['privileges_admin'] == 'yes') {
			$privileges |= 0xffff;
		}
		$active = ( $_POST['active'] == 'yes' ? 't' : 'f' );

		$query =
			'UPDATE "AccessControl".users ' .
			'SET ';
		$vals = array();
		if (isset($user_name)) {
			$vals[] = "user_name = '" . $user_name . "'";
		}
		if (isset($sha256_password)) {
			$vals[] = "sha256_password = '" . $sha256_password . "'";
		}
		if (isset($name)) {
			$vals[] = "name = '" . $name . "'";
		}
		if (isset($privileges)) {
			$vals[] = "privileges = '" . $privileges . "'";
		}
		if (isset($active)) {
			$vals[] = "active = '" . $active . "'";
		}
		if (!count($vals)) {
			header("HTTP/1.1 400 Bad Request");
			break;
		}
		$query .= implode(', ', $vals) . ' ' .
			"WHERE uid = '" . $uid . "'";

		$result = pg_query($pg, $query);
		if (!$result) {
			header("HTTP/1.1 500 Internal Server Error");
			break;
		}
		if (!pg_affected_rows($result)) {
			header("HTTP/1.1 404 Not Found");
			break;
		}
		header("HTTP/1.1 303 See Other");
		header("Location: " . $argv[0] . '/' . $uid);

		break;

	  default:
		header('HTTP/1.1 400 Bad Request');
		break;
	};
	break;

  default:
	header('HTTP/1.1 405 Method Not Allowed');
	header('Allow: GET, POST');
	break;
};

?>
