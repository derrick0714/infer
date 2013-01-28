<?php

include('include/postgreSQL.php');
include('include/shared.php');
include('include/accessControl.php');
include('include/checkSession.php');
include('include/create_table.php');
include('include/search.php');
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

function isSearchQueryID(&$postgreSQL, &$queryID) {
	$result = @pg_query($postgreSQL, 'SELECT * FROM "Indexes"."searchQueries" '
									 . 'WHERE "id" = \'' . $queryID . '\'');
	return (@pg_num_rows($result) > 0);
}

$resultsFormat = array(
	1 => array(
		"heading" => array(
			'Index',
			'Protocol',
			'Source IP',
			'Destination IP',
			'Source Port',
			'Destination Port',
			'Start Time',
			'End Time',
			'Packet Count',
			'Byte Count'
		),
		"display" => array(
			'"index"',
			'"protocol"',
			'"src_ip"',
			'"dst_ip"',
			'"src_port"',
			'"dst_port"',
			'"start_time"',
			'"end_time"',
			'"packet_count"',
			'"byte_count"'
		),
		"type" => array(
			'hidden',
			'protocol',
			'ip',
			'ip',
			NULL,
			NULL,
			'date',
			'date',
			'number',
			'bytes'
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
			'"request_version"',
			'"request_type"',
			'"request_uri"',
			'"request_host"',
			'"request_useragent"',
			'"request_referer"',
			'"response_version"',
			'"response_status"',
			'"response_reason"',
			'"response_response"',
			'"response_contenttype"'
		),
		"type" => array(
			NULL,
			NULL,
			'wrap',
			NULL,
			'wrap',
			'wrap',
			NULL,
			NULL,
			NULL,
			NULL,
			NULL
		),
		"url" => array(
			'"index"'
		),
		"urlType" => array(
			NULL
		)
	)
);

$httpResultsFormat = array(
	1 => array(
		"heading" => array(
			'Index',
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
			'"neoflow_index"',
			'"request_version"',
			'"request_type"',
			'"request_uri"',
			'"request_host"',
			'"request_useragent"',
			'"request_referer"',
			'"response_version"',
			'"response_status"',
			'"response_reason"',
			'"response_response"',
			'"response_contenttype"'
		),
		"type" => array(
			'hidden',
			NULL,
			NULL,
			'wrap',
			NULL,
			'wrap',
			'wrap',
			NULL,
			NULL,
			NULL,
			NULL,
			NULL
		)
	),
	array(
		"heading" => array(
			'Protocol',
			'Source IP',
			'Destination IP',
			'Source Port',
			'Destination Port',
			'Start Time',
			'End Time',
			'Packet Count',
			'Byte Count'
		),
		"display" => array(
			'"protocol"',
			'"src_ip"',
			'"dst_ip"',
			'"src_port"',
			'"dst_port"',
			'"start_time"',
			'"end_time"',
			'"packet_count"',
			'"byte_count"'
		),
		"type" => array(
			'protocol',
			'ip',
			'ip',
			NULL,
			NULL,
			'date',
			'date',
			'number',
			'bytes'
		),
		"url" => array(
			'"neoflow_index"'
		),
		"urlType" => array(
			NULL
		)
	)
);


