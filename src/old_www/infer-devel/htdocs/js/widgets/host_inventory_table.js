function host_inventory_table(div_container, date)
{
	$.ajax({url: 'widgets/host_inventory.php', type: 'GET', dataType: 'json', success: function(content)
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
		table_schema += '<th>Role</th>';
		table_schema += '<th>Active Hosts</th>';
		table_schema += '<th>Activity History</th>';
		table_schema += '</tr>';
		table_schema += '</thead>';
		table_schema += '<tbody id="' + div_container + '_body"></tbody>';
		table_schema += '</table>';
		
		$('#' + div_container).html(table_schema);
		
		$(content.widget.data.role_uris).each(function(index, value)
		{
			$.ajax({url: value + '/' + date, type: 'GET', dataType: 'json', async: false, success: function(role_content)
			{
				var table_row = '<tr>';
				table_row += '<td>' + role_content.role.display_name + '</td>';
				table_row += '<td>' + role_content.role.history[0][1] + '</td>';
				table_row += '<td><div id="' + div_container + '_body_row_' + role_content.role.name + '"></div></td>';
				table_row += '</tr>';
				
				$('#' + div_container + '_body').append(table_row);
				
				var role_history = role_content.role.history;
				var role_chart_width = width * 0.59;
				var role_chart_height = height / (1.25 * content.widget.data.role_uris.length);
				
				var role_chart_xaxis_scale = pv.Scale.linear(role_history, function(d) {return d[0]})
													.range(0, role_chart_width);
				var role_chart_yaxis_scale = pv.Scale.linear(role_history, function(d) {return d[1]})
													.range(0, role_chart_height);
				var role_chart = new pv.Panel()
									.width(role_chart_width)
									.height(role_chart_height)
									.canvas(div_container + '_body_row_' + role_content.role.name);
				role_chart.add(pv.Area)
					.data(role_history)
					.bottom(1)
					.left(function(d) {return role_chart_xaxis_scale(d[0])})
					.height(function(d) {return role_chart_yaxis_scale(d[1])})
					.fillStyle("rgb(121,173,210)");
				
				role_chart.render();
			}});
		});
	}
	});
}