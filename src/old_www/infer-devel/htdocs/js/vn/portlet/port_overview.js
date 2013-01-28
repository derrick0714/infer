// port_overview widget

// ctor
vn.portlet.port_overview = function() {			
	this._ingress_port_traffic_bar = new vn.widget.port_traffic_bar();
	this._egress_port_traffic_bar = new vn.widget.port_traffic_bar();
		
	return this;
}

vn.portlet.port_overview.prototype = vn.extend(vn.widget);

// properties
vn.portlet.port_overview.prototype._data_uri = null;

// accessors
vn.portlet.port_overview.prototype.ingress_port_traffic_bar_data_uri = function(data_uri) {
	if (data_uri == undefined) {
		return this._ingress_port_traffic_bar_data_uri;
	}
	this._ingress_port_traffic_bar_data_uri = data_uri;
	return this;
}
vn.portlet.port_overview.prototype.egress_port_traffic_bar_data_uri = function(data_uri) {
	if (data_uri == undefined) {
		return this._egress_port_traffic_bar_data_uri;
	}
	this._egress_port_traffic_bar_data_uri = data_uri;
	return this;
}

// member functions
vn.portlet.port_overview.prototype.div = function(div) {
	if (div == undefined) {
		return this._div;
	}
	
	if(this._div != null) $('#' + this._div).empty();
	this._div = div;
	
	$('#' + this._div).css('width', this._width);
	$('#' + this._div).css('height', this._height);

	var portlet_layout = '<div id="' + this._div + '_port_traffic_bar_label" style="width: 100%; height: 5%;">Port Traffic</div>';
	portlet_layout += '<div id="' + this._div + '_ingress_port_traffic_bar" style="width: 50%; height: 95%; float: left;"></div>';
	portlet_layout += '<div id="' + this._div + '_egress_port_traffic_bar" style="width: 50%; height: 95%; float: left;"></div>';
	
	$('#' + this._div).html(portlet_layout);
	return this;
}

vn.portlet.port_overview.prototype.width = function(width) {
	if (width == undefined) {
		return this._width;
	}
	
	this._width = width;
	
	$('#' + this._div).css('width', this._width);

	return this;
}

vn.portlet.port_overview.prototype.height = function(height) {
	if (height == undefined) {
		return this._height;
	}
	
	this._height = height;
	
	$('#' + this._div).css('height', this._height);

	return this;
}

vn.portlet.port_overview.prototype.render = function() {
	this._ingress_port_traffic_bar.div(this._div + '_ingress_port_traffic_bar')
		.width($('#' + this._div + '_ingress_port_traffic_bar').width())
		.height($('#' + this._div + '_ingress_port_traffic_bar').height())
		.data_uri(this._ingress_port_traffic_bar_data_uri)
		.render();
	
	this._egress_port_traffic_bar.div(this._div + '_egress_port_traffic_bar')
		.width($('#' + this._div + '_egress_port_traffic_bar').width())
		.height($('#' + this._div + '_egress_port_traffic_bar').height())
		.data_uri(this._egress_port_traffic_bar_data_uri)
		.render();

	return this;
}
