<?php
require_once('../../pg.include.php');
require_once('../../essentials.class.php');
header('Content-type: text/plain');
$path_info=explode('/',$_SERVER['PATH_INFO']);
//print_r($path_info);
//print_r($_GET);
if($path_info[2]==''){
	$colNames=array('protocol','','src_ip','src_port','dst_ip','dst_port','start_time','end_time','packet_count','byte_count');
	$result_set=pg_query($pg,'SELECT * FROM "Indexes"."searchQueries" WHERE "id"=\'' . $path_info[1] . '\'');
	$row=pg_fetch_assoc($result_set);
	if($row['numResults']!=''){
		$numResults=$row['numResults'];
	}else{
		$result_set=pg_query($pg,'SELECT COUNT(*) AS "C" FROM "SearchQueries"."' .$path_info[1] .'_neoflow"');
		$row=pg_fetch_assoc($result_set);
		$numResults=$row["C"];
	}
	$numResults=(int)$numResults;
	$orderBy=array();
	$query='SELECT * FROM "SearchQueries"."'.$path_info[1].'_neoflow" '; 
	for($j=1;$j<$_GET['iColumns'];$j++){
		$i=$j-1;
		if(isset($_GET['iSortCol_' . $i ]) ){//&& $_GET['bSortable_'.$_GET['iSortcol_'.$i]]=='true'){
			//$orderBy[]= ' "' . $colNames[$_GET['iSortCol_'.$i]] . '" ' . $_GET['sSortDir_' . $i];
			$orderBy[]=' "' . $colNames[$_GET['iSortCol_'.$i]] . '" ' . $_GET['sSortDir_'.$i];
		}
	}
	if(sizeof($orderBy)>0){
		$query.=' Order By';
		foreach($orderBy as $o)
			$query.= ' ' . $o . ',';
		$query=rtrim($query,',');
	}
	$query.=' OFFSET ' . ($_GET['iDisplayStart']==''?'0':$_GET['iDisplayStart']) . ' limit '. ($_GET['iDisplayLength']==''?'10':$_GET['iDisplayLength']) ;
	$result_set=pg_query($pg,$query);//'SELECT * FROM "SearchQueries"."'.$path_info[1].'_neoflow" OFFSET ' . ($_GET['iDisplayStart']==''?'0':$_GET['iDisplayStart']) . ' limit '. ($_GET['iDisplayLength']==''?'10':$_GET['iDisplayLength']) );
	$outputArray=array("sEcho"=>$_GET['sEcho'],"iTotalRecords"=>$numResults,"iTotalDisplayRecords"=>$numResults);
	$data=array();
	while($row=@pg_fetch_assoc($result_set)){
	$data[]=array(
				essentials::protocol_name($row['protocol']),
				$row['index'],
				long2ip($row['src_ip']),
				$row['src_port'],
				long2ip($row['dst_ip']),
				$row['dst_port'],
				@date('Y-m-d H:i:s',$row['start_time']),
				@date('Y-m-d H:i:s',$row['end_time']),
				$row['packet_count'],
				essentials::size_display($row['byte_count'])
				);
	}
}else{
	$result_set=@pg_query($pg,'SELECT COUNT(*) AS "C" FROM "SearchQueries"."'.$path_info[1].'_http" where "neoflow_index"=\''.$path_info[2].'\'');
	$row=pg_fetch_assoc($result_set);
	$numResults=$row["C"];
	$result_set=pg_query($pg,'SELECT * FROM "SearchQueries"."'.$path_info[1].'_http" where "neoflow_index"=\''.$path_info[2].'\' offset ' . ($_GET['iDisplayStart']==''?'0':$_GET['iDisplayStart']). ' limit  '.($_GET['iDisplayLength']==''?'10':$_GET['iDisplayLength']) );
	$outputArray=array("sEcho"=>$_GET['sEcho'],"iTotalRecords"=>$numResults,"iTotalDisplayRecords"=>$numResults);
	$data=array();
	while($row=pg_fetch_assoc($result_set)){
	$data[]=array(
				$row['request_version'],
				$row['request_type'],
				$row['request_uri'],
				$row['request_host'],
				$row['request_useragent'],
				$row['request_referer'],
				$row['request_version'],
				$row['request_status'],
				$row['request_reason'],
				$row['request_response'],
				$row['response_contenttype']);
	}

}
$outputArray['aaData']=$data;
echo json_encode($outputArray);
?>
