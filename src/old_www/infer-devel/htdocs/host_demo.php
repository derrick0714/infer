<?php
require_once('./data/essentials.class.php');
require_once('./data/pg.include.php');

$essentials = new essentials();

if($argv[1] != null && $argv[2] != null)
{
	$ip = $argv[1];
	$date = $essentials->dateParser($argv[2], false);
	$date_check = @pg_num_rows(@pg_query($pg, "SELECT \"table_name\" FROM \"information_schema\".\"tables\" WHERE \"table_schema\" = 'LiveIPs' AND \"table_name\" = '".date('Y-m-d', $date)."' LIMIT 1"));
	$ip_check = @pg_num_rows(@pg_query($pg, "SELECT * FROM \"LiveIPs\".\"".date('Y-m-d', $date)."\" WHERE \"ip\" = '".ip2long($ip)."' LIMIT 1"));
	if($date && $date_check == 1 && $ip_check == 1)
	{
		$date = date('Y-m-d', $date);
	}
	else
	{
		require_once('./include/404.php');
	}
}
else
{
	require_once('./include/404.php');
}
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd"> 
<html xmlns="http://www.w3.org/1999/xhtml"> 
 
<head> 
	<meta content="text/html; charset=utf-8" http-equiv="Content-Type" /> 
	<title>Executive Dashboard</title> 
	
	<!-- CASCADING STYLE SHEETS -->
	<link rel="stylesheet" type="text/css" href="/css/smoothness/jquery-ui.css" /> 
	<link rel="stylesheet" type="text/css" href="/css/jquery-ui-mod.css" /> 
	<link rel="stylesheet" type="text/css" href="/css/main.css" /> 
	
	<!-- LIBRARIES -->
	<script type="text/javascript" src="/js/jQuery/jquery.js"></script> 
	<script type="text/javascript" src="/js/jQuery/jquery.ui.core.js"></script> 
	<script type="text/javascript" src="/js/jQuery/jquery.ui.widget.js"></script> 
	<script type="text/javascript" src="/js/jQuery/jquery.ui.tabs.js"></script>
	<!--[if IE]>
	<script type="text/javascript" src="/js/protovis/3rdparty/svg.js" data-path="/js/protovis/3rdparty/"></script>
	<![endif]-->
	<script type="text/javascript" src="/js/protovis/protovis-d3.3.js"></script>

	<!-- WIDGET CLASSES -->
	<script type="text/javascript" src="/js/dashboard_initialization.js"></script>
	<script type="text/javascript" src="/js/vn/vn.js"></script>
	<script type="text/javascript" src="/js/vn/vn-internals.js"></script>
	<script type="text/javascript" src="/js/vn/widget.js"></script>
	<script type="text/javascript" src="/js/vn/portlet.js"></script>
	
	<script type="text/javascript" src="/js/vn/widget/app_inventory_table.js"></script> 
	<script type="text/javascript" src="/js/vn/widget/as_exposure_bar.js"></script> 
	<script type="text/javascript" src="/js/vn/widget/bandwidth_content_stack.js"></script> 
	<script type="text/javascript" src="/js/vn/widget/bandwidth_utilization_area.js"></script> 
	<script type="text/javascript" src="/js/vn/widget/host_inventory_table.js"></script> 
	<script type="text/javascript" src="/js/vn/widget/traffic_content_legend.js"></script> 
	<script type="text/javascript" src="/js/vn/widget/traffic_content_pie.js"></script> 
	<script type="text/javascript" src="/js/vn/widget/port_traffic_bar.js"></script> 
	
	<script type="text/javascript" src="/js/vn/portlet/traffic_overview.js"></script> 
	<script type="text/javascript" src="/js/vn/portlet/app_overview.js"></script> 
	<script type="text/javascript" src="/js/vn/portlet/host_overview.js"></script> 
	<script type="text/javascript" src="/js/vn/portlet/as_overview.js"></script> 
	<script type="text/javascript" src="/js/vn/portlet/port_overview.js"></script> 
	<script type="text/javascript" src="/js/vn/portlet/bandwidth_overview.js"></script> 
	
	<!-- WIDGET INITIALIZATION -->
	<script type="text/javascript+protovis">
	$(document).ready(function()
	{
		var traffic_overview = new vn.portlet.traffic_overview()
			.div("traffic_overview")
			.width(500)
			.height(300)
			.ingress_content_pie_data_uri("/data/traffic_content/ingress/<?=$ip?>/<?=$date?>")
			.egress_content_pie_data_uri("/data/traffic_content/egress/<?=$ip?>/<?=$date?>")
			.traffic_content_legend_data_uri("/data/traffic_content/legend")
			.bandwidth_utilization_areas_data_uri(["/data/bandwidth_utilization/ingress/<?=$ip?>/<?=$date?>", "/data/bandwidth_utilization/egress/<?=$ip?>/<?=$date?>"]);
			
		traffic_overview.render();
		
		var port_overview = new vn.portlet.port_overview()
			.div("port_overview")
			.width(500)
			.height(300)
			.ingress_port_traffic_bar_data_uri("/data/port_traffic/ingress/<?=$ip?>/<?=$date?>")
			.egress_port_traffic_bar_data_uri("/data/port_traffic/egress/<?=$ip?>/<?=$date?>");
		
		port_overview.render();
		
		var ingress_bandwidth_overview = new vn.portlet.bandwidth_overview()
			.div("ingress_bandwidth_overview")
			.width(500)
			.height(150)
			.bandwidth_content_stack_data_uri("/data/bandwidth_content/ingress/<?=$ip?>/<?=$date?>");
		
		ingress_bandwidth_overview.render();
		
		var egress_bandwidth_overview = new vn.portlet.bandwidth_overview()
			.div("egress_bandwidth_overview")
			.width(500)
			.height(150)
			.bandwidth_content_stack_data_uri("/data/bandwidth_content/egress/<?=$ip?>/<?=$date?>");
		
		egress_bandwidth_overview.render();
		
		var as_overview = new vn.portlet.as_overview()
			.div("as_overview")
			.width(500)
			.height(300)
			.ingress_as_exposure_bar_data_uri("/data/host_as_exposure/top_ingress/<?=$ip?>/<?=$date?>")
			.egress_as_exposure_bar_data_uri("/data/host_as_exposure/top_egress/<?=$ip?>/<?=$date?>");
		
		as_overview.render();
		
		var host_overview = new vn.portlet.host_overview()
			.div("host_overview")
			.width(500)
			.height(300)
			.host_inventory_table_data_uri(["/data/host_role_history/web_server/<?=$ip?>/<?=$date?>", "/data/host_role_history/web_client/<?=$ip?>/<?=$date?>", "/data/host_role_history/mail_server/<?=$ip?>/<?=$date?>", "/data/host_role_history/mail_client/<?=$ip?>/<?=$date?>", "/data/host_role_history/secure_web_server/<?=$ip?>/<?=$date?>", "/data/host_role_history/secure_web_client/<?=$ip?>/<?=$date?>", "/data/host_role_history/secure_mail_server/<?=$ip?>/<?=$date?>", "/data/host_role_history/secure_mail_client/<?=$ip?>/<?=$date?>", "/data/host_role_history/dark_space_bot/<?=$ip?>/<?=$date?>", "/data/host_role_history/encrypted_p2p_node/<?=$ip?>/<?=$date?>", "/data/host_role_history/ftp_brute_forced/<?=$ip?>/<?=$date?>", "/data/host_role_history/ftp_brute_forcer/<?=$ip?>/<?=$date?>", "/data/host_role_history/microsoft_sql_brute_forced/<?=$ip?>/<?=$date?>", "/data/host_role_history/microsoft_sql_brute_forcer/<?=$ip?>/<?=$date?>", "/data/host_role_history/multimedia_p2p_node/<?=$ip?>/<?=$date?>", "/data/host_role_history/mysql_brute_forced/<?=$ip?>/<?=$date?>", "/data/host_role_history/mysql_brute_forcer/<?=$ip?>/<?=$date?>", "/data/host_role_history/oracle_sql_brute_forcer/<?=$ip?>/<?=$date?>", "/data/host_role_history/oracle_sql_brute_forced/<?=$ip?>/<?=$date?>", "/data/host_role_history/postgresql_brute_forced/<?=$ip?>/<?=$date?>", "/data/host_role_history/postgresql_brute_forcer/<?=$ip?>/<?=$date?>", "/data/host_role_history/scan_bot/<?=$ip?>/<?=$date?>", "/data/host_role_history/spam_bot/<?=$ip?>/<?=$date?>", "/data/host_role_history/ssh_brute_forced/<?=$ip?>/<?=$date?>", "/data/host_role_history/ssh_brute_forcer/<?=$ip?>/<?=$date?>", "/data/host_role_history/telnet_brute_forced/<?=$ip?>/<?=$date?>", "/data/host_role_history/telnet_brute_forcer/<?=$ip?>/<?=$date?>", "/data/host_role_history/unclassified_p2p_node/<?=$ip?>/<?=$date?>"]);
			
		host_overview.render();
	});
	</script>
</head> 
 
<body>
<div id="wrapper">
	<div id="tabs"> 
		<div id="host_profile">
			<div><h1>Host Profile for <i><?=$ip?></i> on <?=$date?></h1></div>
			<div class="tabs-content">
				<div class="row">
					<div class="column">
						<div class="portlet" id="traffic_overview"></div>
						<div class="portlet" id="ingress_bandwidth_overview"></div>
						<div class="portlet" id="as_overview"></div>
					</div>
					<div class="column">
						<div class="portlet" id="port_overview"></div>
						<div class="portlet" id="egress_bandwidth_overview"></div>
						<div class="portlet" id="host_overview"></div>
					</div>
				</div>
			</div>
		</div>
	</div> 
</div>
</body> 
</html> 