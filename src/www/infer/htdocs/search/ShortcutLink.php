<?php
//when viewing results for IP with multiple symptoms selected, results may be difficult to view
//class represents shortcut link to view a particular symptom
class ShortcutLink
{
	public $link;
		
	function __toString()
	{
		return $this->link;
	}
	function __construct($schemaName,$count,$numericIP,$startDate,$endDate)
	{
		$this->link = "<a class=\"normal\" href=\"https://".$_SERVER['SERVER_ADDR']."/search/processLink/".$schemaName.'/'.$numericIP.'/'.$startDate.'/'.$endDate.'/'.'">'.$schemaName .' ('.$count.')' .'</a> ';
	}
}

?>