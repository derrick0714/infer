<?php
require_once('../essentials.class.php');
require_once('../search_essentials.class.php');
require_once('../pg.include.php');

//if Between <start_date> <end_date> is specified
if(preg_match("/between\s*(\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}|\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}|\d{4}-\d{2}-\d{2}\s\d{2}|\d{4}-\d{2}-\d{2}|\d{4}-\d{2}|\d{4})\s*and\s*(\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}|\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}|\d{4}-\d{2}-\d{2}\s\d{2}|\d{4}-\d{2}-\d{2}|\d{4}-\d{2}|\d{4})/", $_POST['query'], $matches))
{
	$start_time=$matches[1];
	$end_time=$matches[2];
//if past <num> <days|months|years> is specified
}
elseif(preg_match("/past *(\d*) *(day|week|month|year)s?/", $_POST['query'], $matches))
{
	if($matches[1]=="")
		$matches[1]=1; //To change queries like "past day" to "past 1 day"
	$start_time = @date('Y-m-d H:i:s', strtotime($matches[1] . ' ' . $matches[2] . ' ago'));
	$end_time = @date('Y-m-d H:i:s', strtotime('now'));
//Default date Range
}
else {
	$start_time=@date('Y-m-d',strtotime('-1 day')) . " " . "00:00:00";
	$end_time=@date('Y-m-d',strtotime('-1 day')) . " " . "23:59:59";
}
if(get_magic_quotes_gpc())
	$_POST['query']=stripslashes($_POST['query']);
