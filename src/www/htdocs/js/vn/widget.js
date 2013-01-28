// widget base class
vn.widget = function() {};

// properties
vn.widget.prototype._div = null;
vn.widget.prototype._width = null;
vn.widget.prototype._height = null;

// accessor functions
vn.widget.prototype.div = function(div) {
	if (div == undefined) {
		return this._div;
	}
	
	this._div = div;
	return this;
}

vn.widget.prototype.width = function(width) {
	if (width == undefined) {
		return this._width;
	}

	this._width = width;
	return this;
}

vn.widget.prototype.height = function(height) {
	if (height == undefined) {
		return this._height;
	}

	this._height = height;
	return this;
}
