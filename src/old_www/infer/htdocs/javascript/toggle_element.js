<!--
 //Each Autonomous System is displayed as two components: title and body.  The function hides or displays AS body.
function toggleASBody(element){
   var sign = "sign";
   sign = sign + element;	
   if(document.getElementById('hid').value == 'none')
   {
	  if (!(/MSIE (\d+\.\d+);/.test(navigator.userAgent)))
	  {
		  document.getElementById(element).style.display = 'table-row-group';
		  document.getElementById(element).style.width = '100%';		   
	  }
	  else
		   document.getElementById(element).style.display = 'block';
		   
	  document.getElementById('hid').value = 'block';		
		
      document.getElementById(sign).innerHTML='-';	
	  adjustASBodyRowsColor(document.getElementById(element));	  
   }
   else if(document.getElementById('hid').value == 'block')
   {
      document.getElementById(element).style.display = 'none';
      document.getElementById('hid').value = 'none';

      document.getElementById(sign).innerHTML='+';	   
   }
   adjustRowsColor('asns');
}  
//Each Autonomous System is displayed as two components: title and body.  The function hides or displays all AS bodies at once.
function toggleAllASBodies(tableID,signID)
{
   var mainTable=document.getElementById(tableID);
   var tbodies = mainTable.getElementsByTagName("tbody");
   if(toggled==false)
   {
		for(var i=2; i<tbodies.length; i+=2)
		{
			toggleOpenASBody(tbodies[i].id);	 
		}
	toggled=true;
	showSign(signID);
	adjustRowsColor(mainTable); 
   }
   else
   {
   		for(var i=2; i<tbodies.length; i+=2)
		{
			toggleCloseASBody(tbodies[i].id);	 
		}
      toggled=false;
	  showSign(signID);
	  adjustASTitlesColor(mainTable)
   }     
}
var toggled=false;//saves state, whether elements are displayed or hidden
//This function is used internally to open (display) an AS body.  It is one way only and cannot be used to hide the AS body.
function toggleOpenASBody(element)
{
	  var sign = "sign";
      sign = sign + element;	 
	  
 	  if (!(/MSIE (\d+\.\d+);/.test(navigator.userAgent)))
	  {
		  document.getElementById(element).style.display = 'table-row-group';
		  document.getElementById(element).style.width = '100%';		   
	  }
	  else
		   document.getElementById(element).style.display = 'block';
		   
	  document.getElementById('hid').value = 'block';		
		
      document.getElementById(sign).innerHTML='-';	   
}
//This function is used internally to close (hide) an AS body.  It is one way only and cannot be used to show the AS body again.
function toggleCloseASBody(element)
{
   var sign = "sign";
   sign = sign + element;	

   document.getElementById(element).style.display = 'none'; 
   document.getElementById(sign).innerHTML='+';	      
}
function showSign(signID)
{ 
   if(!toggled)
    document.getElementById(signID).innerHTML='+'; 
   else
    document.getElementById(signID).innerHTML='-';     
}
function hideSign(signID)
{
   document.getElementById(signID).innerHTML='';	
}
function toggleTable(tableID, signID)
{
   var mainTable=document.getElementById(tableID);
   
   if(document.getElementById('hid').value == 'none')
   {
	  if (!(/MSIE (\d+\.\d+);/.test(navigator.userAgent)))
	  {
		  document.getElementById(tableID).style.display = 'table';		  	   
	  }
	  else
		   document.getElementById(tableID).style.display = 'block';
		   
	  document.getElementById('hid').value = 'block';		
		
      document.getElementById(signID).innerHTML='-';		
   }
   else if(document.getElementById('hid').value == 'block')
   {
      document.getElementById(tableID).style.display = 'none';
      document.getElementById('hid').value = 'none';

      document.getElementById(signID).innerHTML='+';	   
   }
}
//adjust color of all rows, depending whether it is visible or not
function adjustRowsColor(mainTable)
{ 
   if(mainTable=='asns')
	   mainTable = document.getElementById('asns');
	   
	var rows = mainTable.getElementsByTagName("tr");
	var rowNumber = 0;
	
	for(var j=0; j<rows.length; j++)
	{
        if(isVisible(rows[j]))//process only visible rows
		{
			if(rowNumber%2==0)
				rows[j].className = "odd";
			else
				rows[j].className = "even";		
			
       		rowNumber++;	 
		}		
    }	
}

function adjustASTitlesColor(mainTable)
{
   if(toggled)
   {
     adjustRowsColor(mainTable);
	 return;
   }
   
   var tbodies = mainTable.getElementsByTagName("tbody");
   var rowNumber = 0;
   for(var i=1; i<tbodies.length; i+=2)
   {
	    if(rowNumber%2==0)		
			tbodies[i].rows[0].className ="odd";					
		else
		    tbodies[i].rows[0].className = "even";
			
		rowNumber++;
   }
}
function adjustASBodyRowsColor(asBody)
{
  //set initial color 
  var titleASName = asBody.id.replace(/body/,"title");
  var asTitle = document.getElementById(titleASName);
  var initialColorClass = asTitle.rows[0].className;
  var rowNumber;
  if(initialColorClass == "even")  
    rowNumber=0;  
  else
    rowNumber=1;
	
  if(asBody.rows.length==1)
  {
  		if(rowNumber%2==0)		
		{
			asBody.rows[0].className = "odd";			
		}
		else
		{
		    asBody.rows[0].className = "even";			
		}
	return;		
  }
  
  for(var i=0; i<asBody.rows.length; i++)
  {
     	if(rowNumber%2==0)		
		{
			asBody.rows[i].className = "odd";			
		}
		else
		{
		    asBody.rows[i].className = "even";			
		}
			
		rowNumber++;		
  }   
 
}
//check if row (or DOM element) is currently visible (checks current or inherited style)
function isVisible(row)
{
     if(!row.className)//process only rows with actual data
	   return true;
		 
	 if (row == document) return true
      
        if (!row) return false
       if (!row.parentNode) return false
       if (row.style) {
           if (row.style.display == 'none') return false
         if (row.style.visibility == 'hidden') return false
     }
     
	   //Or get the computed style using IE's proprietary way
       var style = row.currentStyle
       if (style) {
          if (style['display'] == 'none') return false
             if (style['visibility'] == 'hidden') return false
       }
	   
      //Try the computed style in a standard way
      if (window.getComputedStyle) {
           var style = window.getComputedStyle(row, "")
           if (style.display == 'none') return false
          if (style.visibility == 'hidden') return false
       }
      
     return isVisible(row.parentNode)
}

//-->

