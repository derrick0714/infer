// bandwidth_content_stack widget

// ctor
vn.widget.bandwidth_content_stack = function() {
	this._stacks = new Array();
	for(var j = 0; j < 9; j++) this._stacks[j] = new Array();
	
	this._xvalues = new Array();
	this._xscale = pv.Scale.linear();
	this._yscale = pv.Scale.linear();
	this._ytickscale = pv.Scale.linear();

	var that = this;

	this._root_panel = new pv.Panel()
		.height(function() {return that._height - 60})
		.width(function() {return that._width - 75})
		.bottom(40)
		.left(55)
		.top(20)
		.canvas(function() {return that._div});
		
	this._root_panel.add(pv.Layout.Stack)
		.layers(function() {return that._stacks})
		.x(function(d) {return that._xscale(that._xvalues[this.index])})
		.y(function(d) {return that._yscale(d)})
		.layer.add(pv.Area);
	
	this._root_panel.add(pv.Rule) // the x-axis ticks
		.data(function() {return that._xscale.ticks()})
		.strokeStyle(pv.color("#eee"))
		.left(this._xscale)
		.anchor("bottom")
		.add(pv.Label)
		.textBaseline("middle")
		.textAlign("right")
		.text(this._xscale.tickFormat)
		.top(function() {return that._height - 60})
		.textAngle(-Math.PI / 4);
	
	var label_template = new pv.Label()
		.font("bold 12px sans-serif")
		.left(function() {return this.parent.width() / 2})
		.top(function() {return this.parent.height() / 2})
		.textAlign("center")
		.textBaseline("middle")
		.textStyle(pv.Colors.category10().by(pv.child));
	
	this._root_panel.add(pv.Label) // the y-axis label
		.extend(label_template)
		.textAngle(-Math.PI / 2)
		.left(-38)
		.textAlign("center")
		.text("Mbps");
	
	this._root_panel.add(pv.Rule) // the y-axis ticks
		.data(function() {return that._ytickscale.ticks()})
		.strokeStyle(pv.color("#eee"))
		.bottom(this._ytickscale)
		.anchor("left").add(pv.Label)
		.text(this._ytickscale.tickFormat)
		.left(0);
	
	return this;
}
vn.widget.bandwidth_content_stack.prototype = vn.extend(vn.widget);

// properties
vn.widget.bandwidth_content_stack.prototype._data_uri = null;

// accessors
vn.widget.bandwidth_content_stack.prototype.data_uri = function(data_uri) {
	if (data_uri == undefined) {
		return this._data_uri;
	}
	this._data_uri = data_uri;
	
	return this;
}

// member functions
vn.widget.bandwidth_content_stack.prototype.render = function() {
	var that = this;
	var combined_data = new Array();

	$.ajax({url: this._data_uri, type: 'GET', dataType: 'json', async: false, success: function(content) {
		for(var i = 0; i < content.bandwidth_content.history.length; i++) combined_data[i] = 0;
		
		$.each(content.bandwidth_content.history, function(i, value) {
			content.bandwidth_content.history[i][0] = new Date(value[0] * 1000);
			that._xvalues[i] = content.bandwidth_content.history[i][0];
			
			for(var j = 1; j <= 9; j++)
			{
				that._stacks[j - 1][i] = content.bandwidth_content.history[i][j];
				combined_data[i] += content.bandwidth_content.history[i][j];
			}
		});
	}});
	
	that._xscale.domain(that._xvalues).range(0, this._width - 55);
	that._yscale.domain(0, Math.max.apply(Math, combined_data)).range(0, this._height - 60);
	that._ytickscale.domain(combined_data, function(d) {return d * 8 / (1024 * 1024)}).range(0, this._height - 60);
	that._root_panel.render();
	
	return this;
}
