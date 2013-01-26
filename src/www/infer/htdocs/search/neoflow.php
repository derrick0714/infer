<?php
	include('include/postgreSQL.php');
	include('include/shared.php');
	include('include/accessControl.php');
	include('include/checkSession.php');
	include('include/form.php');
	include('include/neoflow.php');
	include('include/config.php');

	if ($_POST['submit']) {
		$queryID = existsBackgroundNeoflowQuery($postgreSQL, $_POST);
		if ($queryID != false) {
			header("Location: /search/neoflow/" . $queryID);
			exit;
		}

		$queryProtocol = $_POST['queryProtocol'];
		$querySourceIP = $_POST['querySourceIP'];
		$querySourcePort = $_POST['querySourcePort'];
		$queryDestinationIP = $_POST['queryDestinationIP'];
		$queryDestinationPort = $_POST['queryDestinationPort'];
		$queryStartTime = $_POST['queryStartTime'];
		$queryEndTime = $_POST['queryEndTime'];
	}
	
	if (isNeoflowQueryID($postgreSQL, $url[2])) {
		// get query parameters...
		$params = getNeoflowQueryParams($postgreSQL, $url[2]);

		$queryProtocol = $params['protocol'];
		if ($params['sourceIP']) {
			$querySourceIP = long2ip($params['sourceIP']);
		}
		$querySourcePort = $params['sourcePort'];
		if ($params['destinationIP']) {
			$queryDestinationIP = long2ip($params['destinationIP']);
		}
		$queryDestinationPort = $params['destinationPort'];
		$queryStartTime = date("Y-m-d H:i:s", $params['startTime']);
		$queryEndTime = date("Y-m-d H:i:s", $params['endTime']);
		$queryComplete = $params['queryEndTime']?true:false;
	}

	$protocolNames = array(6 => 'TCP', 17 => 'UDP');
	$title = 'Neoflow Query';
	include('include/header.php');
	echo '<table align="center">' .
			 '<tr>' .
				 '<td align="center" width="20%">' .
					 '<strong>' .
						 '<a class="text" href="index.php">' .
							 'Host/IP' .
						 '</a>' .
					 '</strong>' .
				 '</td>' .
				 '<td align="center" width="20%">' .
					 '<strong>' .
						 '<a class="text" href="symptom.php">' .
							 'Symptom'.
						 '</a>' .
					 '</strong>' .
				 '</td>' .
				 '<td align="center" width="20%">' .
					 '<strong>' .
						 '<a class="text" href="advanced.php">' .
							 'Advanced' .
						 '</a>' .
					 '</strong>' .
				 '</td>' .
				 '<td align="center" width="20%">' .
				 	 '<a class="text" href="/search/neoflow.php"' .
					 	'<strong>' .
							'Neoflow' .
					 	'</strong>' .
					 '</a>' .
				 '</td>' .
			 '</tr>' .
		 '</table>' .
		 '<br />' .
		 '<div class="table">' .
				 '<form method="post" action="/search/neoflow" enctype="multipart/form-data">' .
					 '<table width="100%" cellspacing="1">' .
						'<tr>' .
							 '<td align="center" class="columnTitle" colspan="5">' .
								 'New Query' .
							 '</td>' .
						 '</tr>' .
						 '<tr class="even">' .
							 '<td>' .
								 '<table width="100%">' .
									 '<tr>' .
									 	 '<td align="center">' .
										 	 'Protocol' .
										 '</td>' .
										 '<td align="center">' .
											 'Source IP Address' .
										 '</td>' .
										 '<td class="center">' .
											 'Source Port' .
										 '</td>' .
										 '<td align="center">' .
											 'Destination IP Address' .
										 '</td>' .
										 '<td class="center">' .
											 'Destination Port' .
										 '</td>' .
										 '<td align="center">' .
											 'Start Time (YYYY-MM-DD HH:MM:SS)' .
										 '</td>' .
										 '<td align="center">' .
											 'End Time (YYYY-MM-DD HH:MM:SS)' .
										 '</td>' .
									 '</tr>' .
									 '<tr>' .
										 '<td class="center">' .
										 	 '<select name="queryProtocol">' .
												 '<option value="any">ANY</option>' .
												 '<option value="tcp" ' . (($queryProtocol == 'tcp')?'"selected"':'') . '>TCP</option>' .
												 '<option value="udp" ' . (($queryProtocol == 'udp')?'"selected"':'') . '>UDP</option>' .
											 '</select>' .
										 '</td>' .
										 '<td class="center">' .
											 '<input type="text" name="querySourceIP"' . (($querySourceIP)?(' value="' . $querySourceIP . '"'):'') . ' />' .
										 '</td>' .
										 '<td class="center">' .
											 '<input type="text" name="querySourcePort" size="5"' . (($querySourcePort)?(' value="' . $querySourcePort . '"'):'') . ' />' .
										 '</td>' .
										 '<td class="center">' .
											 '<input type="text" name="queryDestinationIP"' . (($queryDestinationIP)?(' value="' . $queryDestinationIP . '"'):'') . ' />' .
										 '</td>' .
										 '<td class="center">' .
											 '<input type="text" name="queryDestinationPort"' . (($queryDestinationPort)?(' value="' . $queryDestinationPort . '"'):'') . ' size="5 "/>' .
										 '</td>' .
										 '<td class="center">' .
											 '<input type="text" name="queryStartTime" value="' . (($queryStartTime)?$queryStartTime:(date('Y-m-d', time() - 86400) . ' ' . '00:00:00')) . '" />' .
										 '</td>' .
										 '<td class="center">' .
											 '<input type="text" name="queryEndTime" value="' . (($queryEndTime)?$queryEndTime:(date('Y-m-d', time()) . ' ' . '00:00:00')) . '" />' .
										 '</td>' .
									 '</tr>' .
								 '</table>' .
						 '</td>' .
					 '</tr>' .
					 '<tr>' .
						 '<td>' .
							 '<tr>' .
							 '<td class="top">' .
								 '<input type="submit" name="submit" value="Submit" />' .
						 '</td>' .
					 '</tr>' .
				 '</table>' .
			 '</form>' .
		 '</div>';

	if (isNeoflowQueryID($postgreSQL, $url[2])) {
		$resultsPerPage = 10;

		$query = 'SELECT count(*) FROM "NeoflowQueries"."' . $url[2] . '"';
		$result = @pg_query($postgreSQL, $query);
		$row = @pg_fetch_assoc($result);

		$pages = ceil($row['count'] / $resultsPerPage);

		if ($pages > 0) {
			if ($url[3] !== NULL && is_numeric($url[3])) {
				$page = $url[3];
			} else {
				$page = 1;
			}
			
			if ($page > $pages) {
				$page = 1;
			}

			$pageMsg = 'Displaying page ' . $page . ' of ' . $pages . '. ' . $row['count'] . ' results.';
			message($pageMsg);

			$pageMsg = '';
			if ($page != 1) {
				$pageMsg = '<a class="text" href="/search/neoflow/' . $url[2] . '/' . ($page - 1) . '">&lt;-- Previous</a> ';
			}
			if ($page != $pages) {
				$pageMsg .= '<a class="text" href="/search/neoflow/' . $url[2] . '/' . ($page + 1) . '">Next --&gt;</a> ';
			}
			if ($pageMsg != '') {
				message($pageMsg);
			}
		}

		$query = 'SELECT * FROM "NeoflowQueries"."' . $url[2] .
				 '" OFFSET ' . (($page - 1) * $resultsPerPage) . ' LIMIT ' . 
				 $resultsPerPage;

		$result = @pg_query($postgreSQL, $query);
		if (!$queryComplete) {
			message('This query is still in progress. Refresh page to update results.');
		}
		if (@pg_num_rows($result) == 0) {
			message('There are no results for this query.');
		} else {
			/* Displays results for the specified query ID. */
			$rowNum = 0;
			echo 
				'<div class="table">' .
					'<table width="100%" cellspacing="1" cellpadding="2">';
			while ($row = @pg_fetch_assoc($result)) {
				if ($rowNum % 10 == 0) {
					echo
						'<tr class="even">' .
							'<td class="columnTitle center">' .
								'Protocol' .
							'</td>' .
							'<td class="columnTitle center">' .
								'Source IP' .
							'</td>' .
							'<td class="columnTitle center">' .
								'Source Port' .
							'</td>' .
							'<td class="columnTitle center">' .
								'Destination IP' .
							'</td>' .
							'<td class="columnTitle center">' .
								'Destination Port' .
							'</td>' .
							'<td class="columnTitle center">' .
								'Start Time' .
							'</td>' .
							'<td class="columnTitle center">' .
								'End Time' .
							'</td>' .
						'</tr>';
				}
				echo 
					'<tr class="even">' .
						'<td class="center">' .
							$protocolNames[$row['protocol']] .
						'</td>' .
						'<td class="center">' .
							long2ip($row['sourceIP']) .
						'</td>' .
						'<td class="center">' .
							$row['sourcePort'] .
						'</td>' .
						'<td class="center">' .
							long2ip($row['destinationIP']) .
						'</td>' .
						'<td class="center">' .
							$row['destinationPort'] .
						'</td>' .
						'<td class="center">' .
							date("l, F j, Y \a\\t g:i:s A", $row['startTime']) .
						'</td>' .
						'<td class="center">' .
							date("l, F j, Y \a\\t g:i:s A", $row['endTime']) .
						'</td>' .
					'</tr>';
				$rowNum = $rowNum + 1;
			}
			echo	'</table>' .
				'</div>';
		}
	} else if ($_POST['submit']) {
		// query doesn't exist already. new one...
		$queryID = prepareBackgroundNeoflowQuery($postgreSQL, $_POST); 
		if ($queryID === false) {
			message(@pg_last_error($postgreSQL), true);
		} else {
			$command = 'infer_queryNeoflow-SQL ' . escapeshellcmd($queryID);
			  $command .= ' > /dev/null 2>&1 &';
			  exec($infer_install_prefix . '/bin/' . $command);
			  message('Your query has been started. You may visit <a class="text" href="/search/neoflow/' . $queryID . '">this URL</a> to view results as they become available.');
		}
	} else if (strcmp($url[2], 'results') == 0) {
		$query = 'SELECT * FROM "Indexes"."neoflowQueries"';
		$result = @pg_query($postgreSQL, $query);
		if (@pg_num_rows($result) > 0) {
			$rowNum = 0;
			echo '<div class="table">' .
						 '<table width="100%" cellspacing="1" cellpadding="2">';
			while ($row = @pg_fetch_assoc($result)) {
				if ($rowNum % 10 == 0) {
					echo		 '<tr class="even">' .
										 '<td class="columnTitle center">' .
											 'Protocol' .
										 '</td>' .
										 '<td class="columnTitle center">' .
											 'Source IP' .
										 '</td>' .
										 '<td class="columnTitle center">' .
											 'Source Port' .
										 '</td>' .
										 '<td class="columnTitle center">' .
											 'Destination IP' .
										 '</td>' .
										 '<td class="columnTitle center">' .
											 'Destination Port' .
										 '</td>' .
										 '<td class="columnTitle center">' .
											 'Start Time' .
										 '</td>' .
										 '<td class="columnTitle center">' .
											 'End Time' .
										 '</td>' .
										 '<td class="columnTitle center">' .
											 'Query Completed' .
										 '</td>' .
									 '</tr>';
				}
				echo '<tr class="even">' .
								'<td class="center">' .
									 $protocolNames[$row['protocol']] .
								 '</td>' .
								 '<td class="center">' .
									 ($row['sourceIP']?long2ip($row['sourceIP']):'') .
								 '</td>' .
								 '<td class="center">' .
									 $row['sourcePort'] .
								 '</td>' .
								 '<td class="center">' .
									 ($row['destinationIP']?long2ip($row['destinationIP']):'') .
								 '</td>' .
								 '<td class="center">' .
									 $row['destinationPort'] .
								 '</td>' .
								 '<td class="center">' .
									 date("l, F j, Y \a\\t g:i:s A", $row['startTime']) .
								 '</td>' .
								 '<td class="center">' .
									 date("l, F j, Y \a\\t g:i:s A", $row['endTime']) .
								 '</td>' .
								 '<td class="center">' .
								     '<a class="text" href="/search/neoflow/' . $row['id'] . '">' . ($row['queryEndTime']?'yes':'no') . '</a>' .
								 '</td>' .
							 '</tr>';
				$rowNum = $rowNum + 1;
			}
			echo	 '</table>' .
					 '</div>';
		}
		
	} else {
		message('Click <a class="text" href="/search/neoflow/results">here</a> to view existing query results.');
	}

	include('include/footer.html');
?>
