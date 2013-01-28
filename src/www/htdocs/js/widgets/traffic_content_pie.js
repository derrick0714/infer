function traffic_content_pie(div_container, direction, start_time, end_time)
{
	$.ajax({url: 'widgets/network_traffic.php', type: 'GET', dataType: 'json', success: function(content)
	{
		var widget_name = content.widget.window.name;
		var widget_title = content.widget.window.title;
		var refresh_rate = content.widget.window.refresh * 1000;
		var width = content.widget.window.width;
		var height = content.widget.window.height;
		
		$('#' + div_container).parent().css('width', width).css('height', height);
		
		var path_to_data = direction == 'ingress' ? content.widget.data.traffic_content.ingress_uri : content.widget.data.traffic_content.egress_uri;
		$.ajax({url: path_to_data + '/' + start_time + '/' + end_time, type: 'GET', dataType: 'json', success: function(content)
		{
			// the properties
			var traffic_content_pie_height = height * .5 + 5; // height of this chart
			var traffic_content_pie_width = height * .5; // width of this chart
			var traffic_content_pie_radius = traffic_content_pie_width / 2.5; // radius of the wedges
			
			// the data
			var traffic_content_pie_data = content.traffic_content.breakdown; // the data -- dynamic
			var traffic_content_pie_display_name = content.traffic_content.display_name; // the display name
			
			// the protovis stuff
			var root_panel = new pv.Panel()
								.height(traffic_content_pie_height)
								.width(traffic_content_pie_width + 25)
								.canvas(div_container);
			root_panel.add(pv.Label)
								.extend(label_template)
								.top(5)
								.textAlign("center")
								.text(traffic_content_pie_display_name);
			
			var values = pv.range(traffic_content_pie_data.length).map(function(i) {return traffic_content_pie_data[i][1];});
			
			var wedge_panel = root_panel.add(pv.Wedge)
										.def("o", -1)
										.outerRadius(traffic_content_pie_radius)
										.event("mouseover", function() {return this.o(this.index)})
										.event("mouseout", function() {return this.o(-1)})
										.data(values)
										.title(function(d) {return traffic_content_pie_data[this.index][0] + " - " + (((d / pv.sum(values)) * 100) | 0) + "%"})
										.angle(pv.Scale.linear(0, pv.sum(values)).range(0, 2 * Math.PI))
										.left(function() {return 75 + Math.cos(this.startAngle() + this.angle() / 2) * ((this.o() == this.index) ? 10 : 0)})
										.bottom(function() {return 75 - Math.sin(this.startAngle() + this.angle() / 2) * ((this.o() == this.index) ? 10 : 0)});
			root_panel.render();
		}});
	}});
}