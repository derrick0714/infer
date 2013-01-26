<!--
function bandwidthSort(tableID, columnID, elementIdentifier)
{ 
	if(!overThousandRecords)//confirm that the user want to proceed
	{
	   bandwSort(tableID, columnID, elementIdentifier)
	   return;
	}
	 
	 if(confirm("This operation may take up to one minute. Do you want to proceed?"))
	 {
	    document.body.style.cursor = 'wait';
        setTimeout("bandwSort('"+tableID+"', '"+columnID+"', '"+elementIdentifier+"')", 4000);	    
     } 
	 else
	 {
        return;
     }
}
function bandwSort(tableID, columnID, elementIdentifier)
{
	var mainTable = document.getElementById(tableID);
	var tbodies2 = mainTable.getElementsByTagName('tbody');
	var count1=1;
	var count2=1;
	var id, parsedName, smallestNumber, first, second, toCompare, oldPosition;	
	var column = getColumnIndex(mainTable,columnID);
	
	var data = new Array();
	
	if(sortedColumns.indexOf(column)!=-1)//check if column is sorted
	{
	   reverseTable(tableID); 
	   updateSortedColumns(column);
	   adjustASTitlesColor(mainTable);
	   return;
    }	
	var tbodies2Length=tbodies2.length;
    while(count1<tbodies2Length)
	{
	  parsedName = getN(count1);		
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
	 
  updateSortedColumns(column);
  document.body.style.cursor = 'default';
  adjustASTitlesColor(mainTable);

  function getN(index)
  {
   return convertToBytes(tbodies2[index].rows[0].cells[column].innerHTML);     
  }
 
 function find(value,start)
 {
   for(var i=start; i<tbodies2Length; i+=2)//IE optimization
   {
      if(getN(i)==value)
	    return i;
   }
 //  exit;
 }

}

//-->