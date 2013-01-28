<?php
$argv = array($_SERVER['SCRIPT_NAME']);
$path_info = trim($_SERVER['PATH_INFO'], '/');
if ($path_info) {
	$argv = array_merge($argv, explode('/', $path_info));
}
$argc = sizeof($argv);

if (substr_compare($argv[0], ".php", -4) == 0) {
	$argv[0] = substr($argv[0], 0, -4);
}

class essentials
{
	function __construct()
	{
		// To use or not to use...
	}
	
	function checkTime($date)
	{
		return checkdate($date[3], $date[4], $date[2]) && ($date[6] >= 0 && $date[6] < 24) && ($date[7] >= 0 && $date[7] < 60) && ($date[8] >= 0 && $date[8] < 60);
	}
	
	function dateParser($arg, $exception)
	{
		if(preg_match('/^(([0-9]{4})-([0-9]{1,2})-([0-9]{1,2}))\s?(([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2}))?$/', $arg, $date) && $this->checkTime($date))
		{
			return strtotime($arg);
		}
		else
		{
			//return date('Y-m-d', $exception);
			return $exception; // Sort of redundant
		}
	}
	
	function size_display($bytes)
	{
		$sizes = array('B', 'KiB', 'MiB', 'GiB', 'TiB');
		$i = 0;
		
		while($bytes > 1024 && $i < count($sizes))
		{
			$bytes /= 1024;
			$i++;
		}
		
		return number_format($bytes, 2).' '.$sizes[$i];
	}
	
	function time_display($timestamp)
	{
		return date('Y-m-d H:i:s', $timestamp);
	}
	
	function duration_display($seconds)
	{
		$duration_str = '';
		
		$hours = floor($seconds / 3600);
		if($hours > 0)
		{
			$duration_str .= $hours.'h ';
			$seconds %= 3600;
		}
		
		$minutes = floor($seconds / 60);
		if($minutes > 0)
		{
			$duration_str .= $minutes.'m ';
			$seconds %= 60;
		}
		
		if($seconds > 0) $duration_str .= $seconds.'s ';
		
		if(strlen($duration_str) == 0) return '<1s';
		else return trim($duration_str);
	}

	function protocol_name($protocol_number) {
		switch ($protocol_number) {
		  case 6:
			return 'TCP';
			break;
		  case 17:
			return 'UDP';
			break;
		  default:
		}

		return $protocol_number;
	}

	function country_flag($country_name, $country_code) {
		return '<img style="vertical-align: middle; border: none;" alt="' . 
			$country_name . '" title="' . $country_name .
			'" src="/images/flags/png/' . strtolower($country_code) . '.png">';
	}
}
?>
