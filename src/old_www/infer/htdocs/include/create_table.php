<?php

include('services.php');
include('infectedSources.php');

/**
* Dynamic PostgreSQL Table generator
* @copyright Polytechnic Institute of NYU
* Written by Stanislav Palatnik
*  
* @var pgSQL resource
* @var String schema - the database schema
* @var String table - name of the table to query
* @var Array $display_columns. Takes header(database column name), display, and type 
* @var Array $where, $group, $order : optional paramters to include in the query
* @var int numResults - number of results per page
* @var int page - the page to display 
* Error Values: 
* -1 = No database connection detected;
* 0 = Database query failed
* 1 = No data found in table
* 2 = Invalid number of display results
* 3 = Invalid page to display
* 4 = Invalid format of array. Each array's element must contain at least 2 sub elements, the query name and display name
* 5 = where clause has a missing  paramter, must contain 3 paramters: target, condition, value
* 6 = order by array missing paramter, must have 2 
* example: array('database_name','Display_Name','Display Type(optional)','Format string(optional)') 
* note: $order has two paramters, one the target column and the type of sort IE ASC/DESC
* Available formatting : Date, IP(formats to IP string), Protocol(prints UDP or TCP),
* Status(prints if query is complete), Byte(prints human readable size) 
*enter 'None' in the display to make sure the table option is not displayed ie if you only need to query the ID
*/

