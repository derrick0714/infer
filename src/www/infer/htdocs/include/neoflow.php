<?php
	/*
	 * Returns true if the specified Neoflow query ID exists, or false if it
	 * doesn't.
	 */
	function isNeoflowQueryID(&$postgreSQL, &$queryID) {
		$result = @pg_query($postgreSQL, 'SELECT * FROM "Indexes"."neoflowQueries" ' .
										 'WHERE "id" = \'' . $queryID . '\'');
		return (@pg_num_rows($result) > 0);
	}

	function getNeoflowQueryParams(&$postgreSQL, &$queryID) {
		$result = @pg_query($postgreSQL, 'SELECT * FROM "Indexes"."neoflowQueries" ' .
										 'WHERE "id" = \'' . $queryID . '\'');
		if (@pg_num_rows($result) != 1) {
			message("wtf?");
			return false;
		}

		return @pg_fetch_assoc($result);
	}

	function existsBackgroundNeoflowQuery(&$postgreSQL, &$_POST) {
		$protocol = false;
		if ($_POST['queryProtocol'] == 'tcp') {
			$protocol = 6;
		} else if ($_POST['queryProtocol'] == 'udp') {
			$protocol = 17;
		}	
		$sourceIP = (($_POST['querySourceIP'])?sprintf('%u', ip2long($_POST['querySourceIP'])):false);
		$sourcePort = $_POST['querySourcePort'];
		$destinationIP = (($_POST['queryDestinationIP'])?sprintf('%u', ip2long($_POST['queryDestinationIP'])):false);
		$destinationPort = $_POST['queryDestinationPort'];
		$startTime = strtotime($_POST['queryStartTime']);
		$endTime = strtotime($_POST['queryEndTime']);

		$query = 'SELECT "id" FROM "Indexes"."neoflowQueries" WHERE ';
		if ($protocol) {
			$query .= '"protocol" = \'' . $protocol . '\' AND ';
		} else {
			$query .= '"protocol" IS NULL AND ';
		}
		if ($sourceIP) {
			$query .= '"sourceIP" = \'' . $sourceIP . '\' AND ';
		} else {
			$query .= '"sourceIP" IS NULL AND ';
		}
		if ($sourcePort) {
			$query .= '"sourcePort" = \'' . $sourcePort . '\' AND ';
		} else {
			$query .= '"sourcePort" IS NULL AND ';
		}
		if ($destinationIP) {
			$query .= '"destinationIP" = \'' . $destinationIP . '\' AND ';
		} else {
			$query .= '"destinationIP" IS NULL AND ';
		}
		if ($destinationPort) {
			$query .= '"destinationPort" = \'' . $destinationPort . '\' AND ';
		} else {
			$query .= '"destinationPort" IS NULL AND ';
		}
		$query .= '"startTime" = \'' . $startTime . '\' AND ';
		$query .= '"endTime" = \'' . $endTime . '\'';
		
		$result = @pg_query($postgreSQL, $query);

		if (@pg_num_rows($result)) {
			$row = @pg_fetch_assoc($result);
			return $row['id'];
		}

		return false;
	}

	function prepareBackgroundNeoflowQuery(&$postgreSQL, &$_POST) {
		/*
		 * Computes a random MD5 hash that is not currently a query ID. It will be
		 * the query ID of the new query.
		 */
		do {
			$queryID = hash('md5', rand());
		} while (isNeoflowQueryID($postgreSQL, $queryID));
		/* Adds the Neoflow query about to be started to the Neoflow query index. */
		$protocol = false;
		if ($_POST['queryProtocol'] == 'tcp') {
			$protocol = 6;
		} else if ($_POST['queryProtocol'] == 'udp') {
			$protocol = 17;
		}	
		$sourceIP = (($_POST['querySourceIP'])?sprintf('%u', ip2long($_POST['querySourceIP'])):false);
		$sourcePort = $_POST['querySourcePort'];
		$destinationIP = (($_POST['queryDestinationIP'])?sprintf('%u', ip2long($_POST['queryDestinationIP'])):false);
		$destinationPort = $_POST['queryDestinationPort'];
		$startTime = strtotime($_POST['queryStartTime']);
		$endTime = strtotime($_POST['queryEndTime']);

		if (!insertPGRow($postgreSQL, 'Indexes', 'neoflowQueries', 
						 $queryID,
						 $protocol,
						 $sourceIP,
						 $sourcePort,
						 $destinationIP,
						 $destinationPort,
						 $startTime,
						 $endTime,
						 time()))
		{
			return false;
		}
		/*				
		 * Creates a table in the "Neoflow" schema for the queryNeoflow-SQL program
		 * to store results in.
		 */
		if (!@pg_query($postgreSQL, 'CREATE TABLE "NeoflowQueries"."' . $queryID . '" ' .
									'("protocol" uint16 NOT NULL, "sourceIP" uint32 NOT NULL, "destinationIP" uint32 NOT NULL, ' .
									'"sourcePort" uint16 NOT NULL, "destinationPort" uint16 NOT NULL, ' .
									'"startTime" uint32 NOT NULL, "endTime" uint32 NOT NULL, "bytesTransferred" uint32 NOT NULL, ' .
									'"tcpFlags" uint16 NOT NULL)')) {
			return false;
		}
		return $queryID;
	}
?>
