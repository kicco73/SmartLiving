<div class="boards">
<?php foreach($boards as $board) { ?>
		<h3>Board: <?php echo $board->url; ?></h3>
		<table id="board<?php echo $board->id; ?>" class="resources">
			<thead>
				<th class="value"></th>
				<th class="name">Name</th>
				<th class="description">Description</th>
				<th class="url">Url</th>
			</thead>
			<tbody>
		<?php
		$i = 0;
		foreach($this->Resource_model->list_by_board($board->id) as $resource) {
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
	var board = $("#board<?php echo $board->id; ?>");
	board.find(".switch").click(resource.ajaxSwitchToggle);
	board.find(".dimmer").slider({min: 0, max: 100, step:1,  change: resource.ajaxDimmerChange}).each(function() {
		$(this).slider("value", $(this).attr("data"));
	});
	board.find(".dimmer a").focus(observe.pause).blur(observe.run);
	board.find(".sensor").click(resource.ajaxSensorRefresh);
	board.find(".remove").click(resource.ajaxRemove);
	board.find(".editable")
		.editable(resource.ajaxSet)
		.click(observe.pause)
		.blur(observe.run);
	$("#board<?php echo $board->id; ?> a.chart").off().fancybox({
		type : 'iframe',
		live: false,
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
<?php } ?>
</div>

