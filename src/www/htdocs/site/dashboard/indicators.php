<script type="text/javascript">

tab_uri_notify = function(e) {
	// check for subsection and handle that...
	var section = 'index';
	if (argv[2] != undefined) {
		section = argv[2];
	}

	$.getJSON('/data/date_checker/' + argv[1], function(content) {
		var date =
			(Date.parse(content.date_checker.date.replace(/-/g, '/'))) / 1000;
		var first_date =
			(Date.parse(content.date_checker.first_date.replace(/-/g, '/'))) 
				/ 1000;
		var last_date =
			(Date.parse(content.date_checker.last_date.replace(/-/g, '/')))
				/ 1000;
		var date_object = new Date(date * 1000);
		var current_date = date_format(date_object);
		
		argv[1] = date_format(
			new Date(Date.parse(content.date_checker.date.replace(/-/g, '/'))));

		if ($.address.pathNames()[1] != argv[1]) {
			$.address.path(argv.join("/"));
			return;
		}

		set_page_date(argv[1]);
		document.title = page_title + ' - Indicators - ' + argv.slice(1).join(' - ');
		$('#current_date').text(current_date);
		$('#previous_date').text(function()
		{
			var temp_date = date_format(new Date(date_object.getTime() - 86400000));
			$(this).parent().attr('href', '#/' + argv[0] + '/' + temp_date + '/' + argv.slice(2).join('/'));
			return temp_date;
		});
		
		$('#next_date').text(function()
		{
			var temp_date = date_format(new Date(date_object.getTime() + 86400000));
			$(this).parent().attr('href', '#/' + argv[0] + '/' + temp_date + '/' + argv.slice(2).join('/'));
			return temp_date;
		});
	
		$('#previous_date').css('visibility', date - 86400 < first_date ? 'hidden' : 'visible');
		$('#next_date').css('visibility', date + 86400 > last_date ? 'hidden' : 'visible');


		var prev_current_date = prev_argv[1];

		// parse the ? parameters

		prev_num_results = num_results;
		prev_page_number = page_number;
		prev_sort_name = sort_name;
		prev_sort_order = sort_order;
		prev_columns = columns.slice();

		num_results = $.address.parameter('rp');
		if (num_results == undefined) {
			num_results = d_num_results;
		}

		page_number = $.address.parameter('page');
		if (page_number == undefined) {
			page_number = d_page_number;
		}

		$.ajax({
			url: '/data/role/' + section + '/' + current_date +
					'/' + argv.slice(3).join('/') + '?page=0&rp=0',
			dataType: 'json',
			success: function(content) {
				col_model = content.col_model;
				d_columns = [];

				$.each(col_model, function(i, v) {
					d_columns.push(v.name);
				});

				d_sort_name = content.sort_name;
				d_sort_order = content.sort_order;

				columns = $.address.parameter('columns');
				if (columns == undefined) {
					columns = d_columns.slice();
				}
				else {
					columns = columns.split(',');
				}

				sort_name = $.address.parameter('sort_name');
				if (sort_name == undefined) {
					sort_name = d_sort_name;
				}

				sort_order = $.address.parameter('sort_order');
				if (sort_order == undefined) {
					sort_order = d_sort_order;
				}

				if (num_results != prev_num_results ||
					page_number != prev_page_number ||
					sort_name != prev_sort_name ||
					sort_order != prev_sort_order ||
					argv.toString() != prev_argv.toString())
				{
					var breadcrumb = [];
					breadcrumb.push('<a href="#/' + argv.slice(0, 2).join('/') + '">Indicators - ' + argv.slice(1, 2).join(' - ') + '</a>');
					for (var i = 2; i < argv.length; i++) {
						breadcrumb.push('<a href="#/' + argv.slice(0, i+1).join('/') + '">' + argv[i] + '</a>');
					}
					$('div.flexigrid').replaceWith('<table id="flex1" style="display:none"></table>');
					$('#flex1').flexigrid({
						dataType: 'json',
						//url: '/data/host_role/index/' + ip + '/' + current_date,
						url: '/data/role/' + section + '/' + current_date + '/' + argv.slice(4).join('/'),
						title: breadcrumb.join(' - '),
						rp: num_results,
						newp: page_number,
						sortname: sort_name,
						sortorder: sort_order,
						method: 'GET',
						colModel: col_model,
						usepager: true,
						singleSelect: true,
						useRp: true,
						showTableToggleBtn: false,
						width: 'auto',
						height: 'auto',
						resizable: true,
						onChangePage: function(newp)
						{
							if (newp == d_page) {
								newp = '';
							}
							$.address.parameter('page', newp);
						},
						onRpChange: function(rp)
						{
							if (rp == d_num_results) {
								rp = '';
							}
							$.address.parameter('rp', rp);
						},
						onChangeSort: function(sortname, sortorder)
						{
							if (sortname == d_sort_name &&
								sortorder == d_sort_order)
							{
								sortname = '';
								sortorder = '';
							}
							$.address.parameter('sort_name', sortname);
							$.address.parameter('sort_order', sortorder);
						},
						onToggleCol: flex_toggle_col,
						onDragCol: flex_drag_col,
						onClickRow: function(e, r) {
							if ($(r).attr('id').substr(4, 3) == 'url') {
								if (section == 'index') {
									$.address.path('/' + argv.join('/') + '/' + $(r).attr('id').substr(8));
								}
								else {
									window.location = '/host/#/indicators/' + $(r).attr('id').substr(8) + '/' + current_date + '/' + section;
								}
							}
						},
						onHoverRow: [
							function(e, r) {
								if ($(r).attr('id').substr(4, 3) == 'url') {
									r.css("cursor", "pointer");
								}
								else {
									r.css("cursor", "default");
								}
							}
						],
						onError: function() {
							$.address.history(false);
							$.address.path('/error/invalid_indicator/' + section);
							$.address.history(true);
						}
						/*
						onSuccess: table_updated()
						*/
					});
				}
				block_flex_callbacks = true;
				var reverse_columns = columns.reverse();
				$.each(reverse_columns, function(i, v)
				{
					var pos = col_to_pos(v);
					var cid = pos_to_cid(pos);
					if (pos != 0) {
						$('#flex1').flexToggleCol(cid, true);
						$('#flex1').flexSwitchCol(pos,0);
					}
				});
				for (var i = columns.length; i < col_model.length; i++) {
					$('#flex1').flexToggleCol(pos_to_cid(i), false);
				}
				block_flex_callbacks = false;
			},
			error: function() {
				$.address.history(false);
				$.address.path('/error/invalid_indicator/' + section);
				$.address.history(true);
			}
		});
	});
}

