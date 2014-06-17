<!DOCTYPE html>
<html lang="en">
<?php $this->load->view('header');?>

<div id="container">
	<h1>Sensor<b>Igniter</b></h1>
	<div class="toolbar">
		<div id="refreshDirectory" title="Reload resource directory"></div>
	</div>
	<div id="body">
	<?php $this->load->view('ajax_list_resources', array('resources' => $resources)); ?>		
	</div>

<script>

$(document).ready(function() {
	$("#refreshDirectory").button().click(function() {
		$.ajax("/resources/ajaxResourceDirectoryRefresh")
			.success(function(data) {
				$(".resources").replaceWith(data);
			})
			.error(function(x) {
				alert("Cannot refresh resource directory");
			});
	});
});

</script>

<?php $this->load->view('footer');?>
</html>
