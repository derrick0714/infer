// traffic_overview widget

// ctor
vn.portlet.traffic_overview = function() {			
	this._ingress_content_pie = new vn.widget.traffic_content_pie();
	this._egress_content_pie = new vn.widget.traffic_content_pie();
	this._traffic_content_legend = new vn.widget.traffic_content_legend();
	this._bandwidth_utilization_areas = new vn.widget.bandwidth_utilization_area();
		
	return this;
}

vn.portlet.traffic_overview.prototype = vn.extend(vn.widget);

// properties
vn.portlet.traffic_overview.prototype._data_uri = null;

// accessors
vn.portlet.traffic_overview.prototype.ingress_content_pie_data_uri = function(data_uri) {
	if (data_uri == undefined) {
		return this._ingress_content_pie_data_uri;
	}
	this._ingress_content_pie_data_uri = data_uri;
	return this;
}
vn.portlet.traffic_overview.prototype.egress_content_pie_data_uri = function(data_uri) {
	if (data_uri == undefined) {
		return this._egress_content_pie_data_uri;
	}
	this._egress_content_pie_data_uri = data_uri;
	return this;
}
vn.portlet.traffic_overview.prototype.traffic_content_legend_data_uri = function(data_uri) {
	if (data_uri == undefined) {
		return this._traffic_content_legend_data_uri;
	}
	this._traffic_content_legend_data_uri = data_uri;
	return this;
}
vn.portlet.traffic_overview.prototype.bandwidth_utilization_areas_data_uri = function(data_uri) {
	if (data_uri == undefined) {
		return this._bandwidth_utilization_areas_data_uri;
	}
	this._bandwidth_utilization_areas_data_uri = data_uri;
	return this;
}

// member functions
vn.portlet.traffic_overview.prototype.div = function(div) {
	if (div == undefined) {
		return this._div;
	}
	
	if(this._div != null) $('#' + this._div).empty();
	this._div = div;
	
	$('#' + this._div).css('width', this._width);
	$('#' + this._div).css('height', this._height);
	
	var portlet_layout = '<div id="' + this._div + '_ingress_content_pie" style="width: 40%; height: 50%; float: left;"></div>';
	portlet_layout += '<div id="' + this._div + '_egress_content_pie" style="width: 40%; height: 50%; float: left;"></div>';
	portlet_layout += '<div id="' + this._div + '_traffic_content_legend" style="width: 20%; height: 50%; float: left;"></div>';
	portlet_layout += '<div id="' + this._div + '_bandwidth_utilization_areas" style="width: 100%; height: 50%; clear: both;"></div>';
	
	$('#' + this._div).html(portlet_layout);
	return this;
}

vn.portlet.traffic_overview.prototype.width = function(width) {
	if (width == undefined) {
		return this._width;
	}
	
	this._width = width;
	
	$('#' + this._div).css('width', this._width);

	return this;
}

vn.portlet.traffic_overview.prototype.height = function(height) {
	if (height == undefined) {
		return this._height;
	}
	
	this._height = height;
	
	$('#' + this._div).css('height', this._height);

	return this;
}

vn.portlet.traffic_overview.prototype.render = function() {
	this._ingress_content_pie.div(this._div + '_ingress_content_pie')
		.width($('#' + this._div + '_ingress_content_pie').width())
		.height($('#' + this._div + '_ingress_content_pie').height())
		.data_uri(this._ingress_content_pie_data_uri)
		.render();
	
	this._egress_content_pie.div(this._div + '_egress_content_pie')
		.width($('#' + this._div + '_egress_content_pie').width())
		.height($('#' + this._div + '_egress_content_pie').height())
		.data_uri(this._egress_content_pie_data_uri)
		.render();
		
	this._traffic_content_legend.div(this._div + '_traffic_content_legend')
		.width($('#' + this._div + '_traffic_content_legend').width())
		.height($('#' + this._div + '_traffic_content_legend').height())
		.data_uri(this._traffic_content_legend_data_uri)
		.render();
		
	this._bandwidth_utilization_areas.div(this._div + '_bandwidth_utilization_areas')
		.width($('#' + this._div + '_bandwidth_utilization_areas').width())
		.height($('#' + this._div + '_bandwidth_utilization_areas').height())
		.data_uri(this._bandwidth_utilization_areas_data_uri)
		.render();
	
	return this;
}