function displayResults(&$postgreSQL, &$queryID) {
	if (!existsPGTable($postgreSQL, 'SearchQueries', $queryID . "_neoflow")) {
		message("Invalid query ID: '" . $queryID . "'", true);
		return;
	}
	
	$result = @pg_query($postgreSQL,
						'SELECT * FROM "Indexes"."searchQueries" WHERE "id" = \'' . $queryID . '\'');
	if (!(@pg_num_rows($result) > 0)) {
		return;
	}
	$row = pg_fetch_assoc($result);
	message("Search Interval Start: " . $row['dataStartTime'] .
			"<br>Search Interval End: " . $row['dataEndTime'] .
			"<br>Search Filter: " . $row['filter']);

	global $url, $numResults, $page, $resultsFormat, $httpResultsFormat;

	$isHTTP = true;
	if (strpos($row['filter'], 'http.host = ') === false &&
		strpos($row['filter'], 'http.uri = ') === false &&
		strpos($row['filter'], 'http.referer = ') === false)
	{
		$isHTTP = false;
	}
	
	$baseURL = '/';
	$baseURL .= implode('/', $url);

	$level = 1;
	
	$where = array();
	if (isset($url[3])) {
		$level = 2;
		if ($isHTTP === true) {
			array_push($where, array(array('"index"', '=', $url[3])));
		}
		else {
			array_push($where, array(array('"neoflow_index"', '=', $url[3])));
		}
	}

	$table = array(
		1 => $queryID . "_neoflow",
		$queryID . "_http"
	);
	if ($level == 2 && $page == 1) {
		$result = @pg_query($postgreSQL,
							'SELECT * FROM "SearchQueries"."' . $queryID .
							'_dns" WHERE ' .
							'"neoflow_index" = \'' . $url[3] . '\'');
		if (@pg_num_rows($result) > 0) {
			// display dns info
			$row = pg_fetch_assoc($result);
			message(
				"DNS Query Time: " . 
					strftime("%Y-%m-%d %H:%M:%S", $row['query_time']) . "<br />" .
				"DNS Response Time: " .
					strftime("%Y-%m-%d %H:%M:%S", $row['response_time']) . "<br />" .
				(isset($row['client_ip'])?"DNS Client: " . long2ip($row['client_ip']) . "<br />":"") .
				(isset($row['server_ip'])?"DNS Server: " . long2ip($row['server_ip']) . "<br />":"") .
				"DNS Query name: " . $row['query_name']);

			$result = @pg_query($postgreSQL,
							   'SELECT * FROM "SearchQueries"."' . $queryID .
							   '_dns_response" WHERE ' .
							   '"dns_index" = \'' . $row['index'] . '\' ' .
							   'AND "type" = \'1\'');
			if (@pg_num_rows($result) > 0) {
				// display dns responseinfo
				while ($row = pg_fetch_assoc($result)) {
					$bytes = pg_unescape_bytea($row['resource_data']);
					$tmp = unpack("Nip", $bytes);
					$ip = long2ip($tmp['ip']);
					message("DNS Result: " . $row['name'] . ' --> ' . $ip);
				}
			}
		}
	}

	if ($isHTTP) {
		$table[1] = $queryID . "_http";
		$table[2] = $queryID . "_neoflow";
	
		createTable($postgreSQL, "SearchQueries", $table[$level], $numResults, $page,
					$httpResultsFormat[$level]['heading'],
					$httpResultsFormat[$level]['display'],
					$httpResultsFormat[$level]['type'],
					$baseURL,
					(isset($httpResultsFormat[$level+1])?$baseURL:NULL),
					$httpResultsFormat[$level + 1]['url'],
					$httpResultsFormat[$level + 1]['urlType'],
					$where,
					$httpResultsFormat[$level]['group'],
					$httpResultsFormat[$level]['order'],
					$httpResultsFormat[$level]['sort']);
	}
	else {
		createTable($postgreSQL, "SearchQueries", $table[$level], $numResults, $page,
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
}

function displayQueryList(&$postgreSQL) {
	$result = @pg_query($postgreSQL,
						'SELECT * FROM "Indexes"."searchQueries" ORDER BY "startTime" DESC');
	if (!(@pg_num_rows($result) > 0)) {
		return;
	}
?>
	<div class="table">
		<form method="post" action="/search/search">
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
					'<a class="text" href="/search/search/' . 
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
			<form action="/search/search" method="post" enctype="multipart/form-data">
				<fieldset>
					<legend>Search</legend>
					<div id="filterbox">
						<!-- <div id="label_filter">Enter Query:</div> -->
						<label for="queryname">Name: </label><input type="text" value="<?php echo getUsername($postgreSQL, $_COOKIE['imsSessionID']); ?>'s search" name="queryname" size="20" /> (Name to use for identification purposes)<br /><br />
						<textarea id="filter" name="filter" wrap="physical" onfocus="setbg('#e5fff3');" onblur="setbg('white')">Enter search terms here...</textarea>
						<input id="pssubmit" type="submit" name="submit" value="Search">
						<div id="filter_examples">
							<div class="heading">Here are some sample filters:</div>
							<dl>
								<dt>Focus a search to traffic to or from a single IP address</dt>
								<dd>ip=128.238.35.91</dd>

								<dt>Focus a search to content from an IP address</dt>
								<dd>ip.src=128.238.35.91</dd>

								<dt>Search only in *google.com HTTP requests within the past three days</dt>
								<dd>past 3 days http.host=/.*google\.com/</dd>

								<dt>Well-behaved crawlers accessing poly.edu</dt>
								<dd>http.host=/.*poly\.edu/ http.uri=/\/robots\.txt/</dd>

								<dt>Who's downloading .exe files over http?</dt>
								<dd>http.uri=/.*\.exe/</dd>

								<dt>Who's downloading .exe files over http after 5pm on Feb 14, 2010?</dt>
								<dd>between 2010-02-14 17:00:00 and 2010-02-15 http.uri=/.*\.exe/</dd>

								<dt>All tcp traffic to google.com</dt>
								<dd>ip.proto = tcp domain.dst=google.com</dd>

								<dt>All tcp traffic to google.com on port 8080</dt>
								<dd>tcp.dstport = 8080 domain.dst=google.com</dd>
							</dl>
						</div>
						<div class="heading">Currently supported search filters are:</div>
						<pre>ip, ip.src, ip.dst, ip.proto</pre>
						<pre>domain, domain.src, domain.dst</pre>
						<pre>port, srcport, dstport</pre>
						<pre>tcp.port, tcp.srcport, tcp.dstport</pre>
						<pre>udp.port, udp.srcport, udp.dstport</pre>
						<pre>http.host, http.uri, http.referer</pre>
					</div>
				</fieldset>
			</form>
		</div>

<?php
}


makeHeader($postgreSQL, "Infer - Search - Search");
message(makeNavMsg($url));

if ($_POST['submit']) {
	// check for errors
	$errorMsg = "";
	$query = "";
	$queryLen = 0;
	if (!isset($_POST['filter']))
	{
		$errorMsg = "No search string provided.";
	}
	else {
		// text query
		$query = $_POST['filter'];
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

	$count = preg_match("/http\.host\ *=\ */", $_POST['filter'], $matches, PREG_OFFSET_CAPTURE);
	$filterchars = str_split($_POST['filter']);
	if ($count != 0) {
		$startpos = $matches[0][1];
		$endpos = 0;
		if ($filterchars[$startpos + strlen($matches[0][0])] != '/') {
			message("invalid host regex", true);
			include('include/footer.html');
			exit();
		}
		for ($i = $startpos + strlen($matches[0][0]) + 1; $i < strlen($_POST['filter']); ++$i) {
			if ($filterchars[$i] == '/') {
				$endpos = $i;
				break;
			}
			if ($filterchars[$i] == '\\' && $filterchars[$i + 1] == '/') {
				++$i;
			}
		}
		if ($endpos == 0) {
			message("invalid host regex", true);
			include('include/footer.html');
			exit();
		}
		$hostregex = substr($_POST['filter'], $startpos, $endpos - $startpos + 1);
		$_POST['filter'] = str_replace($hostregex, '', $_POST['filter']);
		$hostregex = substr($hostregex, strlen($matches[0][0]) + 1, -1);
		$hostregex = str_replace('\/', '/', $hostregex);

		if (preg_match("/host\ *=\ *\//", $_POST['filter'], $matches, PREG_OFFSET_CAPTURE)) {
			message("only one host regex is allowed", true);
			include('include/footer.html');
			exit();
		}
	}

	$count = preg_match("/http\.uri\ *=\ */", $_POST['filter'], $matches, PREG_OFFSET_CAPTURE);
	$filterchars = str_split($_POST['filter']);
	if ($count != 0) {
		$startpos = $matches[0][1];
		$endpos = 0;
		if ($filterchars[$startpos + strlen($matches[0][0])] != '/') {
			message("invalid uri regex", true);
			include('include/footer.html');
			exit();
		}
		for ($i = $startpos + strlen($matches[0][0]) + 1; $i < strlen($_POST['filter']); ++$i) {
			if ($filterchars[$i] == '/') {
				$endpos = $i;
				break;
			}
			if ($filterchars[$i] == '\\' && $filterchars[$i + 1] == '/') {
				++$i;
			}
		}
		if ($endpos == 0) {
			message("invalid uri regex", true);
			include('include/footer.html');
			exit();
		}
		$uriregex = substr($_POST['filter'], $startpos, $endpos - $startpos + 1);
		$_POST['filter'] = str_replace($uriregex, '', $_POST['filter']);
		$uriregex = substr($uriregex, strlen($matches[0][0]) + 1, -1);
		$uriregex = str_replace('\/', '/', $uriregex);

		if (preg_match("/uri\ *=\ *\//", $_POST['filter'], $matches, PREG_OFFSET_CAPTURE)) {
			message("only one uri regex is allowed", true);
			include('include/footer.html');
			exit();
		}
	}

	$count = preg_match("/http\.referer\ *=\ */", $_POST['filter'], $matches, PREG_OFFSET_CAPTURE);
	$filterchars = str_split($_POST['filter']);
	if ($count != 0) {
		$startpos = $matches[0][1];
		$endpos = 0;
		if ($filterchars[$startpos + strlen($matches[0][0])] != '/') {
			message("invalid referer regex", true);
			include('include/footer.html');
			exit();
		}
		for ($i = $startpos + strlen($matches[0][0]) + 1; $i < strlen($_POST['filter']); ++$i) {
			if ($filterchars[$i] == '/') {
				$endpos = $i;
				break;
			}
			if ($filterchars[$i] == '\\' && $filterchars[$i + 1] == '/') {
				++$i;
			}
		}
		if ($endpos == 0) {
			message("invalid referer regex", true);
			include('include/footer.html');
			exit();
		}
		$refererregex = substr($_POST['filter'], $startpos, $endpos - $startpos + 1);
		$_POST['filter'] = str_replace($refererregex, '', $_POST['filter']);
		$refererregex = substr($refererregex, strlen($matches[0][0]) + 1, -1);
		$refererregex = str_replace('\/', '/', $refererregex);

		if (preg_match("/referer\ *=\ *\//", $_POST['filter'], $matches, PREG_OFFSET_CAPTURE)) {
			message("only one referer regex is allowed", true);
			include('include/footer.html');
			exit();
		}
	}

	$filter = '';
	if (isset($hostregex)) {
		$arg .= ' --host ' . escapeshellarg($hostregex);
		$filter .= 'http.host = /' . str_replace('/', '\/', $hostregex) . '/ ';
	}
	if (isset($uriregex)) {
		$arg .= ' --uri ' . escapeshellarg($uriregex);
		$filter .= 'http.uri = /' . str_replace('/', '\/', $uriregex) . '/ ';
	}
	if (isset($refererregex)) {
		$arg .= ' --referer ' . escapeshellarg($refererregex);
		$filter .= 'http.referer = /' . str_replace('/', '\/', $refererregex) . '/ ';
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
	$protocols = array();
	foreach ($matches[1] as $idx => $match) {
		switch ($match) {
		  case 'ip':
		  case 'ip.addr':
			$arg .= ' --network "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			break;
		  case 'ip.src':
			$arg .= ' --source-network "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			break;
		  case 'ip.dst':
			$arg .= ' --destination-network "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			break;
		  case 'ip.proto':
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			$protocols[$matches[2][$idx]] = true;
			break;
		  case 'port':
			$arg .= ' --port "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			break;
		  case 'srcport':
			$arg .= ' --source-port "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			break;
		  case 'dstport':
			$arg .= ' --destination-port "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			break;
		  case 'tcp.port':
			$arg .= ' --port "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			$protocols['tcp'] = true;
			break;
		  case 'tcp.srcport':
			$arg .= ' --source-port "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			$protocols['tcp'] = true;
			break;
		  case 'tcp.dstport':
			$arg .= ' --destination-port "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			$protocols['tcp'] = true;
			break;
		  case 'udp.port':
			$arg .= ' --port "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			$protocols['udp'] = true;
			break;
		  case 'udp.srcport':
			$arg .= ' --source-port "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			$protocols['udp'] = true;
			break;
		  case 'udp.dstport':
			$arg .= ' --destination-port "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			$protocols['udp'] = true;
			break;
		  case 'domain':
			$arg .= ' --domain "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			break;
		  case 'domain.src':
			$arg .= ' --source-domain "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			break;
		  case 'domain.dst':
			$arg .= ' --destination-domain "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			break;
		  case 'http.host':
			$arg .= ' --host "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			break;
		  case 'http.uri':
			$arg .= ' --uri "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			break;
		  case 'http.referer':
			$arg .= ' --referer "' . $matches[2][$idx] . '"';
			$filter .= $match . ' = ' . $matches[2][$idx] . ' ';
			break;
		  default:
		  	message("Unrecognized option: " . $match, true);
			include('include/footer.html');
			exit();
		}
	}
	foreach ($protocols as $proto => $val) {
		switch ($proto) {
		  case 'tcp':
			if ($val === true) {
				$arg .= ' --protocol 6';
			}
			break;
		  case 'udp':
			if ($val === true) {
				$arg .= ' --protocol 17';
			}
			break;
		  default:
			message("Unsupported ip protocol type: " . $proto, true);
			include('include/footer.html');
			exit();
		}
	}

	// generate a new random queryID
	$queryID = "";
	do {
		$queryID = hash('md5', rand());
	} while (isSearchQueryID($postgreSQL, $queryID));

	$cmd = "search.php --start-time \"" . $start . 
		 "\" --end-time \"" . $end . "\" " . 
		 "--query-id \"" . $queryID . "\"";
	$cmd .= ' ' . $arg;
	
	// add this query to the query index
	if (!insertPGrow($postgreSQL, 'Indexes', 'searchQueries',
					 $queryID,
					 ($_POST['queryname']!=''?stripslashes($_POST['queryname']):NULL),
					 $filter,
					 $start,
					 $end,
					 NULL, // pid
					 getUsername($postgreSQL, $_COOKIE['imsSessionID']),
					 time(), // startTime
					 NULL, // pauseTime
					 NULL, // resumeTime
					 NULL, // duration
					 NULL, // timeLeft
					 NULL, // numResults
					 NULL, // percentComplete
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
			'<a class="text" href="/search/search/' . $queryID . '">this URL</a> ' .
			'to view results as they become available.');
}
else if (isSearchQueryID($postgreSQL, $url[2])) {
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
				$query = 'SELECT * FROM "Indexes"."searchQueries" WHERE "id" = \'' . $queryID . '\'';
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
							@pg_query($postgreSQL, 'UPDATE "Indexes"."searchQueries" SET "pauseTime" = \'' . $time . '\', ' .
							'"duration" = \'' . $duration . '\', ' .
							'"status" = \'' . HBF_PAUSED . '\', ' .
							'"details" = \'' . getUsername($postgreSQL, $_COOKIE['imsSessionID']) . '\' ' .
							'WHERE "id" = \'' . $queryID . '\'');
							$pausedQueries[] = '"' . $row['name'] . '"';
						}
						if ($resume && $row['status'] == HBF_PAUSED) {
							posix_kill($row['pid'], SIGCONT);
							@pg_query($postgreSQL, 'UPDATE "Indexes"."searchQueries" SET "resumeTime" = \'' . time() . '\', ' .
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
							@pg_query($postgreSQL, 'UPDATE "Indexes"."searchQueries" SET "duration" = \'' . $duration . '\', ' .
							'"status" = \'' . HBF_STOPPED . '\', ' .
							'"details" = \'' . getUsername($postgreSQL, $_COOKIE['imsSessionID']) . '\' ' .
							'WHERE "id" = \'' . $queryID . '\'');
							$stoppedQueries[] = '"' . $row['name'] . '"';
							posix_kill($row['pid'], SIGKILL);
						}
						else {
							if ($delete && ($row['status'] == HBF_COMPLETED || $row['status'] == HBF_STOPPED || $row['status'] == 4)) {
								@pg_query($postgreSQL, 'DELETE FROM "Indexes"."searchQueries" WHERE "id" = \'' . $queryID . '\'');
								@pg_query($postgreSQL, 'DROP TABLE "SearchQueries"."' . $queryID . '_neoflow"');
								@pg_query($postgreSQL, 'DROP TABLE "SearchQueries"."' . $queryID . '_http"');
								@pg_query($postgreSQL, 'DROP TABLE "SearchQueries"."' . $queryID . '_dns"');
								@pg_query($postgreSQL, 'DROP TABLE "SearchQueries"."' . $queryID . '_dns_results"');
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
