<!--
 
function computeColumnTotalInfectedHosts(element)
{
   // alert("Inside column total");//delete
	var tbodies, columnCount, rowCount, sum, titleAsn, tbodies2, count1, mainTable, factor, units, stored;
	 sum=0;
	 count1=1;
	 factor=1;
	 units="none";
	 stored=0;
	 mainTable = document.getElementById(element);	 
	 if(!mainTable)
        return;	 
	 var tbodies = mainTable.getElementsByTagName("tbody");	 
	 var tbodiesLength = tbodies.length;
	

	 while(count1<tbodiesLength)
	 {
	   if(count1%2!=0)
	   {
          count1++;
		  continue;
	   }
	   	   
       rowCount = tbodies[count1].rows.length;	
	   columnCount = tbodies[count1].rows[0].cells.length;
      
	  titleAsn = tbodies[count1-1]; 

      for(var col=1; col<columnCount; col++){
	   //alert("Working with: " + titleAsn.rows[0].cells[col+1].innerHTML);//delete
          for(var row=0; row<rowCount; row++){
		  
		  if(typeof(tbodies[count1].rows[row].cells[col])=="undefined")
		  {
		      alert("Script time out error. This can be corrected by increasing default maximum PHP execution time.");
			  document.getElementById("loading").innerHTML="";
		  }
			  
			    if(col==3)//number of bytes column
				{					
					  sum += convertToBytes(tbodies[count1].rows[row].cells[col].innerHTML);					
				}
				else 
				  sum+=parseFloat(tbodies[count1].rows[row].cells[col].innerHTML.replace(/[','|'KiB'|'MiB'|'B']/g,""));	
          }
		  	
		  if(col==3)//check whether any units need to be added
		  {			
			 titleAsn.rows[0].cells[col+1].innerHTML=convertFromBytes(sum);					  
		  }
		  else if(col!=1)
		    titleAsn.rows[0].cells[col+1].innerHTML=format(sum,2);	

		  sum=0;
		  units="none";
		  factor=1;
       } 	  
	   count1++;
	 } 
document.getElementById("loading").innerHTML="";	 
	 
}

//-->