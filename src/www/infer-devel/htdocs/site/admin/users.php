<script type="text/javascript">

var users_list_table;

function delete_user(uid) {
	$.ajax( {
		"type": "POST",
		"url": '/data/users/delete/' + uid,
		"error": function( jqXHR, textStatus, errorThrown ) {
			alert ("An unexpected error has occurred!");
		},
		"success": function( data, textStatus, jqXHR ) {
			users_list_table.fnReloadAjax();
		}
	} );
}

function clear_user_info_form() {
	$('#user_info_form input[name="uid"]').val(0);
	$('#user_info_form input[name="user_name"]').val("");
	$('#user_info_form input[name="password"]').val("");
	$('#user_info_form input[name="name"]').val("");
	$('#user_info_form input[name="privileges_admin"]').attr('checked', false);
	$('#user_info_form input[name="active"]').attr('checked', true);
}

function fill_user_info_form(opts) {
	$.ajax( {
		"type": "GET",
		"url": '/data/users/' + opts.uid,
		"dataType": 'json',
		"error": function( jqXHR, textStatus, errorThrown ) {
			if (opts.error) {
				opts.error(jqXHR, textStatus, errorThrown);
			}
		},
		"success": function( data, textStatus, jqXHR ) {
			$('#user_info_form input[name="uid"]').val(
				data.aaData[0][0]);
			$('#user_info_form input[name="user_name"]').val(
				data.aaData[0][1]);
			$('#user_info_form input[name="name"]').val(
				data.aaData[0][2]);
			$('#user_info_form input[name="privileges_admin"]').attr(
				'checked',
				(data.aaData[0][3] == 0xffff ? true : false));
			$('#user_info_form input[name="active"]').attr(
				'checked',
				data.aaData[0][4]);

			if (opts.success) {
				return opts.success(data, textStatus, jqXHR);
			}
		}
	} );
}

function submit_user_info_form_new() {
	$.ajax( {
		type: 'POST',
		url: '/data/users/new',
		success: function() {
			users_list_table.fnReloadAjax();
			$.address.path('/users');
		},
		data: $('#user_info_form').serialize()
	} );
	return false;
}

function submit_user_info_form_update() {
	$.ajax( {
		type: 'POST',
		url: '/data/users/update/' +
			$('#user_info_form input[name="uid"]').val(),
		success: function() {
			users_list_table.fnReloadAjax();
			$.address.path('/users');
		},
		data: $('#user_info_form').serialize()
	} );
	return false;
}

tab_uri_notify = function(e) {
	if (e.pathNames.length == 1) {
		// show users list
		$('#users_list_div').css('display', 'block');
		$('#user_info_div').css('display', 'none');
		return;
	}
	// show something sles
	switch (e.pathNames[1]) {
	  case 'new':
		clear_user_info_form();
		$('#users_list_div').css('display', 'none');
		$('#user_info_div').css('display', 'block');
		$('#user_info_submit').unbind('click');
		$('#user_info_submit').click(submit_user_info_form_new);
		break;
	  case 'edit':
		clear_user_info_form();
		fill_user_info_form( {
			"uid": e.pathNames[2],
			"success": function() {
				$('#users_list_div').css('display', 'none');
				$('#user_info_div').css('display', 'block');
			},
			"error": function() {
				alert("Invalid user id!");
				$.address.path("/users");
			}
		} );
		$('#user_info_submit').unbind('click');
		$('#user_info_submit').click(submit_user_info_form_update);
		break;
	  default:
		alert("Invalid URI!");
		$.address.path("/users");
	};
}

$(document).ready(function()
{
	users_list_table = $('#users_list').dataTable( {
		"bFilter": false,
		"bProcessing": true,
		"sAjaxSource": '/data/users',
		"fnServerData": function( sSource, aoData, fnCallback ) {
			// add required data columns to retreived data
			$.ajax( {
				"dataType": "json",
				"type": "GET",
				"url": sSource,
				"data": aoData,
				"success": function( data, textStatus, jqXHR ) {
					for (var i = 0; i < data.aaData.length; i++) {
						data.aaData[i].push("");
					}
					fnCallback(data, textStatus, jqXHR);
				},
			} );
		},
		"aoColumnDefs": [
			{
				"aTargets": [ 0 ],
				"sName": "uid",
				"sTitle": "UID",
				"sType": "numeric",
				"bVisible": false,
			},
			{
				"aTargets": [ 1 ],
				"sName": "user_name",
				"sTitle": "User Name",
				"sType": "string"
			},
			{
				"aTargets": [ 2 ],
				"sName": "name",
				"sTitle": "Real Name",
				"sType": "string"
			},
			{
				"aTargets": [ 3 ],
				"sName": "privileges",
				"sTitle": "Privileges",
				"sType": "string",
				"fnRender": function(o) {
					if (o.aData[o.iDataColumn] == 65535) {
						return "Admin";
					}

					return "Normal";
				},	
			},
			{
				"aTargets": [ 4 ],
				"sName": "active",
				"sTitle": "Active",
			},
			{
				"aTargets": [ 5 ],
				"sTitle": "Actions",
				"fnRender": function(o) {
					var ret =
						'<a href="#/users/edit/' + o.aData[0] + '">Edit</a> ' +
						'<a href="javascript:;" onclick="delete_user(' + o.aData[0] + ');">' +
							'Delete' +
						'</a>';
					return ret;
				},
			},
		],
	} );
} );

</script>


<div id="inner_wrapper">
	<div id="user_actions">
		<a href="#/users">Users List</a>
		|
		<a href="#/users/new">Create New User</a>
	</div>
	<br />
	<div id="users_list_div">
		<table id="users_list" class="display">
		</table>
	</div>
	<div id="user_info_div">
		<form id="user_info_form" action="">
		<input type="hidden" name="uid" value="0"></input>
		<table id="user_info_table">
			<tbody>
				<tr>
					<td>
						User Name:
					</td>
					<td>
						<input type="text" name="user_name"></input>
					</td>
				</tr>
				<tr>
					<td>
						Password:
					</td>
					<td>
						<input type="password" name="password"></input>
					</td>
				</tr>
				<tr>
					<td>
						Real Name:
					</td>
					<td>
						<input type="text" name="name"></input>
					</td>
				</tr>
				<tr>
					<td>
						Priviliges:
					</td>
					<td>
						<input type="checkbox"
							   name="privileges_admin"
							   value="yes">Admin</input>
					</td>
				</tr>
				<tr>
					<td>
						Active:
					</td>
					<td>
						<input type="checkbox"
							   name="active"
							   value="yes">Active
						</input>
					</td>
				</tr>
			</tbody>
		</table>
		<input type="submit" id="user_info_submit" value="Submit" />
		</form>
	</div>
</div>
