<?php
	include('include/postgreSQL.php');
	include('include/shared.php');
	include('include/accessControl.php');
	include('include/checkSession.php');
	include('include/create_table.php');
	include('include/role.php');

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

	if (!$url[1]) {
		/* Displays the role list if no name was provided. */
		$title = makeNavMsg($url, false);
		
		//include('include/header.php');
		makeHeader($postgreSQL, $title);

		message(makeNavMsg($url));

		echo '<div class="table"><table width="100%" cellspacing="1" cellpadding="2"><thead><tr>';
		echo '<th class="columnTitle left">Role</th>' .
			'<th class="columnTitle left">Description</th>' .
			'</tr></thead><tbody>';
		foreach ($rolesConf as $roleGroup) {
			if ($roleGroup['group']) {
				echo
					'<tr><td class="columnTitle" align="center" colspan="3">' .
					$roleGroup['group'] .
					'</td></tr>';
			}
			$rowNum = 0;
			foreach ($roleGroup['elements'] as $role => $levels) {
				$rowNum % 2 ? $rowClass = 'odd' : $rowClass = 'even';
				echo '<tr style="height:100%;" class="' . $rowClass . 
								 '" onmouseover="this.className=\'table_hover\'"' . 
								 ' onmouseout="this.className=\'' . $rowClass . '\'">';
				echo '<td class="left" style="height:100%;">' .
					'<a href="/role/' . $role . '" style="display:block;width:100%;height:100%;margin:0;border:0;padding:0;">' . 
					'<table style="width:100%;height:100%;margin:0;border:0;padding:0;"><tr style="width:100%;height:100%;margin:0;border:0;padding:0;"><td style="width:100%;height:100%;margin:0;border:0;padding:0;">' . $role . '</td></tr></table></a></td>';
				echo '<td class="left" style="height:100%;">' .
					'<a href="/role/' . $role . '" style="display:block;width:100%;height:100%;margin:0;border:0;padding:0;">' . 
					'<table style="width:100%;height:100%;margin:0;border:0;padding:0;"><tr style="width:100%;height:100%;margin:0;border:0;padding:0;"><td style="width:100%;height:100%;margin:0;border:0;padding:0;">' . $levels['description'] . '</td></tr></table></a></td>';
				echo '</tr>';
				$rowNum++;
			}
		}
		echo
			'</tbody></table></div>';
		
		/*
		foreach ($days as &$day) {
			$date = explode('-', $day);
			if ($year != $date[0]) {
				$year = $date[0];
				unset($month);
				echo '<div class="yearContainer">' .
							 '<div class="columnTitle">' .
								 $year .
							 '</div>' .
						 '</div>';
			}
			if ($month != $date[1]) {
				$month = $date[1];
				echo '<div class="monthContainer">';
				drawMonth($postgreSQL, $year, $month, $schemaName, 'existsPGTable', '/roles');
				echo '</div>';
			}
		}
		*/
	} else if (!$url[2]) {
		/* display the calendar for this role if no date is provided */
		$title = 'IMS - role - ';
		
		$roleName = $url[1];
		if (!isRole($roleName, $rolesConf)) {
			$title .= 'Error';
			include('include/header.php');

			message('"' . $roleName . '" is not a valid role.', true);

			include('include/footer.html');
			exit;
		}
		$roleGroup = getRoleGroup($roleName, $rolesConf);
		$schemaName = $roleGroup[$roleName]['schema'];
		
		$title = makeNavMsg($url, false);
		include('include/calendar.php');
		//include('include/header.php');
		makeHeader($postgreSQL, $title);
		
		message(makeNavMsg($url));
		$days = getPGTableRange($postgreSQL, $schemaName,
								getFirstPGTable($postgreSQL, $schemaName),
								getLastPGTable($postgreSQL, $schemaName));

		if (count($days) == 0) {
			message('No data available for "' . $roleName . '"');
			include('include/footer.html');
			exit;
		}

		foreach ($days as &$day) {
			$date = explode('-', $day);
			if ($year != $date[0]) {
				$year = $date[0];
				unset($month);
				echo
					'<div class="yearContainer">' .
						'<div class="columnTitle">' .
							$year .
						'</div>' .
					'</div>';
			}
			if ($month != $date[1]) {
				$month = $date[1];
				echo '<div class="monthContainer">';
				drawMonth($postgreSQL, $year, $month, $schemaName, 'existsPGTable', $roleName);
				echo '</div>';
			}
		}
	} else if (!$url[3]) {
		/* displays all level1 data for this role and day if no host is provided */
		$title = 'IMS - role - ';
		
		$roleName = $url[1];
		if (!isRole($roleName, $rolesConf)) {
			$title .= 'Error';
			include('include/header.php');

			message('"' . $roleName . '" is not a valid role.', true);

			include('include/footer.html');
			exit;
		}
		$roleGroup = getRoleGroup($roleName, $rolesConf);
		$schemaName = $roleGroup[$roleName]['schema'];

		if (!existsPGTable($postgreSQL, $schemaName, $url[2])) {
			$title .= 'Error';
			include('include/header.php');

			message($url[2] . ' is not a valid date for "' . $roleName . '"',
					true);
			message('Click <a class="text" href="/' . $url[0] . '/' . $url[1] . '">here</a> for valid dates for this role.');

			include('include/footer.html');
			exit;
		}

		$date = $url[2];
		$hostIP = false;
		/*
		if ($url[3]) {
			$hostIP = ip2long($url[3]);
		}
		*/
		$title = makeNavMsg($url, false);
		//include('include/header.php');
		makeHeader($postgreSQL, $title, true, '/' . implode('/', array_slice($url, 0, 2)), implode('/', array_slice($url, 3)), $schemaName, $date);
		
		message(makeNavMsg($url));

		$where = array();
		/*
		if ($hostIP) {
			array_push($where, array(array($roleGroup[$roleName]['hostColumn'], '=', $hostIP)));
		}
		*/
		if ($roleGroup[$roleName][1]['where']) {
			$where = array_merge($where, $roleGroup[$roleName][1]['where']);
		}

		$baseURL = '/';
		$baseURL .= implode('/', $url);

		$linkBaseURL = '/' . implode('/', array_slice($url, 0, 3));

		createTable($postgreSQL, $schemaName, $date, $numResults, $page, 
					$roleGroup[$roleName][1]['heading'],
					$roleGroup[$roleName][1]['display'],
					$roleGroup[$roleName][1]['type'],
					$baseURL,
					$linkBaseURL,
					$roleGroup[$roleName][2]['url'],
					$roleGroup[$roleName][2]['urlType'],
					$where,
					$roleGroup[$roleName][1]['group'],
					$roleGroup[$roleName][1]['order'],
					$roleGroup[$roleName][1]['sort']);
	} else if (!$url[4]) {
		/* level 2 stuff...figure this out... */
		$title = 'IMS - role - ';
		
		$roleName = $url[1];
		if (!isRole($roleName, $rolesConf)) {
			$title .= 'Error';
			include('include/header.php');

			message('"' . $roleName . '" is not a valid role.', true);

			include('include/footer.html');
			exit;
		}
		$roleGroup = getRoleGroup($roleName, $rolesConf);
		$schemaName = $roleGroup[$roleName]['schema'];
		if (isset($roleGroup[$roleName][2]['schema'])) {
			$schemaName = $roleGroup[$roleName][2]['schema'];
		}

		if (!existsPGTable($postgreSQL, $schemaName, $url[2])) {
			$title .= 'Error';
			include('include/header.php');

			message($url[2] . ' is not a valid date for "' . $roleName . '"',
					true);
			message('Click <a class="text" href="/' . $url[0] . '/' . $url[1] . '">here</a> for valid dates for this role.');

			include('include/footer.html');
			exit;
		}

		$date = $url[2];

		$title = makeNavMsg($url, false);

		//include('include/header.php');
		makeHeader($postgreSQL, $title, true, '/' . implode('/', array_slice($url, 0, 2)), implode('/', array_slice($url, 3)), $schemaName, $date);
		
		message(makeNavMsg($url));

		if (isset($roleGroup[$roleName][2]['hostColumn'])) {
			$hostIP = ip2long($url[3]);
			$where = array(
				array(
					array($roleGroup[$roleName][2]['hostColumn'],
						  '=',
						  $hostIP))
			);
		} else
		if (isset($roleGroup[$roleName]['hostColumn'])) {
			$hostIP = ip2long($url[3]);
			$where = array(
				array(
					array($roleGroup[$roleName]['hostColumn'],
						  '=',
						  $hostIP))
			);
		} else {
			$hostIP = $url[3];
			$where = array(
				array(
					array($roleGroup[$roleName][2]['url'][0],
						  '=', 
						  $hostIP))
			);
		}
		if ($roleGroup[$roleName][2]['where']) {
			$where = array_merge($where, $roleGroup[$roleName][2]['where']);
		}
		
		$baseURL = '/';
		$baseURL .= implode('/', $url);

		$linkBaseURL = NULL;
		if (isset($roleGroup[$roleName][3])) {
			$linkBaseURL = '/' . implode('/', array_slice($url, 0, 4));
		}

		createTable($postgreSQL, $schemaName, $date, $numResults, $page, 
					$roleGroup[$roleName][2]['heading'],
					$roleGroup[$roleName][2]['display'],
					$roleGroup[$roleName][2]['type'],
					$baseURL,
					$linkBaseURL,
					$roleGroup[$roleName][3]['url'],
					$roleGroup[$roleName][3]['urlType'],
					$where,
					$roleGroup[$roleName][2]['group'],
					$roleGroup[$roleName][2]['order'],
					$roleGroup[$roleName][2]['sort']);

	} else {
		/* level 2 stuff...figure this out... */
		$title = 'IMS - role - ';
		
		$roleName = $url[1];
		if (!isRole($roleName, $rolesConf)) {
			$title .= 'Error';
			include('include/header.php');

			message('"' . $roleName . '" is not a valid role.', true);

			include('include/footer.html');
			exit;
		}
		$roleGroup = getRoleGroup($roleName, $rolesConf);
		$schemaName = $roleGroup[$roleName]['schema'];

		if (!existsPGTable($postgreSQL, $schemaName, $url[2])) {
			$title .= 'Error';
			include('include/header.php');

			message($url[2] . ' is not a valid date for "' . $roleName . '"',
					true);
			message('Click <a class="text" href="/' . $url[0] . '/' . $url[1] . '">here</a> for valid dates for this role.');

			include('include/footer.html');
			exit;
		}

		$date = $url[2];

		$title = makeNavMsg($url, false);

		//include('include/header.php');
		makeHeader($postgreSQL, $title, true, '/' . implode('/', array_slice($url, 0, 2)), implode('/', array_slice($url, 3)), $schemaName, $date);
		
		message(makeNavMsg($url));

		if (isset($roleGroup[$roleName][2]['hostColumn'])) {
			$hostIP = ip2long($url[3]);
		}
		else
		if (isset($roleGroup[$roleName]['hostColumn'])) {
			$hostIP = ip2long($url[3]);
		}
		else {
			$hostIP = $url[3];
		}
		$where = array(
			array(
				array($roleGroup[$roleName][2]['url'][0],
					  '=', 
					  $hostIP)),
			array(
				array($roleGroup[$roleName][3]['url'][0],
					  '=', 
					  ($roleGroup[$roleName][3]['urlType'][0] === 'ip')?ip2long($url[4]):$url[4]))
		);
		if ($roleGroup[$roleName][3]['where']) {
			$where = array_merge($where, $roleGroup[$roleName][3]['where']);
		}
		
		$baseURL = '/';
		$baseURL .= implode('/', $url);

		createTable($postgreSQL, $schemaName, $date, $numResults, $page, 
					$roleGroup[$roleName][3]['heading'],
					$roleGroup[$roleName][3]['display'],
					$roleGroup[$roleName][3]['type'],
					$baseURL,
					NULL,
					NULL,
					NULL,
					$where,
					$roleGroup[$roleName][3]['group'],
					$roleGroup[$roleName][3]['order'],
					$roleGroup[$roleName][3]['sort']);

	}
	
	include('include/footer.html');
?>
