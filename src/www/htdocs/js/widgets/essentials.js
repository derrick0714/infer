var label_template = new pv.Label()
					.font("bold 12px sans-serif")
					.left(function() {return this.parent.width() / 2})
					.top(function() {return this.parent.height() / 2})
					.textAlign("center")
					.textBaseline("middle")
					.textStyle(pv.Colors.category10().by(pv.child));
