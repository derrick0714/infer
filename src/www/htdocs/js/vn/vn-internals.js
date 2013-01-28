// from pv.extend
vn.extend = function(f) {
	function g() {}
	g.prototype = f.prototype || f;
	return new g();
};
