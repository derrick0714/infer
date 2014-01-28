<!--

function alphabeticalSort(tableID, columnID, elementIdentifier)
{
	//alert("Inside sort");//delete
	var mainTable = document.getElementById(tableID);
	var tbodies2 = mainTable.getElementsByTagName('tbody');
	var column = getColumnIndex(mainTable,columnID);
    var sortingLimit = 	tbodies2.length;
	
	if(sortedColumns.indexOf(column)!=-1)//check if column is sorted
	{
	   reverseTable(tableID); 
	   updateSortedColumns(column);
	   return;
    }	
	quicksortAlphabetical(1, sortingLimit-2);	
	
 	
	updateSortedColumns(column);


function quicksortAlphabetical(begin, end)
{   
   // alert("Entered quick sort function");//delete
   // alert("Begin, end: " + begin + " " + end);//delete
  	if(end-2>begin) {
	    //alert("Begin, end: " +begin+" "+end);//delte
		//var sum = begin+end;
		//alert("Begin+end: " + sum);//delete
		var pivot=parseInt(begin/2.0+(begin+end)/4.0);//Math.floor(Math.random()*(end-begin));
		//alert("Pivot index: " + pivot);//delete
		if(pivot%2==0)
		 pivot++;
       // alert("About to call partition function");//delete
		pivot=partitionAlphabetical(begin, end, pivot);
	//	if(pivot%2==0)
	//	 pivot++;

		quicksortAlphabetical(begin, pivot);
		quicksortAlphabetical(pivot+2, end);
	}
			
	/*
	//    alert("Column: "+column);//delete
	  //  alert("lo, hi: " + lo+" "+hi);//delete
		//alert("Point A");
		//alert("hi, lo: " + hi + " " +lo);//delete
				for (var i=1; i<tbodies2.length-2; i+=2)
		{
		   if(getN(i)>getN(i+2))
		   {
		      swap(tbodies2[i],tbodies2[i+2]);
			  swap(tbodies2[i+1],tbodies2[i+2+1]);
		   }
		}
		
		if(hi <= lo+1) return;
	//	 alert("Point B");
		if((hi - lo) == 2) {//CHANGE?
			if(getN(hi-1) > getN(lo)) 
			{
			  swap(tbodies2[hi-1], tbodies2[lo]);
			  swap(tbodies2[hi], tbodies2[lo+1]);
			}
			return;
		}
		//alert("Point C");
		var i = lo + 2;
		var j = hi - 2;
		alert("After First increment:"+i+" "+j);//delete
		//alert("Hi, low: " + j + " " +i);//delete
		//alert("Point A");//delete
		if(getN(lo) > getN(i))
		{
		  swap(tbodies2[i], tbodies2[lo]);
		  swap(tbodies2[i+1], tbodies2[lo+1]);
		}
		//alert("Point B");
		if(getN(j) > getN(lo))
		{
		  swap(tbodies2[lo], tbodies2[j]);
		  swap(tbodies2[lo+1], tbodies2[j+1]);
		}
	//	alert("Point C");
		if(getN(lo) > getN(i)) 
		{
		  swap(tbodies2[i], tbodies2[lo]);
		  swap(tbodies2[i+1], tbodies2[lo+1]);
		}
	//	alert("Point D");
		var pivot = getN(lo);
		
		while(true) {
		//alert("Point D1");
			j=j-2;
			while(pivot > getN(j)&& j>0)
			{
			  j=j-2;
			  alert("J is: " + j);//delete
			}
			//alert("Point D2");
			i=i+2;
			while(getN(i) > pivot && i<(tbodies2.length-1))
            {			
			  i=i+2;
			  alert("I is: " + i);//delete
			}
			//alert("Point D3");
			if(j <= i) break;
			//alert("Point D3.5");
			swap(tbodies2[i], tbodies2[j]);
			swap(tbodies2[i+1], tbodies2[j+1]);
			//alert("Point D4");
		}
		swap(tbodies2[lo], tbodies2[j]);
		swap(tbodies2[lo+1], tbodies2[j+1]);
		//alert("Point E");
		//alert("j,lo,hi,j: " +j+" "+lo+" "+hi+" "+j);//delete

		
		if((j-lo) < (hi-j)) {
		//alert("D4");
			quicksortNumerical(lo,j);
			quicksortNumerical(j+2,hi);
		} else {
			//	alert("Point D5");
			quicksortNumerical(j+2,hi);
			quicksortNumerical(lo,j);
		}

           */
}
function getA(index)
{
   //alert("Index: " +index);//delete 
   if(index>=tbodies2.length)
    return "aaaaaaaa";
	 
   var original = tbodies2[index].rows[0].cells[column].innerHTML;   
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
function partitionAlphabetical(begin, end, pivot)
{
	var piv=getA(pivot);
	if(pivot!=end-2)
	{
	  swap(tbodies2[pivot], tbodies2[end-2]);
	  swap(tbodies2[pivot+1], tbodies2[end-2+1]);
	}
	var store=begin;
	var ix;
	//alert("Before loop");//delete
//	alert("Pivot: " + piv);//delete
	for(ix=begin; ix<end; ix+=2) {
	//alert("Now looking at: " +getA(ix));//delete
	// alert("Ix: " + ix);
		if(getA(ix)>piv) {
		 //  alert("Store: " + store + " ix" + ix);//delete
		 //alert("About to swap: " + tbodies2[store].id + " " + tbodies2[ix].id);//delete
		 if(ix!=store){		    
			swap(tbodies2[store], tbodies2[ix]);
			swap(tbodies2[store+1], tbodies2[ix+1]);
			}
			
			store+=2;
		}
	}
/*	for(var iy=1; iy<tbodies2.length-2; iy+=2) {
	 // alert(getA(ix));//delete
	// alert("Ix: " + ix);
		if(getA(iy)>getA(iy+2)) {
			swap(tbodies2[iy], tbodies2[iy+2]);
			swap(tbodies2[iy+1], tbodies2[iy+2+1]);
			//alert("Swapped: " + tbodies2[store].id + " " + tbodies2[ix].id);//delete			
		}
	}
	
*/	if(end-2!=store)
    {
      swap(tbodies2[end-2], tbodies2[store]);
	  swap(tbodies2[end-1], tbodies2[store+1]);
	}

	return store;
}
}

//-->