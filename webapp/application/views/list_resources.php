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

function toggleSwitch(button) {
	var button = $(this);
	var rel = button.attr("rel");
	$.getJSON("/index.php/resource/ajax_switch_toggle/"+rel, function(data) {
		button.attr("class", "switch");
		button.addClass(data.currentValue? "on" : "off");
	}).error(function() {
		alert("Cannot toggle switch");
	});
}

function changeDimmer(dimmer) {
	var dimmer = $(this);
	var rel = dimmer.attr("rel");
	var old = parseFloat(dimmer.attr("data"));
	var val = dimmer.slider("value");
	if(val != old) 
		$.getJSON("/index.php/resource/ajax_dimmer_change/"+rel+"/"+val, function(data) {
			dimmer.attr("data", data.currentValue);
			dimmer.slider("value", data.currentValue);
		}).error(function() {
			alert("Cannot change dimmer value");
		});
}

function refreshSensor(sensor) {
	var sensor = $(this);
	var rel = sensor.attr("rel");
	$.getJSON("/index.php/resource/ajax_sensor_refresh/"+rel, function(data) {
		sensor.html(data.currentValue+" "+data.unit);
	}).error(function() {
		alert("Cannot refresh sensor value");
	});
}

$(document).ready(function() {
	$(".switch").css("cursor", "pointer").click(toggleSwitch);
	$(".dimmer").slider({min: 0, max: 1, step:0.05, change: changeDimmer}).each(function() {
		$(this).slider("value", $(this).attr("data"));
	});
	$(".sensor").css("cursor", "pointer").click(refreshSensor);
});

</script>

<?php $this->load->view('footer');?>
</html>
