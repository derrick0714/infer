  <!--
  //The  customized functions  below are  used to toggle  (show/hide) host details. The naming convention corresponds to the sequentiol order of elements being toggled.
  
  function toggle(element){
     if(document.getElementById('hid').value == 'none')
     {
	  	  if (!(/MSIE (\d+\.\d+);/.test(navigator.userAgent)))
		  {
		    document.getElementById(element).style.display = 'table';
		    document.getElementById(element).style.width = '100%';
		  }
		  else
		    document.getElementById(element).style.display = 'block'; 
		 
         document.getElementById('hid').value = 'block';

         document.getElementById('sign').innerHTML='-';
		 window.scrollTo(0,9999999);
     }
     else if(document.getElementById('hid').value == 'block')
     {
       document.getElementById(element).style.display = 'none';
         document.getElementById('hid').value = 'none';

                 document.getElementById('sign').innerHTML='+';
     }
  }
  function toggle2(element){
     if(document.getElementById('hid').value == 'none')
     {
	  	  if (!(/MSIE (\d+\.\d+);/.test(navigator.userAgent)))
		  {
		    document.getElementById(element).style.display = 'table';
		    document.getElementById(element).style.width = '100%';
		  }
		  else
		    document.getElementById(element).style.display = 'block'; 
			
         document.getElementById('hid').value = 'block';

                 document.getElementById('sign2').innerHTML='-';
				 window.scrollTo(0,9999999);
     }
     else if(document.getElementById('hid').value == 'block')
     {
       document.getElementById(element).style.display = 'none';
         document.getElementById('hid').value = 'none';

                 document.getElementById('sign2').innerHTML='+';
     }
  }
  
  function toggle3(element){
     if(document.getElementById('hid').value == 'none')
     {
	  	  if (!(/MSIE (\d+\.\d+);/.test(navigator.userAgent)))
		  {
		    document.getElementById(element).style.display = 'table';
		    document.getElementById(element).style.width = '100%';
		  }
		  else
		    document.getElementById(element).style.display = 'block'; 
			
	if(document.getElementById('openPortsTable'))
	{
	   document.getElementById('openPortsTable').style.display = 'none';
	}
	   
         document.getElementById('hid').value = 'block';

                 document.getElementById('sign3').innerHTML='-';
				 window.scrollTo(0,9999999);
     }
     else if(document.getElementById('hid').value == 'block')
     {
       document.getElementById(element).style.display = 'none';
	   if(document.getElementById('openPortsTable'))
	   {
		   if (/MSIE (\d+\.\d+);/.test(navigator.userAgent))
		      document.getElementById('openPortsTable').style.display = 'block';
		    else
			   document.getElementById('openPortsTable').style.display = 'table';
		}  
            document.getElementById('hid').value = 'none';

                 document.getElementById('sign3').innerHTML='+';
     }
  }
 
  function toggle4(element){
     if(document.getElementById('hid').value == 'none')
     {
	  	  if (!(/MSIE (\d+\.\d+);/.test(navigator.userAgent)))
		  {
		    document.getElementById(element).style.display = 'table';
		    document.getElementById(element).style.width = '100%';
		  }
		  else
		    document.getElementById(element).style.display = 'block'; 
			
         document.getElementById('hid').value = 'block';

                 document.getElementById('sign4').innerHTML='-';
				 window.scrollTo(0,9999999);
     }
     else if(document.getElementById('hid').value == 'block')
     {
       document.getElementById(element).style.display = 'none';
         document.getElementById('hid').value = 'none';

                 document.getElementById('sign4').innerHTML='+';
     }
  }
  function toggle5(element)
  {
     if(document.getElementById('hid').value == 'none')
     {
        document.getElementById(element).style.display = 'block'; 			
        document.getElementById('hid').value = 'block';

		document.toggleIcon.src="/img/toggle_close.png";
		window.scrollTo(0,9999999);
     }
     else if(document.getElementById('hid').value == 'block')
     {
        document.getElementById(element).style.display = 'none';
        document.getElementById('hid').value = 'none';

		document.toggleIcon.src="/img/toggle_open.png";
				
     }
  }
  //-->

