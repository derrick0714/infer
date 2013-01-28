<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd"> 
<html xmlns="http://www.w3.org/1999/xhtml"> 
 
<head> 
	<title></title>
	<meta content="text/html; charset=utf-8" http-equiv="Content-Type" /> 
	<link rel="stylesheet" type="text/css" href="/new_ui/css/style.css" />
	<link rel="stylesheet" type="text/css" href="/site/dashboard/main.css" /> 
	<link rel="stylesheet" type="text/css"
		  href="/js/flexigrid/css/flexigrid/flexigrid.css" /> 
	<link rel="stylesheet" type="text/css" href="/css/demo_table.css"/>
	<script type="text/javascript" src="/js/jQuery/jquery.js"></script> 
	<script type="text/javascript" src="/js/jQuery/jquery.address.js"></script>

	<!--[if IE]>
	<script type="text/javascript"
			src="/js/protovis/3rdparty/svg.js"
			data-path="/js/protovis/3rdparty/"></script>
	<![endif]-->
	<script type="text/javascript" src="/js/protovis/protovis-d3.3.js"></script>
	<script type="text/javascript" src="/js/flexigrid/flexigrid.js"></script>
	<script type="text/javascript" src="/js/dataTables/jquery.dataTables.js"></script>
	<script type="text/javascript" src="/js/dataTables/jquery.dataTables.extra.js"></script>


	<script type="text/javascript"
			src="/js/vn/vn.js"></script>
	<script type="text/javascript"
			src="/js/vn/vn-internals.js"></script>
	<script type="text/javascript"
			src="/js/vn/widget.js"></script>
	<script type="text/javascript"
			src="/js/vn/portlet.js"></script>

	<script type="text/javascript"
			src="/js/vn/widget/app_inventory_table.js"></script> 
	<script type="text/javascript"
			src="/js/vn/widget/as_exposure_bar.js"></script> 
	<script type="text/javascript"
			src="/js/vn/widget/bandwidth_content_stack.js"></script> 
	<script type="text/javascript"
			src="/js/vn/widget/bandwidth_utilization_area.js"></script> 
	<script type="text/javascript"
			src="/js/vn/widget/host_inventory_table.js"></script> 
	<script type="text/javascript"
			src="/js/vn/widget/traffic_content_legend.js"></script> 
	<script type="text/javascript"
			src="/js/vn/widget/traffic_content_pie.js"></script> 
	<script type="text/javascript"
			src="/js/vn/widget/port_traffic_bar.js"></script> 
	<script type="text/javascript"
			src="/js/vn/widget/related_hosts_graph.js"></script> 
	<script type="text/javascript"
			src="/js/vn/widget/related_hosts_insight_panel.js"></script> 

	<script type="text/javascript"
			src="/js/vn/portlet/traffic_overview.js"></script> 
	<script type="text/javascript"
			src="/js/vn/portlet/app_overview.js"></script> 
	<script type="text/javascript"
			src="/js/vn/portlet/host_overview.js"></script> 
	<script type="text/javascript"
			src="/js/vn/portlet/as_overview.js"></script> 
	<script type="text/javascript"
			src="/js/vn/portlet/port_overview.js"></script> 
	<script type="text/javascript"
			src="/js/vn/portlet/bandwidth_overview.js"></script> 

	<script type="text/javascript">
	var argv = [];
	var prev_argv = [];
	var page_name = '<?php echo $page_name;?>';
	var page_display = '<?php echo $page_display;?>';
	var page_title = 'INFER - ' + page_display;
	var tab_uri_notify = function(e) {
	};

	var bottom_row_tabs = [];
<?php
if (is_array($bottom_row_tabs)) {
	foreach ($bottom_row_tabs as $name => $display) {
?>
	bottom_row_tabs.tab_<?php echo $name;?> = '<?php echo $display;?>';
<?php
	}
}
?>

	var change_page = function(e)
	{
		prev_argv = argv.slice();
		argv = e.pathNames;

		if (argv[0] == undefined) {
			$.address.path('<?php echo $default_hash_path;?>');
			return;
		}
		
		if (argv[0] != prev_argv[0]) {
			$('#bottom_row_tab_' + prev_argv[0]).removeAttr('class');
			$('#bottom_row_tab_' + prev_argv[0]).attr('href',
													  '#/' + prev_argv[0]);
			$('#body_content').load(
				'/site/' + page_name + '/' + argv[0],
				function(response, status, xhr) {
					if (status == 'error') {
						document.title = page_title;
						set_page_date();
						$('#body_content').html('Invalid URI.');
					}
					else {
						document.title = page_title + ' - ' + bottom_row_tabs['tab_' + argv[0]];
						$('#bottom_row_tab_' + argv[0]).removeAttr('href');
						$('#bottom_row_tab_' + argv[0])
							.attr('class', 'active');
						tab_uri_notify(e);
					}
				}
			);
		}
		else {
			tab_uri_notify(e);
		}
	};

	var set_page_date = function(d) {
		if (d != undefined) {
			$('#mid_row_tab_dashboard').attr('href', '/dashboard/#/stats/' + d);
			$('#mid_row_tab_incidents').attr('href',
											 '/incidents/#/top_incidents/' + d);
		}
		else {
			$('#mid_row_tab_dashboard').attr('href', '/dashboard/');
			$('#mid_row_tab_incidents').attr('href',
											 '/incidents/');
		}
		$('#mid_row_tab_' + page_name).removeAttr('href');
		set_tabs_date(d);
		$('#bottom_row_tab_' + argv[0]).removeAttr('href');
	}

	var set_tabs_date = function(d) {
		<?php echo $tabs_date;?>
	}

	$(document).ready(function()
	{
		document.title = page_title;
		$('#mid_row_tab_' + page_name).attr('class', 'active');
		$('#mid_row_tab_' + page_name).removeAttr('href');
		$.address.change(change_page);
	});

	</script>
</head>

<body>
	<div id="wrapper">
	<?php
		require_once('include/header.php');
		require_once('include/body.php');
		require_once('include/footer.php');
	?>
	</div>
</body>

</html>
