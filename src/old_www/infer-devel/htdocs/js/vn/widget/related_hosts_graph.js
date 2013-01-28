// related_hosts_graph widget

// ctor
vn.widget.related_hosts_graph = function() {
	this._colors = pv.Colors.category19();

	var that = this;

	this._radius_scale = pv.Scale.quantile();
	this._edge_width_scale = pv.Scale.quantile();

	this._root_panel = new pv.Panel()
		.height(function() {return that._height - 8})
		.width(function() {return that._width - 8})
		.canvas(function() {return that._div})
		.fillStyle("white")
		.event("mousedown", pv.Behavior.pan())
		.event("mousewheel", pv.Behavior.zoom());

	this._force = this._root_panel.add(pv.Layout.Force)
		.springLength(80);

	this._force.link.add(pv.Line)
		.lineWidth(function(n, d) {return 1 + 19 * that._edge_width_scale(d.linkValue)})
		.strokeStyle(function(n, d) {return pv.color("gray").alpha(Math.max(that._edge_width_scale(d.linkValue),.2));})
		.title(function(n, d) {return "Shared contacts: " + d.linkValue;})
		.event("click",
				function(n, d)
				{
					if (that._on_click_edge != null) {
						that._on_click_edge(n, d);
					}
				}
			);

	this._force.node.add(pv.Dot)
		.shapeRadius(function(d) {return 10 + that._radius_scale(d.score) * 10;})
		.fillStyle(function(d) {return pv.color("green").alpha(Math.max(that._radius_scale(d.score),.2));})
		.strokeStyle(function() {return this.fillStyle().darker()})
		.shape(function(d) {return (d.score==1?"triangle":"circle");})
		.lineWidth(1)
		.title(function(d) {return d.ip})
		.event("mousedown", pv.Behavior.drag())
		.event("drag", that._force);

	return this;
}
vn.widget.related_hosts_graph.prototype = vn.extend(vn.widget);

// properties
vn.widget.related_hosts_graph.prototype._data_uri = null;
vn.widget.related_hosts_graph.prototype._on_click_edge = null;

// accessors
vn.widget.related_hosts_graph.prototype.data_uri = function(data_uri) {
	if (data_uri == undefined) {
		return this._data_uri;
	}
	this._data_uri = data_uri;
	
	return this;
}

vn.widget.related_hosts_graph.prototype.on_click_edge = function(on_click_edge) {
	if (on_click_edge == undefined) {
		return this._on_click_edge;
	}
	this._on_click_edge = on_click_edge;
	
	return this;
}

// member functions
vn.widget.related_hosts_graph.prototype.render = function() {
	var that = this;
	
	$.ajax({url: that._data_uri, type: 'GET', dataType: 'json', async: false, success: function(content) {
		that._force.nodes(content.related_hosts_graph.nodes);
		that._force.links(content.related_hosts_graph.edges);

		that._radius_scale = pv.Scale.quantile(content.related_hosts_graph.nodes, function(d) {return d.score;});
		that._edge_width_scale = pv.Scale.quantile(content.related_hosts_graph.edges, function(d) {return d.value;});

		that._force.reset();
		that._root_panel.render();
	}});

	$("#" + that._div).css("border", "4px solid rgb(204, 204, 204)");

	return this;
}
