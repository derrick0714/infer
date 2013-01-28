		<div id="header">
			<div id="top_row">
				<div id="top_row_wrapper">
					<div id="top_row_left">INFER</div>
					
					<?php if(isset($_SESSION['logged_in'])) {?>
					<div id="top_row_right">
						<a href="/logout">Logout</a>
					</div>
					<?php }?>
				</div>
			</div>
			
			<div id="mid_row">
				<div id="mid_row_wrapper">
					<ul id="mid_row_tabs">
					<?php
						if (is_array($mid_row_tabs)) {
							foreach($mid_row_tabs as $name => $display)
							{
								if ($display != null) {
									echo '<li><a id="mid_row_tab_' . $name .
										 '" href="/' . $name . '/">' .
										 $display . '</a></li>';
								}
								else {
									echo '<li class="separator">|</li>';
								}
							}
						}
						
						if (isset($_SESSION['privileges']) &&
							$_SESSION['privileges'] == 0xffff)
						{
							echo '<li style="float: right;">' .
								 '<a id="mid_row_tab_admin" href="/admin/">' .
								 'Admin</a></li>';
						}
					?>
					</ul>
				</div>
			</div>
			
			<div id="bottom_row">
				<div id="bottom_row_wrapper">
					<ul id="bottom_row_tabs">
					<?php
						if (is_array($bottom_row_tabs)) {
							foreach($bottom_row_tabs as $name => $display)
							{
								echo "<li><a id=\"bottom_row_tab_" . $name . 
									 "\" href=\"#/" . $name . "\">" . $display . "</a></li>";
							}
						}
					?>
					</ul>
				</div>
			</div>
		</div>
