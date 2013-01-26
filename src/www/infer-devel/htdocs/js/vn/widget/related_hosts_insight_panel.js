// related_hosts_insight_panel widget

// ctor
vn.widget.related_hosts_insight_panel = function() {
	return this;
}
vn.widget.related_hosts_insight_panel.prototype = vn.extend(vn.widget);

// properties
vn.widget.related_hosts_insight_panel.prototype._data_uri = null;
vn.widget.related_hosts_insight_panel.prototype._insight_panel = null;

// accessors
vn.widget.related_hosts_insight_panel.prototype.data_uri = function(data_uri) {
	if (data_uri == undefined) {
		return this._data_uri;
	}
	this._data_uri = data_uri;
	
	return this;
}

vn.widget.related_hosts_insight_panel.prototype.insight_panel = function(insight_panel) {
	if (insight_panel == undefined) {
		return this._insight_panel;
	}
	this._insight_panel = insight_panel;
	
	return this;
}

// member functions
vn.widget.related_hosts_insight_panel.prototype.clear = function() {
	var that = this;

	$('#' + that._div).html('');
	$("#" + that._div).css("border", "4px solid rgb(204, 204, 204)");
	$("#" + that._div).css("width", (that._width-8) + "px");
	$("#" + that._div).css("height", (that._height-8) + "px");
}

vn.widget.related_hosts_insight_panel.prototype.render = function() {
	var that = this;
	
	$.ajax({url: that._data_uri, type: 'GET', dataType: 'json', async: false, success: function(content) {
		var network_id_table =
			'<table class="comp-table">' +
				'<caption>' + content.table.caption + '</caption>'
				'<thead id="' + that._div + '_head">' +
					'<tr>';
		for (var i in content.table.header) {
			network_id_table +=
						'<th>' + content.table.header[i][1] + '</th>';
		}
		network_id_table +=
				'</thead>' +
				'<tbody id="' + that._div + '_body">';
		for (var i in content.table.rows) {
			network_id_table +=
					'<tr>';
			for (var j in content.table.rows[i]) {
				network_id_table +=
						'<td>' + content.table.rows[i][j] + '</td>';
			}
			network_id_table +=
					'</tr>';
		}
		network_id_table +=
				'</tbody>' +
			'</table>';
				
		/*
		$.each(content.related_hosts, function(i, val) {
			network_id_table += '<tr><td><a href="/host/#/identity/' + val.related_ip + '/' + 'current_date' + '">' + val.related_ip + '</a></td>';
			network_id_table += '<td>' + val.score +
				'</td></tr>';
		});
		network_id_table += '</tbody></table>';
		*/
		$('#' + that._div).html(network_id_table);
	}});

	$("#" + that._div).css("border", "4px solid rgb(204, 204, 204)");
	$("#" + that._div).css("width", (that._width-8) + "px");
	$("#" + that._div).css("height", (that._height-8) + "px");

	return this;
}