function create_table($resource, $schema, $table, Array $display_columns, $numResults, $page=1, Array $where = NULL, Array $group = NULL, Array $orders =NULL) 
{
      //checking for errors before we continue
      foreach($display_columns as $column) { if(count($column)<2) return ('4'); }
      if($where!=NULL) {foreach($where as $waldo) { if(count($waldo)<3) return ('5'); } }
      if($orders!=NULL) {foreach($orders as $order) { if(count($order)<2) return ('6'); }}
      if($numResults <1) return ('2');
      if($page<1) return ('3');
      if($resource === FALSE) return('-1');
      if(($query = pg_query($resource,'SELECT COUNT (*) from "'.$schema.'"."'.$table.'"')) === false) return ('0');
      if(($count = pg_num_rows($query)) == 0) return('1');
      
      //check for injections
	  /*
      $schema = pg_escape_string($resource,$schema);
      $table = pg_escape_string($resource,$table);
      $numResults = pg_escape_string($resource,$numResults);
      */
      $counter = count($display_columns);
      $rowNum = 0;
      $offset = $numResults*($page-1); 
      $headers='';
      $selected='';
      $where_query ='WHERE ';
      $group_query ='GROUP BY ';
      $order_query ='ORDER BY ';
      for($i=0;$i<$counter;$i++)
      {
          if($i == ($counter-1) || $counter<2) {$selected .='"'.pg_escape_string($display_columns[$i][0]).'"'; } 
          else{ $selected .='"'.pg_escape_string($display_columns[$i][0]).'", ';} 
      }
      
      if($where !=NULL)
      {
          for($g=0;$g<count($where);$g++)
          {
                  if($g == ($counter-1) || $counter<2) {$where_query .='"'.pg_escape_string($where[$g][0]).'"'.pg_escape_string($where[$g][1])
                  .pg_escape_string($where[$g][2]); } 
                  else{ $where_query .='"'.pg_escape_string($where[$g][0]).'"'.pg_escape_string($where[$g][1])
                  .pg_escape_string($where[$g][2]).'", ';}    
          }
      }
      else { $where_query ='';}
      if($group !=NULL)
      {
          for($o=0;$o<count($group);$o++)
          {
                  if($o == ($counter-1) || $counter<2) {$group_query .='"'.pg_escape_string($group[$o]).'"'; } 
                  else{ $group_query .='"'.pg_escape_string($group[$o]).'", ';}   
          }
      }
      else { $group_query='';}
      if($orders !=NULL)
      {
          for($x=0;$x<count($orders);$x++)
          {
                  if($o == ($counter-1) || $counter<2) {$order_query .='"'.pg_escape_string($orders[$o][0]).'"'. ' '.pg_escape_string($orders[$o][1]); } 
                  else{ $order_query .='"'.pg_escape_string($orders[$o][0]).'"'. ' '.pg_escape_string($orders[$o][1]).'", ';}   
          }
      }
      else {$order_query='';}
      $result = pg_query($resource,"SELECT {$selected} FROM \"{$schema}\".\"{$table}\" t {$group} {$order} {$where_query} {$group_query} {$order_query}  LIMIT {$numResults} OFFSET {$offset}");
      $table_output = '<table id="pgsql_display_table" width="100%" cellspacing="1" cellpadding="2"><thead><tr>';
      foreach($display_columns as $column)
      {
          if($column[1] !='None') {$headers .="<th class=\"columnTitle center\" scope=\"col\">{$column[1]}</th>";}
      }
      $table_output .='</tr></thead><tbody>';
      while(($row = pg_fetch_row($result)))
      {
          if ($rowNum % 10 == 0) { $table_output .=$headers; }
          $rowNum % 2 ? $rowClass = 'odd' : $rowClass = 'even';
          $table_output .='<tr class="'.$rowClass.'" onmouseover="this.className=\'table_hover\'" onmouseout="this.className=\''.$rowClass.'\'">';
          for($j=0;$j<$counter;$j++)
          {
                if($display_columns[$j][1]=='None') {$formatted_data ='';}//no nothing //if specified not to display
                elseif(!empty($display_columns[$j][2]))
                {
                    if($display_columns[$j][2]=='Date') { $formatted_data = date($display_columns[$j][3],(int)$row[$j]);}
                    elseif($display_columns[$j][2]=='IP') { $formatted_data = isset($row[$j])?long2ip($row[$j]):'';}
                    elseif($display_columns[$j][2]=='Protocol') {$formatted_data = ($row[$j]==6)? 'UDP': 'TCP'; }
                    elseif($display_columns[$j][2]=='Status') {$formatted_data = '<a class="text" href="/search/neoflow/' . $row[0] . '">' . ($row[8]?'yes':'no') . '</a>';}
                    elseif($display_columns[$j][2]=='Byte')
                    {
                        $s = array('bytes', 'kb', 'MB', 'GB', 'TB', 'PB');
                        $e = floor(log($row[$j])/log(1024));
                        $formatted_data = sprintf('%.2f '.$s[$e], ($row[$j]/pow(1024, floor($e))));

                    }
                    $table_output.="<td class=\"center\">{$formatted_data}</td>"; 
                }
                else
                { $formatted_data = $row[$j]; $table_output.="<td class=\"center\">{$formatted_data}</td>"; }                                
          }
          $table_output .='</tr>';
          $rowNum++;
      }
      $table_output .='</tbody></td><br/>';
      
      return($table_output);
}

