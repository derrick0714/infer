<link rel="stylesheet" type="text/css" href="/site/dashboard/main.css" /> 

<script type="text/javascript">

tab_uri_notify = function(e) {
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

		var count = parseInt(argv[2]);
		if (isNaN(count)) {
			count = 20;
		}

		if ($.address.pathNames()[1] != argv[1]) {
			$.address.path(argv.join("/"));
			return;
		}

		set_page_date(argv[1]);
		$('#bottom_row_tab_top_incidents').removeAttr('href');
		document.title = page_title + ' - Top Incidents - ' + argv[1];

		$.getJSON('/data/top_hosts/' + argv[1] + '/' + count, function(content) {
			// display the top ten hosts from JSON data...
			$('#current_date').text(current_date);
			$('#previous_date').text(function()
			{
				var temp_date = date_format(new Date(date_object.getTime() - 86400000));
				$(this).parent().attr('href', '#/' + argv[0] + '/' + temp_date);
				return temp_date;
			});
			
			$('#next_date').text(function()
			{
				var temp_date = date_format(new Date(date_object.getTime() + 86400000));
				$(this).parent().attr('href', '#/' + argv[0] + '/' + temp_date);
				return temp_date;
			});
		
		$('#previous_date').css('visibility', date - 86400 < first_date ? 'hidden' : 'visible');
		$('#next_date').css('visibility', date + 86400 > last_date ? 'hidden' : 'visible');
			var table = '<table class="comp-table">' +
						'<thead>' +
							'<tr>' +
								'<th>IP Address</th>' +
								'<th>Name(s)</th>' +
								'<th>Reason(s)</th>' +
							'</tr>' +
						'</thead>' +
						'<tbody id="top_hosts_body">';
			$.each(content.hosts, function(i, val) {
				$.each(val[2], function(j, indicator) {
					val[2][j] = '<a href="/host/#/indicators/' + val[0] + '/' +
									argv[1] + '/' +
									indicator.toLowerCase().replace(/ /g, "_") +
									'">' + indicator + '</a>';
				});
				table +=
					'<tr>' +
						'<td class="ip">' +
							'<a href="/host/#/events/' + val[0] + "/" +
								argv[1] + '">' + val[0] + '</a>' +
						'</td>' +
						'<td class="names">' + 
							val[1].join(', ') +
						'</td>' +
						'<td class="reasons">' + 
							val[2].join(', ') +
						'</td>' +
					'</tr>';
			});
			table += '</tbody></table>';
			$('#top_hosts').html(table);
		});
	});
};

function date_format(date)
{
	var year = date.getFullYear();
	var month = (date.getMonth() + 1) < 10 ? '0' + (date.getMonth() + 1) : (date.getMonth() + 1);
	var day = date.getDate() < 10 ? '0' + date.getDate() : date.getDate();
	return year + '-' + month + '-' + day;
}

</script>

<div id="inner_wrapper">
	<div class="navigation">
		<a><div class="nav_date" id="previous_date"></div></a>
		<div id="status_title">
			Top incidents for <span id="current_date"></span>
		</div>
		<a><div class="nav_date" id="next_date"></div></a>
	</div>
	<div id="top_hosts"></div>
</div>
