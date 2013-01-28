<?php
define('HBF_RUNNING', 0);
define('HBF_COMPLETED', 1);
define('HBF_CANCELLED', 2);
define('HBF_PAUSED', 3);
define('HBF_FAILED',4);

$searchState=array(HBF_RUNNING=>"Running",HBF_COMPLETED=>"Completed",HBF_CANCELLED=>"Cancelled",HBF_PAUSED=>"Paused",HBF_FAILED=>"Failed");

define('PROTOCOL_TCP',6);
define('PROTOCOL_UDP',17);

function int2protocol($p){
	switch($p){
		case PROTOCOL_TCP:
			return 'TCP';
		case PROTOCOL_UDP;
			return 'UDP';
		default:
			return 'UNKNOWN';
			}
		}
/*
* Signal definitions. PHP used to provide these, but no longer does, so we
* define them and hope they don't change.
*/
define('SIGKILL', 9);
define('SIGSTOP', 17);
define('SIGCONT', 19);


define ('UPLOAD_ERR_OK', 0);
define ('UPLOAD_ERR_INI_SIZE', 1);
define ('UPLOAD_ERR_FORM_SIZE', 2);
define ('UPLOAD_ERR_PARTIAL', 3);
define ('UPLOAD_ERR_NO_FILE', 4);
define ('UPLOAD_ERR_NO_TMP_DIR', 6);
define ('UPLOAD_ERR_CANT_WRITE', 7);
define ('UPLOAD_ERR_EXTENSION', 8);
function sec_to_time($seconds) {
   $hours = floor($seconds / 3600);
      $minutes = floor($seconds % 3600 / 60);
	     $seconds = $seconds % 60;

		    return sprintf("%d:%02d:%02d", $hours, $minutes, $seconds);
			}
function getASNByIP(&$postgreSQL, &$ip) {
    $result = pg_query($postgreSQL, 'SELECT "asn" FROM ' .
                                    '"Maps"."asIPBlocks" WHERE ' .
                                    '"firstIP" <= \'' . $ip . '\' AND ' .
                                    '"lastIP" >= \'' . $ip . '\' ORDER BY ' .
                                    '"lastIP" - "firstIP" LIMIT 1');
    if ($row = pg_fetch_row($result)) {
	      return $row[0];
     }
	return 0;
}
function getASDescriptionByNumber(&$postgreSQL, &$asNumber) {
    $result = pg_query($postgreSQL, 'SELECT "asDescription" FROM "Maps".' .
	                                    '"asNames" WHERE "asNumber" = \'' .
                                    $asNumber . '\'');
	$row = pg_fetch_row($result);
	return $row[0];
}
function getCountryNumberByIP(&$postgreSQL, &$ip) {
      $result = pg_query($postgreSQL, 'SELECT "countryNumber" FROM ' .
                                     '"Maps"."countryIPBlocks" WHERE ' .
                                      '"firstIP" <= \'' . $ip . '\' AND ' .
                                      '"lastIP" >= \'' . $ip . '\'');
      if ($row = pg_fetch_row($result)) {
	        return $row[0];
	    }
	 return 0;
	}
function getCountryNameMap($pg,$countryNumber,&$countryCode,&$countryName){
	$result=pg_query($pg,'SELECT "countryCode","countryName" from "Maps"."countryNames" WHERE "countryNumber"=\'' . $countryNumber . '\'');
	$row=pg_fetch_assoc($result);
	$countryCode=$row['countryCode'];
	$countryName=$row['countryName'];
}
function isHTTP($pg,$queryID,$filter=''){
	return false;
	if($filter==''){
		$query='SELECT "filter" FROM "Indexes"."searchQueries" WHERE "id"=\'' . $queryID . '\'';
		$result_set=@pg_query($pg,$query);
		$row=@pg_fetch_assoc($result_set);
		$filter=$row['filter'];
		}
	if(strpos($filter,'http_host')==false && strpos($filter,'http_uri')==false && strpos($filter,'http_referer')==false){
		return false;
		}
	else
		return true;
}

