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
			echo '<td><div class="remove smallButton" rel="'.$resource->id.'">Remove</div>';
			echo '<a class="chart" title="Sensed values" href="/resources/chart/'.$resource->id.'">'.$resource->url."</a></td>";
			echo "</tr>";
			$i++;
		}
		?>

			</tbody>			
		</table>

<script>

$(document).ready(function() {
	$(".new .switch").click(resource.ajaxSwitchToggle);
	$(".new .dimmer").slider({min: 0, max: 1, step:0.05, change: resource.ajaxDimmerChange}).each(function() {
		$(this).slider("value", $(this).attr("data"));
	});
	$(".new .sensor").click(resource.ajaxSensorRefresh);
	$(".new .remove").click(resource.ajaxRemove);
	$(".new .editable").editable(resource.ajaxSet);
	$(".new .chart").fancybox({
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
