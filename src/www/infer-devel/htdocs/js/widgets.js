var label_template = new pv.Label()
					.font("bold 12px sans-serif")
					.left(function() {return this.parent.width() / 2})
					.top(function() {return this.parent.height() / 2})
					.textAlign("center")
					.textBaseline("middle")
					.textStyle(pv.Colors.category10().by(pv.child));

function network_traffic_window_render(charts)
{
	// Renders ingress pie chart
	charts[0][0][4] = pv.Scale.linear(0, pv.sum(charts[0][1][0])).range(0, 2 * Math.PI); // the angle of this chart -- dynamic
	charts[0][0][5] = pv.sum(charts[0][1][0]); // the sum of the data values -- dynamic
	charts[0][2][1].data(charts[0][1][0])
		.title(function(d) {return charts[2][1][0][this.index] + " - " + (((d / charts[0][0][5]) * 100) | 0) + "%"})
		.angle(charts[0][0][4])
		.left(function() {return 75 + Math.cos(this.startAngle() + this.angle() / 2) * ((this.o() == this.index) ? 10 : 0)})
		.bottom(function() {return 75 - Math.sin(this.startAngle() + this.angle() / 2) * ((this.o() == this.index) ? 10 : 0)});
	charts[0][2][0].render();
	
	// Renders egress pie chart
	charts[1][0][4] = pv.Scale.linear(0, pv.sum(charts[1][1][0])).range(0, 2 * Math.PI); // the angle of this chart -- dynamic
	charts[1][0][5] = pv.sum(charts[1][1][0]); // the sum of the data values -- dynamic
	charts[1][2][1].data(charts[1][1][0])
		.title(function(d) {return charts[2][1][0][this.index] + " - " + (((d / charts[1][0][5]) * 100) | 0) + "%"})
		.angle(charts[1][0][4])
		.left(function() {return 75 + Math.cos(this.startAngle() + this.angle() / 2) * ((this.o() == this.index) ? 10 : 0)})
		.bottom(function() {return 75 - Math.sin(this.startAngle() + this.angle() / 2) * ((this.o() == this.index) ? 10 : 0)});
	charts[1][2][0].render();
	
	// Renders legend for the pie charts
	charts[2][2][1].data(charts[2][1][0]);
	charts[2][2][0].render();
	
	// Render Area
	charts[3][0][3] = pv.Scale.linear(0, Math.max.apply(Math, charts[3][1][1].concat(charts[3][1][3]))).range(0, charts[3][0][0]); // y-axis scale -- dynamic
	charts[3][0][4] = pv.Scale.linear(Math.min.apply(Math, charts[3][1][0].concat(charts[3][1][2])), Math.max.apply(Math, charts[3][1][0].concat(charts[3][1][2]))).range(0, charts[3][0][1]); // x-axis scale -- dynamic
	
	charts[3][2][1][0].data(charts[3][0][3].ticks(5))
				.bottom(charts[3][0][3]);
	charts[3][2][1][1].text(charts[3][0][3].tickFormat);
	charts[3][2][2].data(charts[3][1][1])
				.height(function(d) {return charts[3][0][3](d)})
				.left(function(d) {return charts[3][0][4](charts[3][1][0][this.index])});
	charts[3][2][3].data(charts[3][1][3])
				.height(function(d) {return charts[3][0][3](d)})
				.left(function(d) {return charts[3][0][4](charts[3][1][2][this.index])});
	var date_begin = new Date(charts[3][1][0][0] * 1000);
	var date_end = new Date((parseInt(charts[3][1][0][0]) + 86400) * 1000);
	charts[3][2][4].data(new Array(date_begin.getYear() + '-' + date_begin.getMonth() + '-' + date_begin.getDate(), '6:00','12:00','18:00',date_end.getYear() + '-' + date_end.getMonth() + '-' + date_end.getDate()));
	charts[3][2][0].render();
}

