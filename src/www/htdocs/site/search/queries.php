<?php
$path_info=array();
$path_info=explode("/", $_SERVER['PATH_INFO']);
$tableUrl='/data/search/';
//print_r($path_info);
if($path_info[1]!=''){
	$tableUrl.=$path_info[1] ;
	if($path_info[2]=='')
		$tableUrl.=$path_info[1] . '/index';
	else
		$tableUrl.= '/summary/' . $path_info[2];
}else{
	$tableUrl.='index';
}
//Bread Crumbs
$title='<div style="float:left;">';
$title.=isset($path_info[1])?'<a href="/search/#/queries"> All Queries</a>':'All Queries';
$title.=($path_info[1]!='payload'?' | ' . ($path_info[1]=='flow'&&!isset($path_info[2])?'Flow Queries':'<a href="/search/#/queries/flow">Flow Queries</a>'):'');
$title.= ($path_info[1]!='flow'?' | ' . ($path_info[1]=='payload'&&!isset($path_info[2])?'Payload Queries':'<a href="/search/#/queries/payload">Payload Queries</a>'):'');
if($path_info[2]!=''){
	if($path_info[3]=='')
		$title.= ' | ' . ucfirst($path_flow[1]) . ' Search Summary [' . $path_info[2] . ']';
	else{
		$title.=' | <a href="/search/#/queries/' . $path_info[1] . '/' . $path_info[2] . '">' . ucfirst($path_info[1]) . ' Search Summary </a>';
		if($path_info[4]=='')
			$title.=' | ' . ucfirst($path_info[1]) . ' Search Results [' . $path_info[2] . ']';
		else{
			$title.=' | <a href="/search/#/queries/' . $path_info[1] . '/' . $path_info[2] . '/results"> Search Results</a>';
			$title.=' | ' . ($path_info[5]==''?'Index ['. $path_info[4] . ']': '[' . $path_info[4] . ' - ' . $path_info[5] . ']');
		}
	}
}
$title.='</div> ';
//echo $title . '<br/>';
?>
<div id="inner_wrapper">
	<div id=tableDiv >
	<?php $table = '<table id="dTable1" width="100%" class="display" border="0"><thead><tr></tr></thead><tbody style="text-align:center;"></tbody></table>';
		echo $table;
	?>
	</div>
</div>

<script type='text/javascript'>
function func(){
	//$.address.parameter('page','nan');
//	oTable.fnDestroy();
	loadTable();
	}
var oTable;
var tmp;
var table='<?php echo $table;?>';
var statusFilter='<select><option>All</option><option>Running</option><option>Paused</option><option>Cancelled</option></select>';
var prev_url='';
var _url='';
var jsonData;
$(document).ready(function(){
						});
function createSearchSummaryTable(data){
	/*Create Search Summary Table*/
	var table='<table style="text-align:left;" ><tr><td >Filter</td><td>'+data.filter+'</td><td>Duration</td><td>'+data.duration+'</td></tr>';
	table+='<tr><td>Number Of Results</td><td>'+data.numResults+'</td><td>PID</td><td>'+data.pid+'</td></tr>';
	table+='<tr><td>Data Start Time</td><td>' + data.dataStartTime + '</td><td>Data End Time</td><td>'+data.dataEndTime+'</td></tr>';
	table+='<tr><td>Time Left<td>' + (data.TimeLeft==undefined?'':data.TimeLeft) + '</td><td>Details</td><td>'+data.details+'</td></tr>';
	if(data.queryStringLength!=undefined)
		table+='<tr><td>Query String Length</td><td>'+data.queryStringLength+'</td><td>Match Length</td><td>'+data.matchLength+'</td></tr>';
	table+='<tr><td colspan=4 style="text-align:center;"><a href="/search/#/queries/'+data.type+'/'+data.id+'/results" > Results </a></td></tr>';
	table+='</table>';
	return table;
	}
function getSearchSummary(tr){
	/*Fire JSON call to get Search Summary*/
	var aData=oTable.fnGetData(tr);
	var summary;
	$.getJSON(	
			'/data/search/' + aData[2].toLowerCase()+ '/summary/' + aData[1],
			function(data)	{
					$('#dTable1 .search_summary_row_'+aData[2].toLowerCase()+'_'+aData[1]).html(createSearchSummaryTable(data));
					}
			);
	}
