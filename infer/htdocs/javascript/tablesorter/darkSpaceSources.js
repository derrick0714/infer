$(document).ready(function() { 
    $("#darkSpaceSourcesHostDetails").tablesorter({ 
        headers: { 
           
          	5: {
			   sorter: 'time'
			   }
			
		},
		widgets:['zebra']
    }); 
	$("#darkSpaceSourcesTable").tablesorter({ 
        headers: { 
           
          	0: {
			   sorter: false
			   }
			
		},
		widgets:['zebra']
    }); 
});   
  