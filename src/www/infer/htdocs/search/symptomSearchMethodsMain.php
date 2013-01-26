<?php  
  include('symptomSearchMethods.php');
  include('../include/dbLink.php');
  session_start();  
  
  function showCommChannels($startDate,$endDate)
  {		 
	 // echo "<br>"."Inside showCommChannels"."<br>";//delete
	 // $n_arguments = func_num_args();
	 // for ($i=0; $i<$n_arguments; $i++) {

        // var_dump(func_get_arg($i)); // get each argument passed
		// echo "<br>";
    // }	
	// echo "<br>";//delete
	 showDatabaseResultsGeneral("CommChannels",$startDate,$endDate);	 
  }  
  function showInfectedContacts($startDate,$endDate)
  {
     showDatabaseResultsGeneral("InfectedContacts",$startDate,$endDate);
  }
  function showEvasiveTraffic($startDate,$endDate)
  {
    require_once 'Pager/Pager.php';	
	
	$infectedIPs;
    getInfectedIPs($GLOBALS['postgreSQL'], $startDate, $startDate, $infectedIPs);
    $protocolNames = array(6 => 'TCP', 17 => 'UDP');	
	$schemaName = "EvasiveTraffic";	
	
	$countQuery = getCountQuery($schemaName,$startDate,$endDate);	
	setPagerOptionsSymptomSearch($pager_options,$pager,$offset,$num_records,$countQuery);
	
	$innerQuery = getInnerQuery($schemaName,$startDate,$endDate);
	adjustSortParameter("sortParameter");
	if(isset($_SESSION["sortParameter"]))
	{
	   $displayParameterValue = convertSourceToInternal($_SESSION["sortParameter"]);
	   if($_SESSION["sortParameter"]=="duration")
	     $query = 'SELECT "internalIP","externalIP","internalPort","externalPort","numFrags","numBytes","startTime","endTime","content","protocol","initiator", ("endTime"-"startTime") as duration FROM  ('.$innerQuery.') as all_data ORDER BY duration DESC OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];
	   else
		 $query = 'SELECT "internalIP","externalIP","internalPort","externalPort","numFrags","numBytes","startTime","endTime","content","protocol","initiator" FROM  ('.$innerQuery.') as all_data '.getSortQuery($displayParameterValue,$offset,$pager_options['perPage']);
	}
	else
	   $query = 'SELECT "internalIP","externalIP","internalPort","externalPort","numFrags","numBytes","startTime","endTime","content","protocol","initiator" FROM  ('.$innerQuery.') as all_data OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];
	
	//$query = 'SELECT "internalIP","externalIP","internalPort","externalPort","numBytes","startTime","endTime","content","protocol" FROM (select * from "InfectedContacts"."2008-08-20" union all select * from "InfectedContacts"."2008-08-20") as all_data OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];
	$result = pg_query($GLOBALS['postgreSQL'],$query);

	  echo '<div class="table">' .
            '<table width="100%" cellspacing="1" id="asns">' .			 
              '<tr>' .                
                '<td class="columnTitle center" onclick="sortBy(\'protocol\');">'.
	     		 'Protocol'.
                '</td>' .
                '<td class="columnTitle center unsortable">'.
                   'Internal IP' .
                '</td>'  .
                '<td class="columnTitle center unsortable">'.
                  'External IP' .
                '</td>' .
                '<td class="columnTitle center" onclick="sortBy(\'internalPort\');">'. 
                  'Internal Port' .
                '</td>' .
		        '<td class="columnTitle center" onclick="sortBy(\'externalPort\');">'. 
                   'External Port' .
                '</td>' .
				'<td class="columnTitle center" onclick="sortBy(\'startTime\');">'.
                  'First Occurence' .
                '</td>' .
			    '<td class="columnTitle center" onclick="sortBy(\'duration\');">'.
                   'Duration' .
                '</td>' .
				'<td class="columnTitle center" onclick="sortBy(\'numFrags\');">'.
                   'Number of Frags' .
                '</td>' .
                '<td class="columnTitle center" onclick="sortBy(\'numBytes\');">'.
                   'Number of Bytes' .
				'<td class="columnTitle center unsortable">'.
                   'Content Distribution' .
                 '</td>' .
                '</tr>';	

		while ($row = pg_fetch_assoc($result)) {				
        setRowColor($rowNumber,$rowClass);
        echo '<tr class="' . $rowClass . '">' .
               '<td class="center">' .
                 $protocolNames[$row['protocol']] .
               '</td>' .
               '<td class="center">'.			   
                 long2ip($row['internalIP']) .
               '</td>' .
			   '<td class="center">';
			echo styleIP($infectedIPs,$row['externalIP'],false);//show IP in red color if it is on infected IPs list
            echo '</td>' .
			   '<td class="center">'.			   
                 styleInternalPort($row['internalPort'], $row['initiator']) .
               '</td>' .
			    '<td class="center">'.			   
                 styleExternalPort($row['externalPort'], $row['initiator']) .
               '</td>' .
			   '<td class="center">'.			   
                 date('M d, D g:i:s A', $row['startTime']) .
               '</td>' .
			   '<td class="center">'.			   
                 duration($row['endTime']-$row['startTime']) .
               '</td>' .
			   '<td class="center">'.			   
                 $row['numFrags'] .
               '</td>' .
			   '<td class="center">'.			   
                 size($row['numBytes']) .
               '</td>' .
			   '<td class="center">'.			   
                 drawContentStripe($row['content'],$row['numBytes']) .
               '</td>' .
			  '</tr>';
		  ++$rowNumber;
      }
      echo '</table>' .
         '</div>';		

     displayLinks($num_records,$pager);//display pagination links
  }  
  function showDarkSpaceSources($startDate,$endDate)
  {
    require_once 'Pager/Pager.php';		
	
	$infectedIPs;
    getInfectedIPs($GLOBALS['postgreSQL'], $startDate, $startDate, $infectedIPs);	
    $protocolNames = array(6 => 'TCP', 17 => 'UDP');	
	$schemaName = "DarkSpaceSources";	
	
	$countQuery = getCountQuery($schemaName,$startDate,$endDate);	
	setPagerOptionsSymptomSearch($pager_options,$pager,$offset,$num_records,$countQuery);
	
	$innerQuery = getInnerQuery($schemaName,$startDate,$endDate);
	adjustSortParameter("sortParameter");
	if(isset($_SESSION["sortParameter"])&&$_SESSION["sortParameter"]!="numFrags")
	{
	   $displayParameterValue = convertInternalToSource($_SESSION["sortParameter"]);
	   if($_SESSION["sortParameter"]=="duration")
	     $query = 'SELECT "sourceIP","destinationIP","sourcePort","destinationPort","numBytes","startTime","endTime","content","protocol", ("endTime"-"startTime") as duration FROM  ('.$innerQuery.') as all_data ORDER BY duration DESC OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];
	   else
		 $query = 'SELECT "sourceIP","destinationIP","sourcePort","destinationPort","numBytes","startTime","endTime","content","protocol" FROM  ('.$innerQuery.') as all_data '.getSortQuery($_SESSION["sortParameter"],$offset,$pager_options['perPage']);
	}
	else
	   $query = 'SELECT "sourceIP","destinationIP","sourcePort","destinationPort","numBytes","startTime","endTime","content","protocol" FROM  ('.$innerQuery.') as all_data OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];
	
	//$query = 'SELECT "internalIP","externalIP","internalPort","externalPort","numBytes","startTime","endTime","content","protocol" FROM (select * from "InfectedContacts"."2008-08-20" union all select * from "InfectedContacts"."2008-08-20") as all_data OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];
	$result = pg_query($GLOBALS['postgreSQL'],$query);

	  echo '<div class="table">' .
            '<table width="100%" cellspacing="1" id="asns">' .			 
              '<tr>' .                
                '<td class="columnTitle center" onclick="sortBy(\'protocol\');">'.
	     		 'Protocol'.
                '</td>' .
                '<td class="columnTitle center unsortable">'.
                   'Source IP' .
                '</td>'  .
                '<td class="columnTitle center unsortable">'.
                  'Destination IP' .
                '</td>' .
                '<td class="columnTitle center" onclick="sortBy(\'sourcePort\');">'. 
                  'Source Port' .
                '</td>' .
		        '<td class="columnTitle center" onclick="sortBy(\'destinationPort\');">'. 
                   'Destination Port' .
                '</td>' .
				'<td class="columnTitle center" onclick="sortBy(\'startTime\');">'.
                  'First Occurence' .
                '</td>' .
			    '<td class="columnTitle center" onclick="sortBy(\'duration\');">'.
                   'Duration' .
                '</td>' .
                '<td class="columnTitle center" onclick="sortBy(\'numBytes\');">'.
                   'Number of Bytes' .
				'<td class="columnTitle center unsortable">'.
                   'Content Distribution' .
                 '</td>' .
                 '</tr>';	

		while ($row = pg_fetch_assoc($result)) {				
        setRowColor($rowNumber,$rowClass);
        echo '<tr class="' . $rowClass . '">' .
               '<td class="center">' .
                 $protocolNames[$row['protocol']] .
               '</td>' .
			   '<td class="center">';
				echo styleIP($infectedIPs,$row['sourceIP'],false);//show IP in red color if it is on infected IPs list
	            echo '</td>' .
			   '<td class="center">'.			   
                 long2ip($row['destinationIP']) .
               '</td>' .
			   '<td class="center">'.			   
                 $row['sourcePort'] .
               '</td>' .
			    '<td class="center">'.			   
                 $row['destinationPort'] .
               '</td>' .
			   '<td class="center">'.			   
                 date('M d, D g:i:s A', $row['startTime']) .
               '</td>' .
			   '<td class="center">'.			   
                 duration($row['endTime']-$row['startTime']) .
               '</td>' .
			   '<td class="center">'.			   
                 size($row['numBytes']) .
               '</td>' .
			   '<td class="center">'.			   
                 drawContentStripe($row['content'],$row['numBytes']) .
               '</td>' .
			  '</tr>';
		  ++$rowNumber;
      }
      echo '</table>' .
         '</div>';		

     displayLinks($num_records,$pager);//display pagination links 
  }
  
  function showDarkSpaceTargets($startDate,$endDate)
  {
     showDatabaseResultsGeneral("DarkSpaceTargets",$startDate,$endDate);
  }  
  function showMultimediaP2Ptraffic($startDate,$endDate)
  {
   	require_once 'Pager/Pager.php';	
	
	$infectedIPs;
    getInfectedIPs($GLOBALS['postgreSQL'], $startDate, $startDate, $infectedIPs);	
	define('MULTIMEDIA_TYPE', 1);
	
    $protocolNames = array(6 => 'TCP', 17 => 'UDP');	
	$schemaName = "NonDNSTraffic";	
	
	$countQuery = getCountQueryP2P($schemaName,$startDate,$endDate,MULTIMEDIA_TYPE);
	setPagerOptionsSymptomSearch($pager_options,$pager,$offset,$num_records,$countQuery);
	
	$innerQuery = getInnerQuery($schemaName,$startDate,$endDate);
	adjustSortParameter("sortParameter");
	if(isset($_SESSION["sortParameter"])&&$_SESSION["sortParameter"]!="numFrags")
	{
	   $displayParameterValue = convertSourceToInternal($_SESSION["sortParameter"]);
	   if($_SESSION["sortParameter"]=="duration")
	     $query = 'SELECT "internalIP","externalIP","internalPort","externalPort","numBytes","startTime","endTime","content","protocol", ("endTime"-"startTime") as duration FROM  ('.$innerQuery.') as all_data WHERE "type" = ' . MULTIMEDIA_TYPE . ' ORDER BY duration DESC OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];	   
	   else
		 $query = 'SELECT "internalIP","externalIP","internalPort","externalPort","numBytes","startTime","endTime","content","protocol" FROM  ('.$innerQuery.') as all_data WHERE "type" = ' . MULTIMEDIA_TYPE . '  '.getSortQuery($displayParameterValue,$offset,$pager_options['perPage']);
	}
	else
	   $query = 'SELECT "internalIP","externalIP","internalPort","externalPort","numBytes","startTime","endTime","content","protocol" FROM  ('.$innerQuery.') as all_data WHERE "type" = ' . MULTIMEDIA_TYPE . '  OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];

	//$query = 'SELECT "internalIP","externalIP","internalPort","externalPort","numBytes","startTime","endTime","content","protocol" FROM (select * from "InfectedContacts"."2008-08-20" union all select * from "InfectedContacts"."2008-08-20") as all_data OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];
	$result = pg_query($GLOBALS['postgreSQL'],$query);

	showTableHeaderGeneral();

		while ($row = pg_fetch_assoc($result)) {				
        setRowColor($rowNumber,$rowClass);		
        echo '<tr class="' . $rowClass . '">' .
               '<td class="center">' .
                 $protocolNames[$row['protocol']] .
               '</td>' .
               '<td class="center">'.			   
                 long2ip($row['internalIP']) .
               '</td>' .
			   '<td class="center">';
				echo styleIP($infectedIPs,$row['externalIP'],false);//show IP in red color if it is on infected IPs list
	            echo '</td>' .
			   '<td class="center">'.			   
                 $row['internalPort'] .
               '</td>' .
			    '<td class="center">'.			   
                 $row['externalPort'] .
               '</td>' .
			   '<td class="center">'.			   
                  date('M d, D g:i:s A', $row['startTime']) .
               '</td>' .
			   '<td class="center">'.			   
                 duration($row['endTime']-$row['startTime']) .
               '</td>' .
			   '<td class="center">'.			   
                 size($row['numBytes']) .
               '</td>' .
			   '<td class="center">'.			   
                 drawContentStripe($row['content'],$row['numBytes']) .
               '</td>' .
			  '</tr>';
		  ++$rowNumber;
      }
      echo '</table>' .
         '</div>';		

     displayLinks($num_records,$pager);//display pagination links
     
  }  
  function showNonDNSTraffic($startDate,$endDate)
  {  
    require_once 'Pager/Pager.php';	
	//echo "Inside showNonDNSTraffic";//br
	$infectedIPs;
    getInfectedIPs($GLOBALS['postgreSQL'], $startDate, $startDate, $infectedIPs);		
	
    $protocolNames = array(6 => 'TCP', 17 => 'UDP');	
	$schemaName = "NonDNSTraffic";	
	
	$countQuery = getCountQuery($schemaName,$startDate,$endDate);
	setPagerOptionsSymptomSearch($pager_options,$pager,$offset,$num_records,$countQuery);
	
	$innerQuery = getInnerQuery($schemaName,$startDate,$endDate);
	adjustSortParameter("sortParameter");
	if(isset($_SESSION["sortParameter"])&&$_SESSION["sortParameter"]!="numFrags")
	{
	   $displayParameterValue = convertSourceToInternal($_SESSION["sortParameter"]);
	   if($_SESSION["sortParameter"]=="duration")
	     $query = 'SELECT "internalIP","externalIP","internalPort","externalPort","numBytes","startTime","endTime","content","protocol", ("endTime"-"startTime") as duration FROM  ('.$innerQuery.') as all_data ORDER BY duration DESC OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];	   
	   else
		 $query = 'SELECT "internalIP","externalIP","internalPort","externalPort","numBytes","startTime","endTime","content","protocol" FROM  ('.$innerQuery.') as all_data '.getSortQuery($displayParameterValue,$offset,$pager_options['perPage']);
	}
	else
	   $query = 'SELECT "internalIP","externalIP","internalPort","externalPort","numBytes","startTime","endTime","content","protocol" FROM  ('.$innerQuery.') as all_data OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];
	
	//$query = 'SELECT "internalIP","externalIP","internalPort","externalPort","numBytes","startTime","endTime","content","protocol" FROM (select * from "InfectedContacts"."2008-08-20" union all select * from "InfectedContacts"."2008-08-20") as all_data OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];
	$result = pg_query($GLOBALS['postgreSQL'],$query);

	showTableHeaderGeneral();

	while ($row = pg_fetch_assoc($result)) {				
        setRowColor($rowNumber,$rowClass);
        echo '<tr class="' . $rowClass . '">' .
               '<td class="center">' .
                 $protocolNames[$row['protocol']] .
               '</td>' .
               '<td class="center">'.			   
                 long2ip($row['internalIP']) .
               '</td>' .
			   '<td class="center">';
				echo styleIP($infectedIPs,$row['externalIP'],false);//show IP in red color if it is on infected IPs list
	            echo '</td>' .
			   '<td class="center">'.			   
                 $row['internalPort'] .
               '</td>' .
			    '<td class="center">'.			   
                 $row['externalPort'] .
               '</td>' .
			   '<td class="center">'.			   
                  date('M d, D g:i:s A', $row['startTime']) .
               '</td>' .
			   '<td class="center">'.			   
                 duration($row['endTime']-$row['startTime']) .
               '</td>' .
			   '<td class="center">'.			   
                 size($row['numBytes']) .
               '</td>' .
			   '<td class="center">'.			   
                 drawContentStripe($row['content'],$row['numBytes']) .
               '</td>' .
			  '</tr>';
		  ++$rowNumber;
      }
      echo '</table>' .
         '</div>';		

     displayLinks($num_records,$pager);//display pagination links
  }
 function showBruteForcers($startDate,$endDate)
 {  
    require_once 'Pager/Pager.php';    
		
	$infectedIPs;
    getInfectedIPs($GLOBALS['postgreSQL'], $startDate, $startDate, $infectedIPs);
    $protocolNames = array(6 => 'TCP', 17 => 'UDP');	
	$schemaName = "BruteForcers";	
	
	$countQuery = getCountQuery($schemaName,$startDate,$endDate);
	setPagerOptionsSymptomSearch($pager_options,$pager,$offset,$num_records,$countQuery);
	
	$innerQuery = getInnerQuery($schemaName,$startDate,$endDate);
	adjustSortParameter("sortParameter");
	if(isset($_SESSION["sortParameter"])&&$_SESSION["sortParameter"]!="numFrags")
	{
	   $displayParameterValue = convertInternalToSource($_SESSION["sortParameter"]);
	   if($_SESSION["sortParameter"]=="duration")
	     $query = 'SELECT "sourceIP", "destinationIP", "destinationPort", "numAttempts", "startTime", "endTime", ("endTime"-"startTime") as duration FROM  ('.$innerQuery.') as all_data ORDER BY duration DESC OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];
	   else if($_SESSION["sortParameter"]=="numAttempts")//specific to Bruteforcers/Bruteforced
		  $query = 'SELECT "sourceIP", "destinationIP", "destinationPort", "numAttempts", "startTime", "endTime" FROM  ('.$innerQuery.') as all_data ORDER BY "numAttempts" DESC OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];
	   else
		 $query = 'SELECT "sourceIP", "destinationIP", "destinationPort", "numAttempts", "startTime", "endTime"  FROM  ('.$innerQuery.') as all_data '.getSortQuery($displayParameterValue,$offset,$pager_options['perPage']);
	}
	else
	   $query = 'SELECT "sourceIP", "destinationIP", "destinationPort", "numAttempts", "startTime", "endTime"  FROM  ('.$innerQuery.') as all_data OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];

	//$query = 'SELECT "internalIP","externalIP","internalPort","externalPort","numBytes","startTime","endTime","content","protocol" FROM (select * from "InfectedContacts"."2008-08-20" union all select * from "InfectedContacts"."2008-08-20") as all_data OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];
	$result = pg_query($GLOBALS['postgreSQL'],$query);
	
	  echo '<div class="table">' .
            '<table width="100%" cellspacing="1" id="asns">' .			 
              '<tr>' .                
                '<td class="columnTitle center unsortable">'.
                  'Source IP' .
                '</td>'  .   
                '<td class="columnTitle center unsortable">'. 
                  'Destination IP' .
                '</td>' .
				'<td class="columnTitle center" onclick="sortBy(\'destinationPort\');">'. 
                  'Destination Port' .
                '</td>' .
		        '<td class="columnTitle center" onclick="sortBy(\'startTime\');">'. 
                   'Start Time' .
                '</td>' .
                '<td class="columnTitle center" onclick="sortBy(\'duration\');">'.
                   'Duration' .
			    '<td class="columnTitle center" onclick="sortBy(\'numAttempts\');">'.
                   'Number of Attempts' .
                '</td>' .
                 '</tr>';

		while ($row = pg_fetch_assoc($result)) {				
        setRowColor($rowNumber,$rowClass);
        echo '<tr class="' . $rowClass . '">' .
               '<td class="center">'.			   
                 long2ip($row['sourceIP']) .
               '</td>' .
			   '<td class="center">';
				echo styleIP($infectedIPs,$row['destinationIP'],false);//show IP in red color if it is on infected IPs list
	            echo '</td>' .
			   '<td class="center">'.			   
                 $row['destinationPort'] .
               '</td>' .
			   '<td class="center">'.			   
                 date('M d, D g:i:s A', $row['startTime']) .
               '</td>' .
			   '<td class="center">'.			   
                 duration($row['endTime']-$row['startTime']) .
               '</td>' .
			   '<td class="center">'.			   
                $row['numAttempts'] .
               '</td>' .
			  '</tr>';
		  ++$rowNumber;
      }
      echo '</table>' .
         '</div>';		

     displayLinks($num_records,$pager);//display pagination links
  }
  function showReboots($startDate)
  {
    header('Location: /reboots/'.$startDate);
    exit; 
	/*
     require_once 'Pager/Pager.php';	
	 $infectedIPs;
     getInfectedIPs($GLOBALS['postgreSQL'], $startDate, $startDate, $infectedIPs);
     $protocolNames = array(6 => 'TCP', 17 => 'UDP');	
	 $schemaName = "Reboots";	
	 
	 $countQuery = getCountQuery($schemaName,$startDate,$endDate);
	 setPagerOptionsSymptomSearch($pager_options,$pager,$offset,$num_records,$countQuery);
	
	 $innerQuery = getCountReboots($GLOBALS['postgreSQL'],$schemaName,$startDate,$endDate);

	 if(isset($_SESSION["sortParameter"])&&$_SESSION["sortParameter"]!="numFrags"&&$_SESSION["sortParameter"]!="duration")
	 {
	    $displayParameterValue = convertInternalToSource($_SESSION["sortParameter"]);	    
	    $query = 'SELECT DISTINCT ON ("applicationTimes","ip","applicationNames") "ip", "applicationTimes",  COUNT(*) FROM ('.$innerQuery.') as all_data GROUP BY "ip" '.getSortQuery($displayParameterValue,$offset,$pager_options['perPage']);
	 }
	 else
	 {
	    $query = 'SELECT date_trunc(\'day\',TIMESTAMP  \'epoch\' + date * INTERVAL \'1 second\') as date2, ip,COUNT(count) FROM ('.$innerQuery.') as all_data GROUP BY ip,date2 OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];			
	 }
		
	//add and put alias: where total = "Select COUNT(*)  from tableName where "ip"=localIP
	$result = pg_query($GLOBALS['postgreSQL'],$query);
	
    echo '<div class="table">' .
             '<table width="100%" cellspacing="1">' .
               '<tr>' .
			     '<td class="columnTitle center unsortable">'.
                   'Date' .
                 '</td>' .
                 '<td class="columnTitle center unsortable">'.
                   'IP' .
                 '</td>' .
                 '<td class="columnTitle center unsortable">'.
                   'Number of Reboots' .
                 '</td>' .
               '</tr>';
																						
      while ($row = pg_fetch_assoc($result)) {	
        setRowColor($rowNumber,$rowClass);
        echo '<tr class="' . $rowClass . '">' .
		       '<td class="center">' .
             date('D, M d, Y', $row['date2']) .
               '</td>' .
               '<td class="center">' .
                 '<a class="text" href="../reboots/' . date('Y-m-d', $applicationTimes) . '/' . long2ip($row['ip']) . '">' .//change/delete
                   long2ip($row['ip']) .
                 '</a>' .
               '</td>' .
               '<td class="center">' .
                 $row['count'] .
               '</td>' .
             '</tr>';
        ++$rowNumber;
      }
	  
	  echo '</table>' .
         '</div>';		
		 
	if($num_records>50)
	{
	   $links = $pager->getLinks();		
	   echo '<div class="center">'.$links['first'].$links['back'].' '.$links['pages'].$links['next'].' '.$links['last'].'</div>';  
	}         
	  
     */
	 
  }
  
 // function showBruteForced($startDate,$endDate)
 // {
     
 // }
  
  //results for symptoms with most common format internal/external IP symantics, 
         //both internal and external IPs included
  function showDatabaseResultsGeneral($schemaName,$startDate,$endDate)
  {
	require_once 'Pager/Pager.php';		
	  
	$infectedIPs;
    getInfectedIPs($GLOBALS['postgreSQL'], $startDate, $endDate, $infectedIPs);	
	
    $protocolNames = array(6 => 'TCP', 17 => 'UDP');		
		
	$countQuery = getCountQuery($schemaName,$startDate,$endDate);
    setPagerOptionsSymptomSearch($pager_options,$pager,$offset,$num_records,$countQuery);    
	
	$innerQuery = getInnerQuery($schemaName,$startDate,$endDate);
	adjustSortParameter("sortParameter");
	if(isset($_SESSION["sortParameter"])&&$_SESSION["sortParameter"]!="numFrags")
	{
	   if($schemaName=="CommChannels"&&$_SESSION["sortParameter"]=="protocol")//protocol not stored for Communication Channels
	   {
	      $query = 'SELECT "internalIP","externalIP","internalPort","externalPort","numBytes","startTime","endTime","content","initiator" FROM  ('.$innerQuery.') as all_data OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];
	   }
	   else 
	   {
		   $displayParameterValue = convertSourceToInternal($_SESSION["sortParameter"]);
		   if($_SESSION["sortParameter"]=="duration")
			 $query = 'SELECT "internalIP","externalIP","internalPort","externalPort","numBytes","startTime","endTime","content","initiator", ("endTime"-"startTime") as duration FROM  ('.$innerQuery.') as all_data ORDER BY duration DESC OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];
		   else
			 $query = 'SELECT "internalIP","externalIP","internalPort","externalPort","numBytes","startTime","endTime","content","initiator" FROM  ('.$innerQuery.') as all_data '.getSortQuery($displayParameterValue,$offset,$pager_options['perPage']);
	   }
	}
	else
	   $query = 'SELECT "internalIP","externalIP","internalPort","externalPort","numBytes","startTime","endTime","content","initiator" FROM  ('.$innerQuery.') as all_data OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];

	//$query = 'SELECT "internalIP","externalIP","internalPort","externalPort","numBytes","startTime","endTime","content","protocol" FROM (select * from "InfectedContacts"."2008-08-20" union all select * from "InfectedContacts"."2008-08-20") as all_data OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];
	$result = pg_query($GLOBALS['postgreSQL'],$query);

	showTableHeaderGeneral();//shows table header, which is common to several symptoms
				 
	while ($row = pg_fetch_assoc($result)) {
        setRowColor($rowNumber,$rowClass);
        echo '<tr class="' . $rowClass . '">' .
               '<td class="center">' .
                 'TCP' .
               '</td>' .
               '<td class="center">'.			   
                 long2ip($row['internalIP']) .
               '</td>'.	
			   '<td class="center">';
             echo styleIP($infectedIPs,$row['externalIP'],false);//show IP in red color if it is on infected IPs list
            echo '</td>' .
			   '<td class="center">'.			   
                 styleInternalPort($row['internalPort'], $row['initiator']) .
               '</td>' .
			    '<td class="center">'.			   
                 styleExternalPort($row['externalPort'], $row['initiator']) .
               '</td>' .
			   '<td class="center">'.			   
                  date('M d, D g:i:s A', $row['startTime']) .
               '</td>' .
			   '<td class="center">'.			   
                 duration($row['endTime']-$row['startTime']) .
               '</td>' .
			   '<td class="center">'.			   
                 size($row['numBytes']) .
               '</td>' .
			   '<td class="center">'.			   
                 drawContentStripe($row['content'],$row['numBytes']) .
               '</td>' .
			  '</tr>';
		  ++$rowNumber;
      }
      echo '</table>' .
         '</div>';		
	 //echo "About to call displayLinks";//dl
     displayLinks($num_records,$pager);//display pagination links 
  }
 function showTableHeaderGeneral()
 {
    echo '<div class="table">' .
            '<table width="100%" cellspacing="1" id="asns">' .			 
              '<tr>' .                
                '<td class="columnTitle center" onclick="sortBy(\'protocol\');">'.
	     		 'Protocol'.
                '</td>' .
                '<td class="columnTitle center unsortable">'.
                   'Internal IP' .
                '</td>'  .
                '<td class="columnTitle center unsortable">'.
                  'External IP' .
                '</td>' .
                '<td class="columnTitle center" onclick="sortBy(\'internalPort\');">'. 
                  'Internal Port' .
                '</td>' .
		        '<td class="columnTitle center" onclick="sortBy(\'externalPort\');">'. 
                   'External Port' .
                '</td>' .
				'<td class="columnTitle center" onclick="sortBy(\'startTime\');">'.
                  'First Occurence' .
                '</td>' .
			    '<td class="columnTitle center" onclick="sortBy(\'duration\');">'.
                   'Duration' .
                '</td>' .
                '<td class="columnTitle center" onclick="sortBy(\'numBytes\');">'.
                   'Number of Bytes' .
				'<td class="columnTitle center unsortable">'.
                   'Content Distribution' .
              '</td>' .
            '</tr>';
 } 
 function setPagerOptionsSymptomSearch(&$pager_options,&$pager,&$offset,&$num_records,$countQuery)
 {
	      // echo "<br>"."Inside setPagerOptionsSymptomSearch"."<br>";//delete
	 // $n_arguments = func_num_args();
	 // for ($i=0; $i<$n_arguments; $i++) {

        // var_dump(func_get_arg($i)); // get each argument passed
		// echo "<br>";
    // }	
	
	$result = pg_query($GLOBALS['postgreSQL'],$countQuery);	
	
	$row=pg_fetch_row($result);
	$num_records=$row[0];
    checkNumRecords($num_records);
	
	$pager_options = getPagerOptions($num_records,'https://'.$_SERVER['SERVER_ADDR'].'/search/symptom.php');
	$pager = createPager($pager_options);
	$offset = getOffset($pager);
	
	// var_dump($pager_options);
	// echo "<br>";
		// echo "<br>";
			// echo "<br>";
	// var_dump($pager);
	// echo "<br>";
		// echo "<br>";
			// echo "<br>";
	// var_dump($offset);
 }
 
?>
