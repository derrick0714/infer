<?php

$rankWhiteListSchema = "RankWhiteList";

function isRankWhitelisted(&$postgreSQL, $date, $longIP, $rankName) {
	global $rankWhiteListSchema;

	if (existsPGTable($postgreSQL, $rankWhiteListSchema, $date)) {
		$rankWhiteListTable = $date;
	}
	else if (!($rankWhiteListTable = getPreviousPGTable($postgreSQL,
														$rankWhiteListSchema,
														$date)))
	{
		return false;
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

	return false;
}

?>
