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
			echo '</td><td><div class="remove" rel="'.$resource->id.'">Remove</div>';
			echo '<a class="fancybox" title="'.$resource->description.'" href="/resources/chart/'.$resource->id.'">'.$resource->name."</a></td>";
			echo "<td>".$resource->description."</td>";
			echo "</tr>";
			$i++;
		}
		?>

			</tbody>			
		</table>

<script>

$(document).ready(function() {
	$(".switch").css("cursor", "pointer").click(ajaxSwitchToggle);
	$(".dimmer").slider({min: 0, max: 1, step:0.05, change: ajaxDimmerChange}).each(function() {
		$(this).slider("value", $(this).attr("data"));
	});
	$(".sensor").css("cursor", "pointer").click(ajaxSensorRefresh);
	$(".remove").css("cursor", "pointer").click(ajaxResourceRemove);
	$(".fancybox").fancybox({
		type : 'iframe',
    	openEffect	: 'elastic',
    	closeEffect	: 'elastic',
    	helpers : {
    		title : {
    			type : 'float'
    		}
    	}
    });
});

</script>