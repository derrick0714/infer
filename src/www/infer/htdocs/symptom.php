<?php
	include('include/postgreSQL.php');
	include('include/shared.php');
	include('include/accessControl.php');
	include('include/checkSession.php');
	include('include/create_table.php');
	include('include/symptom.php');

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
		/* Displays the symptom list if no name was provided. */
		$title = makeNavMsg($url, false);
		
		//include('include/header.php');
		makeHeader($postgreSQL, $title);

		message(makeNavMsg($url));

		echo '<div class="table"><table width="100%" cellspacing="1" cellpadding="2"><thead><tr>';
		echo '<th class="columnTitle left">Symptom</th>' .
			'<th class="columnTitle left">Description</th>' .
			'</tr></thead><tbody>';
		$rowNum = 0;
		foreach ($symptomsConf as $symptom => $levels) {
			$rowNum % 2 ? $rowClass = 'odd' : $rowClass = 'even';
			echo '<tr style="height:100%;" class="' . $rowClass . 
							 '" onmouseover="this.className=\'table_hover\'"' . 
							 ' onmouseout="this.className=\'' . $rowClass . '\'">';
			echo '<td class="left" style="height:100%;">' .
				'<a href="/symptom/' . $symptom . '" style="display:block;width:100%;height:100%;margin:0;border:0;padding:0;">' . 
				'<table style="width:100%;height:100%;margin:0;border:0;padding:0;"><tr style="width:100%;height:100%;margin:0;border:0;padding:0;"><td style="width:100%;height:100%;margin:0;border:0;padding:0;">' . $symptom . '</td></tr></table></a></td>';
			echo '<td class="left" style="height:100%;">' .
				'<a href="/symptom/' . $symptom . '" style="display:block;width:100%;height:100%;margin:0;border:0;padding:0;">' . 
				'<table style="width:100%;height:100%;margin:0;border:0;padding:0;"><tr style="width:100%;height:100%;margin:0;border:0;padding:0;"><td style="width:100%;height:100%;margin:0;border:0;padding:0;">' . $levels['description'] . '</td></tr></table></a></td>';
			echo '</tr>';
			$rowNum++;
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
		/* display the calendar for this symptom if no date is provided */
		$title = 'IMS - symptom - ';
		
		$symptomName = $url[1];
		if (!$symptomsConf[$symptomName]) {
			$title .= 'Error';
			include('include/header.php');

			message('"' . $symptomName . '" is not a valid symptom.', true);

			include('include/footer.html');
			exit;
		}
		$schemaName = $symptomsConf[$symptomName]['schema'];
		
		$title = makeNavMsg($url, false);
		include('include/calendar.php');
		//include('include/header.php');
		makeHeader($postgreSQL, $title);

		message(makeNavMsg($url));
		$days = getPGTableRange($postgreSQL, $schemaName,
								getFirstPGTable($postgreSQL, $schemaName),
								getLastPGTable($postgreSQL, $schemaName));

		if (count($days) == 0) {
			message('No data available for "' . $symptomName . '"');
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
				drawMonth($postgreSQL, $year, $month, $schemaName, 'existsPGTable', $symptomName);
				echo '</div>';
			}
		}
	} else if (!$url[4]) {
		/* displays all level1 data for this symptom and day if no host is provided */
		$title = 'IMS - symptom - ';
		
		if (!$symptomsConf[$url[1]]) {
			$title .= 'Error';
			include('include/header.php');

			message('"' . $url[1] . '" is not a valid symptom.', true);

			include('include/footer.html');
			exit;
		}
		
		$symptomName = $url[1];
		if (!$symptomsConf[$symptomName]) {
			$title .= 'Error';
			include('include/header.php');

			message('"' . $symptomName . '" is not a valid symptom.', true);

			include('include/footer.html');
			exit;
		}
		$schemaName = $symptomsConf[$symptomName]['schema'];

		if (!existsPGTable($postgreSQL, $schemaName, $url[2])) {
			$title .= 'Error';
			include('include/header.php');

			message($url[2] . ' is not a valid date for "' . $symptomName . '"',
					true);
			message('Click <a class="text" href="/' . $url[0] . '/' . $url[1] . '">here</a> for valid dates for this symptom.');

			include('include/footer.html');
			exit;
		}

		$date = $url[2];
		$hostIP = false;
		if ($url[3]) {
			$hostIP = ip2long($url[3]);
		}
		$title = makeNavMsg($url, false);
		//include('include/header.php');
		makeHeader($postgreSQL, $title, true, '/' . implode('/', array_slice($url, 0, 2)), implode('/', array_slice($url, 3)), $schemaName, $date);

		message(makeNavMsg($url));

		$where = NULL;
		if ($hostIP) {
			$where = array(array(array($symptomsConf[$symptomName]['hostColumn'], '=', $hostIP)));
		}
		$baseURL = '/';
		$baseURL .= implode('/', $url);

		$linkBaseURL = '/' . implode('/', array_slice($url, 0, 3));

		createTable($postgreSQL, $schemaName, $date, $numResults, $page, 
					$symptomsConf[$symptomName][1]['heading'],
					$symptomsConf[$symptomName][1]['display'],
					$symptomsConf[$symptomName][1]['type'],
					$baseURL,
					$linkBaseURL,
					$symptomsConf[$symptomName][2]['url'],
					$symptomsConf[$symptomName][2]['urlType'],
					$where,
					$symptomsConf[$symptomName][1]['group'],
					$symptomsConf[$symptomName][1]['order'],
					$symptomsConf[$symptomName][1]['sort']);

/*
		$level1Query = 'SELECT ' .
			implode(', ', $symptomsConf[$symptomName][1]['display']) .
			' FROM "' . $schemaName . '"."' . $date . '" ' .
			($hostIP?'WHERE ' . $symptomsConf[$symptomName]['hostColumn'] . ' = \'' . $hostIP . '\' ':'') .
			'GROUP BY ' . implode(', ', $symptomsConf[$symptomName][1]['group']) . ' ' .
			'ORDER BY ' . implode(', ', $symptomsConf[$symptomName][1]['order']) . ' ' .
			$symptomsConf[$symptomName][1]['sort'];
		
		message($level1Query);
		$result = @pg_query($postgreSQL, $level1Query);
		if (@pg_num_rows($result) == 0) {
			message('There are no results for this query.');
		} else {
			// Displays results for the specified query ID. 
			//$rowNum = 0;
			echo 
				'<div class="table">' .
					'<table width="100%" cellspacing="1" cellpadding="2">';
			while ($row = @pg_fetch_row($result)) {
				echo 
					'<tr class="even">';
				for ($i = 0; $i < count($row); $i++) {
					echo
						'<td class="center">' .
							$row[$i] .
						'</td>';
				}
				echo
					'</tr>';
				//$rowNum++;
			}
			echo	'</table>' .
				'</div>';
		}
*/
	} else {
		/* level 2 stuff...figure this out... */
		$title = 'IMS - symptom - ';
		
		if (!$symptomsConf[$url[1]]) {
			$title .= 'Error';
			include('include/header.php');

			message('"' . $url[1] . '" is not a valid symptom.', true);

			include('include/footer.html');
			exit;
		}
		
		$symptomName = $url[1];
		if (!$symptomsConf[$symptomName]) {
			$title .= 'Error';
			include('include/header.php');

			message('"' . $symptomName . '" is not a valid symptom.', true);

			include('include/footer.html');
			exit;
		}
		$schemaName = $symptomsConf[$symptomName]['schema'];

		if (!existsPGTable($postgreSQL, $schemaName, $url[2])) {
			$title .= 'Error';
			include('include/header.php');

			message($url[2] . ' is not a valid date for "' . $symptomName . '"',
					true);
			message('Click <a class="text" href="/' . $url[0] . '/' . $url[1] . '">here</a> for valid dates for this symptom.');

			include('include/footer.html');
			exit;
		}

		$date = $url[2];
		$hostIP = ip2long($url[3]);

		$title = makeNavMsg($url, false);

		//include('include/header.php');
		makeHeader($postgreSQL, $title, true, '/' . implode('/', array_slice($url, 0, 2)), implode('/', array_slice($url, 3)), $schemaName, $date);
		
		message(makeNavMsg($url));

		$where = array(
			array(
				array($symptomsConf[$symptomName][2]['url'][0],
					  '=', 
					  $hostIP),
				array($symptomsConf[$symptomName][2]['url'][1],
					  '=',
					  ($symptomsConf[$symptomName][2]['urlType'][1] === 'ip')?ip2long($url[4]):$url[4]))
		);
		
		$baseURL = '/';
		$baseURL .= implode('/', $url);

		createTable($postgreSQL, $schemaName, $date, $numResults, $page, 
					$symptomsConf[$symptomName][2]['heading'],
					$symptomsConf[$symptomName][2]['display'],
					$symptomsConf[$symptomName][2]['type'],
					$baseURL,
					NULL,
					NULL,
					NULL,
					$where,
					$symptomsConf[$symptomName][2]['group'],
					$symptomsConf[$symptomName][2]['order'],
					$symptomsConf[$symptomName][2]['sort']);

	}
	
	include('include/footer.html');
?>
