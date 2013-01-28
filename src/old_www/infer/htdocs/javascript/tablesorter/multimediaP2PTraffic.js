$(document).ready(function() { 
    $("#multimediaP2PTrafficHostDetails").tablesorter({ 
        headers: { 
           
            4: { 
                sorter: 'time' 
            } ,
			 7: { 
                sorter: 'filesize' 
            },
			 8: { 
                sorter: 'filesize' 
            },
			9: { 
                sorter: 'filesize' 
            }			
		},
		widgets:['zebra']
    }); 
});   
