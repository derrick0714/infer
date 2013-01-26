<script type="text/javascript" src="/js/autocompleter/jquery.autocomplete.js"></script>
<link rel="stylesheet" type="text/css" href="/js/autocompleter/jquery.autocomplete.css" />

<script type="text/javascript">
$(document).ready(function()
{
	$('[name=uploaded_file]').change(function()
	{
		$('#file_options').slideDown();
	});
	
	$("[name=query]").autocomplete(
	"/site/search/values.php",
	{
		delay:10,
		cacheLength:false,
		minChars:1,
		matchSubset:1,
		autoFill:true,
		maxItemsToShow:10
	});
	
});
</script>
<?php require_once('./../../data/pg.include.php');?>
<?php if(isset($_GET['queryID']) && $_GET['queryID']!=""):?>
	<div style="text-align:center;">
	Search started. <a href='/search/#/queries/flow/<?php echo $_GET['queryID']; ?>'>Click Here</a> to view Search summary.
	</div>
<?php endif;?>
<form action="/data/search/new"  method="post" enctype="multipart/form-data">
<p>
	<label for="search_name">Name: </label>
	<?php 
		$result_set=@pg_query($pg,'select * from "Indexes"."searchQueries"');
		$count=0;
		$count=@pg_num_rows($result_set)+1;
	?>
	<input type="text" name="search_name" value="_Search_<?php echo @date('M-d',strtotime('now')) . '_' . $count?>" />
</p>
<p>
	<input type="text" name="query" style="width: 100%; height: 20px;" />
</p>
<p>
	<label for="uploaded_file">File: </label>
	<input type="file" name="uploaded_file" />
	<div id="file_options" style="margin-left: 20px; display: none;">
		<label for="file_offset">Offset of string: </label>
		<input type="text" name="file_offset" value="0" /><br />
		<label for="offset_length">Length from offset: </label>
		<input type="text" name="offset_length" value="1024" /><br />
		<label for="match_length">Match length: </label>
		<input type="text" name="match_length" value="512" />
	</div>
</p>
<p>
	<input type="submit" name="submit" value="Submit Query!" />
</p>
</form>
