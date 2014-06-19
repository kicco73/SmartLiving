<?php $this->load->view('header');?>

<?php foreach($directories as $directory) { ?>

<div class="container directory">
	<h1><?php echo $directory->url; ?></h1>
	<div class="toolbar">
		<div class="refreshDirectory" rel="<?php echo $directory->id; ?>" title="Reload resource directory"></div>
	</div>
	<div class="body">
	<?php $this->load->view('ajax_list_resources', array('resources' => $this->Resource_model->list_by_directory($directory->id))); ?>		
	</div>
</div>

<?php } ?>

<script>

$(document).ready(function() {
	$(".refreshDirectory").button().click(ajaxDirectoryRefresh);
});

</script>

<?php $this->load->view('footer');?>
