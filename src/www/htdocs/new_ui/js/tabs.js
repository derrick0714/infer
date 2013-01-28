$(document).ready(function()
{
	$('#bottom_row_tabs li a').click(function()
	{
		$('#body_content').load($(this).attr('href').substring(1));
		$('#bottom_row_active').removeAttr('id');
		$(this).parent().attr('id', 'bottom_row_active');
	});
});