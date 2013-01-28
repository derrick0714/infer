// as_exposure_bar widget

// ctor
vn.widget.as_exposure_bar = function() {
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
		.data(function() {return that._data.entities[that._data.direction]})
		.top(function() {return that._yscale(this.index)})
		.height(function(){return that._yscale.range().band})
		.left(0)
		.width(that._xscale)
		.title(function(d) {return d})
		.anchor("left").add(pv.Label)
		.textMargin(5)
		.textAlign("right")
		.text(function() {return that._data.entities.asn_name[this.index].substring(0, 10) != that._data.entities.asn_name[this.index] ? that._data.entities.asn_name[this.index].substring(0, 10) + "..." : that._data.entities.asn_name[this.index]})
		.events("all")
		.title(function() {return that._data.entities.asn_name[this.index] + ": " + that._data.entities.asn_desc[this.index]});
	
	return this;
}
vn.widget.as_exposure_bar.prototype = vn.extend(vn.widget);

// properties
vn.widget.as_exposure_bar.prototype._data_uri = null;

// accessors
vn.widget.as_exposure_bar.prototype.data_uri = function(data_uri) {
	if (data_uri == undefined) {
		return this._data_uri;
	}
	this._data_uri = data_uri;
	return this;
}

// member functions
vn.widget.as_exposure_bar.prototype.render = function() {
	var that = this;
	$.getJSON(this.data_uri(), function(content) {
		that._data = content.network_exposure;
		that._xscale.domain(that._data.entities[that._data.direction]).range(5, that._width - 85);
		that._yscale.domain(that._data.entities.asn).splitBanded(0, that._height - 10, 4/5);
		that._root_panel.render();
	});

	return this;
}
