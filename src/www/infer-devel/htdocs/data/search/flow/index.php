<?php
require_once('../../pg.include.php');
require_once('../../../site/search/common.php');
header('Content-type: text/plain');
$result_set=@pg_query($pg,'SELECT "id" from "Indexes"."searchQueries"');
$total=@pg_num_rows($result_set);
/*Multi Col Sort*/
$orderBy=array();
$colNames=array(2=>"type",3=>"name",4=>"username",5=>"startTime",6=>"status");
for($j=1;$j<$_GET['iColumns'];$j++){
	$i=$j-1;
	if(isset($_GET['iSortCol_' . $i ]) ){//&& $_GET['bSortable_'.$_GET['iSortcol_'.$i]]=='true'){
		//$orderBy[]= ' "' . $colNames[$_GET['iSortCol_'.$i]] . '" ' . $_GET['sSortDir_' . $i];
		$orderBy[]=' "' . $colNames[$_GET['iSortCol_'.$i]] . '" ' . $_GET['sSortDir_'.$i];
	}
}
if($_GET['sSearch']!='')
	$where=' WHERE "name" like \'%'.$_GET['sSearch'] . '%\' OR "username" like \'%'.$_GET['sSearch'] . '%\'  ';
$query='SELECT "id","name","username","startTime","status",\'Flow\' AS "type" FROM "Indexes"."searchQueries" ' . $where;
//$query.=' UNION SELECT "id","name","username","startTime","status",\'Payload\' AS "type" from "Indexes"."payloadQueries" ' . $where;
if(sizeof($orderBy)>0){
	$query.=' Order By';
	foreach($orderBy as $o)
		$query.= ' ' . $o . ',';
	$query=rtrim($query,',');
}
$result_set=@pg_query($pg,$query);
$displayTotal=@pg_num_rows($result_set);
$query.=' Offset ' . $_GET['iDisplayStart'] . ' limit ' . $_GET['iDisplayLength'];
//echo $query . '                                 ';
$result_set=@pg_query($pg,$query);
$index=($_GET['iDisplayStart']==''?0:$_GET['iDisplayStart']);
$outputArray=array("sEcho"=>$_GET['sEcho'],"iTotalRecords"=>$total,"iTotalDisplayRecords"=>isset($where)?$displayTotal:$total,"aaData"=>array());
//$outputArray['get']=$_GET;
//$outputArray['q']=$query;
//$controls='<input type=submit value="||"/><input type=submit value=X /><input type=submit value="[ ]" />';
while($row=@pg_fetch_assoc($result_set)){
	if ($row['status'] == HBF_PAUSED) {
		$pause_uri = '/data/search/' . strtolower($row['type']) . '/resume/' . $row['id'];
	}
	else {
		$pause_uri = '/data/search/' . strtolower($row['type']) . '/pause/' . $row['id'];
	}
	$cancel_uri = '/data/search/' . strtolower($row['type']) . '/cancel/' . $row['id'];
	$delete_uri = '/data/search/' . strtolower($row['type']) . '/delete/' . $row['id'];
	$outputArray['aaData'][] =
		array(
			++$index,
			$row['id'],
			$row['type'],
			$row['name'],
			$row['username'],
			@date('Y-m-d H:i:s',$row['startTime']),
			$searchState[$row['status']],
			$pause_uri,
			$cancel_uri,
			$delete_uri
		);
}
echo json_encode($outputArray);
?>
