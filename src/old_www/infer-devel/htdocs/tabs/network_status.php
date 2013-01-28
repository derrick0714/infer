<link rel="stylesheet" type="text/css" href="/site/dashboard/main.css" /> 

<script type="text/javascript">
$(document).ready(function()
{
	function date_format(date)
	{
		var year = date.getFullYear();
		var month = (date.getMonth() + 1) < 10 ? '0' + (date.getMonth() + 1) : (date.getMonth() + 1);
		var day = date.getDate() < 10 ? '0' + date.getDate() : date.getDate();
		return year + '-' + month + '-' + day;
	}
	
	function set_navigation(date, date_object, current_date, first_date, last_date)
	{
		window.location.hash = current_date;
		$('#current_date').text(current_date);
		
		$.address.title('INFER | Network Overview | ' + current_date);
		
		$('#previous_date').text(function()
		{
			var temp_date = date_format(new Date(date_object.getTime() - 86400000));
			$(this).parent().attr('href', '#' + temp_date);
			return temp_date;
		});
		
		$('#next_date').text(function()
		{
			var temp_date = date_format(new Date(date_object.getTime() + 86400000));
			$(this).parent().attr('href', '#' + temp_date);
			return temp_date;
		});
		
		$('#previous_date').css('visibility', date - 86400 < first_date ? 'hidden' : 'visible');
		$('#next_date').css('visibility', date + 86400 > last_date ? 'hidden' : 'visible');
	}
	
	function render(current_date, traffic_overview, app_overview, host_overview, as_overview)
	{
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
	}

	$.getJSON('/data/date_checker/' + window.location.hash.substr(1), function(content) {
		var date = (Date.parse(content.date_checker.date.replace(/-/g, '/'))) / 1000;
		var first_date = (Date.parse(content.date_checker.first_date.replace(/-/g, '/'))) / 1000;
		var last_date = (Date.parse(content.date_checker.last_date.replace(/-/g, '/'))) / 1000;
		
		var date_object = new Date(date * 1000);
		var current_date = date_format(date_object);
		
		var traffic_overview = new vn.portlet.traffic_overview()
			.div("traffic_overview")
			.width(500)
			.height(300);
		
		var app_overview = new vn.portlet.app_overview()
			.div("app_overview")
			.width(500)
			.height(300);
		
		var host_overview = new vn.portlet.host_overview()
			.div("host_overview")
			.width(500)
			.height(300);
		
		var as_overview = new vn.portlet.as_overview()
			.div("as_overview")
			.width(500)
			.height(300);

		set_navigation(date, date_object, current_date, first_date, last_date);
		render(current_date, traffic_overview, app_overview, host_overview, as_overview);
		
		$.address.change(function(event)
		{
			var temp_date = (Date.parse(event.value.replace(/-/g, '/'))) / 1000;
			if(first_date <= temp_date && temp_date <= last_date)
			{
				date = temp_date;
			
				date_object = new Date(date * 1000);
				current_date = date_format(date_object);
				
				set_navigation(date, date_object, current_date, first_date, last_date);
				render(current_date, traffic_overview, app_overview, host_overview, as_overview);
			}
		});
	});
});
</script>
	
<div class="tabs-content">
	<div class="navigation">
		<a><div class="nav_date" id="previous_date"></div></a>
		<div id="status_title">Network Overview for <span id="current_date"></span></div>
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