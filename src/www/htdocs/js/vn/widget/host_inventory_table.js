// host_inventory_table widget

// ctor
vn.widget.host_inventory_table = function() {
	return this;
}
vn.widget.host_inventory_table.prototype = vn.extend(vn.widget);

// properties
vn.widget.host_inventory_table.prototype._data_uri = null;

// accessors
vn.widget.host_inventory_table.prototype.data_uri = function(data_uri) {
	if (data_uri == undefined) {
		return this._data_uri;
	}
	this._data_uri = data_uri;
	
	return this;
}

// member functions
vn.widget.host_inventory_table.prototype.render = function() {
	var table_schema = '<table>';
		table_schema += '<thead>';
		table_schema += '<tr>';
		table_schema += '<th style="width: 20%;">Role</th>';
		table_schema += '<th style="width: 20%;">Active Hosts</th>';
		table_schema += '<th style="width: 60%;">Activity History</th>';
		table_schema += '</tr>';
		table_schema += '</thead>';
		table_schema += '<tbody id="' + this._div + '_body"></tbody>';
		table_schema += '</table>';
		
	$('#' + this._div).html(table_schema);
	
	var that = this;
	
	$(this._data_uri).each(function(i, uri)
	{
		$.ajax({url: uri, type: 'GET', dataType: 'json', async: false, success: function(content)
		{
			if(content.role.history != null && content.role.history[0][1] > 0)
			{
				var table_row = '<tr>';
				table_row += '<td>' + content.role.display_name + '</td>';
				table_row += '<td>' + content.role.history[0][1] + '</td>';
				table_row += '<td><div id="' + that._div + '_body_row_' + content.role.name + '"></div></td>';
				table_row += '</tr>';
				
				$('#' + that._div + '_body').append(table_row);
				
				var role_history = content.role.history;
				var role_chart_width = Math.floor(that._width * 0.59);
				var role_chart_height = Math.floor(that._height / (1.3 * 8));
				//var role_chart_height = Math.floor(that._height / (1.3 * that._data_uri.length));
				
				var role_chart_xaxis_scale = pv.Scale.linear(role_history, function(d) {return d[0]})
													.range(0, role_chart_width);
				var role_chart_yaxis_scale = pv.Scale.linear(role_history, function(d) {return d[1]})
													.range(0, role_chart_height);
				var role_chart = new pv.Panel()
									.width(role_chart_width)
									.height(role_chart_height)
									.canvas(that._div + '_body_row_' + content.role.name);
				role_chart.add(pv.Rule)
					.data(role_history)
					.strokeStyle(pv.color("#eee"))
					.left(function(d) {return role_chart_xaxis_scale(d[0])});
				role_chart.add(pv.Area)
					.data(role_history)
					.bottom(1)
					.left(function(d) {return role_chart_xaxis_scale(d[0])})
					.height(function(d) {return role_chart_yaxis_scale(d[1])})
					.fillStyle("rgb(121,173,210)");
				role_chart.render();
			}
		}});
	});
	return this;
}