var block_flex_callbacks = false;

var d_num_results = 15;
var d_page_number = 1;

var d_sort_name;
var d_sort_order;
var d_columns = [];

var num_results;// = d_num_results;
var page_number;// = d_page_number;
var sort_name;// = d_sort_name;
var sort_order;// = d_sort_order;
var columns = [];

var prev_num_results;
var prev_page_number;
var prev_sort_name;
var prev_sort_order;
var prev_columns = [];

var col_model = [];

function flex_toggle_col(cid, visible) {
	if (block_flex_callbacks) {
		return;
	}

	var new_columns = [];

	$('.nDiv input').each(function() {
		var cur_cid = $(this).attr('value');
		var checked = $(this).attr('checked');
		if (checked) {
			new_columns.push(col_model[cur_cid].name);
		}
	});
	if (new_columns.toString() == d_columns.toString()) {
		new_columns = undefined;
	}
	$.address.parameter('columns', new_columns.toString());
}

function flex_drag_col(cdrag, cdrop) {
	if (block_flex_callbacks) {
		return;
	}

	var new_columns = [];

	$('.nDiv input').each(function() {
		var cur_cid = $(this).attr('value');
		var checked = $(this).attr('checked');
		if (checked) {
			new_columns.push(col_model[cur_cid].name);
		}
	});
	if (new_columns.toString() == d_columns.toString()) {
		new_columns = [];
	}
	$.address.parameter('columns', new_columns.toString());
}

function date_format(date)
{
	var year = date.getFullYear();
	var month = (date.getMonth() + 1) < 10 ? '0' + (date.getMonth() + 1) : (date.getMonth() + 1);
	var day = date.getDate() < 10 ? '0' + date.getDate() : date.getDate();
	return year + '-' + month + '-' + day;
}

function col_to_pos(col_name)
{
	for (var i = 0; i < col_model.length; i++) {
		if (col_model[i].name == col_name) {
			return $('.nDiv').find('input[value=' + i + ']').parent().parent().index();
		}
	}
	return -1;
}

function pos_to_cid(col_pos) {
	return $('.nDiv tr:eq(' + col_pos + ') input').attr('value');
}

</script>

<div id="inner_wrapper">
	<div class="navigation">
		<a><div class="nav_date" id="previous_date"></div></a>
		<div id="status_title">Indicators on <span id="current_date"></span></div>
		<a><div class="nav_date" id="next_date"></div></a>
	</div>
	<table id="flex1" style="display:none"></table> 
<!--
	<p>
		<a id='permalink'>Permalink</a>
	</p>
-->
</div>