function showPayloadSearchSummary($pg,$queryId,&$data){
	global $searchState;
	$result_set=@pg_query($pg,'SELECT * FROM "Indexes"."payloadQueries" WHERE "id"=\'' . $queryId . '\'');
	$row=@pg_fetch_assoc($result_set);
	$toggle_btn_value=(($row['status']!=HBF_PAUSED)?'Pause':'Resume');	
	$data['rows'][]=array('id'=>'','cell'=>array('Query ID',$row['id']));
	$data['rows'][]=array('id'=>'','cell'=>array('Query Name',$row['name']));
	$data['rows'][]=array('id'=>'','cell'=>array('Started By',$row['username']));
	$data['rows'][]=array('id'=>'','cell'=>array('Filter',$row['filter']));
	$data['rows'][]=array('id'=>'','cell'=>array('Status',$searchState[$row['status']]));
	$data['rows'][]=array('id'=>'','cell'=>array('PID',$row['pid']));
	$data['rows'][]=array('id'=>'','cell'=>array('Query String Length',$row['queryStringLength']));
	$data['rows'][]=array('id'=>'','cell'=>array('Match Length',$row['matchLength']));	
	$data['rows'][]=array('id'=>'','cell'=>array('Details',$row['details']));
	$data['rows'][]=array('id'=>'','cell'=>array('Start Time',@date('Y-m-d H:i:s',$row['startTime'])));
	$data['rows'][]=array('id'=>'','cell'=>array('Pause Time',($row['pauseTime']==''?'':@date('Y-m-d H:i:s',$row['pauseTime']))));
	$data['rows'][]=array('id'=>'','cell'=>array('Resume Time',($row['resumeTime']==''?'':@date('Y-m-d H:i:s',$row['resumeTime']))));
	$data['rows'][]=array('id'=>'','cell'=>array('Duration',($row['duration']==''?'':sec_to_time($row['duration']))));
	$data['rows'][]=array('id'=>'','cell'=>array('Data Start Time',$row['dataStartTime']));
	$data['rows'][]=array('id'=>'','cell'=>array('Data End Time',$row['dataEndTime']));
	$data['rows'][]=array('id'=>'','cell'=>array(
												'<a href="/search/#/queries/payload/'.$row['id'].'/results">Search Results</a>',
												'<input 
													id="toggle_payload_'.$row['id'] . '" 
													type=submit 
													onClick=\'pauseSearch("' . $row['id'] . '","payload")\' 
													value="'. $toggle_btn_value .'" ' . ($row['status']!=HBF_PAUSED&&$row['status']!=HBF_RUNNING?'disabled=true':'') . ' /> 
												<input 
													type=submit 
													value="Cancel" 
													onClick=\'cancelSearch("'.$row['id'].'","payload")\'
													' . ($row['status']!=HBF_PAUSED&&$row['status']!=HBF_RUNNING?'disabled=true':'') . ' 
													/>
												<input 
													type=submit 
													value="Delete"
													onClick=\'deleteSearch("'.$row['id'].'","payload")\'
												/>'
												));
	}

function showFlowSearchSummary($pg,$queryId,&$data){
			global $searchState;
			$result_set=@pg_query($pg,'SELECT * FROM "Indexes"."searchQueries" WHERE "id"=\'' . $_GET['queryid'] . '\'');
			$row=@pg_fetch_assoc($result_set);
			$toggle_btn_value=(($row['status']!=HBF_PAUSED)?'Pause':'Resume');
			$data['rows'][]=array('id'=>'','cell'=>array('Query ID',$row['id']));
			$data['rows'][]=array('id'=>'','cell'=>array('Query Name',$row['name']));
			$data['rows'][]=array('id'=>'','cell'=>array('Started By',$row['username']));
			$data['rows'][]=array('id'=>'','cell'=>array('Filter',$row['filter']));
			$data['rows'][]=array('id'=>'','cell'=>array('Status',$searchState[$row['status']]));
			$data['rows'][]=array('id'=>'','cell'=>array('Number of Result(s)',$row['numResults']));
			$data['rows'][]=array('id'=>'','cell'=>array('PID',$row['pid']));
			$data['rows'][]=array('id'=>'','cell'=>array('Details',$row['details']));
			$data['rows'][]=array('id'=>'','cell'=>array('Start Time',@date('Y-m-d H:i:s',$row['startTime'])));
			$data['rows'][]=array('id'=>'','cell'=>array('Pause Time',($row['pauseTime']==''?'':@date('Y-m-d H:i:s',$row['pauseTime']))));
			$data['rows'][]=array('id'=>'','cell'=>array('Resume Time',($row['resumeTime']==''?'':@date('Y-m-d H:i:s',$row['resumeTime']))));
			$data['rows'][]=array('id'=>'','cell'=>array('Duration',($row['duration']==''?'':sec_to_time($row['duration']))));
			$data['rows'][]=array('id'=>'','cell'=>array('Time Left',($row['timeLeft']==''?'':@date('Y-m-d H:i:s',$row['timeLeft']))));
			$data['rows'][]=array('id'=>'','cell'=>array('Data Start Time',$row['dataStartTime']));
			$data['rows'][]=array('id'=>'','cell'=>array('Data End Time',$row['dataEndTime']));
			$data['rows'][]=array('id'=>'','cell'=>array(
															'<a href="/search/#/queries/flow/'.$row['id'].'/results">Search Results</a>',
															'<input
																type=submit 
																onClick=\'pauseSearch("' . $row['id'] . '","flow")\' 
																id="toggle_flow_' . $row['id'] . '"
																value="'. $toggle_btn_value .'" ' . ($row['status']!=HBF_PAUSED&&$row['status']!=HBF_RUNNING?'disabled=true':'') . ' 
																/> 
															<input 
																type=submit 
																value="Cancel" 
																onClick=\'cancelSearch("'.$row['id'].'","flow")\'
																' . ($row['status']!=HBF_PAUSED&&$row['status']!=HBF_RUNNING?'disabled=true':'') . ' 
																/>
															<input 
																type=submit 
																value="Delete"
																onClick=\'deleteSearch("'.$row['id'].'","flow")\'
																/>'
															));

}
$col_model_payload_level_1="[
							{display:'Source IP',name:'sourceIP',width:110,align:'center',sortable:true},
							{display:'Source Country',name:'sourcecountry',width:150,align:'center',sortable:false},
							{display:'Source Autonomous System',name:'src_asn',width:150,align:'center',sortable:false},
							{display:'Destination IP',name:'destinationIP',width:110,align:'center',sortable:true},
							{display:'Destination Country',name:'destinationcountry',width:150,align:'center',sortable:false},
							{display:'Destination Autonomous System',name:'dest_asn',width:150,align:'center',sortable:false},
							{display: 'Count',name:'count',width:70,align:'center',sortable:true}
							],";
