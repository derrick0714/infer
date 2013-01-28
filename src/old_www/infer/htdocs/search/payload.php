<?php

include('include/postgreSQL.php');
include('include/shared.php');
include('include/accessControl.php');
include('include/checkSession.php');
include('include/create_table.php');
include('include/hbf.php');
include('include/config.php');

$inputDataDir = $infer_sensor_data_dir;

// this should be done better. A function or something...
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

function file_upload_error_message($error_code) {
	switch ($error_code) {
	  case UPLOAD_ERR_INI_SIZE:
		return 'The uploaded file exceeds the upload_max_filesize directive in php.ini';
	  case UPLOAD_ERR_FORM_SIZE:
		return 'The uploaded file exceeds the MAX_FILE_SIZE directive that was specified in the HTML form';
	  case UPLOAD_ERR_PARTIAL:
		return 'The uploaded file was only partially uploaded';
	  case UPLOAD_ERR_NO_FILE:
		return 'No file was uploaded';
	  case UPLOAD_ERR_NO_TMP_DIR:
		return 'Missing a temporary folder';
	  case UPLOAD_ERR_CANT_WRITE:
		return 'Failed to write file to disk';
	  case UPLOAD_ERR_EXTENSION:
		return 'File upload stopped by extension';
	  default:
		return 'Unknown upload error';
	}
}

function isPayloadQueryID(&$postgreSQL, &$queryID) {
	$result = @pg_query($postgreSQL, 'SELECT * FROM "Indexes"."payloadQueries" '
									 . 'WHERE "id" = \'' . $queryID . '\'');
	return (@pg_num_rows($result) > 0);
}

$resultsFormat = array(
	1 => array(
		"heading" => array(
			'Source IP',
			'Source Country',
			'Source Autonomous System',
			'Destination IP',
			'Destination Country',
			'Destination Autonomous System',
			'Number of Results'
		),
		"display" => array(
			'"sourceIP"',
			'"sourceIP"',
			'"sourceIP"',
			'"destinationIP"',
			'"destinationIP"',
			'"destinationIP"',
			'COUNT(*)'
		),
		"type" => array(
			'ip',
			'countryLookup',
			'asLookup',
			'ip',
			'countryLookup',
			'asLookup',
			NULL
		),
		"group" => array(
			'"sourceIP"', 
			'"destinationIP"'
		),
		"order" => array(
			'COUNT(*)'
		),
		"sort" => 'DESC'
	),
	array(
		"heading" => array(
			'Protocol',
			'Source IP',
			'Destination IP',
			'Source Port',
			'Destination Port',
			'Start Time',
			'End Time'
		),
		"display" => array(
			'"protocol"',
			'"sourceIP"',
			'"destinationIP"',
			'"sourcePort"',
			'"destinationPort"',
			'"startTime"',
			'"endTime"'
		),
		"type" => array(
			'protocol',
			'ip',
			'ip',
			NULL,
			NULL,
			'date',
			'date'
		),
		"url" => array(
			'"sourceIP"',
			'"destinationIP"'
		),
		"urlType" => array(
			'ip',
			'ip'
		)
	),
	array(
		"heading" => array(
			'Matching String (Base64)',
			'Matching String (ASCII Portion)',
			'HTTP Data Available'
		),
		"display" => array(
			'"matchingString"',
			'"matchingString"',
			'CASE WHEN "httpRequestVersion" is NULL AND "httpResponseVersion" is NULL THEN \'NO\' ELSE \'YES\' END'
		),
		"type" => array(
			'base64',
			'base64ascii',
			NULL
		),
		"url" => array(
			'"protocol"',
			'"sourcePort"',
			'"destinationPort"',
			'"startTime"',
			'"endTime"',
			'"country"',
			'"asn"'
		),
		"urlType" => array(
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL
		)
	),
	array(
		"heading" => array(
			'Request Version',
			'Type',
			'URI',
			'Host',
			'User Agent',
			'Referer',
			'Response Version',
			'Status',
			'Reason',
			'Response',
			'Content Type'
		),
		"display" => array(
			'"httpRequestVersion"',
			'"httpRequestType"',
			'"httpURI"',
			'"httpHost"',
			'"httpUserAgent"',
			'"httpReferer"',
			'"httpResponseVersion"',
			'"httpStatus"',
			'"httpReason"',
			'"httpResponse"',
			'"httpContentType"'
		),
		"type" => array(
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL
		),
		"url" => array(
			'http'
		),
		"urlType" => array(
			NULL
		)
	)
);


