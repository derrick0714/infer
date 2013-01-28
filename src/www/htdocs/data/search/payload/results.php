<?php
require_once('../../pg.include.php');
require_once('../../essentials.class.php');
require_once('../../search_essentials.class.php');
header('Content-type: text/plain');
$path_info=explode('/',$_SERVER['PATH_INFO']);
$numResults=0;
$data=array();
if($path_info[2]==''){
	$result_set=pg_query($pg,'SELECT "sourceIP","destinationIP",count("protocol") AS "count" FROM "PayloadQueries"."'.$path_info[1].'" group by "sourceIP","destinationIP"');
	$numResults=@pg_num_rows($result_set);
	$colNames=array(0=>'"sourceIP"',3=>'"destinationIP"');
	if($_GET['iSortCol_0']!=''){
		$orderBy=' Order By ' . $colNames[0] . ' ' . $_GET['sSortDir_0'];
		if($_GET['iSortCol_1']!='')
			$orderBy.=' , ' . $colNames[1] . ' ' . $_GET['sSortDir_1'];
	}
	$result_set=pg_query($pg,'SELECT "sourceIP","destinationIP",count("protocol") AS "count" FROM "PayloadQueries"."'.$path_info[1].'" group by "sourceIP","destinationIP" ' . $orderBy . ' Offset ' . $_GET['iDisplayStart'] . ' limit ' . $_GET['iDisplayLength']);
	while($row=pg_fetch_assoc($result_set)){
		$src_countryNumber=search_essentials::getCountryNumberByIP($pg,$row['sourceIP']);
		search_essentials::getCountryNameMap($pg,$src_countryNumber,$src_countryCode,$src_countryName);
		$dst_countryNumber=search_essentials::getCountryNumberByIP($pg,$row['destinationIP']);
		search_essentials::getCountryNameMap($pg,$dst_countryNumber,$dst_countryCode,$dst_countryName);
		$data[]=array(long2ip($row['sourceIP']),
				essentials::country_flag($src_countryName,$src_countryCode),	
				search_essentials::getASDescriptionByNumber($pg,search_essentials::getASNByIP($pg,$row['sourceIP'])),
				long2ip($row['destinationIP']),
				essentials::country_flag($dst_countryName,$dst_countryCode),	
				search_essentials::getASDescriptionByNumber($pg,search_essentials::getASNByIP($pg,$row['destinationIP'])),
				$row['count']
				);
	}	
}elseif($path_info[4]==''){
 	$result_set=@pg_query($pg,'SELECT * FROM "PayloadQueries"."'.$path_info[1].'" WHERE "sourceIP"=\'' . ip2long($path_info[2]) .'\'  AND "destinationIP"=\''.ip2long($path_info[3]).'\'');
	$numResults=@pg_num_rows($result_set);
	$query='SELECT * FROM "PayloadQueries"."'.$path_info[1].'" WHERE "sourceIP"=\'' . ip2long($path_info[2]) .'\'  AND "destinationIP"=\''.ip2long($path_info[3]).'\'';	
	$colNames=array('protocol','sourceIP','sourcePort','destinationIP','destinationPort','startTime','endTime');
	$orderBy=array();
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
	$result_set=@pg_query($pg,$query . ' OFFSET ' . $_GET['iDisplayStart'] . ' LIMIT ' . $_GET['iDisplayLength']);
	while($row=@pg_fetch_assoc($result_set)){
		$data[]=array($row['protocol'],
						long2ip($row['sourceIP']),
						$row['sourcePort'],
						long2ip($row['destinationIP']),
						$row['destinationPort'],
						@date('Y-m-d H:i:s',$row['startTime']),
						@date('Y-m-d H:i:s',$row['endTime'])
						);
	}
}
else{
	$result_set=pg_query('SELECT * FROM "PayloadQueries"."'.$path_info[1].'" WHERE "sourceIP"=\''.ip2long($path_info[2]).'\' AND "destinationIP"=\''.ip2long($path_info[3]).'\'  AND "protocol"=\''.$path_info[4].'\' AND "sourcePort"=\''.$path_info[5].'\' AND "destinationPort"=\''.$path_info[6].'\'');
	$numResults=@pg_num_rows($result_set);
	while($row=pg_fetch_assoc($result_set)){
		$data[]=array($row['httpRequestVersion'],
						$row['httpRequestType'],
						$row['httpURI'],
						$row['httpHost'],
						$row['httpUserAgent'],
						$row['httpReferer'],
						$row['httpResponseVersion'],
						$row['httpStatus'],
						$row['httpReason'],
						$row['httpResponse'],
						$row['httpContentType']
						);
	}
}
$outputArray=array("sEcho"=>$_GET['sEcho'],"iTotalRecords"=>$numResults,"iTotalDisplayRecords"=>$numResults);
$outputArray['aaData']=$data;
echo json_encode($outputArray);
?>
