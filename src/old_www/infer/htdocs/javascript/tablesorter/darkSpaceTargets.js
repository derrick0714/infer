$(document).ready(function() { 
    $("#darkSpaceTargetsTable").tablesorter({ 
        headers: { 
           
          	6: {
			   sorter: 'time'
			   }
			
		},
		widgets:['zebra']
    }); 
	$("#ipPortSources").tablesorter({ 
        headers: { 
           
          	0: {
			   sorter: false
			   }
			
		},
		widgets:['zebra']
    }); 
	
});   
  