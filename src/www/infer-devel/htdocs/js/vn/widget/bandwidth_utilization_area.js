// bandwidth_utilization_area widget

// ctor
vn.widget.bandwidth_utilization_area = function() {
	this._data = new Array();
	this._directions = new Array();
	this._xscale = pv.Scale.linear();
	this._yscale = pv.Scale.linear();
	this._ytickscale = pv.Scale.linear();
	this._zscale = pv.Colors.category19();

	var that = this;

	this._root_panel = new pv.Panel()
		.height(function() {return that._height - 60})
		.width(function() {return that._width - 75})
		.bottom(40)
		.left(55)
		.top(20)
		.canvas(function() {return that._div});
	
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
		
	this._root_panel.add(pv.Dot)
		.data(function() {return that._directions})
		.shape('circle')
		.right(80)
		.top(function() {return this.index * 15 + 15})
		.strokeStyle(null)
		.fillStyle(function(d) {return that._zscale(this.index).alpha(.5)})
		.anchor("right").add(pv.Label);
	
	return this;
}
vn.widget.bandwidth_utilization_area.prototype = vn.extend(vn.widget);

// properties
vn.widget.bandwidth_utilization_area.prototype._data_uri = null;

// accessors
vn.widget.bandwidth_utilization_area.prototype.data_uri = function(data_uri) {
	if (data_uri == undefined) {
		return this._data_uri;
	}
	this._data_uri = data_uri;
	
	return this;
}

// member functions
vn.widget.bandwidth_utilization_area.prototype.render = function() {
	var that = this;
	var combined_data = new Array();
	this._data = new Array();
	
	$.each(this._data_uri, function(i, uri)
	{
		$.ajax({url: uri, type: 'GET', dataType: 'json', async: false, success: function(content) {
			$.each(content.bandwidth_utilization.history, function(index, value)
			{
				content.bandwidth_utilization.history[index][0] = new Date(content.bandwidth_utilization.history[index][0] * 1000);
			});
			
			var direction_index = $.inArray(content.bandwidth_utilization.direction, that._directions);
			
			if(direction_index == -1)
			{
				that._directions.push(content.bandwidth_utilization.direction);
				direction_index = $.inArray(content.bandwidth_utilization.direction, that._directions);
				
				that._data[direction_index] = new Array();
				that._data[direction_index] = content.bandwidth_utilization.history;
				
				that._root_panel.add(pv.Area)
					.bottom(1)
					.fillStyle(function() {return that._zscale(direction_index).alpha(.5)})
					.data(function() {return that._data[direction_index]})
					.left(function(d) {return that._xscale(d[0])})
					.height(function(d) {return that._yscale(d[1])});
			}
			else
			{
				if(that._data[direction_index] == null)
					that._data[direction_index] = content.bandwidth_utilization.history;
				else
					that._data[direction_index] = that._data[direction_index].concat(content.bandwidth_utilization.history);
				that._data[direction_index].sort(function(a,b)
				{
					if(a[0] == b[0])
					{
						if(a[1] == b[1]) return 0;
						return a[1] < b[1] ? -1 : 1;
					}
					return a[0] < b[0] ? -1 : 1;
				});
			}
			
			combined_data = combined_data.concat(that._data[direction_index]);
		}});
	});
	
	that._xscale.domain(combined_data, function(d) {return d[0]}).range(0, this._width - 55);
	that._yscale.domain(combined_data, function(d) {return d[1]}).range(0, this._height - 60);
	that._ytickscale.domain(combined_data, function(d) {return d[1] * 8 / (1024 * 1024)}).range(0, this._height - 60);
	that._zscale.domain(0, that._directions.length);
	that._root_panel.render();
	
	return this;
}
