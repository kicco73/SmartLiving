<?php $this->load->view('header');?>

<?php foreach($directories as $directory) { ?>

<div class="container directory">
	<h1><?php echo $directory->url; ?></h1>
	<div class="toolbar">
		<a class="smallButton removeDirectory" rel="<?php echo $directory->id; ?>" title="Remove resource directory and all related resources">remove</a>
		<a class="smallButton rebuildDirectory" rel="<?php echo $directory->id; ?>" title="Rebuild resource directory">rebuild</a>
	</div>
	<div class="body">
	<?php $this->load->view('ajax_list_resources', array('resources' => $this->Resource_model->list_by_directory($directory->id))); ?>		
	</div>
<p></p>
</div>

<?php } ?>

	<div class="body directory">
		<a class="smallButton discoverDirectories" title="Discover resource directories">Discover</a>
		<a class="smallButton addDirectory" title="Add new resource directory">Add</a>
		<input class="addDirectory"></input>
	</div>

<script>

$(document).ready(function() {
	$(".discoverDirectories").click(directory.ajaxDiscover);
	$(".rebuildDirectory").click(directory.ajaxRebuild);
	$(".removeDirectory").click(directory.ajaxRemove);
	$("input.addDirectory").button().watermark("Http url of new resource directory");
	$("a.addDirectory").click(directory.ajaxAdd);
	observe.start(".rebuildDirectory");
});

</script>

<?php $this->load->view('footer');?>