$col_model_payload_level_2="[
							{display:'Protocol',name:'protocol',width:100,align:'center',sortable:true},
							{display:'Source IP',name:'sourceIP',width:150,align:'center',sortable:true},
							{display:'Source Port',name:'sourcePort',width:150,align:'center',sortable:true},
							{display:'Destination IP',name:'destinationIP',width:150,align:'center',sortable:true},
							{display:'Destination Port',name:'destinationPort',width:150,align:'center',sortable:true},
							{display:'Start Time',name:'startTime',width:200,align:'center',sortable:true},
							{display:'End Time',name:'endTime',width:200,align:'center',sortable:true},
							],";
$col_model_payload_level_3="[
							{display:'Request Version',name:'httpRequestVersion',width:100,align:'center',sortable:true},
							{display:'Type',name:'httpRequestType',width:50,align:'center',sortable:false},
							{display:'URI',name:'httpURI',width:250,align:'center',sortable:false},
							{display:'Host',name:'httpHost',width:150,align:'center',sortable:true},
							{display:'User Agent',name:'httpUserAgent',width:150,align:'center',sortable:false},
							{display:'Referer',name:'httpReferer',width:200,align:'center',sortable:false},
							{display: 'Response Version',name:'httpResponseVersion',width:100,align:'center',sortable:true},
							{display: 'Status',name:'httpStatus',width:50,align:'center',sortable:true},
							{display: 'Reason',name:'httpReason',width:50,align:'center',sortable:true},
							{display: 'Response',name:'httpResponse',width:50,align:'center',sortable:true},	
							{display: 'Content Type',name:'httpContentType',width:100,align:'center',sortable:true}
							],";

$col_model_http_flow_neoflow="[
								{display:'Protocol',name:'protocol',width: 50,align:'center',sortable: true},
								{display:'Source IP',name:'src_ip',width: 150,align:'center',sortable: true},
								{display:'Source Port', name:'src_port',width: 150, align:'center',sortable:true},
								{display:'Destination IP', name: 'dst_ip',width: 150, align:'center',sortable:true},
								{display:'Destination Port',name:'dst_port',width:150,align:'center',sortable:true},
								{display:'Start Time',name:'start_time',width: 150,align:'center',sortable:true},
								{display:'End Time',name:'end_time',width: 150, align:'center',sortable:true},
								{display:'Packet Count',name:'packet_count',width:100,align:'center',sortable:true},
								{display:'Byte Count',name:'byte_count',width:150,align:'center',sortable:true}
								],";
$col_model_search_results="[
						{display:'#', name:'sno',width:15,align:'center'},
						{display:'Type',name:'type', width:80,align:'center',sortable:true},
						{display: 'Name', name: 'name',width:150,align:'center',sortable:true},
						{display: 'Started By', name:'username',width:100,align:'center',sortable:true},
						{display: 'Start Time', name:'startTime',width:150,align:'center',sortable:true},
						{display: 'Status',name:'status',width:100,align:'center',sortable:true},
						{display: 'Controls',name:'controls',width:300,align:'center'}
					],";

$col_model_http_flow="[
						{display: 'Request Version', name: 'request_version',width:100,align:'center', sortable:true},
						{display: 'Type', name:'request_type',width:50,align:'center',sortable:true},
						{display: 'URI',name:'request_uri',width:200,align:'center',sortable:true},
						{display: 'Host',name:'request_host',width:150,align:'center',sortable:true},
						{display: 'User Agent', name:'request_useragent',width:150,align:'center',sortable:true},
						{display: 'Referer',name:'request_referer',width:150,align:'center',sortable:true},
						{display: 'Response Version',name:'response_version',width:50,align:'center',sortable:true},
						{display: 'Status', name:'response_status',width:50,align:'center',sortable:true},
						{display: 'Reason',name:'response_reason',width:50,align:'center',sortable:true},
						{display: 'Response', name:'response_response', width:50, align:'center',sortable:true},
						{display: 'Content Type', name:'response_contenttype',width:100,align:'center',sortable:true}

					],";

?>
