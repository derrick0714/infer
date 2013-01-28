<?php
	include('include/rankWhiteList.php');
	include('include/postgreSQL.php');
	include('include/shared.php');
	include('include/accessControl.php');
	include('include/checkSession.php');
	include('include/create_table.php');

	$pageToken = '*';
	$page = 1;
	$numResults = 10;
	$tokenIndex = array_search($pageToken, $url);
	if ($tokenIndex !== false) {
		if (isset($url[$tokenIndex + 2])) {
			$numResults = $url[$tokenIndex + 2];
			unset($url[$tokenIndex + 2]);
		}
		if (isset($url[$tokenIndex + 1])) {
			$page = $url[$tokenIndex + 1];
			unset($url[$tokenIndex + 1]);
		}
		unset($url[$tokenIndex]);
	}	

	function showWhiteList(&$url) {
		message(makeNavMsg($url));

		if (count($url) < 3) {
			message("Display whitelist grouped by " .
					'<a class="text" href="/rankWhiteList/modify/host">' .
					'Host' .
					'</a>' .
					' or ' . 
					'<a class="text" href="/rankWhiteList/modify/rankName">' .
					'Rank Name' .
					'</a>.');

			return true;
		}

		if (count($url) > 4) {
			return "too many arguments";
		}

		global $postgreSQL;
		global $rankWhiteListSchema;
		if (!($rankWhiteListTable = getLastPGTable($postgreSQL,
												   $rankWhiteListSchema)))
		{
			message("Rank White List is empty.");
			return true;
		}

		switch ($url[2]) {
		  case 'host':
			if (count($url) == 3) {
				$heading = array('Host');
				$display = array('"ip"');
				$type = array('ip');
				$baseURL = '/' . implode('/', $url);
				$linkBaseURL = $baseURL;
				$tableURL = array('"ip"');
				$urlType = array('ip');
				$where = NULL;
				$group = array('"ip"');
				$order = array('"ip"');
				$sort = 'ASC';
			}
			else {
				$longIP = ip2long($url[3]);
				if ($longIP == -1 || $longIP === FALSE) {
					return "Invalid IP address: " . $host;
				}

				$heading = array('Host', 'Rank Name');
				$display = array('"ip"', '"rankName"');
				$type = array('ip', 'NULL');
				$baseURL = '/' . implode('/', $url);
				$linkBaseURL = '/rankWhiteList/remove';
				$tableURL = array('"ip"', '"rankName"');
				$urlType = array('ip');
				$where = array(array(array('"ip"', '=', $longIP)));
				$group = NULL;
				$order = array('"rankName"');
				$sort = 'ASC';

				message('Click on a row to remove it from the Rank White List');
			}
			break;
		  case 'rankName':
			if (count($url) == 3) {
				$heading = array('Rank Name');
				$display = array('"rankName"');
				$type = array(NULL);
				$baseURL = '/' . implode('/', $url);
				$linkBaseURL = $baseURL;
				$tableURL = array('"rankName"');
				$urlType = array(NULL);
				$where = NULL;
				$group = array('"rankName"');
				$order = array('"rankName"');
				$sort = 'ASC';
			}
			else {
				$heading = array('Rank Name', 'Host');
				$display = array('"rankName"', '"ip"');
				$type = array(NULL, 'ip');
				$baseURL = '/' . implode('/', $url);
				$linkBaseURL = '/rankWhiteList/remove';
				$tableURL = array('"ip"', '"rankName"');
				$urlType = array('ip');
				$where = array(array(array('"rankName"', '=', $url[3])));
				$group = NULL;
				$order = array('"ip"');
				$sort = 'ASC';

				message('Click on a row to remove it from the Rank White List');
			}
			break;
		}

		global $numResults;
		global $page;
		createTable($postgreSQL, $rankWhiteListSchema, $rankWhiteListTable,
					$numResults, $page,
					$heading,
					$display,
					$type,
					$baseURL,
					$linkBaseURL,
					$tableURL,
					$urlType,
					$where,
					$group,
					$order,
					$sort);

		return true;
	}

	function addToWhiteList($host, $rankName) {
		$longIP = ip2long($host);
		if ($longIP == -1 || $longIP === FALSE) {
			return "Invalid IP address: " . $host;
		}

		global $postgreSQL;
		global $rankWhiteListSchema;
		$rankWhiteListTable = date("Y-m-d");

		if (!existsPGTable($postgreSQL,
						   $rankWhiteListSchema,
						   $rankWhiteListTable))
		{
			if (!@pg_query($postgreSQL,
						   'CREATE TABLE "' . $rankWhiteListSchema . '"."' .
						   $rankWhiteListTable .
						   '" ("ip" uint32, "rankName" TEXT)'))
			{
				return "Unable to create RankWhiteList table";
			}

			if ($prevTable = getPreviousPGTable($postgreSQL,
												$rankWhiteListSchema,
												$rankWhiteListTable))
			{
				if (!@(pg_query($postgreSQL,
								'INSERT INTO "' . $rankWhiteListSchema . '"."' .
								$rankWhiteListTable .
								'" SELECT * FROM "' . $rankWhiteListSchema .
								'"."' . $prevTable . '"')))
				{
					return "Unable to seed new RankWhiteList table";
				}
			}
		}

		if (!($result = 
			 @pg_query($postgreSQL,
					   'SELECT * FROM "' . $rankWhiteListSchema . '"."' .
					   $rankWhiteListTable .
					   '" WHERE "ip" = \'' . sprintf("%u", $longIP) .
					   '\' AND "rankName" = \'' . $rankName . '\'')))
		{
			return 'Unable to check RankWhiteList table';
		}

		if (pg_num_rows($result) > 0) {
			return true;
		}

		if (!pg_query($postgreSQL,
					   'INSERT INTO "' . $rankWhiteListSchema . '"."' .
					   $rankWhiteListTable .
					   '" VALUES (\'' . sprintf("%u", $longIP) . '\', \'' .
					   $rankName . '\')'))
		{
			return 'Unable to insert entry into RankWhiteList table';
		}

		return true;
	}

	function removeFromWhiteList($host, $rankName) {
		$longIP = ip2long($host);
		if ($longIP == -1 || $longIP === FALSE) {
			return "Invalid IP address: " . $host;
		}
		
		global $postgreSQL;
		global $rankWhiteListSchema;
		$rankWhiteListTable = date("Y-m-d");

		if (!existsPGTable($postgreSQL,
						   $rankWhiteListSchema,
						   $rankWhiteListTable))
		{
			if (!@pg_query('CREATE TABLE "' . $rankWhiteListSchema . '"."' .
						   $rankWhiteListTable .
						   '" ("ip" uint32, "rankName" TEXT)'))
			{
				return "Unable to create RankWhiteList table";
			}

			if ($prevTable = getPreviousPGTable($postgreSQL,
												$rankWhiteListSchema,
												$rankWhiteListTable))
			{
				if (!@(pg_query('INSERT INTO "' . $rankWhiteListSchema . '"."' .
								$rankWhiteListTable .
								'" SELECT * FROM "' . $rankWhiteListSchema .
								'"."' . $prevTable . '"')))
				{
					return "Unable to seed new RankWhiteList table";
				}
			}
		}

		if (!@pg_query('DELETE FROM "' . $rankWhiteListSchema . '"."' .
					   $rankWhiteListTable .
					   '" WHERE "ip" = \'' . sprintf("%u", $longIP) .
					   '\' AND "rankName" = \'' . $rankName . '\''))
		{
			return "Unable to remove entry from RankWhiteList table";
		}

		return true;
	}

	function processWhiteListCommand($url) {
		switch ($url[1]) {
		  case 'add':
			if (count($url) != 4) {
				message("add: incorrect argument count", true);
			}
			else {
				$ret = addToWhiteList($url[2], $url[3]);
				if ($ret === true) {
					message("Successfully whitelisted '" . $url[3] . 
							"' for host '" . $url[2] . "'");
				}
				else {
					message("add: " . $ret, true);
				}
			}
			break;
		  case 'remove':
			if (count($url) != 4) {
				message("remove: incorrect argument count", true);
			}
			else {
				$ret = removeFromWhiteList($url[2], $url[3]);
				if ($ret === true) {
					message("Successfully un-whitelisted '" . $url[3] .
							"' for host '" . $url[2] . "'");
				}
				else {
					message("remove: " . $ret, true);
				}
			}
			break;
		  case 'modify':
			$ret = showWhiteList($url);
			if ($ret !== true) {
				message("modify: " . $ret, true);
			}
			break;
		  default:
			message("Unrecognized Rank White List command '" . $url[1] . "'", 
					true);
			break;
		}
	}

	if (count($url) == 1) {
		header('Location: /rankWhiteList/modify');
	}
	else {
		$title = 'Rank White List';
		include('include/header.php');

		processWhiteListCommand($url);

		include('include/footer.html');
	}
?>
