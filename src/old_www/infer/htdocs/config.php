<?php
	include('include/postgreSQL.php');
	include('include/shared.php');
	include('include/accessControl.php');
	include('include/checkSession.php');
	include('include/form.php');
	include('include/neoflow.php');

	function getConfigurationTables(&$postgreSQL) {
		$query = 'SELECT "table_name" FROM "information_schema"."tables" ' .
				 'WHERE "table_schema" = \'Configuration\'';

		$result = @pg_query($postgreSQL, $query);

		return @pg_fetch_all_columns($result);
	}

	function isConfigurationTable(&$postgreSQL, $table) {
		if (!is_string($table)) {
			return false;
		}

		$query = 'SELECT "table_name" FROM "information_schema"."tables" ' .
				 'WHERE "table_schema" = \'Configuration\' ' .
				 'AND "table_name" = \'' . $table . '\'';
		$result = @pg_query($postgreSQL, $query);
		if (@pg_num_rows($result) == 0) {
			return false;
		}
		return true;
	}

	
	$title = 'IMS Configuration';
	include('include/header.php');
	
	if (!(getPrivileges($postgreSQL, $_COOKIE['imsSessionID']) & ADMINISTRATOR_PRIVILEGE)) {
		message('You are not authorized to make changes to backend configuration!', true);
		include('include/footer.html');
		exit;
	}

	$isConfTable = isConfigurationTable($postgreSQL, $url[1]);
	if (!$url[1]) {
		// just display a list of configurable items (ie. tables in the Configuration schema)
		$configs = getConfigurationTables($postgreSQL);

		if (count($configs) == 0) {
			message('Nothing to configure.', true);
		} else {
			echo 'Configuration:' . '<br />';
			echo '<ul>';
			foreach ($configs as $config) {
				echo '<li><a class="text" href="/config/' . $config . '">' . $config . '</a></li>';
			}
			echo '</ul>';
		}
	} else if ($isConfTable == true) {
		if ($_POST['submit']) {
			unset($_POST['submit']);
			$POST = array();
			foreach ($_POST as $key => $value) {
				$POST[$key] = trim($value);
			}

			foreach ($POST as $key => $value) {
				$query = 'UPDATE "Configuration"."' . $url[1] . '" SET ' .
						 '"value" = ' . ($value?'\'' . $value . '\'':'NULL') . ' ' . 
						 'WHERE "name" = \'' . $key . '\'';
				if (@pg_query($postgreSQL, $query) === false) {
					$pg_err_msg = pg_last_error($postgreSQL);
					break;
				}
			}
			if ($pg_err_msg) {
				message($pg_err_msg, true);
			} else {
				message('Configuration for "' . $url[1] . '" updated successfully.');
			}
		}

		// display configuration editing table for $url[1]
		$query = 'SELECT * FROM "Configuration"."' . $url[1] . '"';
		$result = @pg_query($postgreSQL, $query);
		if (@pg_num_rows($result) == 0) {
			message('There are currently no options to configure for "' . $row[1] . '"');
		} else {
			echo '<form method="post" action="/config/' . $url[1] . '" enctype="multipart/form-data">';
			echo 
				'<table width="100%" cellspacing="1">' .
					'<tr>' .
						'<th class="tableName" colspan="4" width="100%" align="center">Configuration options for "' . $url[1] . '"</th>' .
					'</tr>' .
					'<tr>' .
						'<td align="center" class="columnTitle">' .
							'Option' .
						'</td>' .
						'<td align="center" class="columnTitle">' .
							'Value' .
						'</td>' .
						'<td align="center" class="columnTitle">' .
							'Default' .
						'</td>' .
						'<td align="center" class="columnTitle">' .
							'Description' .
						'</td>' .
					'</tr>';
			while ($row = pg_fetch_assoc($result)) {
				echo
					'<tr>' .
						'<td align="right">' .
							$row['name'] .
						'</td>' .
						'<td align="left">' .
							'<input name="' . $row['name'] . '" type="text" value="' . $row['value'] . '" />' .
						'</td>' .
						'<td align="left">' .
							$row['default'] .
						'</td>' .
						'<td align="left">' .
							$row['description'] .
						'</td>' .
					'</tr>';
			}
			echo '</table>';
			echo '<input name="submit" type="submit" value="Submit" />';
			echo '</form>';
		}
	} else if (!$isConfTable) {
		// display error no such table
		message('"' . $url[1] . '" is not a valid Configuration section.', true);
	}
		
	include('include/footer.html');
?>
