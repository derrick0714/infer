<?php
require_once('../../data/pg.include.php');
require_once('../search/common.php');
header('Content-type: text/plain');
$result_set=@pg_query($pg,'SELECT "id" from "Indexes"."searchQueries" UNION SELECT "id" from "Indexes"."payloadQueries"');
$total=@pg_num_rows($result_set);
/*Multi Col Sort*/
$orderBy=array();
$colNames=explode(',',$_GET['sNames']);
for($j=1;$j<$_GET['iColumns'];$j++){
	$i=$j-1;
	if(isset($_GET['iSortCol_' . $i ])){
		$orderBy[]= ' "' . $colNames[$_GET['iSortCol_'.$i]] . '" ' . $_GET['sSortDir_' . $i];
	}
}
if($_GET['sSearch']!='')
	$where=' WHERE "name" like \'%'.$_GET['sSearch'] . '%\' OR "username" like \'%'.$_GET['sSearch'] . '%\'  ';
$query='SELECT "id","name","username","startTime","status",\'Flow\' AS "type" FROM "Indexes"."searchQueries" ' . $where;
$query.=' UNION SELECT "id","name","username","startTime","status",\'Payload\' AS "type" from "Indexes"."payloadQueries" ' . $where;
if(sizeof($orderBy)>0){
	$query.=' Order By';
	foreach($orderBy as $o)
		$query.= ' ' . $o . ',';
	$query=rtrim($query,',');
}
$result_set=@pg_query($pg,$query);
$displayTotal=@pg_num_rows($result_set);
$query.=' Offset ' . $_GET['iDisplayStart'] . ' limit ' . $_GET['iDisplayLength'];
$result_set=@pg_query($pg,$query);
$index=1;
$outputArray=array("sEcho"=>$_GET['sEcho'],"iTotalRecords"=>$total,"iTotalDisplayRecords"=>isset($where)?$displayTotal:$total,"aaData"=>array());
$outputArray['get']=$_GET;
//$outputArray['q']=$query;
$controls='<input type=submit value="||"/><input type=submit value=X /><input type=submit value="[ ]" />';
while($row=@pg_fetch_assoc($result_set)){
	$outputArray['aaData'][]=array($index++,$row['type'],$row['name'],$row['username'],@date('Y-m-d H:i:s',$row['startTime']),$searchState[$row['status']],$controls);
}
echo json_encode($outputArray);
?>