function makeBreadCrumbs(bc){
	var _bc='<div class="top" style="height:20px;" ><div style="float:left;clear:left;">';
	var flag=false;
	for(att in bc){
		if(flag)
			_bc+=' | ' ;
		flag=true;
		if(bc[att]=='')
			_bc+=att + ' ' ;
		else
		_bc+= '<a href="' + bc[att] + '">' + att + '</a> ';
		}
	if(isResultsPage()=='search')
		_bc+='</div><div style="float:right">Status Filter '+statusFilter+'</div>';
	else
		_bc+='</div><div style="float:right"></div>';
	_bc+='</div>';
	return _bc;
	}
function loadTable(){
	//Restore tableDiv contents
	$('#tableDiv').html(table );
	//Get table header/url info
	$.ajax({url:getDataTableUri(),
						dataType:'json',
						async:false,
						success:function(json){jsonData=json;}
						});
	//draw table
	// hack hack hack
	for (var col_idx in jsonData.aoColumns) {
		for (var key in jsonData.aoColumns[col_idx]) {
			if (key == "fnRender") {
				eval('jsonData.aoColumns[col_idx][key] = ' + jsonData.aoColumns[col_idx][key]);
			}
		}
	}
	$('#dTable1').dataTable({
		"bProcessing":true,
		"bServerSide":true,
		"sAjaxSource":jsonData.url,
		"sDom": jsonData.sDom,//'<"toolbar">flrtip',
		"aoColumns":jsonData.aoColumns,
		/*
		"fnServerData":function (sSource,aoData,fnCallback){
			$.address.autoUpdate(false);
			var aoData1=[];
			var index=0;
			for(var i=0;i<aoData.length ;i++){
			// eliminate unnecessary params
				if((aoData[i].name.match(/bRegex/)!=null) || (aoData[i].name.match(/bSearchable/)!=null) || (aoData[i].name.match(/bSortable/)!=null)||(aoData[i].name.match(/sSearch_/)!=null) ){
					continue;
				}
				else if(aoData[i].name=='sColumns' || aoData[i].name=='sNames'){// || aoData[i].name=='iColumns'){
					continue;
				}
				aoData1[index++]=aoData[i];
			}
			//Add params to URL todo:sSearch not being added for some reason
			for(var i=0;i<aoData1.length;i++){
				if(aoData1[i].name!='sEcho')
					$.address.parameter(aoData1[i].name,aoData1[i].value);
			}
			$.address.update();
			tmp=aoData;//for debuging purposed
			$.address.autoUpdate(true);
			$.getJSON(sSource,aoData1,function(json){fnCallback(json)});
		},
		*/
		"sPaginationType": "full_numbers",
		"aaSorting": jsonData.aaSorting, //[ [5,'desc'],[6,'asc']],
		"fnRowCallback": formatRows,//( nRow, aData, iDisplayIndex, iDisplayIndexFull )
	});

	$("div.toolbar").html(makeBreadCrumbs(jsonData.breadCrumb));
	oTable=$('#dTable1').dataTable();
	$('#dTable1 tbody tr').live('mouseover',function(){
								$(this).addClass('row_selected');;
								});
	$('#dTable1 tbody tr').live('mouseout',function(){ $(this).removeClass('row_selected'); } );

	$('#dTable1 tbody tr').live('click',function(){
							var rowData=oTable.fnGetData(this);
							var tr=this;
							var r=isResultsPage();
							if(r=='search'){
								oTable.fnOpen(this,'Loading Search Summary....',"search_summary_row_"+rowData[2].toLowerCase() + "_" +rowData[1]+' details' );
								getSearchSummary(this);
								$('#dTable1 .search_summary_row_'+rowData[2].toLowerCase() +"_"+rowData[1]).click(function(){
									oTable.fnClose(tr);
									});
							}else{
								var rsplit=r.split('_');
								if(rsplit[0]=='flow')
									$.address.value($.address.path() +'/' + rowData[1]);
								else if(rsplit[0]=='payload'){
									if(rsplit[1]==0)
										$.address.value($.address.path() + '/' + rowData[0] + '/' + rowData[3]);
									else if(rsplit[1]==1)
										$.address.value($.address.path() + '/' + rowData[0] + '/' + rowData[2] + '/' +rowData[4]);
									}
							}	
					});

}
function tab_uri_notify(e){ 
	var url=getDataTableUri();
	if(url != prev_url ){
		prev_url=url;
		loadTable();
	}
}
function isResultsPage(){
//ar pathN=$.address.value().split('?')[0].split('/');
	var pathN = $.address.pathNames();
	if(pathN[0]=='queries' && pathN[2]!=undefined){
		if(pathN[1]=='flow'){
			return 'flow_0';
		}
		else if(pathN[1]=='payload') {
			if(pathN.length==4)
				return 'payload_0';
			else if(pathN.length==6)
				return 'payload_1';
			else
				return 'payload_2';
			}
		}
	else
		return 'search';
}

