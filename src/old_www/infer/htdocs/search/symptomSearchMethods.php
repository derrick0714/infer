  <?php   
  include('shared.php');

  // $selectQuery = "select count(*) from";//delete
  // $unionQuery = "union all select count(*) as rows from";
  // echo "selectQuery is: $selectQuery";//delete
  // exit;//delete  

  function getCountQueryP2P($schemaName,$startDate,$endDate,$multimediaType)
  {        
		initiliazeCountParameters($dates,$nDates,$counter,$counter2,$schemaName);
		
		if($startDate==$endDate)//one day selected
		{
		   return 'select count(*) from'.'"'.$schemaName.'"'.'."'.$startDate.'"'.'WHERE "type" = '. $multimediaType;
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
			     //echo "Inside first if<br>";//delete
				 if($counter2==0)//two day range
				 {
				    
					 $queryName = 'select count(*) as rows from '.'"'.$schemaName.'"'.'."'.$dates[$counter+1].'"'.' WHERE "type" = '. $multimediaType;
					 $queryName = $queryName." union all ";
					 $queryName = $queryName.'select count(*) as rows from '.'"'.$schemaName.'"'.'."'.$dates[$counter+1].'"'.' WHERE "type" = '. $multimediaType;
					 break;
				 }
					 
				 $queryName = $queryName.' union all select count(*) as rows from '.'"'.$schemaName.'"'.'."'.$dates[$counter].'"'.'WHERE "type" = '. $multimediaType;
				 $queryName = $queryName.' union all select count(*) as rows from '.'"'.$schemaName.'"'.'."'.$dates[$counter+1].'"'.'WHERE "type" = '. $multimediaType;
				 break;
			    }
			   else if($counter2==0)//start of range
			   {
			     //echo "Inside second if<br>";//delete
				  $queryName = ' select count(*) as rows from '.'"'.$schemaName.'"'.'."'.$dates[$counter].'"'.'WHERE "type" = '. $multimediaType;				
			   }
			   else
			   {
				  //echo "Inside third if";//delete
				  $queryName = $queryName.' union all select count(*) as rows from '.'"'.$schemaName.'"'.'."'.$dates[$counter].'"'.'WHERE "type" = '. $multimediaType;				 
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
		//echo "About to return: $queryName";//delete		
		//exit;//delete
		return $queryName;  
  } 
  
  function convertSourceToInternal($displayParameterValue)
  {
    switch ($displayParameterValue)
	{
	  case "sourceIP":
	   return "internalIP";
	  case "destinationIP":
	   return "externalIP";
	  case "sourcePort":
	   return "internalPort";
	  case "destinationPort":
	   return "externalPort";
      default:
       	return $displayParameterValue;  
	}
  }
  
  function  convertInternalToSource($displayParameterValue)
  {
    switch ($displayParameterValue)
	{
	  case "internalIP":
	   return "sourceIP";
	  case "externalIP":
	   return "destinationIP";
	  case "internalPort":
	   return "sourcePort";
	  case "externalPort":
	   return "destinationPort";
      default:
       	return $displayParameterValue;  
	}
  }
 //displays range of dates, if both IP and schemaName/symptom are given
 function displayRangeWithIP($startDate,$endDate,$ip,$schemaName,$linkPath=0)
 {
	// $n_arguments = func_num_args();	
	// for ($i=0; $i<$n_arguments; $i++) {

        // var_dump(func_get_arg($i)); // get each argument passed
		// echo "<br>";
    // }	
	
	if($linkPath)
	{
	    $numericIP = sprintf('%u', ip2long($ip));
		displayIPSearchResults($numericIP,$startDate,$endDate,$schemaName,$linkPath);    	
	    return;//no need to set display parameters
	}
	$startDate = processDisplayParameter('startDate',$startDate);
    setSymptomSearchDisplayParameters($endDate,$ip);		
		
	if(isset($schemaName)&&$schemaName!="")
		$_SESSION["schemaName"]=$schemaName;	
	else
		$schemaName = $_SESSION["schemaName"];	
	
	$numericIP = sprintf('%u', ip2long($ip));	
	
	displayIPSearchResults($numericIP,$startDate,$endDate,$schemaName,$linkPath);   
 }
 function setSymptomSearchDisplayParameters(&$endDate,&$ip=0)
 {
	 // echo "<br>"."setSymptomSearchDisplayParameters"."<br>";//delete
	 // $n_arguments = func_num_args();
	 // for ($i=0; $i<$n_arguments; $i++) {

        // var_dump(func_get_arg($i)); // get each argument passed
		// echo "<br>";
    // }	
	// echo "<br>";//delete
	$startDate = processDisplayParameter('startDate',$starDate);
    //echo "startDate retrieved from session is: $startDate";//dl
    $endDate = processDisplayParameter('endDate',$endDate,$issetEndDate=1,$startDate);	 
	//echo "endDate retrieved from session is: $endDate";//dl
	if($ip)
	 $ip = processDisplayParameter('ip',$ip);
 }
 ?>