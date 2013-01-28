<?php
if(!$_GET['q']) die;

$keys = array("ip","proto","src_ip", "dst_ip", "domain","src_domain","dst_domain",
				"port","src_port","dst_port",
				"tcp_port","tcp_src_port","tcp_dst_port",
				"udp_port","udp_src_port","udp_dst_port",
				"http_host","http_uri","http_referer"
				);
$q = trim(substr($_GET['q'], strrpos($_GET['q'], " ")));

foreach($keys as $key)
{
	if(substr($key, 0, strlen($q)) == $q)
		echo substr($_GET['q'], 0, strlen($_GET['q']) - strlen($q)).$key."\n";
}
?>
