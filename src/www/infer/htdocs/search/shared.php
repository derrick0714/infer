<?php
  
    if(file_exists('../include/dbLink.php'))
         include('../include/dbLink.php');
    else
         include('./include/dbLink.php');
  
  session_start();
 
  function showErrorBox($message)
  {
  	 echo "<script type=\"text/javascript\">alert('$message'); history.go(-1);</script>"; 
	 echo "</body> </html>";
	 exit;
  }    
  function getInnerQuery($schemaName, $startDate, $endDate)//returns all data from corresponding tables in this range
  {	
	  $isInnerQuery = true;
      //echo "getInnerQuery: $schemaName, $startDate, $endDate";//delete
	  return getCountQuery($schemaName, $startDate, $endDate, 0, $isInnerQuery);
  }  
  //returns total number of results available for range of dates, for a particular symptom, but with no IP given
  function getCountQuery($schemaName, $startDate, $endDate, $reboots=0, $isInnerQuery=0)
  {
     // echo "<br>"."Inside getCountQuery"."<br>";//delete
	 // $n_arguments = func_num_args();
	 // for ($i=0; $i<$n_arguments; $i++) {

        // var_dump(func_get_arg($i)); // get each argument passed
		// echo "<br>";
    // }	
	   
	    //echo "getCountQuery: schemaName is: ".$schemaName;	  
		if($reboots)
		{
			$selectPart = 'select ip,"applicationTimes"[1] as date, COUNT(*) as count from';
		}
		else if($isInnerQuery)//select part is just select query, not count query
		{
			$selectPart = 'select * from';//selects all records in range instead of count
		}
		else
		{
			$selectPart = 'select count(*) as rows from';//select part of query			
		}
	    //echo "Select part is: $selectPart";//dl
        initiliazeCountParameters($dates,$nDates,$counter,$counter2,$schemaName);
		
		if($startDate==$endDate)//one day selected
		{
		   return $selectPart.'"'.$schemaName.'"'.'."'.$startDate.'"';
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
				     $queryName = "$selectPart ".'"'.$schemaName.'"'.'."'.$dates[$counter].'"';					 
					 $queryName = $queryName." union all ";
					 $queryName = $queryName."$selectPart ".'"'.$schemaName.'"'.'."'.$dates[$counter+1].'"';
					 break;
				 }
					 
				 $queryName = $queryName." union all $selectPart ".'"'.$schemaName.'"'.'."'.$dates[$counter].'"';
				 $queryName = $queryName." union all $selectPart ".'"'.$schemaName.'"'.'."'.$dates[$counter+1].'"';
				 break;
			    }
			   else if($counter2==0)//start of range
			   {
			     //echo "Inside second if<br>";//delete
				  $queryName = " $selectPart ".'"'.$schemaName.'"'.'."'.$dates[$counter].'"';				
			   }
			   else
			   {
				  //echo "Inside third if";//delete
				  $queryName = $queryName." union all $selectPart ".'"'.$schemaName.'"'.'."'.$dates[$counter].'"';
				 
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
		if(!$isInnerQuery)
		{
		  $queryName = "select sum(rows)
                         as total_rows
				 			from ( $queryName ) as all_data";	
		}
		//echo "<br>Full count query is: $queryName</br>";//delete		
		//exit;//delete
		return $queryName;
  }
  function getSortQuery($sortParameter,$offset,$perPage)
  {
		if($sortParameter == "numBytes" || $sortParameter == "numFrags" || $sortParameter == "count")
		  return 'ORDER BY "'.$sortParameter.'" DESC OFFSET '.$offset.' LIMIT '.$perPage;
		else 
		  return 'ORDER BY "'.$sortParameter.'" ASC OFFSET '.$offset.' LIMIT '.$perPage;
  }  
  function getPagerOptions($num_records,$path)
  {
      	$pager_options = array(
	    'mode'       => 'Sliding',
	    'perPage'    => 50,		    
		'append'   => false, 
		'path'     => $path,
		'delta'    => 4,
		'fileName' => '%d',
	    'totalItems' => $num_records,			
	);
	//echo "getPagerOptions called";//delete
	  return $pager_options;
  }
  
  function createPager($pager_options)
  {
	$pager = Pager::factory($pager_options);
	
	return $pager;
  }
  function checkNumRecords($num_records)
  {
  	if($num_records==0)
	{
	    showErrorBox('No records found');
		exit;
	}  
  }
  function getOffset($pager)
  {
  	list($from, $to) = $pager->getOffsetByPageId();	
	$offset = $from-1;
	return $offset;  
  }
  //shows pagination links at the bottom of search results
  function displayLinks($num_records,$pager)
  {
	// echo "<br>"."Inside getCountQuery"."<br>";//delete
	//echo "Display links called, num_records is: $num_records";//dl
	
	 // $n_arguments = func_num_args();
	// for ($i=0; $i<$n_arguments; $i++) {

        // var_dump(func_get_arg($i)); // get each argument passed
		// echo "<br>";
    // }	
	
	if($num_records>50)
	{
	   $links = $pager->getLinks();		
	   echo '<div class="center">'.$links['first'].$links['back'].' '.$links['pages'].$links['next'].' '.$links['last'].'</div>';  
	}   
  }
  //removes sort parameter passed as argument
  function removeSortParameter()
  {
    $n_arguments = func_num_args();
	for ($i=0; $i<$n_arguments; $i++) {

        unset($_SESSION[func_get_arg($i)]); // get each argument passed
    }	
  }  	
	
   function adjustSortParameter($parameterName)//reads sort cookie saved by JavaScript, and, if necessary, stores value into session
   {
     if(isset($_COOKIE["$parameterName"]))//cookie was set by JavaScript
	 {
        //save sort parameter, remove cookie
        $_SESSION[$parameterName] = $_COOKIE["$parameterName"];		
		removeCookie($parameterName);
	 }
   }
  function removeCookie($parameterName)
  {
    setcookie($parameterName,"",time()-3600);
  }
 function curentPageURL()//shows URL of current page
 {
   $pageURL = 'http';
   if ($_SERVER["HTTPS"] == "on") {$pageURL .= "s";}
   $pageURL .= "://";
   if ($_SERVER["SERVER_PORT"] != "80") {
   $pageURL .= $_SERVER["SERVER_NAME"].":".$_SERVER["SERVER_PORT"].$_SERVER["REQUEST_URI"];
   }  else {
    $pageURL .= $_SERVER["SERVER_NAME"].$_SERVER["REQUEST_URI"];
  }
 return $pageURL;
}
 function __autoload($class_name) 
 {
   if($class_name=="ShortcutLink")
    require_once $class_name . '.php';
 } 
 function processDisplayParameter($parameterName,$parameterValue,$isEndDate=0,$startDate=0)//saves or retrieves display parameters
 {
	if($isEndDate)
	{
		if(isset($parameterValue)&&$parameterValue!="")//set display parameter with data needed for pagination	
        {		
		   $_SESSION["$parameterName"] = $parameterValue;		
        }		
		else	
		{
		   $parameterValue = $_SESSION["$parameterName"];
		   if(!isset($parameterValue)||$parameterValue=="")//if second date field is disabled by default
			 $parameterValue = $startDate;
		}
		
		return $parameterValue;
	}
	 
	if(isset($parameterValue)&&$parameterValue!="")//set display parameter with data needed for pagination
	{		
		$_SESSION["$parameterName"] = $parameterValue;//save parameter	
	}		   
	else
	{
		$parameterValue = $_SESSION["$parameterName"];
	}
	return $parameterValue;
 }
   
  function isIP($string)//a simple function to distinguish IP from date
  {
     return stripos($string,".");
  }
 
  function getRebootTime($applicationTimes)//returns time of first reboot event
  {
    $applicationTimes = explode(',', substr($row['applicationTimes'], 1, -1));
	
	return date("l, F j, Y \a\\t g:i:s A", $applicationTimes[0]);
  }
  
  function initiliazeCountParameters(&$dates,&$nDates,&$counter,&$counter2,$schemaName)
  {
   		$dates = getPGTableRange($GLOBALS['postgreSQL'], $schemaName,getFirstPGTable($GLOBALS['postgreSQL'], $schemaName), getLastPGTable($GLOBALS['postgreSQL'], $schemaName));
		//echo "About to show dates: " .var_dump($dates)."<br>";
		$nDates = count($dates);
		$counter = 0;	
		$counter2 = 0;
  }
  
  function jsSearchIncludeFiles()
  {
    return '<script charset="utf-8" type="text/javascript" src="/javascript/toggle_element.js"></script>'.   
  '<script charset="utf-8" type="text/javascript" src="/javascript/formprocessing/general.js"></script>'.
  '<script charset="utf-8" type="text/javascript" src="/javascript/sort/search.js"></script>'.
  '<script charset="utf-8" type="text/javascript" src="/javascript/formprocessing/symptomSearch.js"></script>'.
  '<script charset="utf-8" type="text/javascript" src="/javascript/formprocessing/formValidation.js"></script>';  
  }
  
  function setRowColor($rowNumber,&$rowClass)
  {
    $rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
  }  
?>