function displayResults(&$postgreSQL, &$queryID) {
	if (!existsPGTable($postgreSQL, 'PayloadQueries', $queryID)) {
		message("Invalid query ID: '" . $queryID . "'", true);
		return;
	}
	
	$result = @pg_query($postgreSQL,
						'SELECT * FROM "Indexes"."payloadQueries" WHERE "id" = \'' . $queryID . '\'');
	if (!(@pg_num_rows($result) > 0)) {
		return;
	}
	$row = pg_fetch_assoc($result);
	message("Search Interval Start: " . $row['dataStartTime'] .
			"<br>Search Interval End: " . $row['dataEndTime'] .
			"<br>Search String Length: " . $row['queryStringLength'] .
			"<br>Match Lengh: " . $row['matchLength'] .
			"<br>Search Filter: " . $row['filter']);

	global $url, $numResults, $page, $resultsFormat;
	
	$baseURL = '/';
	$baseURL .= implode('/', $url);

	$where = array();
	if ($hostIP) {
		array_push($where, array(array($roleGroup[$roleName]['hostColumn'], '=', $hostIP)));
	}
	if ($roleGroup[$roleName][1]['where']) {
		$where = array_merge($where, $roleGroup[$roleName][1]['where']);
	}

	$level = 1;
	
	if ($url[3]) {
		array_push($where, array(array('"sourceIP"', '=', ip2long($url[3]))));
	}
	if ($url[4]) {
		$level = 2;
		array_push($where, array(array('"destinationIP"', '=', ip2long($url[4]))));
	}
	if ($url[5]) {
		$level = 3;
		array_push($where, array(array('"protocol"', '=', $url[5])));
	}
	if ($url[6]) {
		array_push($where, array(array('"sourcePort"', '=', $url[6])));
	}
	if ($url[7]) {
		array_push($where, array(array('"destinationPort"', '=', $url[7])));
	}
	if ($url[8]) {
		array_push($where, array(array('"startTime"', '=', $url[8])));
	}
	if ($url[9]) {
		array_push($where, array(array('"endTime"', '=', $url[9])));
	}
	if ($url[10]) {
		$level = 4;
	}


	createTable($postgreSQL, "PayloadQueries", $queryID, $numResults, $page,
				$resultsFormat[$level]['heading'],
				$resultsFormat[$level]['display'],
				$resultsFormat[$level]['type'],
				$baseURL,
				(isset($resultsFormat[$level+1])?$baseURL:NULL),
				$resultsFormat[$level + 1]['url'],
				$resultsFormat[$level + 1]['urlType'],
				$where,
				$resultsFormat[$level]['group'],
				$resultsFormat[$level]['order'],
				$resultsFormat[$level]['sort']);
}

