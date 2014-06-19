<?php $this->load->view('header');?>

<?php foreach($directories as $directory) { ?>

<div class="container directory">
	<h1><?php echo $directory->url; ?></h1>
	<div class="toolbar">
		<div class="smallButton removeDirectory" rel="<?php echo $directory->id; ?>" title="Remove resource directory and all related resources">remove</div>
		<div class="smallButton refreshDirectory" rel="<?php echo $directory->id; ?>" title="Reload resource directory">refresh</div>
	</div>
	<div class="body">
	<?php $this->load->view('ajax_list_resources', array('resources' => $this->Resource_model->list_by_directory($directory->id))); ?>		
	</div>
<p></p>
</div>

<?php } ?>

	<div class="body directory">
		<div class="smallButton addDirectory" title="Add new resource directory">Add</div>
		<input class="addDirectory"></input>
	</div>

<script>

$(document).ready(function() {
	$(".refreshDirectory").click(directory.ajaxRefresh);
	$(".removeDirectory").click(directory.ajaxRemove);
	$("input.addDirectory").button().watermark("Http url of new resource directory");
	$("div.addDirectory").click(directory.ajaxAdd);
});

</script>

<?php $this->load->view('footer');?>
