function app_inventory_table(div_container, date)
{
	$.ajax({url: 'widgets/app_inventory.php', type: 'GET', dataType: 'json', success: function(content)
	{
		var widget_name = content.widget.window.name;
		var widget_title = content.widget.window.title;
		var refresh_rate = content.widget.window.refresh * 1000;
		var width = content.widget.window.width;
		var height = content.widget.window.height;

		$('#' + div_container).css('width', width).css('height', height);
		
		var table_schema = '<table>';
		table_schema += '<thead>';
		table_schema += '<tr>';
		table_schema += '<th>Type</th>';
		table_schema += '<th>Application</th>';
		table_schema += '<th>Popularity</th>';
		table_schema += '<th>Common Version</th>';
		table_schema += '</tr>';
		table_schema += '</thead>';
		table_schema += '<tbody id="' + div_container + '_body"></tbody>';
		table_schema += '</table>';
		
		$('#' + div_container).html(table_schema);
		
		$.ajax({url: content.widget.data.inventory_uri + '/' + date, type: 'GET', dataType: 'json', success: function(app_content)
		{
			var total_host_count = 0;
			for(var n in app_content.app_inventory.inventory.host_count) 
				total_host_count += app_content.app_inventory.inventory.host_count[n];
			
			$(app_content.app_inventory.inventory.name).each(function(i, browser)
			{
				var top_versions = new Array();
				for(var j = 0; j < 3; j++)
				{
					var max = -1;
					var maxi = -1;
					
					for(var m in app_content.app_inventory.inventory.version_host_count[i])
						if(app_content.app_inventory.inventory.version_host_count[i][m] > max)
						{
							max = app_content.app_inventory.inventory.version_host_count[i][m];
							maxi = m;
						}
					
					if(maxi == -1) 
						break;
					
					top_versions[j] = app_content.app_inventory.inventory.version[i][maxi];
					app_content.app_inventory.inventory.version_host_count[i][maxi] = -1;
				}

				var table_row = '<tr>';
				table_row += '<td>' + app_content.app_inventory.app_type + '</td>';
				table_row += '<td>' + browser + '</td>';
				table_row += '<td>' + (app_content.app_inventory.inventory.host_count[i] / total_host_count * 100 | 0) + '%</td>';
				table_row += '<td>' + top_versions.join(', ') + '</td>';
				table_row += '</tr>';
				
				$('#' + div_container + '_body').append(table_row);
			});
		}});
	}
	});
}