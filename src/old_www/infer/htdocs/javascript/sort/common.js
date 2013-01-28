<!--

if(sortedColumns==undefined)
{
   var sortedColumns = new Array();//identifies which columns have been sorted
}

function getColumnIndex(table,columnID)
{  
    var count = 0;	
	while(table.rows[0].cells[count].id!=columnID)
	{
	   count++;
	}	
	return count;
}


function convertToBytes(rawHTML)
{
	var factor, stored;
	if(rawHTML.indexOf("KiB")!=-1)
		 factor=1024;
	else if(rawHTML.indexOf("MiB")!=-1) 
		 factor=1024*1024;
	else if(rawHTML.indexOf("GiB")!=-1) 
		 factor=1024*1024*1024; 
	else
		 factor=1;
					   
   stored=parseFloat(rawHTML.replace(/[','|'KiB'|'MiB'|'B'|'GiB']/g,""));  
   return (stored*factor);
}
 
function convertFromBytes(sum)
{
   if(sum>=1024*1024*1024)
   {
		sum=sum/(1024*1024*1024.0);
		units="GiB";
   }
   else if(sum>=1024*1024)
   {				    
		sum=sum/(1024*1024.0);					
		units="MiB";
   }
   else if(sum>=1024)
   {
		sum=sum/1024.0;
		units="KiB";
   }
   else
   {
		 units="B";
   }
	
	 sum=format(sum,2);
	 sum = sum +" "+ units;
			 
    return sum;
}			
 
function format(num,dec)//handles precision and thousand separators
{
   dec=Math.pow(10,dec);
   var num=(Math.round(num*dec)/dec) + "";
   var first=num.split(".");
   var tmp=new Array;
   var cnt=0;
   var start=first[0].length % 3;
   if (start) tmp[cnt++]=first[0].substr(0,start);
   for(var i=start;i<first[0].length;i+=3) tmp[cnt++]=first[0].substr(i,3);
   first[0]=tmp.join(",");
   return first.join(".");
} 
 
function swap(a,b)
{
  
  return [
    a.parentNode.replaceChild(b.cloneNode(true), a),
    b.parentNode.replaceChild(a.cloneNode(true), b)
  ];
}

function updateSortedColumns(column)
{
	 if(sortedColumns.indexOf(column)==-1)//store column number as previously sorted
	 {
	    sortedColumns.push(column);	       	
	 }
	 else
	 {	 
		sortedColumns.splice(sortedColumns.indexOf(column),1);//remove the number
	 }
}

 function sortNumber(a,b)
 {
   return b-a;
 }
 
 function sortNumberAscend(a,b)
 {  
   return a-b;
 }
 
function reverseTable(tableID)
{
	//alert("Reverse table called");
	var mainTable = document.getElementById(tableID);
	for(var i = 1; i<mainTable.tBodies.length; i+=2)
	{
		 if(mainTable.tBodies.length-1-i <= i)
		   break;
		  
         //alert("About to swap: " + i + " " + (mainTable.tBodies.length-1-i));		  
		 swap(mainTable.tBodies[i],mainTable.tBodies[mainTable.tBodies.length-1-i]);//swap title
		 swap(mainTable.tBodies[i+1],mainTable.tBodies[mainTable.tBodies.length-1-i+1]);//swap body
	}
}

function verify()
{
  var result= confirm("This operation may take up to one minute. Do you want to proceed?");
  if (!result)
  {
    exit();
  }
  
}

if(!Array.indexOf){
	    Array.prototype.indexOf = function(obj){
	        for(var i=0; i<this.length; i++){
	            if(this[i]==obj){
	                return i;
	            }
	        }
	        return -1;
	    }
	}
//-->