function network_traffic_window()
{
	$.ajax({url: 'widgets/network_traffic.php', type: 'GET', dataType: 'json', success: function(content)
	{
		var widget_name = content.widget.window.name;
		var widget_title = content.widget.window.title;
		var refresh_rate = content.widget.window.refresh * 1000;
		var width = content.widget.window.width;
		var height = content.widget.window.height;
		$('#' + widget_name).css('width', width);
		$('#' + widget_name).css('height', height);
		
		var charts = new Array();
		
		// Ingress pie chart
		$.ajax({url: content.widget.data.traffic_content.ingress_uri + '/2010-6-30', type: 'GET', dataType: 'json', async: false, success: function(content)
		{
			charts[0] = new Array(); // this chart
			charts[0][0] = new Array(); // properties of this chart
			charts[0][1] = new Array(); // data of this chart
			charts[0][2] = new Array(); // protovis stuff
			
			// the properties
			charts[0][0][0] = height * .5 + 5; // height of this chart
			charts[0][0][1] = height * .5; // width of this chart
			charts[0][0][2] = 'bw_pie1'; // the canvas
			charts[0][0][3] = charts[0][0][1] / 2.5; // radius of the wedges
			
			// the data
			charts[0][1][0] = content.traffic_content.breakdown.values; // the data -- dynamic
			charts[0][1][1] = content.traffic_content.display_name; // the display name
			
			// the protovis stuff
			charts[0][2][0] = new pv.Panel()
								.height(charts[0][0][0])
								.width(charts[0][0][1])
								.canvas(charts[0][0][2]);
			charts[0][2][0].add(pv.Label)
								.extend(label_template)
								.top(5)
								.textAlign("center")
								.text(charts[0][1][1]);
			charts[0][2][1] = charts[0][2][0].add(pv.Wedge)
										.def("o", -1)
										.outerRadius(charts[0][0][3])
										.event("mouseover", function() {return this.o(this.index)})
										.event("mouseout", function() {return this.o(-1)});
		}});

		// Egress pie chart
		$.ajax({url: content.widget.data.traffic_content.egress_uri + '/2010-6-30', type: 'GET', dataType: 'json', async: false, success: function(content)
		{
			charts[1] = new Array(); // this chart
			charts[1][0] = new Array(); // properties of this chart
			charts[1][1] = new Array(); // data of this chart
			charts[1][2] = new Array(); // protovis stuff
			
			// the properties
			charts[1][0][0] = height * .5 + 5; // height of this chart
			charts[1][0][1] = height * .5; // width of this chart
			charts[1][0][2] = 'bw_pie2'; // the canvas
			charts[1][0][3] = charts[1][0][1] / 2.5; // radius of the wedges
			
			// the data
			charts[1][1][0] = content.traffic_content.breakdown.values; // the data -- dynamic
			charts[1][1][1] = content.traffic_content.display_name; // the display name
			
			// the protovis stuff
			charts[1][2][0] = new pv.Panel()
								.height(charts[1][0][0])
								.width(charts[1][0][1])
								.canvas(charts[1][0][2]);
			charts[1][2][0].add(pv.Label)
								.extend(label_template)
								.top(5)
								.textAlign("center")
								.text(charts[1][1][1]);
			charts[1][2][1] = charts[1][2][0].add(pv.Wedge)
										.def("o", -1)
										.outerRadius(charts[1][0][3])
										.event("mouseover", function() {return this.o(this.index)})
										.event("mouseout", function() {return this.o(-1)});
			
			// The legend for the pie charts
			charts[2] = new Array(); // this chart
			charts[2][0] = new Array(); // properties of this chart
			charts[2][1] = new Array(); // data of this chart
			charts[2][2] = new Array(); // protovis stuff
			
			// the properties
			charts[2][0][0] = 'bw_legend'; // the canvas
			charts[2][0][1] = 'square'; // the shape for each element in the legend
			
			// the data
			charts[2][1][0] = content.traffic_content.breakdown.types; // the data -- static for now
			
			// protovis stuff
			charts[2][2][0] = new pv.Panel()
							.top((10 - charts[2][1][0].length) * 5)
							.canvas(charts[2][0][0]);
			charts[2][2][1] = charts[2][2][0].add(pv.Dot)
							.shape(charts[2][0][1])
							.left(20)
							.top(function() {return this.index * 15 + 15})
							.strokeStyle(null)
							.fillStyle(pv.Colors.category20());
			charts[2][2][1].anchor("right").add(pv.Label);
		}});

		// Area chart
		$.ajax({url: content.widget.data.bandwidth_utilization.ingress_uri + '/2010-6-30', type: 'GET', dataType: 'json', async: false, success: function(content_ingress)
		{
			$.ajax({url: content.widget.data.bandwidth_utilization.egress_uri + '/2010-6-30', type: 'GET', dataType: 'json', async: false, success: function(content_egress)
			{
				charts[3] = new Array(); // this chart
				charts[3][0] = new Array(); // properties of this chart
				charts[3][1] = new Array(); // data of this chart
				charts[3][2] = new Array(); // protovis stuff
				
				// the properties
				charts[3][0][0] = height * .5 - 40; // height of this chart
				charts[3][0][1] = width - 80; // width of this chart
				charts[3][0][2] = 'bw_bar'; // the canvas
				
				// the data
				charts[3][1][0] = content_ingress.bandwidth_utilization.history.times; // timestamps for the ingress data
				charts[3][1][1] = content_ingress.bandwidth_utilization.history.values; // values for the ingress data
				charts[3][1][2] = content_egress.bandwidth_utilization.history.times; // timestamps for the egress data
				charts[3][1][3] = content_egress.bandwidth_utilization.history.values; // values for the egress data
				
				// protovis stuff
				charts[3][2][0] = new pv.Panel()
									.height(charts[3][0][0])
									.width(charts[3][0][1])
									.bottom(30)
									.left(45)
									.right(10)
									.top(10)
									.canvas(charts[3][0][2]);
				charts[3][2][1] = new Array()
				charts[3][2][1][0] = charts[3][2][0].add(pv.Rule) // the y-axis ticks
					.strokeStyle(function(d) {return d ? '#eee' : '#ccc'});
				charts[3][2][0].add(pv.Rule) // the x-axis ticks
					.data(pv.range(0,24))
					.strokeStyle('#eee')
					.bottom(-4)
					.left(function(d) {return (this.index) * charts[3][0][1] / 24});
					//.height(6);
				charts[3][2][1][1] = charts[3][2][1][0].anchor("left").add(pv.Label);
				charts[3][2][0].add(pv.Label) // the y-axis label
					.extend(label_template)
					.textAngle(-Math.PI / 2)
					.left(-30)
					.textAlign("center")
					.text("Mbps");
				charts[3][2][0].add(pv.Label) // the x-axis label
					.extend(label_template)
					//.top(130)
					.top(0)
					.textAlign('center')
					.text('Network throughput for the last 24 hours');
				charts[3][2][2] = charts[3][2][0].add(pv.Area) // the ingress area chart
					.bottom(1)
					.fillStyle(pv.color("rgb(121,173,210)").alpha(.5));
				charts[3][2][3] = charts[3][2][0].add(pv.Area) // the ingress area chart
					.bottom(1)
					.fillStyle(pv.color("rgb(39,96,137)").alpha(.5));
				charts[3][2][4] = charts[3][2][0].add(pv.Label)
					.textAngle(-Math.PI / 3)
					.textAlign("right")
					.textBaseline("middle")
					.top(charts[3][0][0] + 4)
					.left(function(d) {return (this.index) * charts[3][0][1] / 4})
					.text(function(d) {return d});
				charts[3][2][0].add(pv.Dot)
							.data(new Array("Ingress", "Egress"))
							.shape('circle')
							.right(80)
							.top(function() {return this.index * 15 + 15})
							.strokeStyle(null)
							.fillStyle(function(d) {return [pv.color("rgb(121,173,210)").alpha(.5), pv.color("rgb(39,96,137)").alpha(.5)][this.index]})
							.anchor("right").add(pv.Label);
			}});
		}});
		
		network_traffic_window_render(charts);

		setInterval(function(){
			$.ajax({url: content.widget.data.traffic_content.ingress_uri + '/2010-6-30', type: 'GET', dataType: 'json', async: false, success: function(content)
			{
				charts[0][1][0] = content.traffic_content.breakdown.values;
			}});
			$.ajax({url: content.widget.data.traffic_content.egress_uri + '/2010-6-30', type: 'GET', dataType: 'json', async: false, success: function(content)
			{
				charts[1][1][0] = content.traffic_content.breakdown.values;
			}});
			$.ajax({url: content.widget.data.bandwidth_utilization.ingress_uri + '/2010-6-30', type: 'GET', dataType: 'json', async: false, success: function(content_ingress)
			{
				$.ajax({url: content.widget.data.bandwidth_utilization.egress_uri + '/2010-6-30', type: 'GET', dataType: 'json', async: false, success: function(content_egress)
				{
					charts[3][1][0] = content_ingress.bandwidth_utilization.history.times; // timestamps for the ingress data
					charts[3][1][1] = content_ingress.bandwidth_utilization.history.values; // values for the ingress data
					charts[3][1][2] = content_egress.bandwidth_utilization.history.times; // timestamps for the egress data
					charts[3][1][3] = content_egress.bandwidth_utilization.history.values; // values for the egress data
				}});
			}});
			network_traffic_window_render(charts);
		}, refresh_rate);
	}
	});
}

