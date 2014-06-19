function ajaxSwitchToggle() {
	var button = $(this);
	var rel = button.attr("rel");
	$.getJSON("/resources/ajaxSwitchToggle/"+rel, function(data) {
		button.attr("class", "switch");
		button.addClass(data.currentValue? "on" : "off");
	}).error(function() {
		alert("Cannot toggle switch");
	});
}

function ajaxDimmerChange() {
	var dimmer = $(this);
	var rel = dimmer.attr("rel");
	var old = parseFloat(dimmer.attr("data"));
	var val = dimmer.slider("value");
	if(val != old) 
		$.getJSON("/resources/ajaxDimmerChange/"+rel+"/"+val, function(data) {
			dimmer.attr("data", data.currentValue);
			dimmer.slider("value", data.currentValue);
		}).error(function() {
			alert("Cannot change dimmer value");
		});
}

function ajaxSensorRefresh() {
	var sensor = $(this);
	var rel = sensor.attr("rel");
	$.getJSON("/resources/ajaxSensorRefresh/"+rel, function(data) {
		sensor.html(data.currentValue+" "+data.unit);
	}).error(function() {
		alert("Cannot refresh sensor value");
	});
}

function ajaxResourceRemove() {
	var sensor = $(this);
	var rel = sensor.attr("rel");
	$.getJSON("/resources/ajaxResourceRemove/"+rel, function(data) {
		sensor.closest('.resource').remove();
	}).error(function() {
		alert("Cannot remove resource");
	});
}

function ajaxDirectoryRemove() {
	var sensor = $(this);
	var rel = sensor.attr("rel");
	$.getJSON("/directory/ajaxDirectoryRemove/"+rel, function(data) {
		sensor.closest('.directory').remove();
	}).error(function() {
		alert("Cannot remove resource directory");
	});
}

function ajaxSet(val, settings) {
	var div = $(this);
	var rel = div.attr("rel");
	var field = div.attr("data");
	$.post("/resources/ajaxSet/"+rel, {field: field, value: val}).error(function() {
		alert("Cannot set "+field+" value");
	});
	return val;
}

function ajaxDirectoryRefresh() {
	var div = $(this);
	var rel = div.attr("rel");
	$.ajax("/directories/ajaxDirectoryRefresh/"+rel)
		.success(function(data) {
			div.parent().find(".resources").replaceWith(data);
		})
		.error(function(x) {
			alert("Cannot refresh resource directory");
		});
}
