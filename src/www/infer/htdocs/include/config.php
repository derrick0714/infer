<?php
	$infer_install_prefix = '/usr/local';
	
	$infer_frontend_root =
		exec($infer_install_prefix . '/bin/infer_config frontend-root');
	$infer_sensor_data_dir =
		exec($infer_install_prefix . '/bin/infer_config data-directory');
	$localNetworks =
		exec($infer_install_prefix . '/bin/infer_config local-networks');

?>
