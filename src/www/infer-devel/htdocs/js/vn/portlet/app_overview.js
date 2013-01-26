// app_overview widget

// ctor
vn.portlet.app_overview = function() {			
	this._app_inventory_table = new vn.widget.app_inventory_table();

	return this;
}

vn.portlet.app_overview.prototype = vn.extend(vn.widget);

// properties
vn.portlet.app_overview.prototype._data_uri = null;

// accessors
vn.portlet.app_overview.prototype.app_inventory_table_data_uri = function(data_uri) {
	if (data_uri == undefined) {
		return this._app_inventory_table_data_uri;
	}
	this._app_inventory_table_data_uri = data_uri;
	return this;
}

// member functions
vn.portlet.app_overview.prototype.div = function(div) {
	if (div == undefined) {
		return this._div;
	}
	
	if(this._div != null) $('#' + this._div).empty();
	this._div = div;
	
	$('#' + this._div).css('width', this._width);
	$('#' + this._div).css('height', this._height);
	
	var portlet_layout = '<div id="' + this._div + '_app_inventory_table" style="width: 100%; height: 100%;"></div>';
	
	$('#' + this._div).html(portlet_layout);
	return this;
}

vn.portlet.app_overview.prototype.width = function(width) {
	if (width == undefined) {
		return this._width;
	}
	
	this._width = width;
	
	$('#' + this._div).css('width', this._width);

	return this;
}

vn.portlet.app_overview.prototype.height = function(height) {
	if (height == undefined) {
		return this._height;
	}
	
	this._height = height;
	
	$('#' + this._div).css('height', this._height);

	return this;
}

vn.portlet.app_overview.prototype.render = function() {
	this._app_inventory_table.div(this._div + '_app_inventory_table')
		.width($('#' + this._div + '_app_inventory_table').width())
		.height($('#' + this._div + '_app_inventory_table').height())
		.data_uri(this._app_inventory_table_data_uri)
		.render();

	return this;
}
