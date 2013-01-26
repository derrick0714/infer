function enable(form,fieldIndex)//enable form field 
{
  form[fieldIndex].disabled=false;
}
function disable(form,fieldIndex)//disable form field
{
  form[fieldIndex].disabled=true;
}
function copy(form,fromFieldIndex,toFieldIndex)//copy value of one form field to another field
{
  form[toFieldIndex].value = form[fromFieldIndex].value;
  
  //make sure Reboots option is not selected
  if(form.name=="symptomSearchForm")  
    if(!form.symptom[8].checked)
      enable(form,toFieldIndex);	
}
function changeInnerHTML(elementID,newValue)//change inner HTML of an element
{
  document.getElementById(elementID).innerHTML = newValue;   
}
function show(elementID)//make element visible
{
  document.getElementById(elementID).style.display = "block";  
}
function changeType(elementID,newType)//change type of element
{
  document.getElementById(elementID).type = newType; 
}
function addHour(form,fieldIndex)//add  one hour to current time value
{
  var dateTime = form[fieldIndex].value;
  dateTime = dateTime.split(" ");
  var time = dateTime[1].split(":");
  var hour = time[0];
  hour++;
  time[0]=hour;
  if(time[0]<10)
    time[0]="0"+time[0];
	
  var newDateTime = dateTime[0]+" "+time[0]+":"+time[1]+":"+time[2]+" ";
  
  form[fieldIndex].value = newDateTime;
}
//remove trailing white space
function trimWhitespace(value) 
{
  var result = value.match(/^\s*(.*\S)\s*$/);
 
  if (result !== null && result.length === 2)
    return result[1];
  else
    return value;
}

