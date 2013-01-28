<?php
header('Content-type: text/plain');
require_once('../../../data/search_essentials.class.php');
$path_info=explode('/',$_SERVER['PATH_INFO']);
$output=array();
if($path_info[2]==''){
	$aoColumns=array( 
					array("sName"=>"sourceIP","sTitle"=>"Source IP"),
					array("sName"=>"source_country","bSortable"=>false,"bSearchable"=>false,"sTitle"=>"Source Country","bSearchable"=>false),
					array( "sName"=>"source_autonomous_system","bSortable"=>false,"sTitle"=>"Source Autonomous System","bSearchable"=>false),
					array( "sName"=>"destinationIP","sTitle"=>"Destination IP"),
					array("sName"=>"destination_country","bSortable"=>false,"bSearchable"=>false,"sTitle"=>"Destination Country","bSearchable"=>false),
					array( "sName"=>"destination_autonomous_system","bSortable"=>false,"sTitle"=>"Destination Autonomous System","bSearchable"=>false),
					array( "sName"=>"count","bSortable"=>false,"bSearchable"=>false,"sTitle"=>"Count"),
				);
	$output["url"]='/data/search/payload/results/'.$path_info[1];	
}elseif($path_info[4]==''){
	$aoColumns=array( 
					array("sName"=>"protocol","bSortable"=>true,"sTitle"=>"Protocol","bSearchable"=>false),
					array("sName"=>"sourceIP","bSortable"=>true,"bSearchable"=>"true","sTitle"=>"Source IP","bSearchable"=>false),
					array("sName"=>"sourcePort","bSortable"=>true,"sTitle"=>"Source Port","bSearchable"=>false),
					array("sName"=>"destinationIP","sTitle"=>"Destination IP"),
					array("sName"=>"destinationPort","sTitle"=>"Destination Port"),	
					array("sName"=>"start_time","sTitle"=>"Start Time","bSearchable"=>false),
					array("sName"=>"end_time","sTitle"=>"End Time","bSearchable"=>false),
	
				);
	$output["url"]='/data/search/payload/results/'.$path_info[1].'/' . $path_info[2] . '/' . $path_info[3];		

}else{
	$aoColumns=array( 
					array("sName"=>"request_version","bSortable"=>true,"sTitle"=>"Request Version","bSearchable"=>false),
					array( "sName"=>"request_type","bSortable"=>true,"bSearchable"=>false,"sTitle"=>"Request Type"),	
					array("sName"=>"request_uri","bSortable"=>true,"bSearchable"=>"true","sTitle"=>"Request URI","bSearchable"=>false),
					array( "sName"=>"request_host","bSortable"=>true,"sTitle"=>"Request Host","bSearchable"=>false),
					array( "sName"=>"request_useragent","sTitle"=>"Request User Agent"),
					array( "sName"=>"request_referer","sTitle"=>"Request Referer"),	
					array( "sName"=>"request_version","sTitle"=>"Response Version","bSearchable"=>false),
					array( "sName"=>"request_status","sTitle"=>"Status","bSearchable"=>false),
					array( "sName"=>"request_reason","sTitle"=>"Reason","bSearchable"=>false),
					array( "sName"=>"request_response","bSortable"=>true,"bSearchable"=>false,"sTitle"=>"Response"),
					array( "sName"=>"response_contenttype","bSortable"=>true,"bSearchable"=>false,"sTitle"=>"Content Type"),
				);
	$output["url"]='/data/search/payload/results/'.$path_info[1].'/' . $path_info[2] . '/' . $path_info[3].'/'.$path_info[4].'/'.$path_info[5].'/'.$path_info[6];		
}
$output['aoColumns']=$aoColumns;
$output['sDom']='<"toolbar">lrtip';
$output['aaSorting']=array(array(0,'asc'));
array_unshift($path_info,"");
$path_info[1]='payload';
$output['breadCrumb']=search_essentials::breadCrumbs($path_info);
echo json_encode($output);

?>
