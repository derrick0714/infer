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

		if ($.address.pathNames()[1] != argv[1]) {
			$.address.path(argv.join("/"));
			return;
		}

		set_page_date(argv[1]);
		$('#bottom_row_tab_stats').removeAttr('href');
		document.title = page_title + ' - Stats - ' + argv[1];

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

		traffic_overview.ingress_content_pie_data_uri("/data/traffic_content/ingress/*/" + current_date)
			.egress_content_pie_data_uri("/data/traffic_content/egress/*/" + current_date)
			.traffic_content_legend_data_uri("/data/traffic_content/legend")
			.bandwidth_utilization_areas_data_uri(["/data/bandwidth_utilization/ingress/*/" + current_date, "/data/bandwidth_utilization/egress/*/" + current_date])
			.render();
			
		app_overview.app_inventory_table_data_uri(["/data/app_inventory/browser/" + current_date])
			.render();
		
		host_overview.host_inventory_table_data_uri(["/data/role_history/web_server/" + current_date, "/data/role_history/web_client/" + current_date, "/data/role_history/mail_server/" + current_date, "/data/role_history/mail_client/" + current_date, "/data/role_history/secure_web_server/" + current_date, "/data/role_history/secure_web_client/" + current_date, "/data/role_history/secure_mail_server/" + current_date, "/data/role_history/secure_mail_client/" + current_date])
			.render();
		
		as_overview.ingress_as_exposure_bar_data_uri("/data/as_exposure/ingress/" + current_date)
			.egress_as_exposure_bar_data_uri("/data/as_exposure/egress/" + current_date)
			.render();
	});
}

function date_format(date)
{
	var year = date.getFullYear();
	var month = (date.getMonth() + 1) < 10 ? '0' + (date.getMonth() + 1) : (date.getMonth() + 1);
	var day = date.getDate() < 10 ? '0' + date.getDate() : date.getDate();
	return year + '-' + month + '-' + day;
}


var width;
var traffic_overview;
var app_overview;
var host_overview;
var as_overview;

$(document).ready(function() {
	$('#inner_wrapper').parent().width('auto');
	// width = 980;
	width = $('#inner_wrapper').parent().width() - 20;
	$('#inner_wrapper').width(width);

	traffic_overview = new vn.portlet.traffic_overview()
		.div("traffic_overview")
		.width(width / 2)
		.height(width * .3);

	app_overview = new vn.portlet.app_overview()
		.div("app_overview")
		.width(width / 2)
		.height(width * .3);

	host_overview = new vn.portlet.host_overview()
		.div("host_overview")
		.width(width / 2)
		.height(width * .3);

	as_overview = new vn.portlet.as_overview()
		.div("as_overview")
		.width(width / 2)
		.height(width * .3);
});

</script>
	
<div id="inner_wrapper">
	<div class="navigation">
		<a><div class="nav_date" id="previous_date"></div></a>
		<div id="status_title">Dashboard for <span id="current_date"></span></div>
		<a><div class="nav_date" id="next_date"></div></a>
	</div>
	<div class="row">
		<div class="column">
			<div class="portlet" id="traffic_overview"></div>
			<div class="portlet" id="app_overview"></div>
		</div>
		<div class="column">
			<div class="portlet" id="host_overview"></div>
			<div class="portlet" id="as_overview"></div>
		</div>
	</div>
</div>
