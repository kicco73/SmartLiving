var resource = {
  ajaxSwitchToggle: function() {
	var button = $(this);
	var rel = button.attr("rel");
	$.getJSON("/resources/ajaxSwitchToggle/"+rel, function(data) {
		button.attr("class", "switch");
		button.addClass(data.currentValue? "on" : "off");
	}).error(function() {
		alert("Cannot toggle switch");
	});
  },

  ajaxDimmerChange: function() {
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
  },

  ajaxSensorRefresh: function() {
	var sensor = $(this);
	var rel = sensor.attr("rel");
	$.getJSON("/resources/ajaxSensorRefresh/"+rel, function(data) {
		sensor.html(data.currentValue+" "+data.unit);
	}).error(function() {
		alert("Cannot refresh sensor value");
	});
  },

  ajaxRemove: function() {
	var sensor = $(this);
	var rel = sensor.attr("rel");
	$.getJSON("/resources/ajaxRemove/"+rel, function(data) {
		sensor.closest('.resource').remove();
	}).error(function() {
		alert("Cannot remove resource");
	});
  },
  ajaxSet: function(val, settings) {
	var div = $(this);
	var rel = div.attr("rel");
	var field = div.attr("data");
	$.post("/resources/ajaxSet/"+rel, {field: field, value: val}).error(function() {
		alert("Cannot set "+field+" value");
	});
	return val;
  }
};

var directory = {
  ajaxRefresh: function() {
	var div = $(this);
	var rel = div.attr("rel");
	$.ajax("/directories/ajaxRefresh/"+rel)
		.success(function(data) {
			div.closest(".directory").find(".resources").replaceWith(data);
		})
		.error(function(x) {
			alert("Cannot refresh resource directory");
		});
  },

  ajaxRemove: function() {
	var sensor = $(this);
	var rel = sensor.attr("rel");
	$.getJSON("/directories/ajaxRemove/"+rel, function(data) {
		sensor.closest('.directory').remove();
	}).error(function() {
		alert("Cannot remove resource directory");
	});
  },

  ajaxSet: function() {
	var div = $(this);
	var rel = div.attr("rel");
	var field = div.attr("data");
	$.post("/directories/ajaxSet/"+rel, {field: field, value: val}).error(function() {
		alert("Cannot set "+field+" value for directory");
	});
	return val;
  }

};


