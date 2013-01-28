<?php 
 include('../include/dbLink.php');
 include('../include/services.php');

 session_start();
 //displays search results for particular IP for one or all symptoms (with or without symptom selected)
 function displayIPSearchResults($numericIP,$startDate,$endDate,$schemaName=0,$linkPath=0)
 {
	require_once 'Pager/Pager.php';
    require_once 'shortcutLinksMethods.php';	
	
	$title = setTitle($schemaName,$numericIP,$startDate,$endDate);	
	//echo "title is: $title";//dl
	$javascriptInclude = '<script charset="utf-8" type="text/javascript" src="/javascript/sort/search.js"></script>';	
	include('../include/header.php');
	
	if($schemaName)//search both sourceIP field and destinationIP field
	    $countQuery = getCountQueryWithIP($numericIP,$schemaName,$startDate,$endDate);
	else//show results for all schemas
	{
	    if(isInternal($numericIP))
           $countQuery = 'select COUNT(*) as total_records from ('. getInternalIPQuery($numericIP,$startDate,$endDate). ') as t_records';	
        else
            $countQuery = 'select COUNT(*) as total_records from ('. getExternalIPQuery($numericIP,$startDate,$endDate). ') as t_records';             		
	}    
	//echo "1 countQuery is: $countQuery <br><br>";//dl
	
    setPagerOptionsIPsearch($pager_options,$pager,$offset,$num_records,$currentPage,$countQuery,$schemaName,$linkPath);
	
   	if(isInternal($numericIP))
	  $query = getInternalIPQuery($numericIP,$startDate,$endDate,$schemaName); 	
	else
	  $query = getExternalIPQuery($numericIP,$startDate,$endDate,$schemaName); 	

	$query = addSortParameter($query,$offset,$pager_options);//adds sort parameter and pagination options to query
    //echo "2 query is: $query";//dl
	//var_dump($schemaName);//dl
	//var_dump($currentPage);//dl
	if(!$schemaName&&$currentPage==1)//no need to display links if only one symptom
	{
		$links = getLinks($numericIP,$startDate,$endDate);//create array of shortcut links
		displayShortcutLinks($links);
    }
	
  	showTableHeaderIPsearch();
	showDatabaseResultsIPsearch($query,$numericIP,$startDate,$endDate);	
    displayLinks($num_records,$pager);//display pagination links
  }
 
  function getExternalIPQuery($numericExternalIP,$startDate,$endDate,$schemaName=0)//for external IPs, all schemas or particular schema
  {
    define('MULTIMEDIA_TYPE', 1);
	
    //echo "Inside getExternalIPQuery";//dl
	if(!$schemaName)//select all schemas
	{
		$query = 'select "content",("endTime"-"startTime") as duration,\'0\' as "protocol","internalIP","internalPort","externalPort","initiator","numBytes","startTime", \'Communication Channels\' as "schemaName" from ('.getInnerQuery("CommChannels", $startDate, $endDate).') as all_data WHERE "externalIP" = \'' . $numericExternalIP 
		.'\' union all '
		.'select "content",("endTime"-"startTime") as duration,"protocol","internalIP","internalPort","externalPort","initiator","numBytes","startTime", \'Infected Contacts\' as "schemaName" from ('.getInnerQuery("InfectedContacts", $startDate, $endDate).') as all_data  WHERE "externalIP" = \'' . $numericExternalIP 
		.'\' union all '
		.'select "content",("endTime"-"startTime") as duration,"protocol","internalIP","internalPort","externalPort",\'0\' as initiator,"numBytes","startTime", \'Evasive Traffic\' as "schemaName" from ('.getInnerQuery("EvasiveTraffic", $startDate, $endDate).') as all_data WHERE "externalIP" =  \'' . $numericExternalIP
		.'\' union all '    
		.'select \'{"0"}\'::uint32[] as "content","startTime" as duration,\'0\' as "protocol","destinationIP" as "internalIP",\'0\' as "internalPort","destinationPort" as "externalPort",\'0\' as "initiator",\'0\' as "numBytes","startTime", \'Brute Forcers\' as "schemaName" from ('.getInnerQuery("BruteForcers", $startDate, $endDate).')  as all_data WHERE "sourceIP" =  \''  . $numericExternalIP  
		.'\' union all '
		.'select \'{"0"}\'::uint32[] as "content","startTime" as duration,\'0\' as "protocol","sourceIP" as "internalIP",\'0\' as "internalPort","destinationPort" as "externalPort",\'0\' as "initiator",\'0\' as "numBytes","startTime", \'Brute Forced\' as "schemaName" from ('.getInnerQuery("BruteForcers", $startDate, $endDate).')  as all_data WHERE "destinationIP" =  \''  . $numericExternalIP
		.'\' union all '
		.'select "content",("endTime"-"startTime") as duration,"protocol", "internalIP","internalPort","externalPort","initiator","numBytes","startTime", \'Dark Space Targets\' as "schemaName" from ('.getInnerQuery("DarkSpaceTargets", $startDate, $endDate).')  as all_data WHERE "externalIP" =  \''  . $numericExternalIP
		.'\' union all '
		.'select "content",("endTime"-"startTime") as duration,"protocol","destinationIP" as "internalIP","sourcePort" as "internalPort","destinationPort" as "externalPort",\'0\' as "initiator","numBytes", "startTime", \'Dark Space Sources\' as "schemaName" from ('.getInnerQuery("DarkSpaceSources", $startDate, $endDate).')  as all_data WHERE "sourceIP" =  \''  . $numericExternalIP
		.'\' union all '
		.'select "content",("endTime"-"startTime") as duration,"protocol","destinationIP" as "internalIP","sourcePort" as "internalPort","destinationPort" as "externalPort",\'0\' as "initiator","numBytes", "startTime", \'Dark Space Sources\' as "schemaName" from ('.getInnerQuery("DarkSpaceSources", $startDate, $endDate).')  as all_data WHERE "destinationIP" =  \''  . $numericExternalIP
		.'\' union all '
		// .'select "content",("endTime"-"startTime") as duration,"protocol","internalIP","internalPort","externalPort",\'0\' as "initiator","numBytes","startTime", \'Protocol Violations\' as "schemaName" from ('.getInnerQuery("NonDNSTraffic", $startDate, $endDate).')  as all_data WHERE "externalIP" =  \''  . $numericExternalIP
		// .'\' union all '
		// .'select "content",("endTime"-"startTime") as duration,"protocol","internalIP","internalPort","externalPort",\'0\' as "initiator","numBytes","startTime", \'Multimedia P2P Traffic\' as "schemaName" from ('.getInnerQuery("NonDNSTraffic", $startDate, $endDate).')  as all_data WHERE "externalIP" =  \''  . $numericExternalIP. '\' AND "type" = \'' . MULTIMEDIA_TYPE 
		// .'\' union all '
		.'select \'{"0"}\'::uint32[] as "content",\'0\' as duration,\'0\' as "protocol","ip" as "internalIP",\'0\' as "internalPort",\'0\' as "externalPort",\'0\' as "initiator",\'0\' as "numBytes",\'0\' as "startTime", \'Multiple Reboots\' as "schemaName" from ('.getInnerQuery("Reboots", $startDate, $endDate).')  as all_data WHERE "ip" = \''  . $numericExternalIP.'\'';
		//echo "About to return: $query";//dl
		return $query;
	}
	
	switch ($schemaName)
	{		
	   case "CommChannels":
	   	 $query = 'select "content",("endTime"-"startTime") as duration,\'0\' as "protocol", "internalIP","internalPort","externalPort","initiator","numBytes","startTime", \'Communication Channels\' as "schemaName" from ('.getInnerQuery("CommChannels", $startDate, $endDate).') as all_data WHERE "externalIP" = \'' . $numericExternalIP.'\'';			 	    
	    break;
	   case "InfectedContacts":
	   	 $query = 'select "content",("endTime"-"startTime") as duration,"protocol", "internalIP","internalPort","externalPort","initiator","numBytes","startTime",\'Infected Contacts\' as "schemaName" from ('.getInnerQuery("InfectedContacts", $startDate, $endDate).') as all_data  WHERE "externalIP" = \'' . $numericExternalIP.'\'';		
	    break;
	   case "EvasiveTraffic":
	   	 $query = 'select "content",("endTime"-"startTime") as duration,"protocol", "internalIP","internalPort","externalPort",\'0\' as initiator,"numBytes","startTime", \'Evasive Traffic\' as "schemaName" from ('.getInnerQuery("EvasiveTraffic", $startDate, $endDate).') as all_data WHERE "externalIP" =  \'' . $numericExternalIP.'\'';
	    break;
	   case "DarkSpaceTargets":
	   	 $query = 'select "content",("endTime"-"startTime") as duration,"protocol", "internalIP","internalPort","externalPort","initiator","numBytes","startTime", \'Dark Space Targets\' as "schemaName" from ('.getInnerQuery("DarkSpaceTargets", $startDate, $endDate).')  as all_data WHERE "externalIP" =  \''  . $numericExternalIP.'\'';
	    break;	  
	   case "Reboots":
	     $query = 'select \'{"0"}\'::uint32[] as "content",\'0\' as duration,\'0\' as "protocol","ip" as  "internalIP",\'0\' as "internalPort",\'0\' as "externalPort",\'0\' as "initiator",\'0\' as "numBytes",\'0\' as "startTime", \'Multiple Reboots\' as "schemaName" from ('.getInnerQuery("Reboots", $startDate, $endDate).')  as all_data WHERE "ip" = \''  . $numericExternalIP.'\'';
	    break;
	   case "BruteForced":
	   	 $query = 'select \'{"0"}\'::uint32[] as "content","startTime" as duration,\'0\' as "protocol","sourceIP" as  "internalIP",\'0\' as "internalPort","destinationPort" as "externalPort",\'0\' as "initiator",\'0\' as "numBytes","startTime", \'Brute Forced\' as "schemaName" from ('.getInnerQuery("BruteForcers", $startDate, $endDate).')  as all_data WHERE "destinationIP" =  \''  . $numericExternalIP.'\'';
	     break;
	   case "BruteForcers":
	   	 $query = 'select \'{"0"}\'::uint32[] as "content","startTime" as duration,\'0\' as "protocol","destinationIP" as  "internalIP",\'0\' as "internalPort","destinationPort" as "externalPort",\'0\' as "initiator",\'0\' as "numBytes","startTime", \'Brute Forcers\' as "schemaName" from ('.getInnerQuery("BruteForcers", $startDate, $endDate).')  as all_data WHERE "sourceIP" =  \''  . $numericExternalIP.'\'';
	     break;
	   case "DarkSpaceSources":
	   	 $query = 'select "content",("endTime"-"startTime") as duration,"protocol","destinationIP" as  "internalIP","sourcePort" as "internalPort","destinationPort" as "externalPort",\'0\' as "initiator","numBytes","startTime", \'Dark Space Sources\' as "schemaName" from ('.getInnerQuery("DarkSpaceSources", $startDate, $endDate).')  as all_data WHERE "sourceIP" =  \''  . $numericExternalIP
		 .'\' union all '
		 .'select "content",("endTime"-"startTime") as duration,"protocol","sourceIP" as  "internalIP","sourcePort" as "internalPort","destinationPort" as "externalPort",\'0\' as "initiator","numBytes","startTime", \'Dark Space Sources\' as "schemaName" from ('.getInnerQuery("DarkSpaceSources", $startDate, $endDate).')  as all_data WHERE "destinationIP" =  \''  . $numericExternalIP.'\'';//both IP fields can be either internal or external
		 break;
	   case "NonDNSTraffic":
	     $query = 'select "content",("endTime"-"startTime") as duration,"protocol","internalIP","internalPort","externalPort",\'0\' as initiator,"numBytes","startTime", \'Protocol Violations\' as "schemaName" from ('.getInnerQuery("NonDNSTraffic", $startDate, $endDate).') as all_data WHERE "externalIP" =  \'' . $numericExternalIP.'\'';
	     break;
	   case "MultimediaP2PTraffic":
	    $query = 'select "content",("endTime"-"startTime") as duration,"protocol","internalIP","internalPort","externalPort",\'0\' as initiator,"numBytes","startTime", \'Multimedia P2P Traffic\' as "schemaName" from ('.getInnerQuery("NonDNSTraffic", $startDate, $endDate).') as all_data WHERE "internalIP" =  \'' . $numericInternalIP. '\' AND "type" = \'' . MULTIMEDIA_TYPE;
	    break;
	   default:	
         showErrorBox('Symptom not found');	
		 exit;
	}
	//echo "About to return: $query";//dl
	return $query;
  }  
  function getInternalIPQuery($numericInternalIP,$startDate,$endDate,$schemaName=0)//for internal IPs
  {	

	if(!$schemaName)//select all schemas
	{   
		$query = 'select "content",("endTime"-"startTime") as duration,\'0\' as "protocol","externalIP","internalPort","externalPort","initiator","numBytes", "startTime", \'Communication Channels\' as "schemaName" from ('.getInnerQuery("CommChannels", $startDate, $endDate).') as all_data WHERE "internalIP" = \'' . $numericInternalIP 
		.'\' union all '
		.'select "content",("endTime"-"startTime") as duration,"protocol","externalIP","internalPort","externalPort","initiator","numBytes", "startTime", \'Infected Contacts\' as "schemaName" from ('.getInnerQuery("InfectedContacts", $startDate, $endDate).') as all_data  WHERE "internalIP" = \'' . $numericInternalIP 
		.'\' union all '
		.'select "content",("endTime"-"startTime") as duration,"protocol","externalIP","internalPort","externalPort",\'0\' as initiator,"numBytes","startTime", \'Evasive Traffic\' as "schemaName" from ('.getInnerQuery("EvasiveTraffic", $startDate, $endDate).') as all_data WHERE "internalIP" =  \'' . $numericInternalIP
		.'\' union all '    
		.'select \'{"0"}\'::uint32[] as "content", "startTime" as duration,\'0\' as "protocol","destinationIP" as "externalIP",\'0\' as "internalPort","destinationPort" as "externalPort",\'0\' as "initiator",\'0\' as "numBytes","startTime", \'Brute Forcers\' as "schemaName" from ('.getInnerQuery("BruteForcers", $startDate, $endDate).')  as all_data WHERE "sourceIP" =  \''  . $numericInternalIP  
		.'\' union all '
		.'select \'{"0"}\'::uint32[] as "content","startTime" as duration,\'0\' as "protocol","sourceIP" as "externalIP",\'0\' as "internalPort","destinationPort" as "externalPort",\'0\' as "initiator",\'0\' as "numBytes", "startTime", \'Brute Forced\' as "schemaName" from ('.getInnerQuery("BruteForcers", $startDate, $endDate).')  as all_data WHERE "destinationIP" =  \''  . $numericInternalIP
		.'\' union all '
		.'select "content",("endTime"-"startTime") as duration,"protocol","externalIP","internalPort","externalPort","initiator","numBytes", "startTime", \'Dark Space Targets\' as "schemaName" from ('.getInnerQuery("DarkSpaceTargets", $startDate, $endDate).')  as all_data WHERE "internalIP" =  \''  . $numericInternalIP
		.'\' union all '
		.'select "content",("endTime"-"startTime") as duration,"protocol","destinationIP" as "externalIP","sourcePort" as "internalPort","destinationPort" as "externalPort",\'0\' as "initiator",\'0\' as "numBytes", "startTime", \'Dark Space Sources\' as "schemaName" from ('.getInnerQuery("DarkSpaceSources", $startDate, $endDate).')  as all_data WHERE "sourceIP" =  \''  . $numericInternalIP
		.'\' union all '
		.'select "content",("endTime"-"startTime") as duration,"protocol","sourceIP" as "internalIP","sourcePort" as "internalPort","destinationPort" as "externalPort",\'0\' as "initiator",\'0\' as "numBytes", "startTime", \'Dark Space Sources\' as "schemaName" from ('.getInnerQuery("DarkSpaceSources", $startDate, $endDate).')  as all_data WHERE "destinationIP" =  \''  . $numericInternalIP
		.'\' union all '
		// .'select "content",("endTime"-"startTime") as duration,"protocol","internalIP","internalPort","externalPort",\'0\' as initiator,"numBytes","startTime", \'Protocol Violations\' as "schemaName" from ('.getInnerQuery("NonDNSTraffic", $startDate, $endDate).')  as all_data WHERE "internalIP" =  \''  . $numericInternalIP
		// .'\' union all '
		// .'select "content",("endTime"-"startTime") as duration,"protocol","internalIP","internalPort","externalPort",\'0\' as "initiator","numBytes","startTime", \'Multimedia P2P Traffic\' as "schemaName" from ('.getInnerQuery("NonDNSTraffic", $startDate, $endDate).')  as all_data WHERE "externalIP" =  \''  . $numericExternalIP. '\' AND "type" = \'' . MULTIMEDIA_TYPE 
		// .'\' union all '
		.'select \'{"0"}\'::uint32[] as "content",\'0\' as duration,\'0\' as "protocol","ip" as "externalIP",\'0\' as "internalPort",\'0\' as "externalPort",\'0\' as "initiator",\'0\' as "numBytes", \'0\' as "startTime", \'Multiple Reboots\' as "schemaName" from ('.getInnerQuery("Reboots", $startDate, $endDate).')  as all_data WHERE "ip" = \''  . $numericInternalIP.'\'';

		return $query;
	}

	switch ($schemaName)
	{
	   case "CommChannels":
	   	 $query = 'select "content",("endTime"-"startTime") as duration,\'0\' as "protocol","externalIP","internalPort","externalPort","initiator","numBytes", "startTime", \'Communication Channels\' as "schemaName" from ('.getInnerQuery("CommChannels", $startDate, $endDate).') as all_data WHERE "internalIP" = \'' . $numericInternalIP.'\'';			 	    
	    break;
	   case "InfectedContacts":
	   	 $query = 'select "content",("endTime"-"startTime") as duration,"protocol","externalIP","internalPort","externalPort","initiator","numBytes", "startTime", \'Infected Contacts\' as "schemaName" from ('.getInnerQuery("InfectedContacts", $startDate, $endDate).') as all_data  WHERE "internalIP" = \'' . $numericInternalIP.'\'';		
	    break;
	   case "EvasiveTraffic":
	   	 $query = 'select "content",("endTime"-"startTime") as duration,"protocol","externalIP","internalPort","externalPort",\'0\' as initiator,"numBytes","startTime", \'Evasive Traffic\' as "schemaName" from ('.getInnerQuery("EvasiveTraffic", $startDate, $endDate).') as all_data WHERE "internalIP" =  \'' . $numericInternalIP.'\'';
	    break;
	   case "DarkSpaceTargets":
	   	 $query = 'select "content",("endTime"-"startTime") as duration,"protocol","externalIP","internalPort","externalPort","initiator","numBytes","startTime", \'Dark Space Targets\' as "schemaName" from ('.getInnerQuery("DarkSpaceTargets", $startDate, $endDate).')  as all_data WHERE "internalIP" =  \''  . $numericInternalIP.'\'';
	    break;	  
	   case "Reboots":
	     $query = 'select \'{"0"}\'::uint32[] as "content",\'0\' as duration,\'0\' as "protocol","ip" as "externalIP",\'0\' as "internalPort",\'0\' as "externalPort",\'0\' as "initiator",\'0\' as "numBytes", \'0\' as "startTime", \'Multiple Reboots\' as "schemaName" from ('.getInnerQuery("Reboots", $startDate, $endDate).')  as all_data WHERE "ip" = \''  . $numericInternalIP.'\'';
	    break;
	   case "BruteForced":
	   	 $query = 'select \'{"0"}\'::uint32[] as "content","startTime" as duration,\'0\' as "protocol","sourceIP" as "externalIP",\'0\' as "internalPort","destinationPort" as "externalPort",\'0\' as "initiator",\'0\' as "numBytes", "startTime", \'Brute Forced\' as "schemaName" from ('.getInnerQuery("BruteForcers", $startDate, $endDate).')  as all_data WHERE "destinationIP" =  \''  . $numericInternalIP.'\'';
	     break;
	   case "BruteForcers":
	   	 $query = 'select \'{"0"}\'::uint32[] as "content", "startTime" as duration,\'0\' as "protocol","destinationIP" as "externalIP",\'0\' as "internalPort","destinationPort" as "externalPort",\'0\' as "initiator",\'0\' as "numBytes","startTime", \'Brute Forcers\' as "schemaName" from ('.getInnerQuery("BruteForcers", $startDate, $endDate).')  as all_data WHERE "sourceIP" =  \''  . $numericInternalIP.'\'';
	     break;
	   case "NonDNSTraffic":
	    $query = 'select "content",("endTime"-"startTime") as duration,"protocol","internalIP","internalPort","externalPort",\'0\' as initiator,"numBytes","startTime", \'Protocol Violations\' as "schemaName" from ('.getInnerQuery("NonDNSTraffic", $startDate, $endDate).') as all_data WHERE "internalIP" =  \'' . $numericInternalIP.'\'';
	    break;
		case "MultimediaP2PTraffic":
	    $query = 'select "content",("endTime"-"startTime") as duration,"protocol","internalIP","internalPort","externalPort",\'0\' as initiator,"numBytes","startTime", \'Multimedia P2P Traffic\' as "schemaName" from ('.getInnerQuery("NonDNSTraffic", $startDate, $endDate).') as all_data WHERE "internalIP" =  \'' . $numericInternalIP. '\' AND "type" = \'' . MULTIMEDIA_TYPE;
	    break;
	   case "DarkSpaceSources":
	   	 $query = 'select "content",("endTime"-"startTime") as duration,"protocol","destinationIP" as "externalIP","sourcePort" as "internalPort","destinationPort" as "externalPort",\'0\' as "initiator","numBytes","startTime", \'Dark Space Sources\' as "schemaName" from ('.getInnerQuery("DarkSpaceSources", $startDate, $endDate).')  as all_data WHERE "sourceIP" =  \''  . $numericInternalIP
		 .'\' union all '
		 .'select "content",("endTime"-"startTime") as duration,"protocol","sourceIP" as "internalIP","sourcePort" as "internalPort","destinationPort" as "externalPort",\'0\' as "initiator","numBytes","startTime", \'Dark Space Sources\' as "schemaName" from ('.getInnerQuery("DarkSpaceSources", $startDate, $endDate).')  as all_data WHERE "destinationIP" =  \''  . $numericInternalIP.'\'';
		 break;
	   default:	
         showErrorBox('Symptom not found');	
		 exit;
	}	

	return $query;	
  }

  function getCountQueryWithIP($numericIP,$schemaName,$startDate,$endDate)//number of records found for particular symptom when IP is given
  {
		$ipFieldMap['BruteForcers'] = 'sourceIP';
		$ipFieldMap['BruteForced'] = 'destinationIP';
		$ipFieldMap['Reboots'] = 'ip';
		if($schemaName == 'BruteForced')
		{
			$schemaName1 = 'BruteForcers';//actual table name
			$schemaName2 = 'BruteForced';//display name if destinationIP ("bruteforced" ip) is being looked for
		}
		else
		{
			$schemaName1 = $schemaName2 = $schemaName;
		}

        $ipFieldMap['DarkSpaceSources'] = 'sourceIP';
		//ADD schemaName1, schemaName2, if not BruteForced, set them to equal
		if(isInternal($numericIP))
		{
			$ipFieldMap['CommChannels'] = 'internalIP';
			$ipFieldMap['InfectedContacts'] = 'internalIP';
			$ipFieldMap['EvasiveTraffic'] = 'internalIP';
			$ipFieldMap['NonDNSTraffic'] = 'internalIP';
			$ipFiledMap['MultimediaP2PTraffic'] = 'internalIP';			
			$ipFieldMap['DarkSpaceTargets'] = 'internalIP';
		}
		else//external IP
		{
			$ipFieldMap['CommChannels'] = 'externalIP';
			$ipFieldMap['InfectedContacts'] = 'externalIP';
			$ipFieldMap['EvasiveTraffic'] = 'externalIP';
			$ipFieldMap['NonDNSTraffic'] = 'externalIP';
			$ipFiledMap['MultimediaP2PTraffic'] = 'externalIP';
			$ipFieldMap['DarkSpaceTargets'] = 'externalIP';   
		}
		
        initiliazeCountParameters($dates,$nDates,$counter,$counter2,$schemaName1);
		
		if($startDate==$endDate)//one day selected
		{
		   //echo "Query is: ".'select count(*) from'.'"'.$schemaName.'"'.'."'.$startDate.'" WHERE "' . $ipFieldMap[$_POST['symptom']] . '" = \'' . $numericIP . '\'';
		   return 'select count(*) from'.'"'.$schemaName1.'"'.'."'.$startDate." WHERE ".' "'. $ipFieldMap[$schemaName2] . '" = \'' . $numericIP . '\'';
		}
		 
		foreach($dates as $date)
		{
		   if($startDate == $date)
           {            	   
		     
			 while($dates[$counter]!= $endDate)
			 {
			   //echo "Before first if dates: $dates[$counter], counter: $counter <br>";
			   
			   if($dates[$counter+1]==$endDate)
			   {		
				 if($counter2==0)//two day range
				 {
				    
					 $queryName = 'select count(*) as rows from '.'"'.$schemaName1.'"'.'."'.$dates[$counter].'"'.'WHERE "'. $ipFieldMap[$schemaName2] . '" = \'' . $numericIP . '\'';					 
					 $queryName = $queryName." union all ";
					 $queryName = $queryName.'select count(*) as rows from '.'"'.$schemaName1.'"'.'."'.$dates[$counter+1].'"'.'WHERE "'. $ipFieldMap[$schemaName2] . '" = \'' . $numericIP . '\'';
					 break;
				 }
					 
				 $queryName = $queryName.' union all select count(*) as rows from '.'"'.$schemaName1.'"'.'."'.$dates[$counter].'"'.'WHERE "'. $ipFieldMap[$schemaName2] . '" = \'' . $numericIP . '\'';
				 $queryName = $queryName.' union all select count(*) as rows from '.'"'.$schemaName.'"'.'."'.$dates[$counter+1].'"'.'WHERE "'. $ipFieldMap[$schemaName2] . '" = \'' . $numericIP . '\'';
				 break;
			    }
			   else if($counter2==0)//start of range
			   {			    
				  $queryName = ' select count(*) as rows from '.'"'.$schemaName1.'"'.'."'.$dates[$counter].'"'.'WHERE "'. $ipFieldMap[$schemaName2] . '" = \'' . $numericIP . '\'';				
			   }
			   else
			   {			
				  $queryName = $queryName.' union all select count(*) as rows from '.'"'.$schemaName1.'"'.'."'.$dates[$counter].'"'.'WHERE "'. $ipFieldMap[$schemaName2] . '" = \'' . $numericIP . '\'';
				}
			   
			   if($counter==$nDates)
			   {
			     showErrorBox('Invalid Range');
				 exit;
			   }
			   $counter++;
			   $counter2++;
			 }
			 break;			 
           }
            $counter++;		   
		}
		$queryName = "select sum(rows)
                       as total_rows
							from ( $queryName ) as all_data";
		return $queryName;
  }
  function showTableHeaderIPsearch()
  {
     	echo '<div class="table">' .
            '<table width="100%" cellspacing="1" id="asns">' .			 
              '<tr>' .   
                '<td class="columnTitle center" onclick="sortByWithIP(\'protocol\');">'.
	     		 'Protocol'.
                '</td>' .			  
                '<td class="columnTitle center unsortable">'.
                  'Internal IP' .
                '</td>'  .
				'<td class="columnTitle center unsortable">'.
                  'External IP' .
                '</td>'  .
                '<td class="columnTitle center" onclick="sortByWithIP(\'internalPort\');">'.
                  'Internal Port' .
                '</td>' .
                '<td class="columnTitle center" onclick="sortByWithIP(\'externalPort\');">'. 
                  'External Port' .
                '</td>' .
		        '<td class="columnTitle center" onclick="sortByWithIP(\'numBytes\');">'. 
                  'Number of Bytes' .
                '</td>' .
				'<td class="columnTitle center" onclick="sortByWithIP(\'startTime\');">'.
                  'First Occurence' .
                '</td>' .	
				'<td class="columnTitle center" onclick="sortByWithIP(\'schemaName\');">'.
                  'Symptom' .
                  '</td>' .	
				'<td class="columnTitle center unsortable">'.
                   'Content Distribution' .
                 '</td>' .				  
                 '</tr>';
  }
  function showDatabaseResultsIPsearch($query,$numericIP,$startDate,$endDate)
  {
  	//echo "showDatabaseResultsIPsearch called\n";//dl
	$protocolNames = array(6 => 'TCP', 17 => 'UDP');   
	$infectedIPs;
    getInfectedIPs($GLOBALS['postgreSQL'], $startDate, $endDate, $infectedIPs);	
	//populate $num records from inner query, use code on another page as example
	//echo "Query is: $query";//dl
	$result = pg_query($GLOBALS['postgreSQL'],$query);

	while ($row = pg_fetch_assoc($result)) {
        setRowColor($rowNumber,$rowClass);
        echo '<tr class="' . $rowClass . '">';
		 	if ($row['schemaName']=="Communication Channels") 
		    {
			  echo '<td>'."TCP".'</td>';
		    }
			else if($row['schemaName']=='Brute Forcers'||$row['schemaName']=='Brute Forced')
			  echo  '<td>'.getServiceName(6, $row['internalPort'], $row['externalPort'], $row['initiator']).'</td>';
		    else 
		      echo '<td>'.$protocolNames[$row['protocol']].'</td>';	
			//echo long2ip($numericIP)."|".long2ip($row['externalIP'])."|".long2ip($row['internalIP'])."<br>";//dl
			if(isInternal($numericIP))
			{
               //echo "numericIP is: $numericIP";//dl
        	   echo '<td class="center">'.long2ip($numericIP).'</td>';
			   echo '<td>'.styleIP($infectedIPs,$row['externalIP'],false).'</td>';//show IP in red color if it is on infected IPs list
			}
			else
			{
			   if(!isset($row['internalIP']))
			      $row['internalIP']=$row['externalIP'];//with DarkSpaceTargets or a similar IP scheme
				  
			   echo '<td class="center">'.long2ip($row['internalIP']).'</td>';
			   echo '<td>'.styleIP($infectedIPs,$numericIP,false).'</td>';//show IP in red color if it is on infected IPs list
			}
			
            echo '<td class="center">';
			    if($row['schemaName']=='Brute Forcers'||$row['schemaName']=='Brute Forced')
                { 
                   echo "0";
				}
				else
				  echo styleInternalPort($row['internalPort'], $row['initiator']);				  
			  echo '</td>'.
			  '<td class="center">'.			   
                 styleExternalPort($row['externalPort'], $row['initiator']) .
               '</td>' .
			    '<td class="center">'.			   
                 size($row['numBytes']) .
				 //$row['numBytes'] .
               '</td>' .
			   '<td class="center">'.			   
                 date('M d, D g:i:s A', $row['startTime']) .
               '</td>' .
			   	'<td class="center">'.			   
                $row['schemaName'] .
               '</td>' .
			   	'<td class="center">'.			   
                 drawContentStripe($row['content'],$row['numBytes']) .
               '</td>' .
			  '</tr>';
		  ++$rowNumber;
      }
      echo '</table>' .
         '</div>';	
  }
  function setTitle($schemaName,$numericIP,$startDate,$endDate)
  {
     if($schemaName)
		$title = $schemaName." ".long2ip($numericIP)." ".$startDate." - ".$endDate;
	   else
		$title = long2ip($numericIP)." ".$startDate." - ".$endDate;
		
	return $title;
  }
  function setPagerOptionsIPsearch(&$pager_options,&$pager,&$offset,&$num_records,&$currentPage,$countQuery,$schemaName,$linkPath)
  {	  
	  //var_dump($GLOBALS['postgreSQL']);//dl
	  //echo "Count query is: $countQuery";//dl
	  $result = pg_query($GLOBALS['postgreSQL'],$countQuery);
	  //echo "countQuery is: $countQuery";//dl
	  $row=pg_fetch_row($result);
	  $num_records=$row[0];	
      //echo "linkPath is: $linkPath";//dl
      checkNumRecords($num_records);
     
	  if(!$schemaName)
	    $pager_options = getPagerOptions($num_records,'https://'.$_SERVER['SERVER_ADDR'].'/search/index.php');
      else if($linkPath)
	  {
	    //echo "linkPath is set";//dl
		$pager_options = getPagerOptions($num_records,$linkPath);		
	  }
	  else//if particular symptom, go to symptom search	
        $pager_options = getPagerOptions($num_records,'https://'.$_SERVER['SERVER_ADDR'].'/search/symptom.php');

	  $pager = createPager($pager_options);

	  if($linkPath)
	    $offset = getOffsetShortcutLink($pager);
      else
	    $offset = getOffset($pager);	
	
	  if($offset==0)
		  $currentPage = 1;//current page in results
	  else
		  $currentPage = ($offset+50)/50;
  }
  function addSortParameter($query,$offset,$pager_options)//adds sort parameter and pagination options to query
  {
  	adjustSortParameter("sortParameterWithIP");//reads sort cookie saved by JavaScript, and, if necessary, stores value into session
	if(isset($_SESSION["sortParameterWithIP"]))//check if sort parameter is set for IP Search
	{	   
	   $sortParameter = $_SESSION["sortParameterWithIP"];

	   if($_SESSION["sortParameterWithIP"]=="duration")
	     $query = $query.' ORDER BY duration DESC OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];
	   else
		 $query = $query.getSortQuery($sortParameter,$offset,$pager_options['perPage']);
	}
	else//no sorting parameter set
      $query = $query.' OFFSET '.$offset.' LIMIT '.$pager_options['perPage'];

	//echo "Inside addSortParameter, about to return: $query";//dl
    return $query;	  
  }
  function setIPsearchDisplayParameters(&$endDateIPsearch,&$ipIPsearch,&$numericIP)
  {
	$endDateIPsearch = processDisplayParameter('endDateIPsearch',$endDateIPsearch,$isEndDate=1,$startDateIPsearch);
	$ipIPsearch = processDisplayParameter('ipIPsearch',$ipIPsearch);//saves or retrieves display parameter
	$numericIP = sprintf('%u', ip2long($ipIPsearch));  
  }
?>