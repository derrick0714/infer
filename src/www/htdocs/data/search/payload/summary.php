<?php
require_once('../../pg.include.php');
require_once('../../../site/search/common.php');
require_once('../../essentials.class.php');
$path_info=explode('/',$_SERVER['PATH_INFO']);
$result_set=@pg_query($pg,'SELECT * FROM "Indexes"."payloadQueries" WHERE "id"=\'' . $path_info[1] . '\'');
$row=@pg_fetch_assoc($result_set);
$row['type']='payload';
if($row['duration']!=null)
	$row['duration']=essentials::duration_display($row['duration']);
foreach($row as $k=>$v)
	if($v==null)
		$row[$k]='';
echo json_encode($row);

