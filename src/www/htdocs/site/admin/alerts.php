<script type="text/javascript">
function update_form(content) {
	$('#smtp_notify_enable').attr('checked',
								  content.smtp_notify_enable == 'YES');
	$('#smtp_server').val(content.smtp_server);
	$('#smtp_user').val(content.smtp_user);
	$('#smtp_password').val(content.smtp_password);
	$('#smtp_displayname').val(content.smtp_displayname);
	$('#smtp_from').val(content.smtp_from);
	$('#subject_prefix').val(content.subject_prefix);
	$('#to').val(content.to);
};

$(document).ready(function()
{
	$.getJSON('/data/admin/alerts', update_form);
	$('#submit').click(function() {
		$.ajax({
			url: '/data/admin/alerts',
			type: 'POST',
			dataType: 'json',
			success: function(content) {
				update_form(content);
				alert('Successfully updated configuration!');
			},
			data: $('#alert_settings').serialize()
		});
		return false;
	});
});
</script>

<form id="alert_settings" action="">

<input type="checkbox" id="smtp_notify_enable" name="smtp_notify_enable" value="YES"></input>
<label for="smtp_notify_enable">Enable SMTP Alerts</label>
<br />

<label for="smtp_server">SMTP Server:</label>
<input type="text" id="smtp_server" name="smtp_server"></input>
<br />

<label for="smtp_user">SMTP User:</label>
<input type="text" id="smtp_user" name="smtp_user"></input>
<br />

<label for="smtp_password">SMTP Password:</label>
<input type="text" id="smtp_password" name="smtp_password"></input>
<br />

<label for="smtp_displayname">From Display Name:</label>
<input type="text" id="smtp_displayname" name="smtp_displayname"></input>
<br />

<label for="smtp_from">From Address:</label>
<input type="text" id="smtp_from" name="smtp_from"></input>
<br />

<label for="subject_prefix">Subject Prefix:</label>
<input type="text" id="subject_prefix" name="subject_prefix"></input>
<br />

<label for="to">Alert-recepient Address:</label>
<input type="text" id="to" name="to"></input>
<br />

<input type="submit" id="submit" value="Submit" /> 

</form>
