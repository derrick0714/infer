<?php

require_once(__DIR__ . '/../data/pg.include.php');

class InferSession {
	function __construct() {
		session_set_cookie_params('3600', '/', '', true);
		session_name("INFER");
		session_regenerate_id(true); 
		setcookie(session_name(), $_COOKIE[session_name()], time()+3600, "/");

		session_start();
	}

	public function logged_in() {
		return $_SESSION['logged_in'] === true;
	}
	
	public function redirect_to_login($where = "") {
		if (!$where) {
			$where = $_SERVER['REQUEST_URI'];
		}
		header('Location: /login/?redir=' . $where);
		exit;
	}

	public function authenticate($username, $password) {
		global $pg;

		$sha256_password = hash("sha256", $password);

		$auth_query =
		   'SELECT * from "AccessControl".users '.
		   "WHERE user_name = '" . $username . "' " .
				"AND sha256_password = '" . $sha256_password . "' " .
				"AND active = 'true'";
				//echo $auth_query;
				//exit();
		$result = pg_query($pg, $auth_query);
		if (pg_num_rows($result) === 1) {
			$_SESSION['logged_in'] = true;
			$row = pg_fetch_assoc($result);
			$_SESSION['privileges'] = intval($row['privileges']);
		}
		else {
			$_SESSION['logged_in'] = false;
			$_SESSION['privileges'] = 0x0000;
		}

		return $this->logged_in();
	}
}

?>