$queryID="";
$searchType='query';
if(isset($_POST['submit'])) {
	if((isset($_POST['query']) && $_POST['query']!='') || ($_FILE['uploaded_file']['error']!=UPLOAD_ERR_NO_FILE))
	{
		if(isset($_FILES['uploaded_file']) && $_FILES['uploaded_file']['error']!=UPLOAD_ERR_NO_FILE){
			$searchType='payload';
			$err=verifyPayloadSearchData(); //verify uploaded_file, offset, match_length, offset_length
			if($err!=""){
				echo $err;
				exit(0);
				}
		}
		//Generate unique QueryID
		do{
			$queryID=hash('md5',rand());
		}while(isSearchQueryID($pg,$queryID,$searchType));
		
		$cmd=" --start-time \"" . $start_time . "\" --end-time \"" . $end_time . "\" --query-id " . $queryID;

		//Parse "http_host" filter
		$count=preg_match("/http_host\ *=\ */",$_POST['query'],$matches,PREG_OFFSET_CAPTURE);
		$filter_chars=str_split($_POST['query']);
		if($count!=0){
			if($searchType!='query'){
				echo 'htt_host filter not supported for payload search';
				exit(0);
			}
			$start_pos=$matches[0][1];
			$end_pos=0;
			if($filter_chars[$start_pos+strlen($matches[0][0])]!='/'){
				echo "Invalid host regex";
				exit(0);
			}
			for($i=$start_pos+strlen($matches[0][0])+1;$i<strlen($_POST['query']);++$i){
				if($filter_chars[$i]=='/'){
					$end_pos=$i;
					break;
					}
				if($filter_chars[$i]=='\\' && $filter_chars[$i+1]=='/')
					++$i;

			}
			$host_regEx=substr($_POST['query'],$start_pos,$end_pos-$start_pos+1);
			$_POST['query']=str_replace($host_regEx," ",$_POST['query']);
			$host_regEx=substr($host_regEx,strlen($matches[0][0])+1,-1);
			$host_regEx=str_replace('\/',"/",$host_regEx);
			if(preg_match("/http_host\ *=\ */",$_POST['query'],$matches,PREG_OFFSET_CAPTURE)){
				echo "Only one http_host filter is allowed";
				exit(0);
			}
		}
		//Parse "http_uri" filter
		$count=preg_match("/http_uri\ *=\ */",$_POST['query'],$matches,PREG_OFFSET_CAPTURE);
		$filter_chars=str_split($_POST['query']);
		if($count!=0){
			if($searchType!='query'){
				echo 'htt_uri filter not supported for payload search';
				exit(0);
			}
			$start_pos=$matches[0][1];
			$end_pos=0;
			if($filter_chars[$start_pos+strlen($matches[0][0])]!='/'){
				echo "Invalid uri regex";
				exit(0);
			}
			for($i=$start_pos+strlen($matches[0][0])+1;$i<strlen($_POST['query']);++$i){
				if($filter_chars[$i]=='/'){
					$end_pos=$i;
					break;
				}
				if($filter_chars[$i]=='\\' && $filter_chars[$i+1]=='/')
					++$i;

			}
			$uri_regEx=substr($_POST['query'],$start_pos,$end_pos-$start_pos+1);
			$_POST['query']=str_replace($uri_regEx," ",$_POST['query']);
			$uri_regEx=substr($uri_regEx,strlen($matches[0][0])+1,-1);
			$uri_regEx=str_replace('\/','/',$uri_regEx);
			if(preg_match("/http_uri\ *=\ */",$_POST['query'],$matches,PREG_OFFSET_CAPTURE)){
				echo "Only one http_uri filter is allowed";
				exit(0);
			}
		}
		//Parse "http_referer" filter
		$count=preg_match("/http_referer\ *=\ */",$_POST['query'],$matches,PREG_OFFSET_CAPTURE);
		$filter_chars=str_split($_POST['query']);
		if($count!=0){
			if($searchType!='query'){
				echo 'http_referer filter not supported for payload search';
				exit(0);
			}
			$start_pos=$matches[0][1];
			$end_pos=0;
			if($filter_chars[$start_pos+strlen($matches[0][0])]!='/'){
				echo "Invalid referer regex";
				exit(0);
			}
			for($i=$start_pos+strlen($matches[0][0])+1;$i<strlen($_POST['query']);++$i){
				if($filter_chars[$i]=='/'){
					$end_pos=$i;
					break;
				}
				if($filter_chars[$i]=='\\' && $filter_chars[$i+1]=='/')
					++$i;
			}
			$referer_regEx=substr($_POST['query'],$start_pos,$end_pos-$start_pos+1);
			$_POST['query']=str_replace($referer_regEx," ",$_POST['query']);
			$referer_regEx=substr($referer_regEx,strlen($matches[0][0])+1,-1);
			$referer_regEx=str_replace('\/','/',$referer_regEx);
			if(preg_match("/http_referer\ *=\ */",$_POST['query'],$matches,PREG_OFFSET_CAPTURE)){
				echo "Only one http_referer filter is allowed";
				exit(0);
			}
		}

		$filter="";
		$arg="";
		if(isset($host_regEx)&&$host_regEx!=""){
			$arg.=" --host " . escapeshellarg($host_regEx);
			$filter.= " http_host = /" . str_replace('/','\/',$host_regEx) . '/';
		}
		if(isset($uri_regEx)&&$uri_regEx!=""){
			$arg.=" --uri " . escapeshellarg($uri_regEx);
			$filter.= " http_uri = /" . str_replace('/','\/',$uri_regEx) . '/';
		}
		if(isset($referer_regEx)&&$referer_regEx!=""){
			$arg.=" --referer " . escapeshellarg($referer_regEx);
			$filter.= " http_referer = /" . str_replace('/','\/',$referer_regEx). '/';
		}

		$query_tokens=array();
		preg_match_all("/([\w\.]+)\ *=\ *([\w\.\/-]+)/",$_POST['query'],$matches);
		$index=0;
		foreach($matches[1] as $token){
			switch($token){
				//Flow based Filters
				case 'src_ip':
					if(isset($query_tokens['--network'])){
						echo "'ip' and 'src_ip' cannot both be used";
						exit(0);
					}
					else
						$query_tokens['--source-network']=$matches[2][$index];
					break;
				
				case 'dst_ip':
					if(isset($query_tokens['--network'])){
						echo "'ip' and 'dst_ip' cannot both be used";
						exit(0);
					}
					else
						$query_tokens['--destination-network']=$matches[2][$index];
					break;

				case 'ip':
					if(isset($query_tokens['--source-network']) || isset($query_tokens['--destination-network'])){
						echo "'ip' cannot be used with 'src_ip' and/or 'dst_ip'";
						exit(0);
					}
					else
						$query_tokens['--network']=$matches[2][$index];
					break;

				case 'port':
					if(isset($query_tokens['--source-port']) || isset($query_tokens['--destination-port'])){
						echo "'port' cannot be used with 'dst_port' and/or 'src_port'";
						exit(0);
					}
					else
						$query_tokens['--port']=$matches[2][$index];
					break;

				case 'src_port':
					if(isset($query_tokens['--port'])){
						echo "'port' and 'src_port' cannot both be specified";
						exit(0);
					}
					else
						$query_tokens['--source-port']=$matches[2][$index];
					break;

				case 'dst_port':
					if(isset($query_tokens['--port'])){
						echo "'port' and 'dst_port' cannot both be specified";
						exit(0);
					}
					else
						$query_tokens['--destination-port']=$matches[2][$index];
					break;

				case 'proto':
					if($searchType!='query'){
						echo 'proto filter not supported for payload search';
						exit(0);
					}
					$query_tokens['--protocol']=$matches[2][$index];
					break;

				//DNS based filters

				case 'domain':
					if($searchType!='query'){
						echo 'domain filter not supported for payload search';
						exit(0);
					}
					if(isset($query_tokens['--source-domain']) || isset($query_tokens['--destination-domain'])){
						echo "'domain' cannot be used with 'src_domain' and/or 'dst_domain'";
						exit(0);
					}
					else
						$query_tokens['--domain']=$matches[2][$index];
					break;

				case 'src_domain':
					if($searchType!='query'){
						echo 'src_domain filter not supported for payload queries';
						exit(0);
					}
					if(isset($query_tokens['--domain'])){
						echo "'src_domain' and 'domain' cannot both be specified";
						exit(0);
					}
					else
						$query_tokens['--source-domain']=$matches[2][$index];
					break;

				case 'dst_domain':
					if($searchType!='query'){
						echo 'dst_domain filter not supported for payload queries';
						exit(0);
					}
					if(isset($query_tokens['--domain'])){
						echo "'dst_domain' and 'domain' cannot both be specified";
						exit(0);
					}
					else
						$query_tokens['--destination-domain']=$matches[2][$index];
					break;

				//Invalid filters
				default:
					echo "Invalid filter [" . $token . "]";
					exit(0);
			}
			$index++;
		}
		//Create argument string
		foreach($query_tokens as $key=>$value)
			$arg .= ' ' . $key . " " . $value . " ";
		if($arg=="" && $searchType=='query'){
			echo "No Filters specified";
			exit(0);
		}
		//echo $cmd . " "  . $arg . "<br/>" .  $_POST['query'] . " " . $filter ;
		//run search script
		if($searchType=='query'){
		//	echo $cmd . '  ' . $arg;
			$pid=exec("../../../scripts/search.php " . $cmd . " " . $arg . " >> /var/log/infer_search.log 2>&1 &");
			//if($pid==NULL || $pid==""){
			//	echo "Error running search script ";
			//	exit(0);
			//	}
			if(!insertPGRow($pg,"Indexes","searchQueries",
						$queryID,
						($_POST['search_name']==''?'unnamed':stripslashes($_POST['search_name'])),
						$_POST['query'] . $filter ,
						$start_time,
						$end_time,
						NULL,//pid
						'admin', //replace with a function call to get user name
						time(),//start time
						NULL,//pause time
						NULL,//resume time
						NULL,//duration
						NULL,//time left
						NULL,//num results
						NULL,//percent complete
						0,//status
						NULL//details
						)){
				exit(0);
			}
			header('HTTP/1.1 303 See Other');
			$redirect_url = 'http://';
			if ($_SERVER['HTTPS']) {
				$redirect_url = 'https://';
			};
			$redirect_url .= $_SERVER['HTTP_HOST'] . '/search/#/queries/flow/';//.$queryID; FIXME we prob need a summary section...
			header('Location: ' . $redirect_url);
		}else{
			$arg=str_replace('--source-network' , '--source-net',$arg); //Payload search expects --source-net and --dest-net as options
			$arg=str_replace('--destination-network','--dest-net',$arg);
			$input_dir='/mnt/sensor'; // FIXME Change this (get from conf)
			$arg .= ' ' . '-i "' . $input_dir . '" ';
			$queryData=file_get_contents($_FILES['uploaded_file']['tmp_name'],0,NULL,(isset($_POST['file_offset'])?$_POST['file_offset']:0),$_POST['offset_length']);
			$arg.= ' -d "' . base64_encode($queryData) . '" -q ' . $_POST['offset_length'] . ' -m ' . $_POST['match_length']; 
			if (!insertPGrow($pg, 'Indexes', 'payloadQueries',
							 $queryID,
							 ($_POST['search_name']!=''?stripslashes($_POST['search_name']):NULL),
							 base64_encode($queryData),
							 $start_time,
							 $end_time,
							 $_POST['offset_length'],
							 $_POST['match_length'],
							 $_POST['query'] . $filter,
							 NULL, // pid
							 "Admin",
							 time(), // startTime
							 NULL, // pauseTime
							 NULL, // resumeTime
							 NULL, // duration
							 HBF_RUNNING, // status
							 NULL)) // details
			{
				echo @pg_last_error();
				exit(0);
			}
			$out=array();
			$pid=exec('../../../scripts/payload_search.php ' . 
								" --start-time \"" . $start_time . "\" " .
								" --end-time \"" . $end_time . "\""  .
								' -t "' . $queryID . '" ' .
								 $arg . ' >> /var/log/infer_search.log 2>&1 &');
			//	print_r($out);
			header('HTTP/1.1 303 See Other');
			$redirect_url = 'http://';
			if ($_SERVER['HTTPS']) {
				$redirect_url = 'https://';
			};
			$redirect_url .= $_SERVER['HTTP_HOST'] . '/search/#/queries/payload/';//.$queryID; FIXME we prob need a summary section...
			header('Location: ' . $redirect_url);
		}
	}
}
require('page_template.php');

