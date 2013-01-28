<?php
header('Content-type: text/plain');
$DOCUMENT_ROOT = $_SERVER["DOCUMENT_ROOT"];
require_once($DOCUMENT_ROOT . '/data/search_essentials.class.php');
$path_info=explode('/',$_SERVER['PATH_INFO']);
function fnRenderFunction($type) {
	return <<<EOF
function(oObj) {
	var action_type = '$type';
	var button_label = action_type;
	if (action_type == 'Pause' && oObj.aData[6] == 'Paused') {
		button_label = "Resume";
	}
	return '<form method="post" action="' + oObj.aData[oObj.iDataColumn] + '"><input type="submit" name="submit" value="' + button_label + '"></form>';
}
EOF;
}
	
$aoColumns=array( 
	array(
		"sName"=>"Index",
		"sType"=>"numeric",
		"bSortable"=>false,
		"sTitle"=>"Index",
		"bSearchable"=>false
	),
	array(
		"sName"=>"row_id",
		"sType"=>"string",
		"bSortable"=>false,
		"bSearchable"=>false,
		"bVisible"=>false,
		"sTitle"=>"row_id",
		"bSearchable"=>false
	),
	array(
		"sName"=>"type",
		"sType"=>"string",
		"bSortable"=>true,
		"sTitle"=>"Type",
		"bSearchable"=>false
	),
	array(
		"sName"=>"name",
		"sType"=>"string",
		"sTitle"=>"Name"
	),
	array(
		"sName"=>"username",
		"sType"=>"string",
		"sTitle"=>"Started By",
		"bSearchable"=>false
	),
	array(
		"sName"=>"startTime",
		"sType"=>"date",
		"sTitle"=>"Start Time",
		"bSearchable"=>false
	),
	array(
		"sName"=>"status",
		"sType"=>"string",
		"sTitle"=>"Status",
		"bSearchable"=>false
	),
	array(
		"sName"=>"pauseResume",
		"sType"=>"string",
		"bSortable"=>false,
		"bSearchable"=>false,
		"fnRender"=> fnRenderFunction("Pause")
	),
	array(
		"sName"=>"cancel",
		"sType"=>"string",
		"bSortable"=>false,
		"bSearchable"=>false,
		"fnRender"=> fnRenderFunction("Cancel")
	),
	array(
		"sName"=>"delete",
		"sType"=>"string",
		"bSortable"=>false,
		"bSearchable"=>false,
		"fnRender"=> fnRenderFunction("Delete")
	)
);

$output["url"]='/data/search/index';
$output['aoColumns']=$aoColumns;
$output['rowClick']='alert("xxx");';
$output['breadCrumb']=search_essentials::breadCrumbs($path_info);
$output['sDom']='<"toolbar">flrtip';
$output['aaSorting']=array(array(5,'desc'),array(6,'asc'));
echo json_encode($output);

?>