function displayQueryList(&$postgreSQL) {
	$result = @pg_query($postgreSQL,
						'SELECT * FROM "Indexes"."payloadQueries" ORDER BY "startTime" DESC');
	if (!(@pg_num_rows($result) > 0)) {
		return;
	}
?>
	<div class="table">
		<form method="post" action="/search/payload">
			<table width="100%" cellspacing="1">
				<tr>
					<td class="columnTitle center">Name</td>
					<td class="columnTitle center">Started by</td>
					<td class="columnTitle center">Started at</td>
					<td class="columnTitle center">Ran for</td>
					<td class="columnTitle center">Status</td>
					<td class="columnTitle center">Pause</td>
					<td class="columnTitle center">Resume</td>
					<td class="columnTitle center">Stop</td>
					<td class="columnTitle center">Delete</td>
				</tr>
<?php
	$rowNumber = 0;
	while ($row = @pg_fetch_assoc($result)) {
        $rowNumber++ % 2 ? $rowClass = 'odd' : $rowClass = 'even';
		echo
			'<tr class="' . $rowClass . '" width="100%">' .
				'<td align="left">' .
					'<a class="text" href="/search/payload/' . 
							$row['id'] . '">' . ($row['name']!=NULL?$row['name']:$row['id']) . '</a>' .
				'</td>' .
				'<td align="left">' .
					$row['username'] .
				'</td>' .
				'<td align="left">' .
					date('Y-m-d H:i:s', $row['startTime']) .
				'</td>' .
				'<td align="left">';
		if ($row['status'] == HBF_RUNNING) {
			$time = time();
			if ($row['pauseTime'] == NULL) {
				$duration = $time - $row['startTime'];
			}
			else {
				$duration = $time - $row['resumeTime'] + $row['duration'];
			}
		}
		else {
			$duration = $row['duration'];
		}
		echo duration($duration) .
				'</td>' .
				'<td align="left">';
		switch ($row['status']) {
		  case 0:
			echo 'Running';
			break;
		  case 1:
			echo 'Completed';
			break;
		  case 2:
			echo 'Stopped by ' . $row['details'];
			break;
		  case 3:
			echo 'Paused by ' . $row['details'];
			break;
		  case 4:
			echo 'Failed';
			break;
		}
		echo    '</td>' .
				'<td align="center">';
		if ($row['status'] == 0) {
			echo '<input type="checkbox" name="pause_' . $row['id'] . '" />';
		}
		echo    '</td>' .
				'<td align="center">';
		if ($row['status'] == 3) {
			echo '<input type="checkbox" name="resume_' . $row['id'] . '" />';
		}
		echo    '</td>' .
				'<td align="center">';
		if ($row['status'] == 0) {
			echo '<input type="checkbox" name="stop_' . $row['id'] . '" />';
		}
		echo    '</td>' .
				'<td align="center">';
		if ($row['status'] == 1 || $row['status'] == 2 || $row['status'] == 4) {
			echo '<input type="checkbox" name="delete_' . $row['id'] . '" />';
		}
		echo    '</td>' .
			'</tr>';
	}
	echo    '<tr>' .
				'<td class="top">' .
					'<input type="submit" name="apply" value="Apply" />' .
				'</td>' .
			'</tr>' .
		'</table></form></div>';
}

function displaySearchForm() {
?>
		<div id="centered_content">
			<form action="/search/payload" method="post" enctype="multipart/form-data">
				<fieldset>
					<legend>Payload Search</legend>
					<div id="searchbox">
						<div class="ps-text">
							<div class="psquestion">Copy-paste a string to search<span class="note"> (Or, <a title="Click here to upload a file" href="#" onclick="javascript:toggleTabs('ps-text','ps-file');">upload a file</a>)</span></div>
							<textarea id="searchtext" name="textquery" wrap="physical" onfocus="setbg('#e5fff3');" onblur="setbg('white')">Enter your search text here... </textarea>
						</div>
						<div class="ps-file">
							<div class="psquestion">Upload a file to search <span class="note">(Or, <a title="Click here to copy-paste some text" href="#" onclick="javascript:toggleTabs('ps-file','ps-text');">copy-paste text</a>)</span></div>
							<p><label for="datafile">File: </label><input class="prompt" type="file" name="datafile" size="40" /></p>
						</div>

						<div id="psfields">
							<label for="offset">Offset of string: </label><input type="text" value="0" name="offset" size="5" maxlength="5" /> (Offset of actual search string within the string/file)<br />
							<label for="length">Length from offset: </label><input type="text" value="1024" name="length" size="5" maxlength=5" /> (Length of the search string from the offset.)<br />
							<label for="matchlength">Match length: </label><input type="text" value="512" name="matchlength" size="5" maxlength="5" /> (Maximum length of the search string match)<br />
							<label for="queryname">Name: </label><input type="text" value="<?=getUsername($postgreSQL, $_COOKIE['imsSessionID'])?>'s search" name="queryname" size="20" /> (Name to use for identification purposes)<br />
						</div>
					</div>

					<div id="filterbox">
						<div id="label_filter">Refine Search with Filters</div>
						<textarea id="filter" name="filter" wrap="physical" onfocus="setbg('#e5fff3');" onblur="setbg('white')">Use Wireshark type filters here...</textarea>
						<div id="filter_examples">
							<div class="heading">Here are some sample filters:</div>
							<dl>
								<dt>Focus a search to payload to or from a single IP address</dt>
								<dd>ip=128.238.35.91</dd>

								<dt>Focus a search to content from an IP address</dt>
								<dd>ip.src=128.238.35.91</dd>

								<dt>Search only in HTTP traffic</dt>
								<dd>port=80</dd>
							</dl>
						</div>
						<div class="heading">Currently supported search filters are:</div>
						<pre>ip, ip.src, ip.dst, port, src_port, dst_port</pre>
					</div>
					<input id="pssubmit" type="submit" name="submit" value="Search">
				</fieldset>
			</form>
		</div>

<?php
}


