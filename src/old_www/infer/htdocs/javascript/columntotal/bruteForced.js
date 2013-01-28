<!--
 
function computeColumnTotalBruteForced(element)
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
					  sum += parseInt(tbodies[count1].rows[row].cells[col].innerHTML);					
				}
				
          }
		  	
		  if(col==3)//check whether any units need to be added
		  {			
			 titleAsn.rows[0].cells[col+1].innerHTML=sum;					  
		  }
		  
		  titleAsn.rows[0].cells[2].innerHTML=findMin(count1,1);	
		  titleAsn.rows[0].cells[3].innerHTML=findMax(count1,2);
		 
		  sum=0;
		  units="none";
		  factor=1;
       } 	  
	   count1++;
	 } 
document.getElementById("loading").innerHTML="";

 function getN(string)
 {
    //alert("About to return: " + parseInt(tbodies2[index].rows[0].cells[column].innerHTML.replace(/[',']/g,"")));//delete
    return parseInt(string.substring(string.indexOf('>')+1));   
 }
  
 function findMin(index,column)//find the smallest time in row and return html value of corresponding cell
 {	 
     //alert("Column is: "+column);//delete
	 var tempLength=tbodies[index].rows.length;
	 var tempMin, tempValue, rawHTML, tempRawHTML;
	 
	 tempMin=getN(tbodies[index].rows[0].cells[column].innerHTML);
	 rawHTML=tbodies[index].rows[0].cells[column].innerHTML;
	
	 for(var i=0; i<tbodies[index].rows.length; i++)
	 {
	   //alert(tbodies[index].rows[i].cells[column].innerHTML);
	   tempValue = getN(tbodies[index].rows[i].cells[column].innerHTML);
	   tempRawHTML=tbodies[index].rows[i].cells[column].innerHTML;//keep track of unconverted time
	   if(tempValue<tempMin)
	   {
	        tempMin = tempValue;
			rawHTML = tempRawHTML;			
	   }
	 }
	 
	 return rawHTML;
 }
 
 function findMax(index,column)//find the smallest time in row and return html value of corresponding cell
 {	 
     //alert("Column is: "+column);//delete
	 var tempLength=tbodies[index].rows.length;
	 var tempMax, tempValue, rawHTML, tempRawHTML;
	 
	 tempMax=getN(tbodies[index].rows[0].cells[column].innerHTML);
	 rawHTML=tbodies[index].rows[0].cells[column].innerHTML;
	
	 for(var j=0; j<tbodies[index].rows.length; j++)
	 {
	   tempValue = getN(tbodies[index].rows[j].cells[column].innerHTML);
	   tempRawHTML=tbodies[index].rows[j].cells[column].innerHTML;//keep track of unconverted time
	   if(tempValue>tempMax)
	   {
	        //alert("Yes, "+tempValue+" "+tempMax);//delete
			//alert("Yes, "+tempRawHTML+" "+rawHTML);//delete
			tempMax = tempValue;
			rawHTML = tempRawHTML;
			//alert("Current maximum is: "+rawHTML);//delete
	   }
	 }
	// alert("The smallest value was: " + tempMin);//delete
	 return rawHTML;
 }

	 
}

//-->