function app_inventory_window_render(content)
{
	$('#ai_table_body').empty();
	$.ajax({url: content.widget.data.inventory_uri + '/2010-6-30', type: 'GET', dataType: 'json', success: function(app_content)
	{
		var total_host_count = 0;
		for(var n in app_content.app_inventory.inventory.host_count) total_host_count += app_content.app_inventory.inventory.host_count[n];
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
				if(maxi == -1) break;
				top_versions[j] = app_content.app_inventory.inventory.version[i][maxi];
				app_content.app_inventory.inventory.version_host_count[i][maxi] = -1;
			}
			$('#ai_table_body').append('<tr class="ai_rows">'
										+ '<td>' + app_content.app_inventory.app_type + '</td>'
										+ '<td>' + browser + '</td>'
										+ '<td>' + (app_content.app_inventory.inventory.host_count[i] / total_host_count * 100 | 0) + '%</td>'
										+ '<td>' + top_versions.join(', ') + '</td>'
										+ '</tr>');
		});
	}});
}

function app_inventory_window()
{
	$.ajax({url: 'widgets/app_inventory.php', type: 'GET', dataType: 'json', success: function(content)
	{
		var widget_name = content.widget.window.name;
		var widget_title = content.widget.window.title;
		var refresh_rate = content.widget.window.refresh * 1000;
		var width = content.widget.window.width;
		var height = content.widget.window.height;
		$('#' + widget_name).css('width', width);
		$('#' + widget_name).css('height', height);
		
		$('#ai_table_head_title').text(widget_title);
		$('#ai_table_cols_type').css('width', width * 0.2);
		$('#ai_table_cols_application').css('width', width * 0.3);
		$('#ai_table_cols_popularity').css('width', width * 0.2);
		$('#ai_table_cols_common_version').css('width', width * 0.3);
		
		app_inventory_window_render(content);
		setInterval(function(){
			host_inventory_window_render(content);
		}, refresh_rate);
	}
	});
}

