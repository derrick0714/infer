function traffic_content_legend(div_container)
{
	// the properties
	var traffic_content_legend_shape = 'square'; // the shape for each element in the legend
	
	// the data
	var traffic_content_types = new Array('Plaintext', 'BMP image', 'WAV audio', 'Compressed', 'JPEG image', 'MP3 audio', 'MPEG video', 'Encrypted', 'Unknown'); // the data -- static for now
	
	// protovis stuff
	var root_panel = new pv.Panel()
					.canvas(div_container)
					.width(100)
					.height(150)
					.add(pv.Dot)
					.shape(traffic_content_legend_shape)
					.data(traffic_content_types)
					.left(10)
					.top(function() {return this.index * 15 + 15})
					.strokeStyle(null)
					.fillStyle(pv.Colors.category20())
					.anchor("right").add(pv.Label);
	
	root_panel.render();
}