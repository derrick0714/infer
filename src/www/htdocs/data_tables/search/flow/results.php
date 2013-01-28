<?php
header('Content-type: text/plain');
require_once('../../../data/search_essentials.class.php');
$path_info=explode('/',$_SERVER['PATH_INFO']);
if($path_info[2]!=''){
	$aoColumns=array( 
					array("sName"=>"request_version","bSortable"=>true,"sTitle"=>"Request Version","bSearchable"=>false),
					array( "sName"=>"request_type","bSortable"=>true,"bSearchable"=>false,"sTitle"=>"Request Type"),	
					array("sName"=>"request_uri","bSortable"=>true,"bSearchable"=>"true","sTitle"=>"Request URI","bSearchable"=>false),
					array( "sName"=>"request_host","bSortable"=>true,"sTitle"=>"Request Host","bSearchable"=>false),
					array( "sName"=>"request_useragent","sTitle"=>"Request User Agent"),
					array( "sName"=>"request_referer","sTitle"=>"Request Referer"),	
					array( "sName"=>"request_version","sTitle"=>"Request Version","bSearchable"=>false),
					array( "sName"=>"request_status","sTitle"=>"Request Status","bSearchable"=>false),
					array( "sName"=>"request_reason","sTitle"=>"Request Reason","bSearchable"=>false),
					array( "sName"=>"request_response","bSortable"=>true,"bSearchable"=>false,"sTitle"=>"Request Response"),
					array( "sName"=>"response_contenttype","bSortable"=>true,"bSearchable"=>false,"sTitle"=>"Response Content Type"),
				);
}else{
	$aoColumns=array( 
					array("sName"=>"protocol","bSortable"=>true,"sTitle"=>"Protocol","bSearchable"=>false),
					array( "sName"=>"index","bSortable"=>true,"bSearchable"=>false,"bVisible"=>false,"sTitle"=>"index"),	
					array("sName"=>"src_ip","bSortable"=>true,"bSearchable"=>"true","sTitle"=>"Source IP","bSearchable"=>false),
					array( "sName"=>"src_port","bSortable"=>true,"sTitle"=>"Source Port","bSearchable"=>false),
					array( "sName"=>"dst_ip","sTitle"=>"Destination IP"),
					array( "sName"=>"dst_port","sTitle"=>"Destination Port"),	
					array( "sName"=>"startTime","sTitle"=>"Start Time","bSearchable"=>false),
					array( "sName"=>"endTime","sTitle"=>"End Time","bSearchable"=>false),
					array( "sName"=>"packet_count","sTitle"=>"Packet Count","bSearchable"=>false),
					array( "sName"=>"byte_count","bSortable"=>true,"bSearchable"=>false,"sTitle"=>"Byte Count"),
				);


}
$output["url"]='/data/search/flow/results/' . $path_info[1] . ($path_info[2]==''?'':'/'.$path_info[2]);
array_unshift($path_info,"");
$path_info[1]='flow';
$output['breadCrumb']=search_essentials::breadCrumbs($path_info);
$output['aoColumns']=$aoColumns;
$output['sDom']='<"toolbar">lrtip';
$output['aaSorting']=array(array(5,'desc'),array(6,'asc'));
echo json_encode($output);

?>
