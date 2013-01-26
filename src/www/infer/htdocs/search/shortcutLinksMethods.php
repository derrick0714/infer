<?php
   include('../include/dbLink.php');
	
//used to show shortcut links
function displayShortcutLinks($links)
{
	$counter;//used to split links over more than one line
	//do not show anything if just one link
	if(count($links)==1)
		return;
	//split over second row
	echo '&nbsp;&nbsp;&nbsp;';
	foreach($links as $link)
	{
		echo $link;
		$counter++;
		if($counter%5==0)
			echo "<br>";
	}
	echo "<br><br>";
}
//populates array of links
function getLinks($numericIP,$startDate,$endDate)
{	
	if(isInternal($numericIP))
	 $allResults = getInternalIPQuery($numericIP,$startDate,$endDate);       
    else
	 $allResults = getExternalIPQuery($numericIP,$startDate,$endDate);  
 
    $query = "SELECT DISTINCT \"schemaName\", COUNT(\"schemaName\") as schema_count FROM ($allResults) as total_records GROUP BY \"schemaName\" ORDER BY schema_count DESC";
    //echo "Inside getLinks, query is: $query";//delete
	$result = pg_query($GLOBALS['postgreSQL'],$query);
	while($row = pg_fetch_row($result))
    {
		//echo $row[0]." ".$row[1]."<br>";
		$shortcutLink = new ShortcutLink($row[0],$row[1],$numericIP,$startDate,$endDate);
		$links[]=$shortcutLink;
	}	
	//print_r($links);	
	return $links;
}
function getOffsetShortcutLink($pager)
{
	//include('../include/Pager.php');
	$request = explode('/', substr($_SERVER['PATH_INFO'], 1));
	if(count($request)==4)//page one
	  return 0;
    else
	  return $request[4]*50-50;
}
?>