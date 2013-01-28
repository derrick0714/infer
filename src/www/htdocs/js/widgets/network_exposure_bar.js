function network_exposure_bar(div_container, direction, date)
{
	$.ajax({url: 'widgets/network_exposure.php', type: 'GET', dataType: 'json', success: function(content)
	{
		var widget_name = content.widget.window.name;
		var widget_title = content.widget.window.title;
		var refresh_rate = content.widget.window.refresh * 1000;
		var width = content.widget.window.width;
		var height = content.widget.window.height;
		
		//$('#' + div_container).parent().css('width', width).css('height', height);
		var path_to_data = '/data/as_exposure/' + direction + '.php'; // make this dynamic later?

		$.ajax({url: path_to_data + '/' + date, type: 'GET', dataType: 'json', success: function(exposure_content)
		{
			var network_exposure_bar_height = height - 50; // height of this chart
			var network_exposure_bar_width = width / 2; // width of this chart
			
			var network_exposure_bar_data = exposure_content.network_exposure.entities;
			
			var network_exposure_bar_xaxis_scale = pv.Scale.linear(network_exposure_bar_data[direction], function(d) {return d}).range(0, network_exposure_bar_width);
			var network_exposure_bar_yaxis_scale = pv.Scale.ordinal(network_exposure_bar_data.asn).splitBanded(0, network_exposure_bar_height, 4/5);
			
			var root_panel = new pv.Panel()
								.height(network_exposure_bar_height)
								.width(network_exposure_bar_width)
								.left(40)
								.canvas(div_container);
			
			var bars = root_panel.add(pv.Bar)
					.data(network_exposure_bar_data[direction])
					.top(function() {return network_exposure_bar_yaxis_scale(this.index)})
					.height(network_exposure_bar_yaxis_scale.range().band)
					.left(0)
					.width(network_exposure_bar_xaxis_scale)
					.title(function(d) {return d});
			
			bars.anchor("left").add(pv.Label)
				.textMargin(5)
				.textAlign("right")
				.text(function() {return network_exposure_bar_data.asn[this.index]});
			
			root_panel.render();
		}});
	}});
}