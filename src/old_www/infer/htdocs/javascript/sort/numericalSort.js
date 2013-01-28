<!--
function numericalSort(tableID, columnID, elementIdentifier)
{ 
	if(!overThousandRecords)//confirm that the user want to proceed
	{
	   numSort(tableID, columnID, elementIdentifier)
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
function numSort(tableID, columnID, elementIdentifier)
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
	var tbodies2Length=tbodies2.length;//IE optimization
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
    return parseFloat(tbodies2[index].rows[0].cells[column].innerHTML.replace(/[',']/g,""));   
 }
 
 function find(value,start)
 {
   for(var i=start; i<tbodies2Length; i+=2)
   {
      if(getN(i)==value)
	    return i;
   }

 }

}



















/*selection sort

function alphabeticalSort(tableID, columnID, elementIdentifier)
{
	var mainTable = document.getElementById(tableID);
	var tbodies2 = mainTable.getElementsByTagName('tbody');
	var count1=1;
	var count2=1;
	var id, parsedName, smallestNumber, first, second, toCompare;	
	var column = getColumnIndex(mainTable,columnID);
	
	var data = new Array();
		
    while(count1<tbodies2.length)
	{
	   id = tbodies2[count1].id;	 
	   if(id.indexOf(elementIdentifier)==-1)
	   {
          count1++;
		  continue;
	   }	  
	   smallestIndex = count1;	  
	   parsedName = getA(smallestIndex);	
	   
      data.push(parsedName);
      data.sort();	 
	  
	  // for (counter=0; counter<data.length; counter++)
       //   alert(data[counter]);
   
   
	   count2=count1+1;		  
	   while(count2<tbodies2.length)
	   {
		   id = tbodies2[count2].id;		  
	       if(id.indexOf(elementIdentifier)==-1)
	       {
             count2++;
		     continue;
	       }		    		   
		   toCompare=getA(count2);				   		  
			if(sortedColumns.indexOf(column)!=-1)//check if column is sorted
			{		  
			  if(toCompare<smallestNumber)
		      {			
			     smallestIndex=count2;
			     smallestNumber=getA(smallestIndex);					   
				
			     swap(tbodies2[count1],tbodies2[count2]);
			     swap(tbodies2[count2+1],tbodies2[count1+1]);				
		      }
			} 
			else
			{
		      if(toCompare>smallestNumber)
		      {			
			     smallestIndex=count2;
			     smallestNumber=getA(count2);
				
			     swap(tbodies2[count1],tbodies2[count2]);
			     swap(tbodies2[count2+1],tbodies2[count1+1]);				
		       }
			}
		     count2++;			
	   }
	   count1++;	
	}	
	updateSortedColumns(column);

  function getA(index)
  {
   //alert("Index: " +index);//delete 
   if(index>=tbodies2.length)
    return "aaaaaaaa";
	 
	//alert("Index: " + index);//delete
   var original = tbodies2[index].rows[0].cells[column].innerHTML;//tbodies2[index].rows[0].cells[column].innerHTML;   change
   var value=original;
   //alert(": "+index+" "+column);//delete
   if(value.indexOf('span')!= -1)//ASN name
   {
	 strip_start = value.lastIndexOf('>')+1;
 	 strip_end = value.length;
	  //alert("about to return: " + value.substring(strip_start, strip_end).toLowerCase());//delete
 	 return value.substring(strip_start, strip_end).toLowerCase();
   }
   else if(value.indexOf('img')!= -1)//country image
   {
	 strip_start = value.lastIndexOf('title')+7;
	 strip_end = value.length;
 	 value = value.substring(strip_start, strip_end);
	 strip_end = value.indexOf('"');
	 value = value.substring(0, strip_end);
	 //alert("About to return: " + value.toLowerCase());
	 return value.toLowerCase();
   }   
 //  alert("Failed to parse: " +value);   
  // exit;
 }
}




*/
//-->