makeHeader($postgreSQL, "Infer - Search - Payload");
message(makeNavMsg($url));

if ($_POST['submit']) {
	// check for errors
	$offset = 0;
	if ($_POST['offset'] != NULL) {
		$offset = $_POST['offset'];
	}

	$errorMsg = "";
	$query = "";
	$queryLen = 0;
	if ($_FILES['datafile']['error'] == UPLOAD_ERR_NO_FILE &&
		!isset($_POST['textquery']))
	{
		$errorMsg = "No search data provided.";
	}
	else if ($_POST['length'] == NULL) {
		$errorMsg = "String length must be specified.";
	}
	else if ($_POST['length'] % 64 != 0) {
		$errorMsg = "String length must be a multiple of 64.";
	}
	else if ($_POST['matchlength'] == NULL) {
		$errorMsg = "Match length must be specified.";
	}
	else if ($_POST['matchlength'] % 64 != 0) {
		$errorMsg = "Match length must be a multiple of 64.";
	}
	else if ($_POST['matchlength'] > $_POST['length']) {
		$errorMsg = "Match length must be less than or equal to String length.";
	}
	else if ($_FILES['datafile']['error'] != UPLOAD_ERR_NO_FILE) {
		// datafile query
		if ($_FILES['datafile']['error'] != UPLOAD_ERR_OK) {
			$errorMsg = file_upload_error_message($_FILES['datafile']['error']);
		}
		else if ($offset > $_FILES['datafile']['size']) {
			$errorMsg = "String offset past end of input file.";
		}
		else if ($offset + $_POST['length']
											> $_FILES['datafile']['size']) {
			$errorMsg = "Not enough input data for given offset/length.";
		}
		else {
			$query = file_get_contents($_FILES['datafile']['tmp_name'],
									   FILE_BINARY,
									   NULL,
									   $offset,
									   $_POST['length']);
			if ($query === false) {
				$errorMsg = "Unable to read from data file.";
			}
		}
	}
	else {
		// text query
		$query = $_POST['textquery'];
		if ($offset > strlen($query)) {
			$errorMsg = "String offset is past end of input text.";
		}
		else if ($offset + $_POST['length'] > strlen($query)) {
			$errorMsg = "Not enough input data for given offset/length.";
		}
	}

	if ($errorMsg != "") {
		message($errorMsg, true);
		include('include/footer.html');
		exit(0);
	}

	// determine the start and end time
	// FIXME check that start/end are valid
	$matches = array();
	$count = preg_match("/between\s*(\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}|\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}|\d{4}-\d{2}-\d{2}\s\d{2}|\d{4}-\d{2}-\d{2}|\d{4}-\d{2}|\d{4})\s*and\s*(\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}|\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}|\d{4}-\d{2}-\d{2}\s\d{2}|\d{4}-\d{2}-\d{2}|\d{4}-\d{2}|\d{4})/", $_POST['filter'], $matches);
	if ($count != 0) {
		$start = $matches[1];
		$end = $matches[2];
	}
	else {
		$count = preg_match("/past *(\d*) *(day|week|month|year)s?/", $_POST['filter'], $matches);
		if ($count != 0) {
			if ($matches[1] == '') {
				$matches[1] = 1;
			}
			$start = date('Y-m-d H:i:s', strtotime($matches[1] . ' ' . $matches[2] . ' ago'));
			$end = date('Y-m-d H:i:s', strtotime('now'));
		}
		else {
			$start = date("Y-m-d", strtotime("yesterday"));
			$end = date("Y-m-d", strtotime("today"));
		}
	}

	preg_match_all("/([\w\.]+)\ *=\ *([\w\.\/-]+)/", $_POST['filter'], $matches);
	if (array_search('ip', $matches[1]) !== false || array_search('ip.addr', $matches[1]) !== false) {
		if (array_search('ip.src', $matches[1]) !== false) {
			message('ip and ip.src cannot both be specified', true);
			include('include/footer.html');
			exit();
		}
		if (array_search('ip.dst', $matches[1]) !== false) {
			message('ip and ip.dst cannot both be specified', true);
			include('include/footer.html');
			exit();
		}
	}
	if (array_search('port', $matches[1]) !== false) {
		if (array_search('src_port', $matches[1]) !== false) {
			message('port and src_port cannot both be specified', true);
			include('include/footer.html');
			exit();
		}
		if (array_search('dst_port', $matches[1]) !== false) {
			message('port and dst_port cannot both be specified', true);
			include('include/footer.html');
			exit();
		}
	}
	// FIXME check that arguments are valid
	$filter = '';
	foreach ($matches[1] as $idx => $match) {
		switch ($match) {
		  case 'ip':
		  case 'ip.addr':
			$arg .= ' --network "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			break;
		  case 'ip.src':
			$arg .= ' --source-net "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			break;
		  case 'ip.dst':
			$arg .= ' --dest-net "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			break;
		  case 'port':
			$arg .= ' --port "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			break;
		  case 'src_port':
			$arg .= ' --source-port "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			break;
		  case 'dst_port':
			$arg .= ' --dest-port "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			break;
		  default:
		  	message("Unrecognized option: " . $match, true);
			include('include/footer.html');
			exit();
		}
	}

	// generate a new random queryID
	$queryID = "";
	do {
		$queryID = hash('md5', rand());
	} while (isPayloadQueryID($postgreSQL, $queryID));

	$cmd = "payload_search.php -s \"" . $start . 
		 "\" -e \"" . $end . "\" " . 
		 "-i \"" . $inputDataDir . "\" " .
		 "-t \"" . $queryID . "\" " .
		 "-d \"" . base64_encode($query) . "\" " .
		 "-q " . $_POST['length'] . " " .
		 "-m " . $_POST['matchlength'];
	$cmd .= ' ' . $arg;
	
	// add this query to the query index
	if (!insertPGrow($postgreSQL, 'Indexes', 'payloadQueries',
					 $queryID,
					 ($_POST['queryname']!=''?stripslashes($_POST['queryname']):NULL),
					 base64_encode($query),
					 $start,
					 $end,
					 $_POST['length'],
					 $_POST['matchlength'],
					 $filter,
					 NULL, // pid
					 getUsername($postgreSQL, $_COOKIE['imsSessionID']),
					 time(), // startTime
					 NULL, // pauseTime
					 NULL, // resumeTime
					 NULL, // duration
					 0, // status
					 NULL)) // details
	{
		message("Error inserting search index. Search aborted.", true);
		include('include/footer.html');
		exit(0);
	}

	// execute the query
	$cmd = $infer_frontend_root . '/scripts/' . $cmd . ' > /dev/null 2>&1 &';

	$pid = exec($cmd);

	message('Your search has been started. You may visit ' .
			'<a class="text" href="/search/payload/' . $queryID . '">this URL</a> ' .
			'to view results as they become available.');
}
else if (isPayloadQueryID($postgreSQL, $url[2])) {
	displayResults($postgreSQL, $url[2]);
}
else {
	if ($_POST['apply']) {
		foreach ($_POST as $key => &$value) {
			$pause = false;
			$resume = false;
			$stop = false;
			$delete = false;
			if (substr($key, 0, 6) == 'pause_') {
				$pause = true;
				$queryID = substr($key, 6);
			}
			if (substr($key, 0, 7) == 'resume_') {
				$resume = true;
				$queryID = substr($key, 7);
			}
			if (substr($key, 0, 5) == 'stop_') {
				$stop = true;
				$queryID = substr($key, 5);
			}
			else if (substr($key, 0, 7) == 'delete_') {
				$delete = true;
				$queryID = substr($key, 7);
			}

			if ($pause || $resume || $stop || $delete) {
				$query = 'SELECT * FROM "Indexes"."payloadQueries" WHERE "id" = \'' . $queryID . '\'';
				$result = @pg_query($postgreSQL, $query);
				if (@pg_num_rows($result) > 0) {
					$row = @pg_fetch_assoc($result);
					/*
					* A query may only be paused, resumed, stopped, or deleted by an
					* administrator or the user who started it.
					*/
					if ($row['username'] == getUsername($postgreSQL, $_COOKIE['imsSessionID']) ||
						getPrivileges($postgreSQL, $_COOKIE['imsSessionID']) & ADMINISTRATOR_PRIVILEGE)
					{
						if ($pause && $row['status'] == HBF_RUNNING) {
							posix_kill($row['pid'], SIGSTOP);
							$time = time();
							if ($row['pauseTime'] == NULL) {
							$duration = $time - $row['startTime'];
							}
							else {
							$duration = $row['duration'] + $time - $row['pauseTime'];
							}
							@pg_query($postgreSQL, 'UPDATE "Indexes"."payloadQueries" SET "pauseTime" = \'' . $time . '\', ' .
							'"duration" = \'' . $duration . '\', ' .
							'"status" = \'' . HBF_PAUSED . '\', ' .
							'"details" = \'' . getUsername($postgreSQL, $_COOKIE['imsSessionID']) . '\' ' .
							'WHERE "id" = \'' . $queryID . '\'');
							$pausedQueries[] = '"' . $row['name'] . '"';
						}
						if ($resume && $row['status'] == HBF_PAUSED) {
							posix_kill($row['pid'], SIGCONT);
							@pg_query($postgreSQL, 'UPDATE "Indexes"."payloadQueries" SET "resumeTime" = \'' . time() . '\', ' .
							'"status" = \'' . HBF_RUNNING . '\', ' .
							'"details" = NULL WHERE "id" = \'' . $queryID . '\'');

							$resumedQueries[] = '"' . $row['name'] . '"';
						}
						if ($stop && $row['status'] == HBF_RUNNING) {
							$time = time();
							if ($row['pauseTime'] == NULL) {
								$duration = $time - $row['startTime'];
							}
							else {
								$duration = $row['duration'] + $time - $row['resumeTime'];
							}
							@pg_query($postgreSQL, 'UPDATE "Indexes"."payloadQueries" SET "duration" = \'' . $duration . '\', ' .
							'"status" = \'' . HBF_STOPPED . '\', ' .
							'"details" = \'' . getUsername($postgreSQL, $_COOKIE['imsSessionID']) . '\' ' .
							'WHERE "id" = \'' . $queryID . '\'');
							$stoppedQueries[] = '"' . $row['name'] . '"';
							posix_kill($row['pid'], SIGKILL);
						}
						else {
							if ($delete && ($row['status'] == HBF_COMPLETED || $row['status'] == HBF_STOPPED || $row['status'] == 4)) {
								@pg_query($postgreSQL, 'DELETE FROM "Indexes"."payloadQueries" WHERE "id" = \'' . $queryID . '\'');
								@pg_query($postgreSQL, 'DROP TABLE "PayloadQueries"."' . $queryID . '"');
								$deletedQueries[] = '"' . $row['name'] . '"';
							}
						}
					}
				}
			}
		}
	}
	displaySearchForm();
	displayQueryList($postgreSQL);
}

include('include/footer.html');

?>
