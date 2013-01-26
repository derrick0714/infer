$(document).ready(function() { 	
    $("#evasiveTrafficHostDetails").tablesorter({ 
        headers: { 
			13: { 
                sorter: false 
               } 
		},
		widgets:['zebra']
    });
	
}); 