function verifyPayloadSearchData(){
	$error="";
	switch($_FILES['uploaded_file']['error']){
		case UPLOAD_ERR_OK:
			break;
		case UPLOAD_ERR_INI_SIZE:
			return 'The uploaded file exceeds the upload_max_filesize directive in php.ini';
		case UPLOAD_ERR_FORM_SIZE:
			return 'The uploaded file exceeds the MAX_FILE_SIZE directive that was specified in the HTML form';
		case UPLOAD_ERR_PARTIAL:
			return 'The uploaded file was only partially uploaded';
		case UPLOAD_ERR_NO_FILE:
	  		return 'No file was uploaded';
		case UPLOAD_ERR_NO_TMP_DIR:
			return 'Missing a temporary folder';
		case UPLOAD_ERR_CANT_WRITE:
	  		return 'Failed to write file to disk';
		case UPLOAD_ERR_EXTENSION:
			return 'File upload stopped by extension';
		default:
			return 'Unknown upload error';
	}
	$offset=(isset($_POST['file_offset'])?$_POST['file_offset']:0);
	if($_POST['offset_length']=="")
		return 'Offset length must be provided';
	else if($_POST['offset_length']%64!=0)
		return 'Offset length must be multiple of 64';
	else if($_POST['match_length']=="")
		return 'Match length must be provided';
	else if($_POST['match_length']%64!=0)
		return 'Match length must be multiple of 64';
	else if($_POST['match_length']>$_POST['offset_length'])
		return 'Match length must be less than or equal to Offset Length';
	else if($offset> $_FILES['uploaded_file']['size'])
		return 'Offset past the end of File';
	else if($offset+$_POST['offset_length'] > $_FILES['uploaded_file']['size'])
		return 'Not enough input data for given offset/length';
	else
		return '';
}

?>
