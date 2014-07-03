<html>
  <head>
    <script type="text/javascript" src="https://www.google.com/jsapi"></script>
    <script type="text/javascript">
      google.load("visualization", "1", {packages:["corechart"]});
      google.setOnLoadCallback(drawChart);
      function drawChart() {

        var data = google.visualization.arrayToDataTable([
          ['Time', '[<?php echo $resource->unit;?>]']
          <?php
          	foreach($samples as $sample) 
          		echo ",['".$sample->timeStamp."', ".$sample->value."]";
          ?>
        ]);

        var options = {
          title: '<?php echo $resource->description; ?> ',
          curveType: 'function',
	      legend: { position: 'bottom' }
        };

        var chart = new google.visualization.LineChart(document.getElementById('chart_div'));
        chart.draw(data, options);
      }
    </script>
  </head>
  <body>
    <div id="chart_div" style="width: 784px; height: 500px;"></div>
  </body>
</html>
