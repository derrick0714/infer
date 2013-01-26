// traffic_content_legend widget

// ctor
vn.widget.traffic_content_legend = function() {
	this._data = new Array();

	var that = this;

	this._root_panel = new pv.Panel()
		.height(function() {return that._height})
		.width(function() {return that._width})
		.canvas(function() {return that._div})
		.add(pv.Dot)
		.shape("square")
		.data(function() {return that._data})
		.left(10)
		.top(function() {return this.index * 15 + 15})
		.strokeStyle(null)
		.fillStyle(pv.Colors.category20())
		.anchor("right").add(pv.Label);
	
	return this;
}
vn.widget.traffic_content_legend.prototype = vn.extend(vn.widget);

// properties
vn.widget.traffic_content_legend.prototype._data_uri = null;

// accessors
vn.widget.traffic_content_legend.prototype.data_uri = function(data_uri) {
	if (data_uri == undefined) {
		return this._data_uri;
	}
	this._data_uri = data_uri;
	return this;
}

// member functions
vn.widget.traffic_content_legend.prototype.render = function() {
	var that = this;
	$.getJSON(this.data_uri(), function(content) {
		that._data = content.traffic_content_types;
		that._root_panel.render();
	});

	return this;
}
