// port_traffic_bar widget

// ctor
vn.widget.port_traffic_bar = function() {
	this._data = new Array();
	this._xscale = pv.Scale.linear();
	this._yscale = pv.Scale.ordinal();

	var that = this;
	
	this._root_panel = new pv.Panel()
		.height(function() {return that._height - 10})
		.width(function() {return that._width - 85})
		.canvas(function() {return that._div})
		.left(85)
		.top(10);
	
	var label_template = new pv.Label()
		.font("bold 12px sans-serif")
		.left(function() {return this.parent.width() / 2})
		.top(function() {return this.parent.height() / 2})
		.textAlign("center")
		.textBaseline("middle")
		.textStyle(pv.Colors.category10().by(pv.child));

	this._root_panel.add(pv.Label)
		.extend(label_template)
		.textAlign("center")
		.text(function() {return that._data.direction})
		.top(-5);
	
	this._root_panel.add(pv.Bar)
		.data(function() {
			var values = new Array();
			$.each(that._data.breakdown, function(i, v) {
				values[i] = v[2];
			});
			return values;
		})
		.top(function() {return that._yscale(this.index)})
		.height(function(){return that._yscale.range().band})
		.left(0)
		.width(that._xscale)
		.title(function(d) {return d})
		.anchor("left").add(pv.Label)
		.textMargin(5)
		.textAlign("right")
		.text(function() {return that._data.breakdown[this.index][1]});
	
	return this;
}
vn.widget.port_traffic_bar.prototype = vn.extend(vn.widget);

// properties
vn.widget.port_traffic_bar.prototype._data_uri = null;

// accessors
vn.widget.port_traffic_bar.prototype.data_uri = function(data_uri) {
	if (data_uri == undefined) {
		return this._data_uri;
	}
	this._data_uri = data_uri;
	return this;
}

// member functions
vn.widget.port_traffic_bar.prototype.render = function() {
	var that = this;
	$.getJSON(this.data_uri(), function(content) {
		that._data = content.port_traffic;
		that._xscale.domain(that._data.breakdown, function(d) {return d[2]}).range(5, that._width - 85);
		that._yscale.domain(that._data.breakdown, function(d) {return d[1]}).splitBanded(0, that._height - 10, 4/5);
		that._root_panel.render();
	});

	return this;
}
