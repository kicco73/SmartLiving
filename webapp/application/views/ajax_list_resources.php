		<table class="new resources">
			<thead>
				<th class="value"></th>
				<th class="name">Name</th>
				<th class="description">Description</th>
				<th class="url">Url</th>
			</thead>
			<tbody>
		<?php
		$i = 0;
		foreach($resources as $resource) {
			echo '<tr class="resource '.($i % 2? 'odd' : 'even').'">';
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
			echo '</td>';
			echo '<td><div class="editable" rel="'.$resource->id.'" data="name">'.$resource->name."</div></td>";
			echo '<td><div class="editable" rel="'.$resource->id.'" data="description">'.$resource->description."</div></td>";
			echo '<td><a class="remove smallButton" rel="'.$resource->id.'">Remove</a>';
			if($this->Resource_model->count_samples($resource->id)) {
				echo '<a class="chart smallButton" title="Sensed values" href="/resources/chart/'.$resource->id.'">Chart</a>';
				echo '<span class="chart">'.$resource->url."</span>";
			} else 
				echo '<span class="noChart" title="No data logged">'.$resource->url."</span>";
			echo "</td>";
			echo "</tr>";
			$i++;
		}
		?>

			</tbody>			
		</table>

<script>

$(document).ready(function() {
	$(".new .switch").click(resource.ajaxSwitchToggle);
	$(".new .dimmer").slider({min: 0, max: 1, step:0.05,  change: resource.ajaxDimmerChange}).each(function() {
		$(this).slider("value", $(this).attr("data"));
	});
	$(".new .dimmer a").focus(observe.pause).blur(observe.run);
	$(".new .sensor").click(resource.ajaxSensorRefresh);
	$(".new .remove").click(resource.ajaxRemove);
	$(".new .editable")
		.editable(resource.ajaxSet)
		.click(observe.pause)
		.blur(observe.run);
	$("a.chart").fancybox({
		type : 'iframe',
    	openEffect	: 'elastic',
    	closeEffect	: 'elastic',
    	helpers : {
    		title : {
    			type : 'float'
    		}
    	}
    });
    $(".new").toggleClass("new");
});

</script>
