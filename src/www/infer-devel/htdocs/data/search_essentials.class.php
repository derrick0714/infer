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

function isSearchQueryID(&$postgreSQL, &$queryID, $searchType) {
	if($searchType=='flow') {
		$result = pg_query($postgreSQL,
							'SELECT * FROM "Indexes"."searchQueries" ' .
							'WHERE "id" = \'' . $queryID . '\'');
	}
	else {
		$result=pg_query($postgreSQL,
						  'SELECT * FROM "Indexes"."payloadQueries" ' .
						  'WHERE "id" = \'' . $queryID . '\'');
	}
	return (pg_num_rows($result) > 0);
}

class search_essentials
{
function breadCrumbs($path_info){
	$bc=array();
//	print_r($path_info);
	if(sizeof($path_info)==1){
		$bc['All Queries']=($path_info[1]==''?'':'/search/#/queries');
		$bc['Flow Queries']='/search/#/queries/flow';
		$bc['Payload Queries']='/search/#/queries/payload';
	}
	else if(sizeof($path_info)==2){
		$bc=array('All Queries'=>'/search/#/queries',ucfirst($path_info[1]). ' Queries'=>'');	
	}
	else{
		$bc=array('All Queries'=>'/search/#/queries',ucfirst($path_info[1]). ' Queries'=>'/search/#/queries/'.$path_info[1]);
		if($path_info[1]=='flow'){
			$bc['Results']=($path_info[3]!=''?'/search/#/queries/flow/'.$path_info[2].'/results':'');
			if($path_info[3]!=''){
				$bc['Index [' .$path_info[3].']']='';
			}
		}
		if($path_info[1]=='payload'){
			$bc['Results']=($path_info[3]!=''?'/search/#/queries/payload/'.$path_info[2].'/results':'');
			if($path_info[3]!=''){
				if($path_info[5]==''){
					$bc[$path_info[3]. ' - ' . $path_info[3]]='';
				}else{
					$bc[$path_info[3] . ' - ' . $path_info[4]]='/search/#/queries/payload/'.$path_info[2].'/results/'.$path_info[3].'/'.$path_info[4];
					$bc[int2protocol($path_info[5]) . ' - Ports '. $path_info[6] . ' - '  . $path_info[7]]='';
				}
			}
		}
	}
	return $bc;
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


}
?>
