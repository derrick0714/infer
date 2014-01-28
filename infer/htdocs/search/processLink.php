<?php
    include('../include/accessControl.php');
	include('ipSearchMethods.php');
	include('symptomSearchMethods.php');//to process request when symptom is known
	include('../include/shared.php');
	$request = explode('/', substr($_SERVER['PATH_INFO'], 1));
  
	$schemaName = $request[0];
	$ip = $request[1];
  	$startDate = $request[2];		
    $endDate = $request[3];  
	
	$linkPath = 'https://'.$_SERVER['SERVER_ADDR'].'/search/processLink';
	$linkPath = $linkPath.'/'.$schemaName.'/'.$ip.'/'.$startDate.'/'.$endDate;

		switch ($schemaName)
		{
		   case "Communication Channels":
		   	 $schemaName = "CommChannels";
			break;
		   case "Infected Contacts":
		   	 $schemaName = "InfectedContacts";
			 break;
		   case "Evasive Traffic":
		   	 $schemaName = "EvasiveTraffic";			
		    break;
		   case "Dark Space Sources":
		   	 $schemaName = "DarkSpaceSources";		
		    break;
		   case "Dark Space Targets":
		   	 $schemaName = "DarkSpaceTargets";		
		    break;
		   case "MultimediaP2PTraffic":
		   	 $schemaName = "MultimediaP2PTraffic";	
		    break;
		   case "Protocol Violations":
		   	 $schemaName = "NonDNSTraffic";	
		    break;
		   case "Multiple Reboots":
		     $schemaName = "Reboots";
		    break;
		   case "Brute Forcers":
		   	 $schemaName = "BruteForcers";
		     break;
		   case "Brute Forced":
		   	 $schemaName = "BruteForced";
		     break;
		   default:	
             showErrorBox('Symptom not found');		   
		}  

	displayRangeWithIP($startDate,$endDate,$ip,$schemaName,$linkPath);
    //echo $request[0]." ".$request[1]." ".$request[2]." ".$request[3];
?>