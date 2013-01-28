// app_inventory_table widget

// ctor
vn.widget.app_inventory_table = function() {
	return this;
}
vn.widget.app_inventory_table.prototype = vn.extend(vn.widget);

// properties
vn.widget.app_inventory_table.prototype._data_uri = null;

// accessors
vn.widget.app_inventory_table.prototype.data_uri = function(data_uri) {
	if (data_uri == undefined) {
		return this._data_uri;
	}
	this._data_uri = data_uri;
	
	return this;
}

// member functions
vn.widget.app_inventory_table.prototype.render = function() {
		var table_schema = '<table>';
		table_schema += '<thead>';
		table_schema += '<tr>';
		table_schema += '<th style="width: 15%">Type</th>';
		table_schema += '<th style="width: 20%">Application</th>';
		table_schema += '<th style="width: 15%">Popularity</th>';
		table_schema += '<th style="width: 50%">Common Version</th>';
		table_schema += '</tr>';
		table_schema += '</thead>';
		table_schema += '<tbody id="' + this._div + '_body"></tbody>';
		table_schema += '</table>';
		
	$('#' + this._div).html(table_schema);
	
	var that = this;
	
	$(this._data_uri).each(function(h, uri)
	{
		$.ajax({url: uri, type: 'GET', dataType: 'json', async: false, success: function(content)
		{
			var total_host_count = 0;
			$.each(content.app_inventory.inventory.host_count, function(i, value)
			{
				total_host_count += value;
			});
			$(content.app_inventory.inventory.name).each(function(i, browser)
			{
				var top_versions = new Array();
				for(var j = 0; j < 3; j++)
				{
					var max = -1;
					var maxi = -1;
					
					for(var m in content.app_inventory.inventory.version_host_count[i])
						if(content.app_inventory.inventory.version_host_count[i][m] > max)
						{
							max = content.app_inventory.inventory.version_host_count[i][m];
							maxi = m;
						}
					
					if(maxi == -1) 
						break;
					
					top_versions[j] = content.app_inventory.inventory.version[i][maxi];
					content.app_inventory.inventory.version_host_count[i][maxi] = -1;
				}
				
				var temp = (content.app_inventory.inventory.host_count[i] / total_host_count * 100 | 0);
				if(temp < 1) temp = "<1";
				
				var table_row = '<tr>';
				table_row += '<td>' + content.app_inventory.app_type + '</td>';
				table_row += '<td>' + browser + '</td>';
				table_row += '<td>' + temp + '%</td>';
				table_row += '<td>' + top_versions.join(', ') + '</td>';
				table_row += '</tr>';
				
				$('#' + that._div + '_body').append(table_row);
			});
		}});
	});
	return this;
}
