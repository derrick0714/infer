// traffic_content_pie widget

// ctor
vn.widget.traffic_content_pie = function() {
	this._data = new Array();
	this._scale = pv.Scale.linear().range(0, 2 * Math.PI);
	
	var that = this;
	
	var label_template = new pv.Label()
		.font("bold 12px sans-serif")
		.left(function() {return this.parent.width() / 2})
		.top(function() {return this.parent.height() / 2})
		.textAlign("center")
		.textBaseline("middle")
		.textStyle(pv.Colors.category10().by(pv.child));
	
	this._root_panel = new pv.Panel()
		.height(function() {return that._height})
		.width(function() {return that._width})
		.canvas(function() {return that._div});
		
	this._root_panel.add(pv.Label)
		.extend(label_template)
		.textAlign("center")
		.top(10)
		.text(function() {return that._label});
	
	this._root_panel.add(pv.Wedge)
		.data(function() {return that._data})
		.def("o", -1)
		.outerRadius(function() {return (that._height < that._width ? that._height : that._width) / 2.5})
		.angle(function(d) {return that._scale(d[1])})
		.event("mouseover", function() {return this.o(this.index)})
		.event("mouseout", function() {return this.o(-1)})
		.left(function() {return that._width/2 + Math.cos(this.startAngle() + this.angle() / 2) * ((this.o() == this.index) ? 10 : 0)})
		.bottom(function() {return that._height/2.2 - Math.sin(this.startAngle() + this.angle() / 2) * ((this.o() == this.index) ? 10 : 0)})
		.title(function(d) {
			var temp = (((that._data[this.index][1] / pv.sum(that._data, function(d) {return d[1]})) * 100) | 0);
			if(temp == 0) temp = "<1";
			return that._data[this.index][0] + " " + temp + "%";
		
		});
	
	return this;
}
vn.widget.traffic_content_pie.prototype = vn.extend(vn.widget);

// properties
vn.widget.traffic_content_pie.prototype._data_uri = null;

// accessors
vn.widget.traffic_content_pie.prototype.data_uri = function(data_uri) {
	if (data_uri == undefined) {
		return this._data_uri;
	}
	this._data_uri = data_uri;
	return this;
}

// member functions
vn.widget.traffic_content_pie.prototype.render = function() {
	var that = this;
	$.getJSON(this.data_uri(), function(content) {
		that._data = content.traffic_content.breakdown;
		that._scale.domain(0, pv.sum(that._data, function(d) {return d[1]}));
		that._label = content.traffic_content.direction;

		that._root_panel.render();
	});

	return this;
}