function getDataTableUri(){
//	var pathN=$.address.value().split('?')[0].split('/');
	var pathN = $.address.pathNames();
//	alert('geturi ->' +pathN);
	if(pathN[3]!=undefined){
//		var r='/data_tables/search/' + pathN[2] + '/results/' + pathN[3];
		return '/data_tables/search/'+pathN[1]+'/results/' + pathN[2] + (pathN.length>4?('/' + pathN.slice(4,pathN.length).join('/')):''); //(pathN[5]==undefined?'':('/'+pathN[5]));
	}
	else if(pathN[2]!=undefined)
		return '/data_tables/search/'+ pathN[1]  +'/results/' + pathN[2];
	else if(pathN[1]!=undefined)
		return '/data_tables/search/' + pathN[1] + '/index';
	else
		return '/data_tables/search/index';
}

function getTableUri(){
//	var pathN=$.address.value().split('?')[0].split('/');
	var pathN = $.address.pathNames();
//	alert('geturi ->' +pathN);
	if(pathN[3]!=undefined)
		return '/data/search/'+pathN[1]+'/results/' + pathN[2] +(pathN[4]==undefined?'':('/'+pathN[4]));
	else if(pathN[2]!=undefined)
		return '/data/search/'+ pathN[1]  +'/results/' + pathN[2];
	else if(pathN[1]!=undefined)
		return '/data/search/' + pathN[1] + '/index';
	else
		return '/data/search/index';
}

function formatRows(nRow, aData, iDisplayIndex, iDisplayIndexFull ){
	var r=isResultsPage();
	//oTable.fnSetColumnVis(1,false);
	//oTable.fnSetColumnVis(10,false);
	if(r!='search'){
		if(r.split('_')[1]=='0'){
			if(aData[0]=='TCP')
				$(nRow).addClass('gradeA');
			else if(aData[0]=='UDP')
				$(nRow).addClass('gradeB');
			else
				$(nRow).addClass('gradeU');
		}else{
	//		oTable.fnSetColumnVis(1,true);
	//		oTable.fnSetColumnVis(10,true);
			if(aData[1]=='GET')
				$(nRow).addClass('gradeA');
			else if(aData[1]=='POST')
				$(nRow).addClass('gradeB');
			else
				$(nRow).addClass('gradeU');
		}
	}else{
/* // WTF??? -Justin
		$('td:eq(6)',nRow).bind('click',onCellClick);
		$('td:eq(7)',nRow).bind('click',onCellClick);
		$('td:eq(8)',nRow).bind('click',onCellClick);
		$('td:eq(6)',nRow).html('<input type=submit value="Pause"/>');
		$('td:eq(7)',nRow).html('<input type=submit value="Cancel"/>');
		$('td:eq(8)',nRow).html('<input type=submit value="Delete"/>');
*/
		if(aData[6]=='Completed'){
			$(nRow).addClass('gradeA');
		}
		else if(aData[6]=='Failed')
			$(nRow).addClass('gradeX');
		else if(aData[6]=='Running')
			$(nRow).addClass('gradeB');
		else if(aData[6]=='Paused')
			$(nRow).addClass('gradeC');
		else if(aData[6]=='Cancelled')
			$(nRow).addClass('gradeU');
	}
	return nRow;
}

function onCellClick(){
	$(this).html('Processing..');
	var pos=oTable.fnGetPosition(this);
	var tr=oTable.fnGetNodes(pos[0]);
	$('td:eq(5)',tr).html('Changed');
	$.address.parameter('new','ola');
	alert($.address.baseURL() +'' + $.address.hash() + ' -->' + location.hash);
}
</script>