function host_inventory_window_render(content)
{
	$('#hi_table_body').empty();
	$(content.widget.data.role_uris).each(function(index, value)
	{
		$.ajax({url: value + '/2010-6-30', type: 'GET', dataType: 'json', success: function(role_content)
		{
			$('#hi_table_body').append('<tr class="hi_rows">'
										+ '<td>' + role_content.role.display_name + '</td>'
										+ '<td>' + role_content.role.history[0][1] + '</td>'
										+ '<td><div class="hi_chart" id="hi_chart_' + role_content.role.name + '"></div></td>'
										+ '</tr>');
			var area_data = role_content.role.history;
			var area_w = content.widget.window.width * 0.59;
			var area_h = content.widget.window.height / (1.25 * content.widget.data.role_uris.length);
			var area_x = pv.Scale.linear(area_data, function(d) {return d[0]})
								.range(0, area_w);
			var area_y = pv.Scale.linear(area_data, function(d) {return d[1]})
								.range(0, area_h);
			var area_chart = new pv.Panel()
								.width(area_w)
								.height(area_h)
								.canvas('hi_chart_' + role_content.role.name);
			area_chart.add(pv.Area)
				.data(area_data)
				.bottom(1)
				.left(function(d) {return area_x(d[0])})
				.height(function(d) {return area_y(d[1])})
				.fillStyle("rgb(121,173,210)");
			area_chart.render();
		}});
	});
}

