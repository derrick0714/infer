<?php
require_once('../essentials.class.php');
require_once('../pg.include.php');

$essentials = new essentials();

$roleid = 0;
$date = $essentials->dateParser($argv[1], strtotime(date('Y-m-d', time() - 86400)));
$daysToShow = 14;

$currentDate = $date;
for($i = 0; $i < $daysToShow; $i++, $currentDate -= 86400)
{
	$query = @pg_query($pg, "SELECT COUNT(*) FROM \"Roles\".\"".date('Y-m-d', $currentDate)."\" WHERE \"role\" = '$roleid'");
	if($query)
	{
		$query = @pg_fetch_assoc($query);
		if($query) $history[] = array($currentDate, $query['count']);
		else $history[] = array($currentDate, 0);
	}
}

$role = array(
	"role" => array(
		"name" => "dark_space_bot",
		"display_name" => "Dark Space Bot",
		"history" => $history
	)
);

echo json_encode($role);
?>