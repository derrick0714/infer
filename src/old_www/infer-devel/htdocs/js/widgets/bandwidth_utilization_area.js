function bandwidth_utilization_area(div_container, start_time, end_time)
{
	$.ajax({url: 'widgets/network_traffic.php', type: 'GET', dataType: 'json', success: function(content)
	{
		var widget_name = content.widget.window.name;
		var widget_title = content.widget.window.title;
		var refresh_rate = content.widget.window.refresh * 1000;
		var width = content.widget.window.width;
		var height = content.widget.window.height;
		
		$.ajax({url: content.widget.data.bandwidth_utilization.ingress_uri + '/' + start_time + '/' + end_time, type: 'GET', dataType: 'json', async: false, success: function(content_ingress)
		{
			$.ajax({url: content.widget.data.bandwidth_utilization.egress_uri + '/' + start_time + '/' + end_time, type: 'GET', dataType: 'json', async: false, success: function(content_egress)
			{
				// the properties
				var bandwidth_utilization_area_height = height * .5 - 60; // height of this chart
				var bandwidth_utilization_area_width = width - 80; // width of this chart
				
				// the data
				var bandwidth_utilization_area_ingress_data = content_ingress.bandwidth_utilization.history;
				var bandwidth_utilization_area_egress_data = content_egress.bandwidth_utilization.history;
				
				// converting to date format
				bandwidth_utilization_area_ingress_data.forEach(function(d) {
					d[0] = new Date(d[0] * 1000);
				});
				bandwidth_utilization_area_egress_data.forEach(function(d) {
					d[0] = new Date(d[0] * 1000);
				});
				
				// combined data to use in scale
				var combined_data = bandwidth_utilization_area_ingress_data.concat(bandwidth_utilization_area_egress_data);
				combined_data.sort();
				
				// protovis stuff
				var root_panel = new pv.Panel()
									.height(bandwidth_utilization_area_height)
									.width(bandwidth_utilization_area_width)
									.bottom(40)
									.left(45)
									.right(10)
									.top(10)
									.canvas(div_container);
									
				var bandwidth_utilization_area_xaxis_scale = pv.Scale.linear(combined_data, function(d) {return d[0];}).range(0, bandwidth_utilization_area_width).nice(); // x-axis scale -- dynamic
				
				var bandwidth_utilization_area_yaxis_scale = pv.Scale.linear(combined_data, function(d) {return d[1];}).range(0, bandwidth_utilization_area_height).nice(); // y-axis scale -- dynamic
				
				root_panel.add(pv.Area) // the ingress area chart
					.bottom(1)
					.fillStyle(pv.color("rgb(121,173,210)").alpha(.5))
					.data(bandwidth_utilization_area_ingress_data)
					.left(function(d) {return bandwidth_utilization_area_xaxis_scale(d[0])})
					.height(function(d) {return bandwidth_utilization_area_yaxis_scale(d[1])})
					.interpolate("cardinal");
					
				root_panel.add(pv.Area) // the egress area chart
					.bottom(1)
					.fillStyle(pv.color("rgb(39,96,137)").alpha(.5))
					.data(bandwidth_utilization_area_egress_data)
					.left(function(d) {return bandwidth_utilization_area_xaxis_scale(d[0])})
					.height(function(d) {return bandwidth_utilization_area_yaxis_scale(d[1])})
					.interpolate("cardinal");
					
				var bandwidth_utilization_area_yticks_scale = pv.Scale.linear(combined_data, function(d) {return d[1] * 8 / (1024*1024);}).range(0, bandwidth_utilization_area_height);
				root_panel.add(pv.Rule) // the y-axis ticks
					.strokeStyle(pv.color("#eee").alpha(.2))
					.data(bandwidth_utilization_area_yticks_scale.ticks())
					.bottom(bandwidth_utilization_area_yticks_scale)
					.anchor("left").add(pv.Label);
					
				root_panel.add(pv.Rule) // the x-axis ticks
					.strokeStyle(pv.color("#eee").alpha(.2))
					.data(bandwidth_utilization_area_xaxis_scale.ticks())
					.left(bandwidth_utilization_area_xaxis_scale)
					.anchor("bottom")
					.add(pv.Label)
					.top(bandwidth_utilization_area_height + 15)
					.text(bandwidth_utilization_area_xaxis_scale.tickFormat)
					.textAngle(-Math.PI / 4);
					
				root_panel.add(pv.Label) // the y-axis label
					.extend(label_template)
					.textAngle(-Math.PI / 2)
					.left(-30)
					.textAlign("center")
					.text("Mbps");
					
				/*root_panel.add(pv.Label) // the x-axis label
					.extend(label_template)
					.top(bandwidth_utilization_area_height + 30)
					.textAlign('center')
					.text('Network throughput for the last 24 hours');*/
				
				/*root_panel.add(pv.Dot)
							.data(new Array("Ingress", "Egress"))
							.shape('circle')
							.right(80)
							.top(function() {return this.index * 15 + 15})
							.strokeStyle(null)
							.fillStyle(function(d) {return [pv.color("rgb(121,173,210)").alpha(.5), pv.color("rgb(39,96,137)").alpha(.5)][this.index]})
							.anchor("right").add(pv.Label);*/
				root_panel.render();
			}});
		}});
	}});
}