function host_inventory_window()
{
	$.ajax({url: 'widgets/host_inventory.php', type: 'GET', dataType: 'json', success: function(content)
	{
		var widget_name = content.widget.window.name;
		var widget_title = content.widget.window.title;
		var refresh_rate = content.widget.window.refresh * 1000;
		var width = content.widget.window.width;
		var height = content.widget.window.height;
		$('#' + widget_name).css('width', width);
		$('#' + widget_name).css('height', height);
		
		$('#hi_table_cols_role').css('width', width * 0.2);
		$('#hi_table_cols_active_hosts').css('width', width * 0.2);
		
		host_inventory_window_render(content);
		setInterval(function(){
			host_inventory_window_render(content);
		}, refresh_rate);
	}
	});
}

function network_exposure_window_render(charts)
{
	charts[0][0][3] = pv.Scale.log(1, Math.max.apply(Math, charts[0][1][2])).range(0, charts[0][0][1]); // ingress scale
	charts[0][0][4] = pv.Scale.log(1, Math.max.apply(Math, charts[0][1][3])).range(0, charts[0][0][0]); // egress scale
	charts[0][0][5] = pv.Scale.log(1, charts[0][1][0].length).range("orange", "brown"); // distinct contacts scale
	charts[0][0][6] = pv.Scale.log(Math.min.apply(Math, charts[0][1][1]), Math.max.apply(Math, charts[0][1][1])).range(0, charts[0][0][0]);
	
	charts[0][2][1].bottom(charts[0][0][4]);
	charts[0][2][2].data(charts[0][1][0])
				.left(function(d) {var t = charts[0][1][2][this.index]; if(t <= 0) return charts[0][0][3](1); else return charts[0][0][3](t)})
				.bottom(function(d) {var t = charts[0][1][3][this.index]; if(t <= 0) return charts[0][0][4](1); else return charts[0][0][4](t)})
				.strokeStyle(function(d) {return charts[0][0][5](charts[0][1][1][this.index])})
				.fillStyle(function() {return this.strokeStyle().alpha(.2)})
				.size(function(d) {var t = charts[0][1][1][this.index]; if(t <= 0) return charts[0][0][6](1); else return charts[0][0][6](t)});
	//charts[0][2][3][0].data(charts[0][0][4].ticks())
	//			.bottom(charts[0][0][4]);
	//charts[0][2][3][1].text(charts[0][0][4].tickFormat);
	charts[0][2][0].render();
}

