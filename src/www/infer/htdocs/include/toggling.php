<?php
function showTitle($row,$rowNumber,$asnCount,$postgreSQL, $countryCodeMap, $countryNameMap)
{
		$rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
		$temp;
        echo '<tr class="' . $rowClass . '">';		
	
		$asnID='asn'.$asnCount.'_body';
		echo '<td class="asTitle left" title=AS'.$row[3].' onclick="toggleASBody('.'\''.$asnID.'\''.')"> <span id="sign'.$asnID.'"'.'>+</span>'; 
								   
        if (!isInternal($row[0])) {
		       $temp=getASDescriptionByNumber($postgreSQL, $row[3]);			   	
               echo $temp;		   
        } 
		else
		   echo getASDescriptionByNumber($postgreSQL, $row[3]);
					 
        echo '</td>';
         echo '<td class="center">';           
       
         echo  getCountryPicture($row[4], $countryCodeMap, $countryNameMap);	
			  
		 echo '</td>' ;
         echo '<td class="center">';
				
         echo   '</td>' .
            '<td class="center">';
			    
                 echo  '</td>' .
                    '<td class="center">';
				 
                 echo   '</td>' .
         '</tr>';
}

function showTitleM($row,$rowNumber,$asnCount,$postgreSQL, $countryCodeMap, $countryNameMap)
{
		$rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
		$temp;
        echo '<tr class="' . $rowClass . '">';		
	
		$asnID='asn'.$asnCount.'_body';
		echo '<td class="asTitle left" title=AS'.getASNByIP($postgreSQL, $row['destinationIP']).' onclick="toggleASBody('.'\''.$asnID.'\''.')"> <span id="sign'.$asnID.'"'.'>+</span>'; 
								   
  	    echo getASDescriptionByNumber($postgreSQL, getASNByIP($postgreSQL, $row['destinationIP']));
		
        echo '</td>';
        echo '<td class="center">';           
      
		echo  getCountryPicture(getCountryNumberByIP($postgreSQL, $row['destinationIP']), $countryCodeMap, $countryNameMap);	
			  
		echo '</td>' ;
        echo '<td class="center">';
				
        echo   '</td>' .
            '<td class="center">';
			    
                echo  '</td>' .
                   '<td class="center">';
			 
                echo   '</td>' .
         '</tr>';
}

function showTitleMr($row,$rowNumber,$asnCount,$postgreSQL, $countryCodeMap, $countryNameMap)
{
		$rowNumber % 2 ? $rowClass = 'odd' : $rowClass = 'even';
		$temp;
        echo '<tr class="' . $rowClass . '">';		
	
		$asnID='asn'.$asnCount.'_body';
		echo '<td class="asTitle left" title=AS'.getASNByIP($postgreSQL, $row['sourceIP']).' onclick="toggleASBody('.'\''.$asnID.'\''.')"> <span id="sign'.$asnID.'"'.'>+</span>'; 
								   
  	    echo getASDescriptionByNumber($postgreSQL, getASNByIP($postgreSQL, $row['sourceIP']));
		
        echo '</td>';
        echo '<td class="center">';           
      
		echo  getCountryPicture(getCountryNumberByIP($postgreSQL, $row['sourceIP']), $countryCodeMap, $countryNameMap);	
			  
		echo '</td>' ;
        echo '<td class="center">';
				
        echo   '</td>' .
            '<td class="center">';
			    
                echo  '</td>' .
                   '<td class="center">';
			 
                echo   '</td>' .
         '</tr>';
}


?>