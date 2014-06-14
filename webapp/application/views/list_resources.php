<!DOCTYPE html>
<html lang="en">
<?php $this->load->view('header');?>

<div id="container">
	<h1>Sensor<b>Igniter</b></h1>

	<div id="body">
		
		<table class="resources">
			<thead>
				<th class="value"></th>
				<th class="name">Name</th>
				<th class="description">Description</th>
			</thead>
			<tbody>
		<?php
		$i = 0;
		foreach($resources as $resource) {
			echo "<tr class=".($i % 2? 'odd' : 'even').">";
			echo '<td class="value">';
			switch($resource->resType) {
			case "switch":
				echo '<div class="switch '.($resource->currentValue > 0? "on" : "off").'" rel="'.$resource->id.'"></div>';
				break;
			case "dimmer":
				echo '<div class="dimmer" rel="'.$resource->id.'" data="'.$resource->currentValue.'"></div>';
				break;
			default:
				echo '<div class="sensor" title="Refresh sensor value" rel="'.$resource->id.'">'.$resource->currentValue." ".$resource->unit."</div>";
				break;
			}
			echo "</td><td><a>".$resource->name."</a></td>";
			echo "<td>".$resource->description."</td>";
			echo "</tr>";
			$i++;
		}
		?>

			</tbody>			
		</table>
</div>

<script>

function ajaxSwitchToggle(button) {
	var button = $(this);
	var rel = button.attr("rel");
	$.getJSON("/index.php/resource/ajaxSwitchToggle/"+rel, function(data) {
		button.attr("class", "switch");
		button.addClass(data.currentValue? "on" : "off");
	}).error(function() {
		alert("Cannot toggle switch");
	});
}

function ajaxDimmerChange(dimmer) {
	var dimmer = $(this);
	var rel = dimmer.attr("rel");
	var old = parseFloat(dimmer.attr("data"));
	var val = dimmer.slider("value");
	if(val != old) 
		$.getJSON("/index.php/resource/ajaxDimmerChange/"+rel+"/"+val, function(data) {
			dimmer.attr("data", data.currentValue);
			dimmer.slider("value", data.currentValue);
		}).error(function() {
			alert("Cannot change dimmer value");
		});
}

function ajaxSensorRefresh(sensor) {
	var sensor = $(this);
	var rel = sensor.attr("rel");
	$.getJSON("/index.php/resource/ajaxSensorRefresh/"+rel, function(data) {
		sensor.html(data.currentValue+" "+data.unit);
	}).error(function() {
		alert("Cannot refresh sensor value");
	});
}

$(document).ready(function() {
	$(".switch").css("cursor", "pointer").click(ajaxSwitchToggle);
	$(".dimmer").slider({min: 0, max: 1, step:0.05, change: ajaxDimmerChange}).each(function() {
		$(this).slider("value", $(this).attr("data"));
	});
	$(".sensor").css("cursor", "pointer").click(ajaxSensorRefresh);
});

</script>

<?php $this->load->view('footer');?>
</html>