function network_exposure_window()
{
	$.ajax({url: 'widgets/network_exposure.php', type: 'GET', dataType: 'json', success: function(content)
	{
		var widget_name = content.widget.window.name;
		var widget_title = content.widget.window.title;
		var refresh_rate = content.widget.window.refresh * 1000;
		var width = content.widget.window.width;
		var height = content.widget.window.height;
		$('#' + widget_name).css('width', width);
		$('#' + widget_name).css('height', height);
		
		var charts = new Array();
		
		// Scatterplot chart
		$.ajax({url: content.widget.data.exposure_uri + '/2010-6-30', type: 'GET', dataType: 'json', async: false, success: function(content)
		{
			charts[0] = new Array(); // this chart
			charts[0][0] = new Array(); // properties of this chart
			charts[0][1] = new Array(); // data of this chart
			charts[0][2] = new Array(); // protovis stuff
			
			// the properties
			charts[0][0][0] = height - 30; // height of this chart
			charts[0][0][1] = width - 30; // width of this chart
			charts[0][0][2] = 'ne_bubble'; // the canvas
			
			// the data
			charts[0][1][0] = content.network_exposure.entities.asn; // the data -- dynamic
			charts[0][1][1] = content.network_exposure.entities.internal_hosts_contacted; // the data -- dynamic
			charts[0][1][2] = content.network_exposure.entities.ingress; // the data -- dynamic
			charts[0][1][3] = content.network_exposure.entities.egress; // the data -- dynamic
			
			// the protovis stuff
			charts[0][2][0] = new pv.Panel()
									.height(charts[0][0][0])
									.width(charts[0][0][1])
									.bottom(30)
									.left(20)
									.right(10)
									.top(5)
									.canvas(charts[0][0][2]);
			charts[0][2][1] = charts[0][2][0].add(pv.Rule) // bottom line
					.textAngle(-Math.PI / 2)
					.strokeStyle(function(d) {return d ? "#eee" : "#000"});
			charts[0][2][0].add(pv.Label) // ingress traffic label
					.extend(label_template)
					.top(height - 20)
					.textAlign("center")
					.text("Ingress traffic in MB");
			charts[0][2][0].add(pv.Rule) // left line
					.left(0)
					.strokeStyle(function(d) {return d ? "#eee" : "#000"});
			charts[0][2][0].add(pv.Label) // egress traffic label
					.extend(label_template)
					.textAngle(-Math.PI / 2)
					.left(-10)
					.textAlign("center")
					.text("Egress traffic in MB");
			charts[0][2][2] = charts[0][2][0].add(pv.Panel).overflow("hidden")
					.add(pv.Dot)
					.title(function(d) {return charts[0][1][0][this.index] + " --> ingress: " + charts[0][1][2][this.index] + ", egress: " + charts[0][1][3][this.index] + ", distinct contact: " + charts[0][1][1][this.index]});
			//charts[0][2][3] = new Array();
			//charts[0][2][3][0] = charts[0][2][0].add(pv.Rule) // the y-axis ticks
			//	.strokeStyle(function(d) {return d ? '#eee' : '#ccc'});
			//charts[0][2][3][1] = charts[0][2][3][0].anchor("left").add(pv.Label);
		}});
		
		network_exposure_window_render(charts);
		setInterval(function(){
			$.ajax({url: content.widget.data.exposure_uri + '/2010-6-30', type: 'GET', dataType: 'json', async: false, success: function(content)
			{
				charts[0][1][0] = content.network_exposure.entities.asn; // the data -- dynamic
				charts[0][1][1] = content.network_exposure.entities.internal_hosts_contacted; // the data -- dynamic
				charts[0][1][2] = content.network_exposure.entities.ingress; // the data -- dynamic
				charts[0][1][3] = content.network_exposure.entities.egress; // the data -- dynamic
				network_exposure_window_render(charts);
			}});
		}, refresh_rate);
	}});
}
