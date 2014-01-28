<!--
function timeMaxSort(tableID, columnID, elementIdentifier)
{ 
	if(!overThousandRecords)//confirm that the user want to proceed
	{
	   timeMxSort(tableID, columnID, elementIdentifier)
	   return;
	}
	 
	 if(confirm("This operation may take up to one minute. Do you want to proceed?"))
	 {
	    document.body.style.cursor = 'wait';
        setTimeout("numSort('"+tableID+"', '"+columnID+"', '"+elementIdentifier+"')", 4000);	  
     } 
	 else
	 {
        return;
     }
}
function timeMxSort(tableID, columnID, elementIdentifier)
{
	var mainTable = document.getElementById(tableID);
	var tbodies2 = mainTable.getElementsByTagName('tbody');
	var count1=1;
	var count2=1;
	var id, parsedName, smallestNumber, first, second, toCompare, oldPosition;	
	var column = getColumnIndex(mainTable,columnID);	
	
	var data = new Array();	

	var tbodies2Length=tbodies2.length;//IE optimization
    while(count1<tbodies2Length)
	{
	  parsedName = getN(tbodies2[count1].rows[0].cells[column].innerHTML);//find minimum time value in corresponding asn body	 
	  data.push(parsedName);
	  
      count1+=2;	  	
	}
   
   data.sort(sortNumber);    
  
   for(var i=0; i<data.length; i++)
   {
	 oldPosition=find(data[i],count2);//find index of tbody where value is stored
	 if(oldPosition!=count2)
	 {
	  swap(tbodies2[count2],tbodies2[oldPosition]);//move value to beginning
	  swap(tbodies2[count2+1],tbodies2[oldPosition+1]);	  
	 }
	 count2+=2;
   }	 
 
  document.body.style.cursor = 'default';
  adjustASTitlesColor(mainTable);

  function getN(string)
  {
    return parseFloat(string.substring(string.indexOf('>')+1));   
  }
  
 function find(value,start)
 {
   for(var i=start; i<tbodies2Length; i+=2)
   {
      if(getN(tbodies2[i].rows[0].cells[column].innerHTML)==value)
	    return i;		
   }
 }

}

//-->


