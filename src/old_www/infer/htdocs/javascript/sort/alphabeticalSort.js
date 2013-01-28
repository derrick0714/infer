<!--

function alphabeticalSort(tableID, columnID, elementIdentifier)
{ 
	if(!overThousandRecords)//confirm that the user want to proceed
	{
	   alphaSort(tableID, columnID, elementIdentifier)
	   return;
	}
	 
	 if(confirm("This operation may take up to one minute. Do you want to proceed?"))
	 {
	    document.body.style.cursor = 'wait';
        setTimeout("alphaSort('"+tableID+"', '"+columnID+"', '"+elementIdentifier+"')", 4000);	  
     } 
	 else
	 {
        return;
     }
}

function alphaSort(tableID, columnID, elementIdentifier)
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
	   document.body.style.cursor = 'default';
	   adjustASTitlesColor(mainTable);
	   return;
    }	
	var tbodies2Length=tbodies2.length;//IE optimization
    while(count1<tbodies2Length)
	{
	  parsedName = getA(count1);	
	 
      data.push(parsedName);
	  
      count1+=2;	  	
	}
	
   data.sort();  
 
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
   
  function getA(index)
  {
   var value = tbodies2[index].rows[0].cells[column].innerHTML;
   
   var strip_start = value.lastIndexOf('>')+1;
   var strip_end = value.length;
 
   return value.substring(strip_start, strip_end).toLowerCase(); 
 }
 
 function find(value,start)
 {
   for(var i=start; i<tbodies2Length; i+=2)
   {
      if(getA(i)==value)
	    return i;
   }
   exit();
 }
}
//-->