function createTable($resource, $schema, $table, $numResults, $page,
					  Array $heading, Array $display, Array $type, 
					  $baseURL = NULL,
					  $linkBaseURL = NULL,
					  $linkCols = NULL,
					  $linkColTypes = NULL,
					  Array $where = NULL,
					  Array $group = NULL,
					  Array $order = NULL,
					  $sort = NULL) 
{
	$query = 'SELECT ' .
		implode(', ', $display) .
		' FROM "' . $schema . '"."' . $table . '" ';
	if ($where != NULL) {
		$query .= 'WHERE ';
		foreach ($where as $i => $andGroup) {
			$query .= '(';
			foreach ($andGroup as $j => $clause) {
				$query .= $clause[0] . $clause[1] . " '" . $clause[2] . "' ";
				if (isset($andGroup[$j + 1])) {
					$query .= 'OR ';
				}
			}
			$query .= ') ';
			if (isset($where[$i + 1])) {
				$query .= 'AND ';
			}
		}
	}
	$query .=
		($group?'GROUP BY ' . implode(', ', $group) . ' ':'') .
		($order?'ORDER BY ' . implode(', ', $order) . ' ':'') .
		($sort?$sort . ' ':'');

	$result = pg_query($resource, 'SELECT count(*) from (' . $query . ') as foo');
	$row = pg_fetch_row($result);
	$totalResults = $row[0];
	if (($page - 1) * $numResults > $totalResults) {
		$page = 1;
	}
	$queryLimits =
		'LIMIT ' . $numResults . ' ' .
		'OFFSET ' . (($page - 1) * $numResults);
	
	//message($query);
	
	$pages = ceil($totalResults / $numResults);
	$firstResult = (($page - 1) * $numResults) + 1;
	$lastResult = $firstResult + $numResults - 1;
	if ($lastResult > $totalResults) {
		$lastResult = $totalResults;
	}
	$link = '';
	if ($baseURL != NULL && $pages > 1) {
		if ($page > 1) {
			$link = '<a href="' . $baseURL . '/*/' . ($page - 1) . '/' . $numResults . '" class="text">&lt;-- Previous</a>';
		}
		if ($page < $pages) {
			if ($link != '') {
				$link .= '&nbsp;&nbsp';
			}
			$link .= '<a href="' . $baseURL . '/*/' . ($page + 1) . '/' . $numResults . '" class="text">Next --&gt;</a>';
		}
	}		

	$query .= $queryLimits;
	$result = pg_query($resource, $query);

	$table_output = '<div class="table"><table id="pgsql_display_table" width="100%" cellspacing="1" cellpadding="2"><thead><tr>';
	$displayColCount = 0;
	foreach($heading as $i => $head)
	{
		if ($type[$i] !='hidden') {
			$column_titles .= '<td class="columnTitle center" scope="col">' .
							 $head .
							 '</td>';
			$displayColCount++;
		}
	}
	$table_output .= '<th class="columnTitle" style="text-align:right;" colspan="' . $displayColCount . '">';
	$table_output .= 'Page ' . $page . ' of ' . $pages . '. Results ' . $firstResult . ' - ' . $lastResult . ' of ';
	if ($baseURL != NULL) {
		$table_output .= '<a href="' . $baseURL . '/*/1/' . $totalResults . '" class="text">' . $totalResults . '</a>';
	} else {
		$table_output .= $totalResults;
	}
	$table_output .= '. ' . $link;
	$table_output .= '</th></tr></thead><tbody>';

	$countryCodeMap = null;
	$countryNameMap = null;
	if (array_search("country", $type) !== false || array_search("countryLookup", $type) !== false) {
		getCountryNameMap($resource, $countryCodeMap, $countryNameMap);
	}

	$rowNum = 0;
	while($row = pg_fetch_row($result))
	{
		if ($rowNum % 10 == 0) {
			$table_output .= $column_titles;
		}

		$rowNum % 2 ? $rowClass = 'odd' : $rowClass = 'even';
		$table_output .= '<tr style="height:100%;" class="' . $rowClass . 
						 '" onmouseover="this.className=\'table_hover\'"' . 
						 ' onmouseout="this.className=\'' . $rowClass . '\'"';
		/*
		if ($linkBaseURL) {
			$table_output .= 'onclick="javascript:document.location.href=\'' .
							 $linkBaseURL . '/';
			foreach ($linkCols as $i => $col) {
				switch ($linkColTypes[$i]) {
				  case 'ip':
				  	$table_output .= long2ip($row[array_search($col, $display)]);
					$table_output .= '/';
					break;
				  default:
				  	$table_output .= $row[array_search($col, $display)];
				}
			}
			$table_output .= '\';"';
		}	
		*/
		$table_output .= '>';

		foreach ($row as $i => $col) {
			if ($type[$i] == 'hidden') {
				continue;
			}

			$table_output .= '<td class="center" style="height:100%;">';
			if ($linkBaseURL && $type[$i] != 'infectedSources') {
				$table_output .= '<a href="' . $linkBaseURL;
				foreach ($linkCols as $j => $jcol) {
					$table_output .= '/';
					switch ($linkColTypes[$j]) {
					  case 'ip':
						$table_output .= long2ip($row[array_search($jcol, $display)]);
						break;
					  default:
						if ($jcol[0] == '"') {
							$table_output .= $row[array_search($jcol, $display)];
						}
						else {
							$table_output .= $jcol;
						}
					}
				}
				$table_output .= '" style="display:block;width:100%;height:100%;margin:0;border:0;padding:0;">';
				$table_output .= '<table style="width:100%;height:100%;margin:0;border:0;padding:0;"><tr style="width:100%;height:100%;margin:0;border:0;padding:0;"><td style="width:100%;height:100%;margin:0;border:0;padding:0;">';
			}
			switch ($type[$i]) {
			  case 'ip':
				$table_output .= long2ip($col);
				break;
			  case 'number':
				$table_output .= number_format($col);
				break;
			  case 'bytes':
				$sizeLabels = array('KiB', 'MiB', 'GiB', 'TiB');

				$bytes = $col;
				$disp = $bytes . ' B';
				foreach ($sizeLabels as $label) {
					if ($bytes > 1024){
						$bytes = $bytes / 1024;
						$disp = sprintf('%.2f ' . $label, $bytes);
					} else {
						break;
					} 
				}
				$table_output .= $disp;
				break;
			  case 'date':
			  	$table_output .= date("Y-m-d H:i:s", $col);
				break;
			  case 'duration':
			  	$table_output .= duration($col);
				break;
			  case 'protocol':
				if ($col == 6) {
			  		$table_output .= 'TCP';
				} else if ($col == 17) {
					$table_output .= 'UDP';
				} else {
					$table_output .= $col;
				}
				break;
			  case 'as':
			  	$table_output .= getASDescriptionByNumber($resource, $col);
				break;
			  case 'asLookup':
			  	$table_output .= getASDescriptionByNumber($resource, getASNByIP($resource, $col));
				break;
			  case 'country':
				$table_output .= getCountryPicture($col, $countryCodeMap, $countryNameMap);
				break;
			  case 'countryLookup':
				$table_output .= getCountryPicture(getCountryNumberByIP($resource, $col), $countryCodeMap, $countryNameMap);
				break;
			  case 'service':
				$foo = sscanf($col, '(%d,%d)');
			  	$table_output .= getServiceNameByPort($foo[0], $foo[1]);
				break;
			  case 'infectedSources':
				$table_output .= getInfectedSourceLinks($col);
				break;
			  case 'base64':
				$table_output .= '<tt>' . wordwrap($col, 76, '<br />', true) . '</tt>';
				break;
			  case 'base64ascii':
				$table_output .= '<tt>' . wordwrap(ascii(base64_decode($col)), 50, '<br />', true) . '</tt>';
				break;
			  case 'wrap':
				$table_output .= wordwrap($col, 20, '<br />', true);
				break;
			  case 'linkdateip2':
				$col = trim($col, '()');
				$link = explode(',', $col);
				$linkdate = strftime("%Y-%m-%d", $link[1]);
				$table_output .= '<a class="text" href="' . $link[0] . '/' . $linkdate . '/' . long2ip($link[2]) . '/' . long2ip($link[3]) . '">' . $link[4] . '</a>';
				break;
			  default:
			  	$table_output .= $col;
			}
			if ($linkBaseURL && $type[$i] != 'infectedSources') {
				$table_output .= '</td></tr></table>';
				$table_output .= '</a>';
			}
			$table_output .= '</td>';
		}
		$table_output .= '</tr>';
		$rowNum++;
	}
	$table_output .= '</tbody></table>';

	echo $table_output;
}
?>
